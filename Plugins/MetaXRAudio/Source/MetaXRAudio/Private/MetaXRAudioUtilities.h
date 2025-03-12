// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#ifndef METAXRAUDIOUTILITIES
#define METAXRAUDIOUTILITIES

#include "AudioPluginUtilities.h"
#include "Engine/Engine.h"
#include "MetaXRAudioEditorInfo.h"
#include "MetaXRAudioPlatform.h"
#include "MetaXR_Audio.h"

class MetaXRAudioUtilities {
 public:
  // Helper function to convert from UE coords to OVR coords.
  static FVector ToOVRVector(const FVector& InVec) {
    return FVector(InVec.Y, InVec.Z, -InVec.X);
  }

  // Helper function to convert from OVR coords to UE coords.
  static FVector ToUEVector(const FVector& InVec) {
    return FVector(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector3f ToUEVector3f(const FVector3f& InVec) {
    return FVector3f(-InVec.Z, InVec.X, InVec.Y);
  }

  static FVector ToOVRVector(const Audio::FChannelPositionInfo& ChannelPositionInfo) {
    FVector OvrVector;
    OvrVector.X = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Cos(ChannelPositionInfo.Elevation);
    OvrVector.Y = ChannelPositionInfo.Radius * FMath::Sin(ChannelPositionInfo.Azimuth) * FMath::Sin(ChannelPositionInfo.Elevation);
    OvrVector.Z = ChannelPositionInfo.Radius * FMath::Cos(ChannelPositionInfo.Azimuth);

    return OvrVector;
  }

  static float dbToLinear(float db) {
    return powf(10.0f, db / 20.0f);
  }

  static TArray<float> ConvertUETransformToOVRTransform(FTransform InTransform) {
    // UE:        x:forward, y:right, z:up
    // Meta XR:  x:right,   y:up,    z:backward
    // UE y = Meta x | UE z = meta y | negative UE x = meta z
    FMatrix Matrix = InTransform.ToMatrixWithScale();
    TArray<float> OutTransform = {
        (float)Matrix.M[1][1],
        (float)Matrix.M[1][2],
        (float)-Matrix.M[1][0],
        (float)Matrix.M[1][3], // UE y right - > Meta x right
        (float)Matrix.M[2][1],
        (float)Matrix.M[2][2],
        (float)-Matrix.M[2][0],
        (float)Matrix.M[2][3], // UE z up-> Meta y up
        (float)-Matrix.M[0][1],
        (float)-Matrix.M[0][2],
        (float)Matrix.M[0][0],
        (float)-Matrix.M[0][3], // UE x forward -> Meta z backward
        (float)Matrix.M[3][1],
        (float)Matrix.M[3][2],
        (float)-Matrix.M[3][0],
        (float)Matrix.M[3][3], // position
    };
    return OutTransform;
  }

  static bool PlayModeActive(UWorld* World) {
    if (World != nullptr) {
      if (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE) {
        return true;
      }
    }
    return false;
  }

#if WITH_EDITOR
  static void CreateMetaXRAcousticContentDirectory(FString MetaContentDirectoryName) {
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FString DesiredDirectory = FPaths::ProjectContentDir() + MetaContentDirectoryName;
    if (!PlatformFile.DirectoryExists(*DesiredDirectory)) {
      PlatformFile.CreateDirectoryTree(*DesiredDirectory);
      UE_LOG(
          LogAudio,
          Display,
          TEXT("Generating the %s folder in Content as the recommend place to save Meta XR Audio files"),
          *MetaContentDirectoryName)
    }
  }
#endif

  static bool IsMetaXRAudioTheCurrentSpatializationPlugin() {
#ifdef META_NATIVE_UNREAL_PLUGIN
#if WITH_EDITOR
    static FString MetaXRDisplayName = FString(TEXT(META_XR_AUDIO_DISPLAY_NAME));
    return AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
#else // !WITH_EDITOR
    // For non editor situations, we can cache whether this is the current plugin or not the first time we check.
    static FString MetaXRDisplayName = FString(TEXT(META_XR_AUDIO_DISPLAY_NAME));
    static bool bCheckedSpatializationPlugin = false;
    static bool bIsMetaXRCurrentSpatiatizationPlugin = false;

    if (!bCheckedSpatializationPlugin) {
      bIsMetaXRCurrentSpatiatizationPlugin =
          AudioPluginUtilities::GetDesiredPluginName(EAudioPlugin::SPATIALIZATION).Equals(MetaXRDisplayName);
      bCheckedSpatializationPlugin = true;
    }

    return bIsMetaXRCurrentSpatiatizationPlugin;
#endif // WITH_EDITOR
#else // !META_NATIVE_UNREAL_PLUGIN
    return false;
#endif // META_NATIVE_UNREAL_PLUGIN
  }
};

#endif // METAXRAUDIOUTILITIES
