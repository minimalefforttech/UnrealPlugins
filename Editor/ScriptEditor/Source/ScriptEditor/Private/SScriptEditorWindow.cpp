#include "SScriptEditorWindow.h"
#include "SScriptEditorPanel.h"
#include "SPythonTextEditor.h"
#include "ScriptEditorStyle.h"
#include "ScriptEditorCommands.h"
#include "SlateOptMacros.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "ToolMenus.h"
#include "PythonScriptTypes.h"
#include "IPythonScriptPlugin.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"
#include "Widgets/Text/SlateEditableTextLayout.h"
#include "Toolkits/AssetEditorToolkit.h"

#define LOCTEXT_NAMESPACE "ScriptEditorWindow"

static const FName EditorTabID("PythonEditorPanel");

void SScriptEditorWindow::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	ToolBarCommands = MakeShareable(new FUICommandList);
	const FScriptEditorCommands& ScriptEditorCommands = FScriptEditorCommands::Get();

	ToolBarCommands->MapAction(
		ScriptEditorCommands.Execute,
		FExecuteAction::CreateSP(this, &SScriptEditorWindow::ExecuteCB));

	FToolBarBuilder ToolbarBuilder(ToolBarCommands, FMultiBoxCustomization("ScriptEditorToolbar"), TSharedPtr<FExtender>(), false);
	ToolbarBuilder.SetLabelVisibility(EVisibility::Collapsed);

    ToolbarBuilder.AddToolBarButton(ScriptEditorCommands.Execute);

	TSharedRef<SWidget> Toolbar = ToolbarBuilder.MakeWidget();

	ChildSlot
    [
        SNew(SBox)
        .VAlign(VAlign_Fill)
        .HAlign(HAlign_Fill)
        [
			SNew(SVerticalBox)
			//Toolbar
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FScriptEditorStyle::Get().GetBrush("TextEditor.Background"))
				[
					Toolbar
				]
			]
			// Text Editor
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.FillHeight(1.f)
			[
				SAssignNew(ScriptEditor, SScriptEditorPanel)
            ]
		]
    ];
}

void SScriptEditorWindow::ExecuteCB()
{
    ScriptEditor->ExecutePython( ScriptEditor->GetAllText().ToString(), true );
}