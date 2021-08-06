#include "SPythonTextEditor.h"
#include "ScriptEditorStyle.h"
#include "ScriptEditorCommands.h"
#include "PythonSyntaxMarshaller.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Text/SlateEditableTextLayout.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/GenericCommands.h"


#define LOCTEXT_NAMESPACE "SPythonTextEditor"

void SPythonTextEditor::Construct(const FArguments& Args)
{
	bCompleterIsShown = false;
	OnExecuteTriggered = Args._OnExecuteTriggered;
	OnAutoCompleteRequested = Args._OnAutoCompleteRequested;
	OnHideAutoCompleteRequested = Args._OnHideAutoCompleteRequested;
	OnAcceptCompleter = Args._OnAcceptCompleter;
	OnCompleterNavUpRequested = Args._OnCompleterNavUpRequested;
	OnCompleterNavDownRequested = Args._OnCompleterNavDownRequested;
	OnDocumentationRequested = Args._OnDocumentationRequested;

	RichTextMarshaller = FPythonSyntaxMarshaller::Create(
			FPythonSyntaxMarshaller::FSyntaxTextStyle()
			);
	Super::Construct(
		Super::FArguments()
		.Font(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("TextEditor.NormalText").Font)
		.TextStyle(&FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("TextEditor.NormalText"))
		.Marshaller(RichTextMarshaller.ToSharedRef())
		.Text(Args._Text)
		.AutoWrapText(false)
		.Margin(0.0f)
		.HScrollBar(Args._HScrollBar)
		.VScrollBar(Args._VScrollBar)
		.OnTextChanged(Args._OnTextChanged)
	);

	UICommandList = MakeShareable(new FUICommandList());

	UICommandList->MapAction(FScriptEditorCommands::Get().Help,
		FExecuteAction::CreateRaw(this, &SPythonTextEditor::PrintHelp)
		);
	UICommandList->MapAction(FScriptEditorCommands::Get().OpenDocumentation,
		FExecuteAction::CreateRaw(this, &SPythonTextEditor::OpenDocumentation)
		);
}
FTextLocation SPythonTextEditor::GetTextLocation() const
{
	return CaretLocation;
}
void SPythonTextEditor::PrintHelp() const
{
	if (OnExecuteTriggered.IsBound()) {
		if ( AnyTextSelected() )
			OnExecuteTriggered.Execute(FText::Format(LOCTEXT("HelpCmd", "help({0})"), {GetSelectedText()}).ToString(), false);
		else
			OnExecuteTriggered.Execute(LOCTEXT("HelpCmd", "print('No Text Selected')").ToString(), false);
	}
}
void SPythonTextEditor::OpenDocumentation() const
{
	if (OnDocumentationRequested.IsBound()) {
		if ( AnyTextSelected() ) {
			OnDocumentationRequested.Execute(GetSelectedText().ToString());
		}
	}
}
FReply SPythonTextEditor::OnKeyChar( const FGeometry& MyGeometry,const FCharacterEvent& InCharacterEvent )
{
	// Check for special characters
	const TCHAR Character = InCharacterEvent.GetCharacter();
	switch (Character)
	{
	case TCHAR(10):		// Enter
	case TCHAR(13):		// Ctrl+Enter
	case TCHAR(8):		// Ctrl+H
	case TCHAR(104):	// H
	case TCHAR(112):	// P
	case TCHAR(32):		// Space
		if ( InCharacterEvent.GetModifierKeys().IsControlDown() )
			return FReply::Handled();
		break;
	default:
		break;
	}
	// UE_LOG(LogTemp, Display, TEXT("KEY: %d"), Character );
	return Super::OnKeyChar(MyGeometry, InCharacterEvent);
}

