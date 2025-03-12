// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.
#include "MetaXRAudioDllManager.h"
#include "MetaXRAudioRoomAcousticProperties.h"

#ifdef META_WWISE_UNREAL_PLUGIN
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "GameFramework/PlayerController.h"
#endif // META_WWISE_UNREAL_PLUGIN

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"

// forward decleration should match OAP_Globals
extern "C" ovrResult ovrAudio_GetPluginContext(ovrAudioContext* context);
extern "C" ovrResult ovrAudio_SetFMODPluginUnitScale(float unitScale);
#ifdef META_WWISE_UNREAL_PLUGIN
extern "C" int WwiseEndpoint_SetListenerTransform(float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float rotW);
#endif // META_WWISE_UNREAL_PLUGIN

FMetaXRAudioLibraryManager& FMetaXRAudioLibraryManager::Get() {
  static FMetaXRAudioLibraryManager* sInstance;
  if (sInstance == nullptr) {
    sInstance = new FMetaXRAudioLibraryManager();
  }

  // Because the constructor of FMetaXRAudioLibraryManager is empty, expect we can only fail here if out of memory
  if (sInstance == nullptr) {
    UE_LOG(LogAudio, Error, TEXT("Meta XR Audio: Failed to get or create Meta XR Audio library manager"));
  }
  check(sInstance != nullptr);
  return *sInstance;
}

FMetaXRAudioLibraryManager::FMetaXRAudioLibraryManager()
    : MetaXRAudioDllHandle(nullptr), NumInstances(0), bInitialized(false), CachedPluginContext(nullptr) {}

