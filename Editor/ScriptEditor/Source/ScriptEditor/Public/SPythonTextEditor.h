#pragma once

#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Text/SMultiLineEditableText.h"
class ITextLayoutMarshaller;

class SPythonTextEditor : public SMultiLineEditableText
{
public:
	DECLARE_DELEGATE_TwoParams(FOnExecuteTriggered, const FString& /*Text*/, bool /*LogOutput*/);
	DECLARE_DELEGATE_OneParam(FOnDocumentationRequested, const FString& /*Text*/);

	SLATE_BEGIN_ARGS(SPythonTextEditor) {}
		/** The initial text that will appear in the widget. */
		SLATE_ATTRIBUTE(FText, Text)

		/** The marshaller used to get/set the raw text to/from the text layout. */
		SLATE_ARGUMENT(TSharedPtr< ITextLayoutMarshaller >, Marshaller)

		/** The horizontal scroll bar widget */
		SLATE_ARGUMENT(TSharedPtr< SScrollBar >, HScrollBar)

		/** The vertical scroll bar widget */
		SLATE_ARGUMENT(TSharedPtr< SScrollBar >, VScrollBar)

		/** Called whenever the text is changed interactively by the user */
		SLATE_EVENT(FOnTextChanged, OnTextChanged)

		/** Emitted when Ctrl+Space is pressed */
		SLATE_EVENT(FSimpleDelegate, OnAutoCompleteRequested)

		/** Emitted when Enter or Right is pressed with completer shown */
		SLATE_EVENT(FSimpleDelegate, OnHideAutoCompleteRequested)
		
		/** Emitted when Enter is pressed with completer shown */
		SLATE_EVENT(FSimpleDelegate, OnAcceptCompleter)

		/** Emitted when Up arrow is pressed with completer shown */
		SLATE_EVENT(FSimpleDelegate, OnCompleterNavUpRequested)

		/** Emitted when Down arrow is pressed with completer shown */
		SLATE_EVENT(FSimpleDelegate, OnCompleterNavDownRequested)

		/** Emitted when Ctrl+Enter is pressed */
		SLATE_EVENT(FOnExecuteTriggered, OnExecuteTriggered)

		/** Emitted when Documentation menu item is pressed */
		SLATE_EVENT(FOnDocumentationRequested, OnDocumentationRequested)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	FTextLocation GetTextLocation() const;
	bool GetSuggestionText( FString* OutObjectString, FString* OutPartialString );
	void ApplySuggestion( const FString& Suggestion );
	const FTextBlockStyle& GetTextStyle() const;

	void SetCompleterIsShown( bool bIsShown ) { bCompleterIsShown = bIsShown; }
	void PrintHelp() const;
	void OpenDocumentation() const;

protected:
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual FReply OnKeyChar( const FGeometry& MyGeometry,const FCharacterEvent& InCharacterEvent ) override;
	virtual FReply OnMouseWheel( const FGeometry& MyGeometry,const FPointerEvent& InMouseEvent ) override;
	virtual void OnCursorMoved(const FTextLocation& InLocation) override;
	virtual TSharedPtr<SWidget> BuildContextMenuContent() const override;

private:
	FString GetPreceedingWhitespace( const FString& Line ) const;
	void IndentLine( uint32 LineNumber );
	int32 UnIndentLine( uint32 LineNumber, const FString& Line );

	TSharedPtr<FUICommandList> UICommandList;
	TSharedPtr<class FPythonSyntaxMarshaller> RichTextMarshaller;
	typedef SMultiLineEditableText Super;
	FOnExecuteTriggered OnExecuteTriggered;
	FOnDocumentationRequested OnDocumentationRequested;
	FSimpleDelegate OnAutoCompleteRequested;
	FSimpleDelegate OnHideAutoCompleteRequested;
	FSimpleDelegate OnAcceptCompleter;
	FSimpleDelegate OnCompleterNavUpRequested;
	FSimpleDelegate OnCompleterNavDownRequested;
	FTextLocation CaretLocation;
	bool bCompleterIsShown;

};