FReply SPythonTextEditor::OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent )
{
	FReply Reply = FReply::Unhandled();

	if ( InKeyEvent.GetKey() == EKeys::Enter && InKeyEvent.IsControlDown() ) {
		if (OnExecuteTriggered.IsBound()) {
			if ( AnyTextSelected() )
				OnExecuteTriggered.Execute(GetSelectedText().ToString(), true);
			else
				OnExecuteTriggered.Execute(GetText().ToString(), true);
		}
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::Enter && InKeyEvent.IsAltDown() ) {
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::H && InKeyEvent.IsControlDown() ) {
		PrintHelp();
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::P && InKeyEvent.IsControlDown() ) {
		if (OnExecuteTriggered.IsBound()) {
			if ( AnyTextSelected() )
				OnExecuteTriggered.Execute(FText::Format(LOCTEXT("PPrintCmd", "import pprint;pprint.pprint({0})"), {GetSelectedText()}).ToString(), false);
		}
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::Tab ) {
		if ( !GetText().IsEmpty() ) {
			FTextLocation StoredLocation = CaretLocation;
			FString CurrentLine;
			GetCurrentTextLine( CurrentLine );
			if ( AnyTextSelected() ) {
				FString String = GetSelectedText().ToString();
				TArray<FString> SelectedLines;
				uint32 NumLines = String.ParseIntoArrayLines( SelectedLines, false );

				if ( SelectedLines.Num() == 1 ) {
					// Only one line
					GoTo(FTextLocation(StoredLocation.GetLineIndex(), 0));
					if ( InKeyEvent.IsShiftDown() ) {
						TArray<FString> AllLines;
						GetText().ToString().ParseIntoArrayLines( AllLines, false );
						FString Line = AllLines[StoredLocation.GetLineIndex()];
						int32 Removed = UnIndentLine( StoredLocation.GetLineIndex(), Line );
						GoTo(FTextLocation(StoredLocation.GetLineIndex(), StoredLocation.GetOffset() - Removed));

					} else {
						IndentLine( StoredLocation.GetLineIndex() );
						GoTo(FTextLocation(StoredLocation.GetLineIndex(), StoredLocation.GetOffset() + 4));
					}
					// Restore vertical scroll?
				} else {
					TArray<FString> AllLines;
					GetText().ToString().ParseIntoArrayLines( AllLines, false );
					uint32 Start;
					uint32 End;
					if ( StoredLocation.GetLineIndex() - SelectedLines.Num() < 0 ) {
						// Selecting Down
						Start = StoredLocation.GetLineIndex();
						End = Start + SelectedLines.Num() - 1;
					} else {
						// Selecting Up
						End = StoredLocation.GetLineIndex();
						Start = End - SelectedLines.Num() + 1;
					}
					// Double check selection order
					for ( uint32 i=Start;i<=End;++i ) {
						if ( i > (uint32)AllLines.Num() ) {
						} else if (AllLines[i] != SelectedLines[i-Start]) {
							auto& Line = SelectedLines[i-Start];
							if ( (i-Start) == 0 && (Line.Len() == 0 || AllLines[i].EndsWith(Line)) )
								continue;
							if ( i-Start == SelectedLines.Num() && (Line.Len() == 0 || AllLines[i].StartsWith(Line)) )
								continue;
							uint32 Temp = Start;
							Start = End;
							End = Temp;
							break;
						}
					}
					int32 Change = 4;
					if ( InKeyEvent.IsShiftDown() ) {
						for ( uint32 i=Start;i<=End;++i ) {
							int32 LineChange = UnIndentLine( i, AllLines[i] );
							if ( i == StoredLocation.GetLineIndex() )
								Change = LineChange;
						}
					} else {
						for ( uint32 i=Start;i<=End;++i )
							IndentLine( i );
					}
					GoTo(FTextLocation(StoredLocation.GetLineIndex(), StoredLocation.GetOffset() + Change));
				}
			} else {
				// Only one line
				GoTo(FTextLocation(StoredLocation.GetLineIndex(), 0));
				if ( InKeyEvent.IsShiftDown() ) {
					TArray<FString> AllLines;
					GetText().ToString().ParseIntoArrayLines( AllLines, false );
					FString Line = AllLines[StoredLocation.GetLineIndex()];
					int32 Removed = UnIndentLine( StoredLocation.GetLineIndex(), Line );
					GoTo(FTextLocation(StoredLocation.GetLineIndex(), StoredLocation.GetOffset() - Removed));

				} else {
					IndentLine( StoredLocation.GetLineIndex() );
					GoTo(FTextLocation(StoredLocation.GetLineIndex(), StoredLocation.GetOffset() + 4));
				}
			}
			// Currently unreal doesn't expose code to set selection :/
			ClearSelection();
		}
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::Enter ) {
		if ( bCompleterIsShown && OnAcceptCompleter.IsBound() ) {
			OnAcceptCompleter.Execute();
		} else {
			FString CurrentLine;
			GetCurrentTextLine( CurrentLine );
			FString Whitespace = GetPreceedingWhitespace( CurrentLine );
			if ( CurrentLine.EndsWith(":") )
				Whitespace.InsertAt(0, "    ");
			Whitespace.InsertAt( 0, "\n" );
			InsertTextAtCursor( Whitespace );
		}
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::SpaceBar && InKeyEvent.IsControlDown() ) {
		if ( !AnyTextSelected() && OnAutoCompleteRequested.IsBound() ) {
			FString CurrentLine;
			GetCurrentTextLine( CurrentLine );
			CurrentLine.MidInline(0, CaretLocation.GetOffset()+1 );
			CurrentLine.TrimStartAndEndInline();
			OnAutoCompleteRequested.Execute();

		}
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::Down && bCompleterIsShown && OnCompleterNavDownRequested.IsBound() ) {
		OnCompleterNavDownRequested.Execute();
		Reply = FReply::Handled();
	} else if ( InKeyEvent.GetKey() == EKeys::Up && bCompleterIsShown && OnCompleterNavUpRequested.IsBound() ) {
		OnCompleterNavUpRequested.Execute();
		Reply = FReply::Handled();

	} else if ( InKeyEvent.GetKey() == EKeys::Escape ) {
		if ( !AnyTextSelected() && OnHideAutoCompleteRequested.IsBound() ) {
			OnHideAutoCompleteRequested.Execute();

		}
		Reply = FReply::Handled();

	} else if ( InKeyEvent.GetKey() == EKeys::Add && InKeyEvent.IsControlDown() ) {
		RichTextMarshaller->SetFontSize( RichTextMarshaller->GetFontSize() * 1.2 );
		EditableTextLayout->SetTextStyle( RichTextMarshaller->GetTextStyle() );
	} else if ( InKeyEvent.GetKey() == EKeys::Subtract && InKeyEvent.IsControlDown() ) {
		RichTextMarshaller->SetFontSize( RichTextMarshaller->GetFontSize() / 1.2 );
		EditableTextLayout->SetTextStyle( RichTextMarshaller->GetTextStyle() );
	} else {
		Reply = Super::OnKeyDown( MyGeometry, InKeyEvent );
	}
	return Reply;
}

