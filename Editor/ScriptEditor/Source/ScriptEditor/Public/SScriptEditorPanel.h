#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Framework/Commands/UICommandList.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Layout/Margin.h"

class SPythonTextEditor;

/*
 * Script Editor Interface
 */
class SScriptEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SScriptEditorPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

    void ExecutePython( const FString& Text, bool LogCommand ) const;
    void OpenDocumentation( const FString& Text ) const;
	FText GetSelectedText() const;
	FText GetAllText() const;

	
private:
	// Callbacks
    void ShowAutoCompleter();
    void HideAutoCompleter();
	void AcceptAutoCompleter();
	void CompleterNavUp();
	void CompleterNavDown();
	void OnTextChanged(const FText& InText);

	FOptionalSize GetSelectionListMaxWidth() const;
	TSharedRef<ITableRow> MakeSuggestionListItemWidget(TSharedPtr<FString> Message, const TSharedRef<STableViewBase>& OwnerTable);
	void SuggestionSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void SetSuggestions(TArray<FString>& Elements, FText Highlight);
	void MarkActiveSuggestion();
	void ClearSuggestions();
	FReply OnCompleterKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	struct FSuggestions
	{
		FSuggestions()
			: SelectedSuggestion(INDEX_NONE) {}

		void Reset() {
			SelectedSuggestion = INDEX_NONE;
			SuggestionsList.Reset();
			SuggestionsHighlight = FText::GetEmpty();
		}

		bool HasSuggestions() const {
			return SuggestionsList.Num() > 0;
		}

		bool HasSelectedSuggestion() const {
			return SuggestionsList.IsValidIndex(SelectedSuggestion);
		}

		void StepSelectedSuggestion(const int32 Step) {
			SelectedSuggestion += Step;
			if (SelectedSuggestion < 0)
			{
				SelectedSuggestion = SuggestionsList.Num() - 1;
			}
			else if (SelectedSuggestion >= SuggestionsList.Num())
			{
				SelectedSuggestion = 0;
			}
		}

		TSharedPtr<FString> GetSelectedSuggestion() const {
			return SuggestionsList.IsValidIndex(SelectedSuggestion) ? SuggestionsList[SelectedSuggestion] : nullptr;
		}

		/** INDEX_NONE if not set, otherwise index into SuggestionsList */
		int32 SelectedSuggestion;

		/** All log messages stored in this widget for the list view */
		TArray<TSharedPtr<FString>> SuggestionsList;

		/** Highlight text to use for the suggestions list */
		FText SuggestionsHighlight;
	};

	TSharedPtr< SMenuAnchor > SuggestionBox;
	TSharedPtr< SOverlay > Overlay;
	TSharedPtr< SListView< TSharedPtr<FString> > > SuggestionListView;
	FSuggestions Suggestions;

	TSharedPtr <SPythonTextEditor> PythonEditor;
	TSharedPtr<SScrollBar> HorizontalScrollbar;
	TSharedPtr<SScrollBar> VerticalScrollbar;
	bool bIgnoreUIUpdate;
};