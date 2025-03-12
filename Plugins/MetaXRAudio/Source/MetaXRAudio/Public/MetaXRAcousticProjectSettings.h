// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MetaXR_Audio.h"

#include "MetaXRAcousticProjectSettings.generated.h"

UENUM()
enum class EMetaXRAudioAcousticModel : int8 {
  Automatic = ovrAudioAcousticModel_Automatic,
  None = ovrAudioAcousticModel_None,
  ShoeboxRoom = ovrAudioAcousticModel_StaticRoom,
  AcousticRayTracing = ovrAudioAcousticModel_PropagationSystem
};

#define META_XR_AUDIO_DEFAULT_SAVE_FOLDER "MetaXRAcoustics"

UCLASS(config = Game, defaultconfig, BlueprintType)
class METAXRAUDIO_API UMetaXRAcousticProjectSettings : public UObject {
  GENERATED_BODY()

 public:
  UMetaXRAcousticProjectSettings();

  // Select which type of acoustic modeling system is used to generate reverb and reflections.
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  EMetaXRAudioAcousticModel AcousticModel;

  // When enabled and using geometry, all spatailized AudioSources will diffract (propagate around corners and obstructions)
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  bool bDiffractionEnabled;

  // Exclude tags all you to specify Actor Tags which cause meshes with this tag to be excluded from acoustic simulation.
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  TArray<FString> ExcludeTags;

  // When you bake an acoustic map, also bake all the acoustic geometry files
  UPROPERTY(GlobalConfig, BlueprintReadWrite, EditAnywhere, Category = "AcousticsSettings")
  bool bMapBakeWriteGeo;
};