FReply SPythonTextEditor::OnMouseWheel( const FGeometry& MyGeometry,const FPointerEvent& InMouseEvent )
{
	FReply Reply = FReply::Unhandled();
	if ( InMouseEvent.IsControlDown() && InMouseEvent.GetWheelDelta() > 0 ) {
		RichTextMarshaller->SetFontSize( RichTextMarshaller->GetFontSize() * 1.2 );
		EditableTextLayout->SetTextStyle( RichTextMarshaller->GetTextStyle() );

		Reply = FReply::Handled();
	}
	else if ( InMouseEvent.IsControlDown() && InMouseEvent.GetWheelDelta() < 0 ) {
		RichTextMarshaller->SetFontSize( RichTextMarshaller->GetFontSize() / 1.2 );
		EditableTextLayout->SetTextStyle( RichTextMarshaller->GetTextStyle() );
	}
	else {
		Reply = Super::OnMouseWheel( MyGeometry, InMouseEvent );
	}
	return Reply;
}

void SPythonTextEditor::IndentLine( uint32 LineNumber ) {
	GoTo(FTextLocation(LineNumber, 0));
	InsertTextAtCursor("    ");
}

int32 SPythonTextEditor::UnIndentLine( uint32 LineNumber, const FString& Line ) {
	GoTo(FTextLocation(LineNumber, 0));
	FString Whitespace = GetPreceedingWhitespace( Line );
	int32 ToRemove = FGenericPlatformMath::Min( Whitespace.Len(), int32(4) );
	for ( int i=0;i<ToRemove;++i ) {
		EditableTextLayout->HandleDelete();
	}
	return ToRemove;
}
FString SPythonTextEditor::GetPreceedingWhitespace( const FString& Line ) const
{
	const FRegexPattern Pattern(TEXT("^([ ]+).*$"));
	FRegexMatcher Matcher(Pattern, Line);
	if ( Matcher.FindNext() )
		return Matcher.GetCaptureGroup( 1 );
	return FString();
}

