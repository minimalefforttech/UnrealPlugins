// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScriptEditorCommands.h"

#define LOCTEXT_NAMESPACE "FScriptEditorModule"

FScriptEditorCommands::FScriptEditorCommands()
	: TCommands<FScriptEditorCommands>( /*InContextName=*/    TEXT("ScriptEditor"),
	                                    /*InContextDesc=*/    NSLOCTEXT("Contexts", "ScriptEditor", "ScriptEditor Plugin"),
	                                    /*InContextParent=**/ NAME_None,
										/*InStyleSetName=*/   FScriptEditorStyle::GetStyleSetName())
{
}
void FScriptEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ScriptEditor", "Bring up ScriptEditor window", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(Execute, "Execute", "Execute the code", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(Help, "Help", "Print Docstring", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(OpenDocumentation, "OpenDocumentation", "Open Documentation in Browser", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
