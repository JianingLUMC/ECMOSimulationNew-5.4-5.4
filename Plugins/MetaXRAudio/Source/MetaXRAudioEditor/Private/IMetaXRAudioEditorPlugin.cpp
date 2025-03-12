// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IMetaXRAudioEditorPlugin.h"
#include "MetaXRAudioEditorInfo.h"
#include "MetaXRAudioPlatform.h"

#include "EditorStyleSet.h"
#include "MetaXRAcousticControlZone.h"
#include "MetaXRAcousticControlZoneDetails.h"
#include "MetaXRAcousticControlZoneVisualizer.h"
#include "MetaXRAcousticGeometry.h"
#include "MetaXRAcousticGeometryDetails.h"
#include "MetaXRAcousticMap.h"
#include "MetaXRAcousticMapDetails.h"
#include "MetaXRAcousticMaterial.h"
#include "MetaXRAcousticMaterialDetails.h"
#include "MetaXRAcousticMaterialPropertiesDetails.h"
#include "MetaXRAcousticMaterialPropertiesFactory.h"
#include "MetaXRAcousticProjectSettings.h"
#include "MetaXRAudioEditorMode.h"
#include "MetaXRAudioRoomAcousticProperties.h"
#include "MetaXRAudioRoomAcousticVisualizer.h"

#ifdef META_NATIVE_UNREAL_PLUGIN
#include "MetaXRAmbisonicSettingsFactory.h"
#include "MetaXRAudioSettings.h"
#include "MetaXRAudioSourceSettingsFactory.h"
#endif // META_NATIVE_UNREAL_PLUGIN

#include "DynamicMeshBuilder.h"

#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "FrameTypes.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "MaterialShared.h"
#include "Modules/ModuleManager.h"
#include "NavigationSystem.h"
#include "UnrealEdGlobals.h"

DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<FExtender>, FShowMenuExtenderDelegate, const TSharedRef<FUICommandList>);

void FMetaXRAudioEditorPlugin::StartupModule() {
  // Register asset types
  IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
  {
    ISettingsModule* SettingsModule = FModuleManager::Get().GetModulePtr<ISettingsModule>("Settings");

#ifdef META_NATIVE_UNREAL_PLUGIN
    AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MetaXRAudioSourceSettings));
    // At this time the ambisonic module has no settings so don't allow users to create them
    // However, it is required for it to exist so the ambisonic virtual functions can be fufilled
    // AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MetaXRAmbisonicsSettings));
    if (SettingsModule) {
      SettingsModule->RegisterSettings(
          "Project",
          "Plugins",
          "Meta XR Audio",
          NSLOCTEXT("MetaXRAudio", "Meta XR Audio", "Meta XR Audio"),
          NSLOCTEXT("MetaXRAudio", "Configure Meta XR Audio settings", "Configure Meta XR Audio settings"),
          GetMutableDefault<UMetaXRAudioSettings>());
    }
#endif

    if (SettingsModule) {
      SettingsModule->RegisterSettings(
          "Project",
          "Plugins",
          "Meta XR Audio Acoustics",
          NSLOCTEXT("MetaXRAcoustics", "Meta XR Acoustics", "Meta XR Acoustics"),
          NSLOCTEXT("MetaXRAcoustics", "Configure Meta XR Acoustic settings", "Configure Meta XR Acoustic Settings"),
          GetMutableDefault<UMetaXRAcousticProjectSettings>());
    }

    // Register the custom editor mode
    FEditorModeRegistry::Get().RegisterMode<FMetaXRAudioEditorMode>(
        FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId,
        NSLOCTEXT("MetaXRAudio", "Meta XR Audio", "Meta XR Audio"),
        FSlateIcon(),
        true);
  }

  // Register Visualizers
  if (GUnrealEd) {
    TSharedPtr<FMetaXRAudioRoomAcousticVisualizer> RoomAcousticVisualizer = MakeShareable(new FMetaXRAudioRoomAcousticVisualizer());
    GUnrealEd->RegisterComponentVisualizer(UMetaXRAudioRoomAcousticProperties::StaticClass()->GetFName(), RoomAcousticVisualizer);
    RoomAcousticVisualizer->OnRegister();
  }

  if (GUnrealEd) {
    TSharedPtr<FMetaXRAcousticControlZoneVisualizer> ControlZoneVisualizer = MakeShareable(new FMetaXRAcousticControlZoneVisualizer());
    GUnrealEd->RegisterComponentVisualizer(UMetaXRAcousticControlZoneWrapper::StaticClass()->GetFName(), ControlZoneVisualizer);
    ControlZoneVisualizer->OnRegister();
  }

  IConsoleManager::Get().RegisterConsoleVariable(
      TEXT("MetaXRAudioGizmos"),
      1,
      TEXT("Shows or hide Gizmos for MetaXR Audio Plugin\n") TEXT("<=0: Hide\n") TEXT("  1: Show\n"),
      ECVF_Default);

  AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_MetaXRAcousticMaterialProperties));

  FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

  // Create a section for Meta XR Audio controls to be displayed
#define LOCTEXT_NAMESPACE "PropertySection"
  TSharedRef<FPropertySection> Section = PropertyModule.FindOrCreateSection(
      "Object", META_XR_AUDIO_DISPLAY_NAME, LOCTEXT(META_XR_AUDIO_DISPLAY_NAME, META_XR_AUDIO_DISPLAY_NAME));
  Section->AddCategory(META_XR_AUDIO_DISPLAY_NAME);
