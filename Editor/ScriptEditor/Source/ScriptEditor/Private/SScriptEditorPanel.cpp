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
#include "Fonts/FontMeasure.h"

#define LOCTEXT_NAMESPACE "ScriptEditorPanel"

void SScriptEditorPanel::Construct(const FArguments& InArgs)
{

	HorizontalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(14.0f, 14.0f));

	VerticalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(14.0f, 14.0f));


	EPopupMethod PopupMethod = GIsEditor ? EPopupMethod::CreateNewWindow : EPopupMethod::UseCurrentWindow;

	ChildSlot
    [
		SAssignNew(Overlay, SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FScriptEditorStyle::Get().GetBrush("TextEditor.Background"))
		]

		+ SOverlay::Slot()
		[
			
			SAssignNew( SuggestionBox, SMenuAnchor )
			.Method(PopupMethod)
			.Placement(EMenuPlacement::MenuPlacement_AboveAnchor)
			[
				SNew(SBox)
				.HeightOverride(10)
				.WidthOverride(10)
			]
			.MenuContent
			(
				SNew(SBorder)
				.BorderImage(FScriptEditorStyle::Get().GetBrush("Menu.Background"))
				.Padding( FMargin(2) )
				[
					SNew(SBox)
					.MinDesiredWidth(300)
					.MaxDesiredHeight(250)
					.MaxDesiredWidth(this, &SScriptEditorPanel::GetSelectionListMaxWidth)
					[
						SAssignNew(SuggestionListView, SListView< TSharedPtr<FString>>)
						.ListItemsSource(&Suggestions.SuggestionsList)
						.SelectionMode( ESelectionMode::Single )							// Ideally the mouse over would not highlight while keyboard controls the UI
						.OnGenerateRow(this, &SScriptEditorPanel::MakeSuggestionListItemWidget)
						.OnSelectionChanged(this, &SScriptEditorPanel::SuggestionSelectionChanged)
						.OnKeyDownHandler(this, &SScriptEditorPanel::OnCompleterKeyDown)
						.ItemHeight(18)
					]
				]
			)
		]


		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBorder)
			.BorderImage(FScriptEditorStyle::Get().GetBrush("TextEditor.Border"))
			.Clipping(EWidgetClipping::ClipToBounds)
			[
				SNew(SGridPanel)
				.FillColumn(0, 1.0f)
				.FillRow(0, 1.0f)
				+SGridPanel::Slot(0, 0)
				[
					SAssignNew(PythonEditor, SPythonTextEditor)
					.OnAutoCompleteRequested_Raw(this, &SScriptEditorPanel::ShowAutoCompleter)
					.OnHideAutoCompleteRequested_Raw(this, &SScriptEditorPanel::HideAutoCompleter)
					.OnAcceptCompleter_Raw(this, &SScriptEditorPanel::AcceptAutoCompleter)
					.OnCompleterNavUpRequested_Raw(this, &SScriptEditorPanel::CompleterNavUp)
					.OnCompleterNavDownRequested_Raw(this, &SScriptEditorPanel::CompleterNavDown)
					.OnExecuteTriggered_Raw(this, &SScriptEditorPanel::ExecutePython)
					.OnDocumentationRequested_Raw(this, &SScriptEditorPanel::OpenDocumentation)
					.OnTextChanged_Raw(this, &SScriptEditorPanel::OnTextChanged)
					.HScrollBar(HorizontalScrollbar)
					.VScrollBar(VerticalScrollbar)
				]
				+SGridPanel::Slot(1, 0)
				[
					VerticalScrollbar.ToSharedRef()
				]
				+SGridPanel::Slot(0, 1)
				[
					HorizontalScrollbar.ToSharedRef()
				]
			]

		]
	];

	// Setup python helpers
	ExecutePython(LOCTEXT("SScriptEditorPanel_OpenDocumentation", "def SScriptEditorPanel_OpenDocumentation(obj):\n\
    import inspect, unreal, webbrowser\n\
    if hasattr(obj, '__self__'):\n\
        cls = obj.__self__.__name__\n\
        method = obj.__name__\n\
    else:\n\
        cls = obj.__name__\n\
        method = None\n\
    if cls not in dir(unreal):\n\
        print('{} not from unreal'.format(cls))\n\
        return\n\
    html = 'https://docs.unrealengine.com/5.0/en-US/PythonAPI/class/{0}.html#unreal.{0}'.format(cls)\n\
    if method:\n\
        html += '.{}'.format(method)\n\
    print('Opening: {}'.format(html))\n\
    webbrowser.open(html)").ToString(), false);
}

