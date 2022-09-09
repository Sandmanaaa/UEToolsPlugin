// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetBundleData.h"
#include "AssetRegistry/AssetData.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "FileHelpers.h"
#include "UObject/Package.h"
#include "Animation/AnimSequenceBase.h"
#include <Kismet2/KismetEditorUtilities.h>
#include <BlueprintEditorSettings.h>
#include <Editor/BlueprintGraph/Public/BlueprintEditorSettings.h>
#include <Kismet2/CompilerResultsLog.h>
#include <Engine/ObjectLibrary.h>

void UMyBlueprintFunctionLibrary::ChangeMatialName(FString CheckString, FName ChangeParamName, EAssetOperationType OperationType, FName SearchPath)
{
	FAssetRegistryModule* AssetRegistryModule = FModuleManager::Get().GetModulePtr<FAssetRegistryModule>("AssetRegistry");
	if (nullptr == AssetRegistryModule)
	{
		return;
	}
	const IAssetRegistry& AssetRegistry = AssetRegistryModule->Get();

	// change uobject class name 
	FName ChangeClassName;
	switch (OperationType)
	{
	case EAssetOperationType::ModifyMaterial:ChangeClassName = UMaterial::StaticClass()->GetFName();
		break;
	case EAssetOperationType::FindCurves:ChangeClassName = UAnimSequenceBase::StaticClass()->GetFName();
		break;
	case EAssetOperationType::CheckBlueprintError:ChangeClassName = AActor::StaticClass()->GetFName();
	default:
		break;
	}

	TArray<FAssetData> Assets;

	switch (OperationType)
	{
	case EAssetOperationType::ModifyMaterial:
	case EAssetOperationType::FindCurves:
	{
		FARFilter Filter;
		Filter.ClassNames.Add(ChangeClassName);
		Filter.bRecursiveClasses = true;
		if (SearchPath != FName(TEXT("None")))
		{
			Filter.PackagePaths.Add(SearchPath);
			Filter.bRecursivePaths = true;
		}
		AssetRegistry.GetAssets(Filter, Assets);
	}
		break;
	case EAssetOperationType::CheckBlueprintError:
	{
		UObjectLibrary* lib = UObjectLibrary::CreateLibrary(UObject::StaticClass(), false, GIsEditor);
		lib->AddToRoot();
		lib->LoadAssetDataFromPath(SearchPath.ToString());
		lib->LoadAssetsFromAssetData();
		lib->GetAssetDataList(Assets);
	}
	default:
		break;
	}

	for (int32 i = 0; i < Assets.Num(); i++)
	{
		switch (OperationType)
		{
		case EAssetOperationType::ModifyMaterial:
		{
			const FString CurString = Assets[i].AssetName.ToString();
			if (CurString.Contains(CheckString, ESearchCase::IgnoreCase))
			{
				UMaterial* MaterialInstancePtr = Cast<UMaterial >(Assets[i].GetAsset());
				if (nullptr == MaterialInstancePtr)
				{
					continue;
				}
				UE_LOG(LogTemp, Log, TEXT(" ChangeDir = %s"), *Assets[i].ObjectPath.ToString());

				// set material member property

				MaterialInstancePtr->TwoSided = true;
				//MaterialInstancePtr->bContactShadows = true;
				//MaterialInstancePtr->DitheredLODTransition = true;

				// set editor param 
				//MaterialInstancePtr->SetScalarParameterValueEditorOnly(TEXT("ChangeParamName"), 0.5);
				//MaterialInstancePtr->SetVectorParameterValueEditorOnly(TEXT("ChangeParamName"), 0.5);
				//MaterialInstancePtr->SetTextureParameterValueEditorOnly(TEXT("ChangeParamName"), UTexture{});

				// save change, if we don't ,when we start game next time we will lost change
				MaterialInstancePtr->PostEditChange();
				TArray<UPackage*> Packages;
				Packages.Add(MaterialInstancePtr->GetOutermost());
				UEditorLoadingAndSavingUtils::SavePackages(Packages, false);
			}
			break;
		}
		case EAssetOperationType::FindCurves:
		{
			UAnimSequenceBase* AnimSequencePtr = Cast<UAnimSequenceBase >(Assets[i].GetAsset());
			if (nullptr == AnimSequencePtr)
			{
				continue;
			}
			TArray<FString> AssetFlies;
			bool IsAdd = false;
			for (auto FloatCurve : AnimSequencePtr->RawCurveData.FloatCurves)
			{
				if (FloatCurve.Name.DisplayName.ToString() == CheckString)
				{
					const FString AssetDataFlie = Assets[i].ObjectPath.ToString();
					AssetFlies.AddUnique(AssetDataFlie);
					IsAdd = true;
					continue;
				}
			}
			if (!IsAdd)
			{
				for (auto VectorCurve : AnimSequencePtr->RawCurveData.VectorCurves)
				{
					if (VectorCurve.Name.DisplayName.ToString() == CheckString)
					{
						const FString AssetDataFlie = Assets[i].ObjectPath.ToString();
						AssetFlies.AddUnique(AssetDataFlie);
						IsAdd = true;
						continue;
					}
				}
			}

			if (!IsAdd)
			{
				for (auto TransformCurves : AnimSequencePtr->RawCurveData.TransformCurves)
				{
					if (TransformCurves.Name.DisplayName.ToString() == CheckString)
					{
						const FString AssetDataFlie = Assets[i].ObjectPath.ToString();
						AssetFlies.AddUnique(AssetDataFlie);
						IsAdd = true;
						continue;
					}
				}
			}

			for (auto FlieString : AssetFlies)
			{
				UE_LOG(LogTemp, Log, TEXT("Inclusion Asset = %s"), *FlieString);
			}
			break;
		}
		case EAssetOperationType::CheckBlueprintError:
		{
			FString filepath(Assets[i].GetExportTextName());
			FStringAssetReference asset_stream_ref(filepath);
			TAssetPtr<UBlueprint> asset(asset_stream_ref);
			UBlueprint* BlueprintObj = asset.LoadSynchronous();
			if (nullptr == BlueprintObj)
			{
				continue;
			}
			TArray<FString> AssetFlies;

			EBlueprintCompileOptions CompileOptions = EBlueprintCompileOptions::None;
			FCompilerResultsLog LogResults;
			LogResults.SetSourcePath(BlueprintObj->GetPathName());
			LogResults.BeginEvent(TEXT("Compile"));
			//LogResults.bLogDetailedResults = GetDefault<UBlueprintEditorSettings>()->bShowDetailedCompileResults;
			//LogResults.EventDisplayThresholdMs = GetDefault<UBlueprintEditorSettings>()->CompileEventDisplayThresholdMs;


			FKismetEditorUtilities::CompileBlueprint(BlueprintObj, CompileOptions, &LogResults);
			if (LogResults.Messages.Num() > 0)
			{
				const FString AssetDataFlie = Assets[i].ObjectPath.ToString();
				UE_LOG(LogTemp, Log, TEXT("Error Blueprint = %s"), *AssetDataFlie);
			}
		}
		default:
			break;
		}
	}
}
