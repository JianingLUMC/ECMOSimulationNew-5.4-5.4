// Fill out your copyright notice in the Description page of Project Settings.


#include "GameScoreStatic.h"

bool UGameScoreStatic::AppendStringsToCSV(const FString& FilePath, const FString& String1, const FString& String2, const FString& String3, const FString& String4)
{
    FString RowData = FString::Printf(TEXT("\"%s\",\"%s\",\"%s\",\"%s\"\n"), *String1, *String2, *String3, *String4);

    return FFileHelper::SaveStringToFile(RowData, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}