FReply SScriptEditorPanel::OnCompleterKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if ( InKeyEvent.GetKey() == EKeys::Right || InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::Tab ) {
		if ( SuggestionBox->IsOpen() ) {
			AcceptAutoCompleter();
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void SScriptEditorPanel::ExecutePython( const FString& Text, bool LogCommand ) const
{
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if ( !PythonPlugin || !PythonPlugin->IsPythonAvailable() ) {
    	UE_LOG(LogTemp, Error, TEXT("Python Plugin not loaded!"));
	} else {
		if ( LogCommand ) {
			UE_LOG(LogTemp, Display, TEXT("Code:\n%s"), *Text);

			if ( Text.Contains(TEXT("\n"))) {
				if ( !PythonPlugin->ExecPythonCommand(*Text) ) {
					UE_LOG(LogTemp, Error, TEXT("Python Execution Failed!"));
				}
			} else {
				FPythonCommandEx Ex;
				Ex.ExecutionMode = EPythonCommandExecutionMode::ExecuteStatement;
				Ex.Command = Text;
				if ( !PythonPlugin->ExecPythonCommandEx(Ex) ) {
					UE_LOG(LogTemp, Error, TEXT("Python Execution Failed!"));
				}
			}
		} else {
			if ( !PythonPlugin->ExecPythonCommand(*Text) ) {
				UE_LOG(LogTemp, Error, TEXT("Python Execution Failed!"));
			}
		}
		
	}
}


void SScriptEditorPanel::OpenDocumentation( const FString& Text ) const
{
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if ( !PythonPlugin || !PythonPlugin->IsPythonAvailable() ) {
    	UE_LOG(LogTemp, Error, TEXT("Python Plugin not loaded!"));
	} else {
		ExecutePython(FString::Printf(TEXT("SScriptEditorPanel_OpenDocumentation(%s)"), *Text), false);
		
	}
}
FText SScriptEditorPanel::GetSelectedText() const
{
	return PythonEditor->GetSelectedText();
}
FText SScriptEditorPanel::GetAllText() const
{
	return PythonEditor->GetText();
}
void SScriptEditorPanel::OnTextChanged(const FText& InText)
{
	if ( SuggestionBox->IsOpen() ) {
		FString CurrentLine;
		PythonEditor->GetCurrentTextLine( CurrentLine );
		CurrentLine.MidInline(0, PythonEditor->GetTextLocation().GetOffset() + 1 );
		if ( CurrentLine.EndsWith(" ") ) {
			HideAutoCompleter();
		} else {
			ShowAutoCompleter();
		}
	}
}
void SScriptEditorPanel::HideAutoCompleter()
{
	PythonEditor->SetCompleterIsShown(false);
	SuggestionBox->SetIsOpen( false );
}
void SScriptEditorPanel::AcceptAutoCompleter()
{
	HideAutoCompleter();
	MarkActiveSuggestion();
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), PythonEditor, EFocusCause::SetDirectly);
}
void SScriptEditorPanel::CompleterNavUp()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), SuggestionListView, EFocusCause::SetDirectly);
}
void SScriptEditorPanel::CompleterNavDown()
{
	FSlateApplication::Get().SetUserFocus(FSlateApplication::Get().GetUserIndexForKeyboard(), SuggestionListView, EFocusCause::SetDirectly);
}
void SScriptEditorPanel::ShowAutoCompleter()
{
	if(bIgnoreUIUpdate)
		return;
		
	IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
	if ( !PythonPlugin || !PythonPlugin->IsPythonAvailable() ) {
		HideAutoCompleter();
        return;
	}
	FString ObjectString, PartialString;
	if ( !PythonEditor->GetSuggestionText( &ObjectString, &PartialString ) ) {
		HideAutoCompleter();
		return;
	}
	
	if(!ObjectString.IsEmpty()) {
		TArray<FString> AutoCompleteList;

        FPythonCommandEx Ex;
        Ex.ExecutionMode = EPythonCommandExecutionMode::EvaluateStatement;
        Ex.Command = FString::Printf(TEXT("dir(%s)"), *ObjectString );
 
        if ( PythonPlugin->ExecPythonCommandEx(Ex) ) {
			// Remove non string characters from the result, split it by comma into an array
            Ex.CommandResult.Replace(TEXT("'"), TEXT("")).Replace(TEXT("["), TEXT("")).Replace(TEXT("]"), TEXT("")).ParseIntoArray(AutoCompleteList, TEXT(", ") );
		}
		// User has typed a partial match
		if ( PartialString.Len() )
			AutoCompleteList = AutoCompleteList.FilterByPredicate([&PartialString](const FString& Each) { return Each.Contains( PartialString, ESearchCase::IgnoreCase ); });

		SetSuggestions(AutoCompleteList, FText::FromString(PartialString));
	} else {
		ClearSuggestions();
	}
}


FOptionalSize SScriptEditorPanel::GetSelectionListMaxWidth() const
{
	// Limit the width of the suggestions list to the work area that this widget currently resides on
	const FSlateRect WidgetRect(GetCachedGeometry().GetAbsolutePosition(), GetCachedGeometry().GetAbsolutePosition() + GetCachedGeometry().GetAbsoluteSize());
	const FSlateRect WidgetWorkArea = FSlateApplication::Get().GetWorkArea(WidgetRect);
	return FMath::Max(300.0f, WidgetWorkArea.GetSize().X - 12.0f);
}

