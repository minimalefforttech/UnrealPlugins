// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ScriptEditorStyle.h"

class FScriptEditorCommands : public TCommands<FScriptEditorCommands>
{
public:

	FScriptEditorCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	// open the window
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
	// execute code
	TSharedPtr< FUICommandInfo > Execute;
	// print help string
	TSharedPtr< FUICommandInfo > Help;
	// open documentation in browser
	TSharedPtr< FUICommandInfo > OpenDocumentation;
};