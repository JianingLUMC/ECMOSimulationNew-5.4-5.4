// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaXRAcousticProjectSettings.h"

UMetaXRAcousticProjectSettings::UMetaXRAcousticProjectSettings()
    : AcousticModel(EMetaXRAudioAcousticModel::Automatic), bDiffractionEnabled(true), ExcludeTags(), bMapBakeWriteGeo(true) {}
