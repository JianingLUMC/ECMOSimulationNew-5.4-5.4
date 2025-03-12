// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Components/ActorComponent.h"
#include "MetaXRAcousticMaterialProperties.h"
#include "MetaXR_Audio.h"
#include "MetaXR_Audio_Propagation.h"

#include "MetaXRAcousticMaterial.generated.h"

/*
 * MetaXRAudio material components are used to set the acoustic properties of the geometry.
 */
UCLASS(
    ClassGroup = (Audio),
    HideCategories = (Activation, Collision, Cooking),
    meta =
        (BlueprintSpawnableComponent,
         DisplayName = "Meta XR Acoustic Material",
         ToolTip = "Change the acoustic properties of a geometry via an acoustic material"))
class METAXRAUDIO_API UMetaXRAcousticMaterial : public UActorComponent {
  GENERATED_BODY()
 public:
  void ConstructMaterial(ovrAudioMaterial Material);

#if WITH_EDITOR
  void SetMaterialPropertiesAsset(UMetaXRAcousticMaterialProperties* NewProperties);
  void AppendHash(FString& hash);
  UMetaXRAcousticMaterialProperties* GetMaterialPreset() {
    return MaterialPreset;
  }
#endif
 private:
  UPROPERTY()
  UMetaXRAcousticMaterialProperties* MaterialPreset = nullptr;

  friend class FMetaXRAcousticMaterialComponentDetails;
};
