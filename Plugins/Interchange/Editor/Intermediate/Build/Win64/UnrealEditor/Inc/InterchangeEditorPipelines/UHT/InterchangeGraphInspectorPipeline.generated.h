// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "InterchangeGraphInspectorPipeline.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef INTERCHANGEEDITORPIPELINES_InterchangeGraphInspectorPipeline_generated_h
#error "InterchangeGraphInspectorPipeline.generated.h already included, missing '#pragma once' in InterchangeGraphInspectorPipeline.h"
#endif
#define INTERCHANGEEDITORPIPELINES_InterchangeGraphInspectorPipeline_generated_h

#define FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_21_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUInterchangeGraphInspectorPipeline(); \
	friend struct Z_Construct_UClass_UInterchangeGraphInspectorPipeline_Statics; \
public: \
	DECLARE_CLASS(UInterchangeGraphInspectorPipeline, UInterchangePipelineBase, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/InterchangeEditorPipelines"), NO_API) \
	DECLARE_SERIALIZER(UInterchangeGraphInspectorPipeline)


#define FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_21_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UInterchangeGraphInspectorPipeline(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	UInterchangeGraphInspectorPipeline(UInterchangeGraphInspectorPipeline&&); \
	UInterchangeGraphInspectorPipeline(const UInterchangeGraphInspectorPipeline&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UInterchangeGraphInspectorPipeline); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UInterchangeGraphInspectorPipeline); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UInterchangeGraphInspectorPipeline) \
	NO_API virtual ~UInterchangeGraphInspectorPipeline();


#define FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_18_PROLOG
#define FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_21_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_21_INCLASS_NO_PURE_DECLS \
	FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h_21_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> INTERCHANGEEDITORPIPELINES_API UClass* StaticClass<class UInterchangeGraphInspectorPipeline>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Engine_Plugins_Interchange_Editor_Source_Pipelines_Public_InterchangeGraphInspectorPipeline_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
