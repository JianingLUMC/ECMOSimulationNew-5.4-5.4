// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#pragma once

#include "Async/Async.h"
#include "Async/AsyncWork.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DebugRenderSceneProxy.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticMap.generated.h"

// Fwd declare
class UMetaXRAcousticMaterial;
class UMetaXRAcousticGeometry;

#define META_XR_AUDIO_MAP_SPHERE_RADIUS 25.0f

/*
 * MetaXRAudio geometry components are used to customize an a static mesh actor's acoustic properties.
 */
UENUM(BlueprintType)
enum class EAcousticMapStatus : uint8 {
  Empty = 0 UMETA(DisplayName = "Empty"),
  Mapped = (1 << 0) UMETA(DisplayName = "Mapped"),
  Ready = (1 << 1) | Mapped UMETA(DisplayName = "Ready"),
};

class FAsyncSceneMappingTask : public FNonAbandonableTask {
 public:
  UMetaXRAcousticMap* AcousticMapComponent;
  ovrAudioSceneIR Map;
  ovrAudioSceneIRParameters Parameters;
  bool bMapOnly = false;

  FAsyncSceneMappingTask(UMetaXRAcousticMap* InMapComponent, ovrAudioSceneIR InMap, ovrAudioSceneIRParameters InParameters)
      : AcousticMapComponent(InMapComponent), Map(InMap), Parameters(InParameters) {}

  FAsyncSceneMappingTask(FAsyncSceneMappingTask& Other)
      : AcousticMapComponent(Other.AcousticMapComponent), Map(Other.Map), Parameters(Other.Parameters) {}

  void DoWork();

  FORCEINLINE TStatId GetStatId() const {
    RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncSceneMappingTask, STATGROUP_ThreadPoolAsyncTasks);
  }
};

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Map",
         ToolTip = "Precompute information about the acoustics into an Acoustic Map to reduce resource usage"))
class METAXRAUDIO_API UMetaXRAcousticMap : public UPrimitiveComponent {
  GENERATED_BODY()

 public:
  UMetaXRAcousticMap();
  virtual ~UMetaXRAcousticMap() override;

  FString GetFilePath() {
    return FilePath;
  }
  void LoadData();
  void StartInternal(bool AutoLoad = true);
  void DestroyInternal();
  virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
  virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

#if WITH_EDITOR
  bool Compute(bool bMapOnly);
  void FinishCompute();
  void CancelCompute();
  void AddPoint(FVector NewPoint);
  FVector GetNewPointForRay(FVector EditorCameraPosition, FVector EditorCameraDirection);
  void UpdateCachedPoints();
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
  bool IsComputeCanceled() {
    return bComputeCanceled;
  }
  void SetDescription(FString NewDescription) {
    Description = NewDescription;
  }
  void SetComputeProgress(float NewComputeProgress) {
    ComputeProgress = NewComputeProgress;
  }
  void SetComputeTimeSeconds(float NewComputeTime) {
    ComputeTime = NewComputeTime;
  }
  FString GetDescription() {
    return Description;
  }
  float GetComputeProgress() {
    return ComputeProgress;
  }
  float GetComputeTimeSeconds() {
    return ComputeTime;
  }
  void StartTimer() {
    StartTimeSeconds = FPlatformTime::Seconds();
  }
  double CheckTimer() {
    return FPlatformTime::Seconds() - StartTimeSeconds;
  }
  void SetStageStartingTimeSeconds(double NewTime) {
    StageStartingTimeSeconds = NewTime;
  }
  double GetStageStartingTimeSeconds() {
    return StageStartingTimeSeconds;
  }
#endif

 private:
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void BeginDestroy() override;
  void CheckMapTransformValid();
  void ApplyTransform();

  UPROPERTY()
  FString FilePath;
  UPROPERTY()
  bool bStaticOnly = false;
  UPROPERTY()
  bool bNoFloating = true;
  UPROPERTY()
  bool bDiffraction = true;
  UPROPERTY()
  float MinSpacing = 100.0f;
  UPROPERTY()
  float MaxSpacing = 1000.0f;
  UPROPERTY()
  float HeadHeight = 150.0f;
  UPROPERTY()
  float MaxHeight = 300.0f;
  UPROPERTY()
  FVector GravityVector = FVector(0, 0, -1.0f); // Default gravity vector
  UPROPERTY()
  int32 ReflectionCount = 6;
  UPROPERTY()
  bool bCustomPointsEnabled = false;
  UPROPERTY()
  bool bHasCustomPoints = false;

  // Cached Points are the generated Acoustic Map points stored in ovrAudio coordinates without the Acoustic Map transform applied
  TArray<FVector> CachedPoints;
  ovrAudioSceneIR CachedMap = nullptr;
  ovrAudioSceneIRParameters MapParameters;
  FTransform PreviousTransform;

#if WITH_EDITOR
  virtual void PostEditComponentMove(bool bFinished) override;
  virtual void Activate(bool bReset = false) override;
  virtual void Deactivate() override;
  TSharedPtr<FAsyncTask<FAsyncSceneMappingTask>> MappingTask;
  void GenerateFileNameIfEmpty();
  void GatherGeometriesAndMaterials();

  FString Hash;
  bool bComputing = false;
  bool bComputeFinished = false;
  bool bComputeCanceled = false;
  bool bComputeSucceeded = false;
  FString Description;
  float ComputeProgress;
  float ComputeTime;
  double StartTimeSeconds;
  double StageStartingTimeSeconds;
  int32 DataSize;
  EAcousticMapStatus Status;
  int32 SelectedPointIndex = MAX_int32;
  TArray<UMetaXRAcousticGeometry*> Geometries;
  TArray<UMetaXRAcousticMaterial*> Materials;
#endif

  friend class FMetaXRAcousticMapDetails;
  friend class FAsyncSceneMappingTask;
  friend class FMetaXRAudioEditorMode;
  friend class FMetaXRAcousticMapSceneProxy;
};

#if WITH_EDITOR
class FMetaXRAcousticMapSceneProxy : public FDebugRenderSceneProxy {
 public:
  FMetaXRAcousticMapSceneProxy(const UPrimitiveComponent* InComponent);
  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
  virtual void GetDynamicMeshElements(
      const TArray<const FSceneView*>& Views,
      const FSceneViewFamily& ViewFamily,
      uint32 VisibilityMap,
      FMeshElementCollector& Collector) const override;
  virtual bool CanBeOccluded() const override {
    return false;
  }

 private:
  // Define sphere parameters
  const uint32 NumSlices = 32;
  const uint32 NumStacks = 32;
  const UMetaXRAcousticMap* AcousticMapComponent = nullptr;
  TArray<FDynamicMeshVertex> Vertices;
  TArray<uint32_t> Indices;
};
#endif