TSharedRef<ITableRow> SScriptEditorPanel::MakeSuggestionListItemWidget(TSharedPtr<FString> Text, const TSharedRef<STableViewBase>& OwnerTable)
{
	check(Text.IsValid());

	FString SanitizedText = *Text;
	SanitizedText.ReplaceInline(TEXT("\r\n"), TEXT("\n"), ESearchCase::CaseSensitive);
	SanitizedText.ReplaceInline(TEXT("\r"), TEXT(" "), ESearchCase::CaseSensitive);
	SanitizedText.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);

	return
		SNew(STableRow< TSharedPtr<FString> >, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(SanitizedText))
			.HighlightText(Suggestions.SuggestionsHighlight)
		];
}
void SScriptEditorPanel::SuggestionSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if(bIgnoreUIUpdate)
		return;

	Suggestions.SelectedSuggestion = Suggestions.SuggestionsList.IndexOfByPredicate([&NewValue](const TSharedPtr<FString>& InSuggestion) { return InSuggestion == NewValue; });

	MarkActiveSuggestion();

	// If the user selected this suggestion by clicking on it, then go ahead and close the suggestion
	// box as they've chosen the suggestion they're interested in.
	if( SelectInfo == ESelectInfo::OnMouseClick ) {
		HideAutoCompleter();
	}
}
void SScriptEditorPanel::SetSuggestions(TArray<FString>& Elements, FText Highlight)
{
	FString SelectionText;
	if (Suggestions.HasSelectedSuggestion())
		SelectionText = *Suggestions.GetSelectedSuggestion();

	Suggestions.Reset();
	Suggestions.SuggestionsHighlight = Highlight;

	for(int32 i = 0; i < Elements.Num(); ++i) {
		Suggestions.SuggestionsList.Add(MakeShared<FString>(Elements[i]));

		if (Elements[i] == SelectionText)
			Suggestions.SelectedSuggestion = i;
	}
	SuggestionListView->RequestListRefresh();

	if(Suggestions.HasSuggestions()) {
		// Ideally if the selection box is open the output window is not changing it's window title (flickers)
		TPanelChildren<SOverlay::FOverlaySlot>& OverlaySlots = *(TPanelChildren<SOverlay::FOverlaySlot>*)Overlay->GetChildren();
		FTextLocation Location = PythonEditor->GetTextLocation();
		
		TArray<FString> Lines;
		PythonEditor->GetText().ToString().ParseIntoArrayLines( Lines, false );
		Lines.SetNum( Location.GetLineIndex() + 1, true );
		Lines[Lines.Num() - 1].MidInline(0, Location.GetOffset()+1);
		FString SubText = FString::Join(Lines, TEXT("\n"));

		FVector2D EditorSize = PythonEditor->GetDesiredSize();
		float HScroll = (HorizontalScrollbar->IsNeeded() ? HorizontalScrollbar->DistanceFromTop() : 0.0 ) * EditorSize.X;
		float VScroll = (VerticalScrollbar->IsNeeded() ? VerticalScrollbar->DistanceFromTop() : 0.0 ) * EditorSize.Y;
		const TSharedRef< FSlateFontMeasure > FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		auto FontInfo = PythonEditor->GetTextStyle().Font;
		float TextBlockHeight = FontMeasure->Measure( SubText, FontInfo, 1.0).Y;
		float TextLineWidth = FontMeasure->Measure( Lines[Lines.Num() - 1], FontInfo, 1.0).X;
		OverlaySlots[1].Padding( TextLineWidth - HScroll, (((TextBlockHeight / Lines.Num()) * (Lines.Num() - 1) )) - VScroll);
		PythonEditor->SetCompleterIsShown(true);
		SuggestionBox->SetIsOpen(true, false);
		if (Suggestions.HasSelectedSuggestion())
			SuggestionListView->RequestScrollIntoView(Suggestions.GetSelectedSuggestion());
		else
			SuggestionListView->ScrollToTop();
	} else {
		HideAutoCompleter();
	}
}
void SScriptEditorPanel::MarkActiveSuggestion()
{
	bIgnoreUIUpdate = true;
	if (Suggestions.HasSelectedSuggestion()) {
		TSharedPtr<FString> SelectedSuggestion = Suggestions.GetSelectedSuggestion();

		SuggestionListView->SetSelection(SelectedSuggestion);
		SuggestionListView->RequestScrollIntoView(SelectedSuggestion);	// Ideally this would only scroll if outside of the view
		PythonEditor->ApplySuggestion(*SelectedSuggestion);
	} else {
		SuggestionListView->ClearSelection();
	}
	bIgnoreUIUpdate = false;
}
void SScriptEditorPanel::ClearSuggestions()
{
	HideAutoCompleter();
	Suggestions.Reset();
}

#undef LOCTEXT_NAMESPACE