FMetaXRAudioLibraryManager::~FMetaXRAudioLibraryManager() {
  FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void FMetaXRAudioLibraryManager::Initialize() {
  if (NumInstances == 0) {
    if (!LoadDll()) {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio: Failed to load OVR Audio dll"));
      return;
    }
  }

  NumInstances++;
  if (!bInitialized) {
    // Check the version number
    int32 MajorVersionNumber = -1;
    int32 MinorVersionNumber = -1;
    int32 PatchNumber = -1;

    const char* OvrVersionString = OVRA_CALL(ovrAudio_GetVersion)(&MajorVersionNumber, &MinorVersionNumber, &PatchNumber);
    if (MajorVersionNumber != OVR_AUDIO_MAJOR_VERSION || MinorVersionNumber != OVR_AUDIO_MINOR_VERSION) {
      UE_LOG(
          LogAudio,
          Warning,
          TEXT("Meta XR Audio: Using mismatched OVR Audio SDK Version! %d.%d vs. %d.%d"),
          OVR_AUDIO_MAJOR_VERSION,
          OVR_AUDIO_MINOR_VERSION,
          MajorVersionNumber,
          MinorVersionNumber);
      return;
    }
    bInitialized = true;
  }
}

void FMetaXRAudioLibraryManager::Shutdown() {
  if (NumInstances == 0) {
    // This means we failed to load the OVR Audio module during initialization and there's nothing to shutdown.
    return;
  }

  NumInstances--;

  if (NumInstances == 0) {
    // Shutdown OVR audio
    ReleaseDll();
    bInitialized = false;
  }
}

bool FMetaXRAudioLibraryManager::LoadDll() {
  if (MetaXRAudioDllHandle == nullptr) {
    const TCHAR* WWISE_DLL_NAME = TEXT("MetaXRAudioWwise");
    const TCHAR* FMOD_DLL_NAME = TEXT("MetaXRAudioFMOD");
    const TCHAR* UE_DLL_NAME = (sizeof(void*) == 4) ? TEXT("metaxraudio32") : TEXT("metaxraudio64");

#if PLATFORM_WINDOWS
    TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();

    int32 WwiseIndex =
        EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("Wwise"); });
    int32 FMODIndex =
        EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("FMODStudio"); });
    int32 UnrealNativeIndex =
        EnabledPlugins.IndexOfByPredicate([](const TSharedRef<IPlugin>& Plugin) { return Plugin->GetName() == TEXT("MetaXRAudio"); });

    bool DllFound = false;
    FString DllPath;
    const TCHAR* DLL_NAME = nullptr;
    if (WwiseIndex != INDEX_NONE) {
      FString PluginPath = EnabledPlugins[WwiseIndex]->GetBaseDir();

      // The user could have integrated various versions all build with different Windows SDK, so we do a general search for the binary.
      TArray<FString> FilePaths;
      IFileManager::Get().FindFilesRecursive(FilePaths, *PluginPath, *(WWISE_DLL_NAME + FString(".dll")), true, false, true);
      if (FilePaths.Num() > 1) {
        UE_LOG(LogAudio, Warning, TEXT("Meta XR Audio: Multiple versions of Meta's Wwise plugin found:"));
        for (auto& FilePath : FilePaths) {
          UE_LOG(LogAudio, Warning, TEXT("\t%s"), *FilePath);
        }
        UE_LOG(LogAudio, Warning, TEXT("Meta XR Audio: Loading the first one found, i.e. the binary at: %s"), *FilePaths[0]);
      }

      if (FilePaths.Num() > 0) {
        DllFound = true;
        DllPath = FPaths::GetPath(FilePaths[0]);
        DLL_NAME = WWISE_DLL_NAME;
        UE_LOG(
            LogAudio,
            Display,
            TEXT(
                "Meta XR Audio: MetaXRAudioWwise.dll found in Engine Plugins, using the Wwise version of the Meta XR Audio UE integration"));
      }
    }

    if (!DllFound && FMODIndex != INDEX_NONE) {
      DllPath = EnabledPlugins[FMODIndex]->GetBaseDir() / "Binaries" / "Win64";
      if (FPaths::FileExists(*(DllPath / FMOD_DLL_NAME + ".dll"))) {
        DllFound = true;
        DLL_NAME = FMOD_DLL_NAME;
        UE_LOG(
            LogAudio,
            Display,
            TEXT("Meta XR Audio: MetaXRAudioFMOD.dll found in Engine Plugins, using the FMOD version of the Meta XR Audio UE integration"));
      }
    }

    if (!DllFound && UnrealNativeIndex != INDEX_NONE) {
      DllPath = EnabledPlugins[UnrealNativeIndex]->GetBaseDir() / "Binaries" / "Win64";
      if (FPaths::FileExists(*(DllPath / UE_DLL_NAME + ".dll"))) {
        DllFound = true;
        DLL_NAME = UE_DLL_NAME;
        UE_LOG(LogAudio, Display, TEXT("Meta XR Audio: Found native UE AudioMixer."));
      }
    }

    if (!DllFound) {
      UE_LOG(
          LogAudio,
          Error,
          TEXT(
              "Meta XR Audio: Unable to find Meta XR Audio DLL in any of the expected integration folders. Make sure the binary exists and if in a non-standard location, edit the logice of the function MetaXRAudioDLLManager::LoadDll() accordingly."));
      return false;
    }

    FPlatformProcess::PushDllDirectory(*DllPath);
    MetaXRAudioDllHandle = FPlatformProcess::GetDllHandle(*(DllPath / DLL_NAME));
    FPlatformProcess::PopDllDirectory(*DllPath);

#elif PLATFORM_ANDROID
    FString Path = TEXT("lib");
    MetaXRAudioDllHandle = FPlatformProcess::GetDllHandle(*(Path + WWISE_DLL_NAME + ".so"));
    if (MetaXRAudioDllHandle != nullptr) {
      UE_LOG(
          LogAudio, Display, TEXT("Meta XR Audio: %s found, using the Wwise version of the Meta XR Audio UE integration"), WWISE_DLL_NAME);
      return true;
    }
    MetaXRAudioDllHandle = FPlatformProcess::GetDllHandle(*(Path + FMOD_DLL_NAME + ".so"));
    if (MetaXRAudioDllHandle != nullptr) {
      UE_LOG(LogAudio, Display, TEXT("Meta XR Audio: %s found, using the FMOD version of the Meta XR Audio UE integration"), FMOD_DLL_NAME);
      return true;
    }
    MetaXRAudioDllHandle = FPlatformProcess::GetDllHandle(*(Path + UE_DLL_NAME + ".so"));
    if (MetaXRAudioDllHandle != nullptr) {
      UE_LOG(LogAudio, Display, TEXT("Meta XR Audio: Middleware plugins not found, %s found, assuming native UE AudioMixer"), UE_DLL_NAME);
      return true;
    } else {
      UE_LOG(LogAudio, Error, TEXT("Meta XR Audio: Unable to load Meta XR Audio UE integratiton"), UE_DLL_NAME);
    }
