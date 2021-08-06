// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScriptEditorModule.h"
#include "SScriptEditorWindow.h"
#include "ScriptEditorStyle.h"
#include "ScriptEditorCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"

static const FName ScriptEditorTabName("ScriptEditor");

#define LOCTEXT_NAMESPACE "FScriptEditorModule"

void FScriptEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FScriptEditorStyle::Initialize();
	FScriptEditorStyle::ReloadTextures();

	FScriptEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FScriptEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FScriptEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FScriptEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterTabSpawner(ScriptEditorTabName, FOnSpawnTab::CreateRaw(this, &FScriptEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FScriptEditorTabTitle", "ScriptEditor"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);
}


void FScriptEditorModule::ShutdownModule()
{

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FScriptEditorStyle::Shutdown();

	FScriptEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ScriptEditorTabName);
}

TSharedRef<SDockTab> FScriptEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	TSharedPtr<SWidget> TabContent = SNew(SBox)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SScriptEditorWindow, MajorTab, SpawnTabArgs.GetOwnerWindow())
		];

	MajorTab->SetContent(TabContent.ToSharedRef());
	return MajorTab;
}


void FScriptEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ScriptEditorTabName);
}

void FScriptEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FScriptEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FScriptEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScriptEditorModule, ScriptEditor)