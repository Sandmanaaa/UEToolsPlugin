// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MyBlueprintFunctionLibrary.generated.h"


UENUM(BlueprintType)
enum class EAssetOperationType : uint8
{
	ModifyMaterial,		// modify material param or engin value
	FindCurves,			// Find curves by curve name
	CheckBlueprintError,// check blueprint is error
};
/**
 * 
 */
UCLASS()
class MODIFYMATERIALPLUGIN_API UMyBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static void ChangeMatialName(FString CheckString, FName ChangeParamName, EAssetOperationType OperationType, FName SearchPath = FName(TEXT("None")));
};
