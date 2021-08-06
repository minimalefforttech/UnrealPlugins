// Copyright Epic Games, Inc. All Rights Reserved.
// Adapted from Engine\Plugins\Experimental\CodeEditor\Source\CodeEditor\Private\CPPRichTextSyntaxHighlighterTextLayoutMarshaller.h
#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "ScriptEditorStyle.h"
#include "Framework/Text/SyntaxTokenizer.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"

class FTextLayout;

/**
 * Syntax highlighter for python code
 */
class FPythonSyntaxMarshaller : public FSyntaxHighlighterTextLayoutMarshaller
{
public:
	virtual ~FPythonSyntaxMarshaller();

	struct FSyntaxTextStyle
	{
		FSyntaxTextStyle()
			: NormalTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.Normal"))
			, OperatorTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.Operator"))
			, KeywordTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.Keyword"))
			, StringTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.String"))
			, NumberTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.Number"))
			, CommentTextStyle(FScriptEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("SyntaxHighlight.PY.Comment"))
		{
		}

		FSyntaxTextStyle(const FTextBlockStyle& InNormalTextStyle, const FTextBlockStyle& InOperatorTextStyle, const FTextBlockStyle& InKeywordTextStyle, const FTextBlockStyle& InStringTextStyle, const FTextBlockStyle& InNumberTextStyle, const FTextBlockStyle& InCommentTextStyle)
			: NormalTextStyle(InNormalTextStyle)
			, OperatorTextStyle(InOperatorTextStyle)
			, KeywordTextStyle(InKeywordTextStyle)
			, StringTextStyle(InStringTextStyle)
			, NumberTextStyle(InNumberTextStyle)
			, CommentTextStyle(InCommentTextStyle)
		{
		}

		FTextBlockStyle NormalTextStyle;
		FTextBlockStyle OperatorTextStyle;
		FTextBlockStyle KeywordTextStyle;
		FTextBlockStyle StringTextStyle;
		FTextBlockStyle NumberTextStyle;
		FTextBlockStyle CommentTextStyle;
	};

	static TSharedRef< FPythonSyntaxMarshaller > Create(const FSyntaxTextStyle& InSyntaxTextStyle);

	// Get the font size of NormalTextStyle
	int32 GetFontSize() const;
	// Set the font size for all styles
	void SetFontSize( int32 Size );
	// Return a reference to the plain text style
	const FTextBlockStyle& GetTextStyle() const { return SyntaxTextStyle.NormalTextStyle; }

protected:
	// Parse the sytax for the found token.
	virtual void ParseTokens( const FString& SourceString,
	                          FTextLayout& TargetTextLayout,
							  TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines) override;

	FPythonSyntaxMarshaller(TSharedPtr< FSyntaxTokenizer > InTokenizer, const FSyntaxTextStyle& InSyntaxTextStyle);

	// Text styles container
	FSyntaxTextStyle SyntaxTextStyle;
	// Spaces representing a tab
	FString TabString;
};
