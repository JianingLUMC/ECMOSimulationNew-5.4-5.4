// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAcousticMaterial.h"
#include "MetaXRAudioDllManager.h"

void UMetaXRAcousticMaterial::ConstructMaterial(ovrAudioMaterial ovrMaterial) {
  if (!MaterialPreset) {
    return;
  }
  const FMetaXRAcousticMaterialData& Data = MaterialPreset->Data;

  for (const FMetaXRAudioPoint& Point : Data.Absorption.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Absorption, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material absorption at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }

  for (const FMetaXRAudioPoint& Point : Data.Transmission.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Transmission, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material transmission at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }

  for (const FMetaXRAudioPoint& Point : Data.Scattering.Points) {
    ovrResult Result =
        OVRA_CALL(ovrAudio_AudioMaterialSetFrequency)(ovrMaterial, ovrAudioMaterialProperty_Scattering, Point.Frequency, Point.Data);
    if (Result != ovrSuccess) {
      UE_LOG(LogAudio, Warning, TEXT("Unable to set material scattering at frequency %f with value %f"), Point.Frequency, Point.Data);
    }
  }
}

#if WITH_EDITOR
void UMetaXRAcousticMaterial::SetMaterialPropertiesAsset(UMetaXRAcousticMaterialProperties* NewProperties) {
  MaterialPreset = NewProperties;
}

void UMetaXRAcousticMaterial::AppendHash(FString& hash) {
  if (MaterialPreset == nullptr) {
    return;
  }

  const FMetaXRAcousticMaterialData& Data = MaterialPreset->Data;
  FString materialHash;

  for (const FMetaXRAudioPoint& Point : Data.Absorption.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  for (const FMetaXRAudioPoint& Point : Data.Transmission.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  for (const FMetaXRAudioPoint& Point : Data.Scattering.Points) {
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Frequency));
    materialHash.Append(FString::Printf(TEXT("%f"), Point.Data));
  }

  // Unreal FMD5Hash only accepts string, so convert to string first
  hash.Append(FMD5::HashAnsiString(*materialHash));
}
#endif
