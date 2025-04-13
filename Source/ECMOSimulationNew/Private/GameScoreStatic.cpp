// Fill out your copyright notice in the Description page of Project Settings.


#include "GameScoreStatic.h"
#include "Misc/FileHelper.h"


bool UGameScoreStatic::AppendStringsToCSV(const FString& FilePath, const FString& String1, const FString& String2)
{
    FString RowData = FString::Printf(TEXT("\"%s\",\"%s\"%s"), *String1, *String2, LINE_TERMINATOR);

    return FFileHelper::SaveStringToFile(RowData, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}