#endif

    return (MetaXRAudioDllHandle != nullptr);
  }
  return true;
}

void FMetaXRAudioLibraryManager::ReleaseDll() {
#if PLATFORM_WINDOWS
  if (NumInstances == 0 && MetaXRAudioDllHandle) {
    FPlatformProcess::FreeDllHandle(MetaXRAudioDllHandle);
    MetaXRAudioDllHandle = nullptr;
  }
#endif
}

bool FMetaXRAudioLibraryManager::UpdatePluginContext(float DeltaTime) {
  QUICK_SCOPE_CYCLE_COUNTER(STAT_FMetaXRAudioLibraryManager_UpdatePluginContext);

  ovrAudioContext Context = GetPluginContext();

#ifdef META_WWISE_UNREAL_PLUGIN
  // Wwise provides positional information as listener-relative. We pass information here to convert to world-relative
  if (GEngine) {
    TArray<APlayerController*> players;
    GEngine->GetAllLocalPlayerControllers(players);
    if (!players.IsEmpty()) {
      FVector DeviceRight = FVector::RightVector;
      FVector DeviceFront = FVector::ForwardVector;
      FVector DevicePosition = FVector::ZeroVector;
      FQuat UEQuaternion;
      players[0]->GetAudioListenerPosition(DevicePosition, DeviceFront, DeviceRight);
      UEQuaternion = players[0]->GetControlRotation().Quaternion();
      // This function requests data in Wwise convention (left handed, x is right, y is up, z is forward)
      // Unreal convention is (x:forward, y:right, z:up)
      auto WwiseEndpoint_SetListenerTransformFunction = OVRA_CALL(WwiseEndpoint_SetListenerTransform);
      if (WwiseEndpoint_SetListenerTransformFunction != nullptr) {
        int result = WwiseEndpoint_SetListenerTransformFunction(
            (float)DevicePosition.Y,
            (float)DevicePosition.Z,
            (float)DevicePosition.X,
            (float)UEQuaternion.Y,
            (float)UEQuaternion.Z,
            (float)UEQuaternion.X,
            (float)UEQuaternion.W);
      }
    }
  }
#endif // META_WWISE_UNREAL_PLUGIN

  UMetaXRAudioRoomAcousticProperties* RoomAcoustics = UMetaXRAudioRoomAcousticProperties::GetActiveRoomAcoustics();
  if (RoomAcoustics && Context != nullptr) {
    RoomAcoustics->UpdateRoomModel(Context);
  }

  return true;
}

ovrAudioContext FMetaXRAudioLibraryManager::GetPluginContext() {
  if (CachedPluginContext == nullptr) {
    auto GetPluginContext = OVRA_CALL(ovrAudio_GetPluginContext);
    if (GetPluginContext != nullptr) {
      OVR_AUDIO_CHECK(GetPluginContext(&CachedPluginContext), "Failed to get Meta XR Audio context");

      // FMOD or Wwise plugins need to be informed about the unit scale (which is centimeters for UE)
      auto SetUnitScale = OVRA_CALL(ovrAudio_SetUnitScale);
      if (SetUnitScale != nullptr) {
        OVR_AUDIO_CHECK(SetUnitScale(CachedPluginContext, 0.01f), "Failed to set unit scale");
      }

      // Tick the scene from here since there is no listener
      auto TickDelegate = FTickerDelegate::CreateRaw(this, &FMetaXRAudioLibraryManager::UpdatePluginContext);
      TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
    }
  }
  return CachedPluginContext;
}