#undef LOCTEXT_NAMESPACE

  // Register our custom GUIs with the property editor
  PropertyModule.RegisterCustomClassLayout(
      UMetaXRAcousticGeometry::StaticClass()->GetFName(),
      FOnGetDetailCustomizationInstance::CreateStatic(&FMetaXRAcousticGeometryDetails::MakeInstance));
  PropertyModule.RegisterCustomClassLayout(
      UMetaXRAcousticMap::StaticClass()->GetFName(),
      FOnGetDetailCustomizationInstance::CreateStatic(&FMetaXRAcousticMapDetails::MakeInstance));
  PropertyModule.RegisterCustomClassLayout(
      UMetaXRAcousticMaterial::StaticClass()->GetFName(),
      FOnGetDetailCustomizationInstance::CreateStatic(&FMetaXRAcousticMaterialDetails::MakeInstance));
  PropertyModule.RegisterCustomClassLayout(
      UMetaXRAcousticMaterialProperties::StaticClass()->GetFName(),
      FOnGetDetailCustomizationInstance::CreateStatic(&FMetaXRAcousticMaterialPropertiesDetails::MakeInstance));
  PropertyModule.RegisterCustomClassLayout(
      AMetaXRAcousticControlZone::StaticClass()->GetFName(),
      FOnGetDetailCustomizationInstance::CreateStatic(&FMetaXRAcousticControlZoneDetails::MakeInstance));

  UMetaXRAcousticMaterialPropertiesFactory* MyFactory = NewObject<UMetaXRAcousticMaterialPropertiesFactory>();
  MyFactory->SupportedClass = UMetaXRAcousticMaterialProperties::StaticClass();
  MyFactory->AddToRoot();

  // Assume FShowMenuExtenderDelegate has been declared as shown above
  FShowMenuExtenderDelegate ShowMenuExtenderDelegate;
  ShowMenuExtenderDelegate.BindStatic(&FMetaXRAudioEditorPlugin::ExtendShowMenu);

  FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
  LevelEditorModule.GetAllLevelViewportShowMenuExtenders().Add(ShowMenuExtenderDelegate);
}

void FMetaXRAudioEditorPlugin::ShutdownModule() {
  if (GUnrealEd) {
    GUnrealEd->UnregisterComponentVisualizer(UMetaXRAudioRoomAcousticProperties::StaticClass()->GetFName());
  }

  // Unregister the custom details view class
  if (FModuleManager::Get().IsModuleLoaded("PropertyEditor")) {
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.UnregisterCustomClassLayout(UMetaXRAcousticGeometry::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(UMetaXRAcousticMap::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(UMetaXRAcousticMaterial::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(UMetaXRAcousticMaterialProperties::StaticClass()->GetFName());
    PropertyModule.UnregisterCustomClassLayout(AMetaXRAcousticControlZone::StaticClass()->GetFName());
  }

  if (GUnrealEd) {
    GUnrealEd->UnregisterComponentVisualizer(UMetaXRAcousticGeometry::StaticClass()->GetFName());
  }

  FEditorModeRegistry::Get().UnregisterMode(FMetaXRAudioEditorMode::EM_MetaXRAcousticMapEditorModeId);
}

#define LOCTEXT_NAMESPACE "MetaXRShowMenu"
TSharedRef<FExtender> FMetaXRAudioEditorPlugin::ExtendShowMenu(const TSharedRef<FUICommandList> CommandList) {
  TSharedRef<FExtender> Extender = MakeShareable(new FExtender);

  // Add your custom commands or options here
  Extender->AddMenuExtension(
      "LevelViewportEditorShow", EExtensionHook::After, CommandList, FMenuExtensionDelegate::CreateLambda([](FMenuBuilder& MenuBuilder) {
        // Add your custom widgets or commands here
        MenuBuilder.BeginSection("LevelViewportShowFlagsMetaXR", LOCTEXT("MetaXRShowFlagHeader", "Meta XR Audio"));
        MenuBuilder.AddSubMenu(
            FText::FromString("Visualization"),
            FText::FromString("Toggle the display of Meta XR features"),
            FNewMenuDelegate::CreateLambda([](FMenuBuilder& SubMenuBuilder) {
              // Add checkboxes or other items to the submenu
              SubMenuBuilder.AddMenuEntry(
                  FText::FromString("Show Gizmos"),
                  FText::FromString("Show Gizmos"),
                  FSlateIcon(),
                  FUIAction(
                      FExecuteAction::CreateLambda([] {
                        IConsoleVariable* MetaXRAudioGizmoCVAR = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos"));
                        MetaXRAudioGizmoCVAR->Set(!MetaXRAudioGizmoCVAR->GetInt());
                        // Invalidate the viewport to force a refresh
                        for (FEditorViewportClient* ViewportClient : GEditor->GetAllViewportClients()) {
                          if (ViewportClient) {
                            ViewportClient->Invalidate();
                          }
                        }
                      }),
                      FCanExecuteAction(),
                      FIsActionChecked::CreateLambda([] {
                        IConsoleVariable* MetaXRAudioGizmoCVAR = IConsoleManager::Get().FindConsoleVariable(TEXT("MetaXRAudioGizmos"));
                        return MetaXRAudioGizmoCVAR->GetInt() != 0; /* Implement your checked logic for Feature 1 here */
                      })),
                  "Show Gizmos",
                  EUserInterfaceActionType::ToggleButton);
            }));
        MenuBuilder.EndSection();
      }));

  return Extender;
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMetaXRAudioEditorPlugin, MetaXRAudioEditor)
