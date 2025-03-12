// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
#pragma once

#include "IDetailCustomization.h"

class METAXRAUDIOEDITOR_API FMetaXRAcousticGeometryDetails : public IDetailCustomization {
 public:
  static TSharedRef<IDetailCustomization> MakeInstance() {
    return MakeShareable(new FMetaXRAcousticGeometryDetails);
  }
  FString SaveFileDialog(AActor* Context) const;
  FDateTime GetCurrentTimestamp(FString FilePath) const;
  virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) override;
  virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

 private:
  TWeakPtr<IDetailLayoutBuilder> CachedDetailBuilder;
};
