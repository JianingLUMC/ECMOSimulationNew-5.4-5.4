// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DebugRenderSceneProxy.h"
#include "LandscapeInfo.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticGeometry.generated.h"

// Fwd declare
class UMetaXRAcousticMaterial;
class UMetaXRAcousticGeometry;

/*
 * MetaXRAudio geometry components are used to customize an a static mesh actor's acoustic properties.
 */
typedef uint32_t MetaXRAudioMeshFlags;

UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Geometry",
         ToolTip = "Analyze a mesh to generate acoustics, occlusion, and diffraction"))
class METAXRAUDIO_API UMetaXRAcousticGeometry : public UPrimitiveComponent {
  GENERATED_BODY()

 public:
  TArray<FDynamicMeshVertex> SimplifiedVertices;
  TArray<uint32> SimplifiedIndices;

  UMetaXRAcousticGeometry();
  ~UMetaXRAcousticGeometry();
#if WITH_EDITOR
  virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
  bool CreatePropagationGeometry();
  bool UploadGeometry();
  bool StartInternal();
  bool DestroyInternal();
  bool ReadFile();
  bool WriteFile();
  bool IsOlder(FDateTime TimeStamp);
  void AppendHash(FString& Hash);
  bool WriteFileInternal(ovrAudioGeometry GeometryHandle);
  bool IncludesChildren() const {
    return bIncludeChildren;
  }

  bool DiffractionEnabled() {
    return MeshFlags & ovrAudioMeshFlags_enableDiffraction;
  }

  void SetDiffractionEnabled(bool bEnabled) {
    if (bEnabled)
      MeshFlags |= ovrAudioMeshFlags_enableDiffraction;
    else
      MeshFlags &= ~ovrAudioMeshFlags_enableDiffraction;
  }

  bool IsFileEnabled() {
    return bFileEnabled;
  }

  ovrAudioGeometry GetHandle() {
    return OvrGeometry;
  }

  FString GetFilePath() {
    return FilePath;
  }

  virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
  virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

  // Structures
  struct FMeshMaterial {
    UStaticMeshComponent* StaticMesh;
    int32 LOD;
    TArray<UMetaXRAcousticMaterial*> Materials;
  };

  struct FLandscapeMaterial {
    ULandscapeInfo* LandscapeInfo = nullptr;
    TArray<UMetaXRAcousticMaterial*> Materials;
  };

  // Define visitor class skeleton and declare the implementations
  class ITransformVisitor {
   public:
    virtual ~ITransformVisitor(){};
    virtual void* Visit(AActor* Transform, void* UserData) = 0;
  };

  class IGatherer : public ITransformVisitor {
   public:
    TArray<UMetaXRAcousticGeometry::FLandscapeMaterial> GetTerrains() {
      return Terrains;
    }
    TArray<UMetaXRAcousticGeometry::FMeshMaterial> GetMeshes() {
      return Meshes;
    }
    TArray<UMetaXRAcousticGeometry::FLandscapeMaterial> Terrains;
    TArray<UMetaXRAcousticGeometry::FMeshMaterial> Meshes;
  };

  class FAgeChecker : public ITransformVisitor {
   public:
    FAgeChecker(FDateTime TimeStamp);
    void* Visit(AActor* Transform, void* UserData) override;

    FDateTime TimeStamp;
    bool bIsOlder = false;
  };

  class FHashAppender : public ITransformVisitor {
   public:
    FHashAppender(FString Hash);
    void* Visit(AActor* Transform, void* UserData) override;
    FString Hash;
  };

  class FMeshGatherer : public IGatherer {
   public:
    FMeshGatherer(bool IgnoreStatic, int LODSelection = 0);
    void* Visit(AActor* Transform, void* UserData) override;
    int LodSelection = 0;
    int IgnoredMeshCount = 0;
    bool bIgnoreStatic;
  };

 private:
  virtual void OnRegister() override;
  virtual void OnUnregister() override;
  virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
  virtual void PostLoad() override;
  virtual void BeginDestroy() override;
  virtual void DestroyComponent(bool bPromoteChildren) override;
  virtual void Activate(bool bReset = false) override;
  virtual void Deactivate() override;

  // Hierarchy Traversal
  static void TraverseMeshHierarchy(
      AActor* Actor,
      bool IncludeChildren,
      TArray<FString> ExcludeTags,
      bool bParentWasExcluded,
      int LodSelection,
      ITransformVisitor& Visitor,
      void* ParentData = nullptr);

#if WITH_EDITOR
  void UpdateGizmoMesh();
  void UpdateGizmoMesh(ovrAudioGeometry GeometryHandle);
  void GenerateFileNameIfEmpty();
#endif

  bool DestroyPropagationGeometry();
  bool UploadMesh(ovrAudioGeometry GeometryHandle);
  bool UploadMesh(ovrAudioGeometry GeometryHandle, AActor* Owner, bool IgnoreStatic, int& OutIgnoredMeshCount);
  void ApplyTransform();
  void LoadGeometryAsync();
  bool IsStatic();
  void CheckGeoTransformValid();

  // Mesh hierarchy optimization for both content editing and runtime performance
  // if IncludeChildren is true, children (attached) meshes will be merged
  UPROPERTY()
  bool bIncludeChildren = true;
  UPROPERTY()
  float MaxError = 10.0f;
  UPROPERTY()
  int32 LOD = 0;
  UPROPERTY()
  FString FilePath;
  UPROPERTY()
  bool bFileEnabled = true;
  UPROPERTY()
  int32 MeshFlags = ovrAudioMeshFlags_enableMeshSimplification;

  int32 DataSize;
  ovrAudioGeometry OvrGeometry;
  ovrAudioContext CachedContext;
  FTransform PreviousTransform;
  ovrAudioGeometry PreviousGeometry;
  TArray<FString> ExcludeTags;

  friend class FMetaXRAcousticGeometryDetails;
};

class FMetaXRAcousticGeometrySceneProxy : public FDebugRenderSceneProxy {
 public:
  FMetaXRAcousticGeometrySceneProxy(const UPrimitiveComponent* InComponent);
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
  const UMetaXRAcousticGeometry* GeometryComponent = nullptr;
};