void SPythonTextEditor::OnCursorMoved(const FTextLocation& InLocation)
{
	CaretLocation = InLocation;
}
TSharedPtr<SWidget> SPythonTextEditor::BuildContextMenuContent() const
{
	// Set the menu to automatically close when the user commits to a choice
	const bool bShouldCloseWindowAfterMenuSelection = true;
	
	// This is a context menu which could be summoned from within another menu if this text block is in a menu
	// it should not close the menu it is inside
	bool bCloseSelfOnly = true;

	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, UICommandList, MenuExtender, bCloseSelfOnly, &FCoreStyle::Get());
	{
		MenuBuilder.BeginSection("Help");
		{
			MenuBuilder.AddMenuEntry(FScriptEditorCommands::Get().Help);
			MenuBuilder.AddMenuEntry(FScriptEditorCommands::Get().OpenDocumentation);
		}
		MenuBuilder.EndSection();
		MenuBuilder.BeginSection("EditText", LOCTEXT("Heading", "Modify Text"));
		{
			// Undo
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Undo);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("EditableTextModify2");
		{
			// Cut
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Cut);

			// Copy
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);

			// Paste
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);

			// Delete
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("EditableTextModify3");
		{
			// Select All
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().SelectAll);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
	return EditableTextLayout->BuildDefaultContextMenu(MenuExtender);
}

bool SPythonTextEditor::GetSuggestionText( FString* OutObjectString, FString* OutPartialString )
{
	if ( !OutObjectString || !OutPartialString )
		return false;
	FString CurrentLine;
	GetCurrentTextLine( CurrentLine );
	CurrentLine.MidInline(0, CaretLocation.GetOffset()+1 );
	CurrentLine.TrimStartAndEndInline();

	const FRegexPattern Pattern(TEXT("^.*?([a-zA-Z_\\.]+)$"));
	FRegexMatcher Matcher(Pattern, CurrentLine);
	if ( !Matcher.FindNext() )
		return false;
	FString Result = Matcher.GetCaptureGroup( 1 );
	if ( Result.StartsWith(".") )
		return false;

	OutObjectString->Reset();
	OutPartialString->Reset();

	if ( Result.EndsWith(".") ) {
		OutObjectString->Append( Result.Mid(0, Result.Len() - 1) );
	} else {
		int32 DotPos = Result.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd, CurrentLine.Len());
		if ( DotPos > 0 ) {
			OutObjectString->Append( Result.Mid(0, DotPos) );
			OutPartialString->Append( Result.Mid(DotPos+1) );
		} else {
			OutObjectString->Append( "globals()" );
			OutPartialString->Append( Result );
		}
	}
	return true;
}
void SPythonTextEditor::ApplySuggestion(const FString& Suggestion ) 
{
	FString ObjectString, PartialString;
	if ( GetSuggestionText( &ObjectString, &PartialString ) ) {
		if ( PartialString.Len() ) {
			for ( int i=0;i<PartialString.Len();++i )
				EditableTextLayout->HandleBackspace();
		}
		InsertTextAtCursor(Suggestion);
	}
}
const FTextBlockStyle& SPythonTextEditor::GetTextStyle() const
{
	return EditableTextLayout->GetTextStyle();
}



#undef LOCTEXT_NAMESPACE