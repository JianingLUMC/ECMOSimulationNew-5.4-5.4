// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameScoreStatic.generated.h"

/**
 * 
 */
UCLASS()
class ECMOSIMULATIONNEW_API UGameScoreStatic : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "File")
	static bool AppendStringsToCSV(const FString& FilePath, const FString& String1, const FString& String2, const FString& String3, const FString& String4, const FString& String5);
	
};
