#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/SCompoundWidget.h"

class SScriptEditorWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SScriptEditorWindow) {}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs,
                    const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                    const TSharedPtr<SWindow>& ConstructUnderWindow );
    
private:
    void ExecuteCB();
	TSharedPtr<class FUICommandList> ToolBarCommands;
	TSharedPtr<class SScriptEditorPanel> ScriptEditor;
};
