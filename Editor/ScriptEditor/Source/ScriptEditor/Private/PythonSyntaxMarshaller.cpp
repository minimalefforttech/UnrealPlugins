// Copyright Epic Games, Inc. All Rights Reserved.

#include "PythonSyntaxMarshaller.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Framework/Text/ISlateRun.h"
#include "Framework/Text/SlateTextRun.h"
#include "Misc/ExpressionParserTypes.h"
#include "WhiteSpaceTextRun.h"
#include "Internationalization/Regex.h"

const TCHAR* Keywords[] =
{
	TEXT("False"),
	TEXT("True"),
	TEXT("None"),
	TEXT("and"),
	TEXT("or"),
	TEXT("from"),
	TEXT("import"),
	TEXT("as"),
	TEXT("in"),
	TEXT("with"),
	TEXT("await"),
	TEXT("break"),
	TEXT("class"),
	TEXT("def"),
	TEXT("continue"),
	TEXT("assert"),
	TEXT("async"),
	TEXT("del"),
	TEXT("if"),
	TEXT("not"),
	TEXT("else"),
	TEXT("elif"),
	TEXT("try"),
	TEXT("except"),
	TEXT("finally"),
	TEXT("for"),
	TEXT("while"),
	TEXT("global"),
	TEXT("with"),
	TEXT("yield"),
	TEXT("pass"),
	TEXT("return"),
};

const TCHAR* Operators[] =
{
	TEXT("#"),
	TEXT("\""),
	TEXT("\'"),
	TEXT("+="),
	TEXT("+"),
	TEXT("-="),
	TEXT("-"),
	TEXT("("),
	TEXT(")"),
	TEXT("["),
	TEXT("]"),
	TEXT("."),
	TEXT("!="),
	TEXT("!"),
	TEXT("&="),
	TEXT("~"),
	TEXT("&"),
	TEXT("*="),
	TEXT("*"),
	TEXT("/="),
	TEXT("/"),
	TEXT("%="),
	TEXT("%"),
	TEXT("<<"),
	TEXT("<="),
	TEXT("<"),
	TEXT(">>"),
	TEXT(">="),
	TEXT(">"),
	TEXT("=="),
	TEXT("&"),
	TEXT("^="),
	TEXT("^"),
	TEXT("|="),
	TEXT("|"),
	TEXT("="),
	TEXT(","),
	TEXT("{"),
	TEXT("}"),
	TEXT(";"),
};

const TCHAR* Numbers[] =
{
	TEXT("1"),
	TEXT("2"),
	TEXT("3"),
	TEXT("4"),
	TEXT("5"),
	TEXT("6"),
	TEXT("7"),
	TEXT("8"),
	TEXT("9"),
	TEXT("0"),
	TEXT("."),
};

TSharedRef< FPythonSyntaxMarshaller > FPythonSyntaxMarshaller::Create(const FSyntaxTextStyle& InSyntaxTextStyle)
{
	TArray<FSyntaxTokenizer::FRule> TokenizerRules;

	// operators
	for(const auto& Operator : Operators)
		TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Operator));

	// keywords
	for(const auto& Keyword : Keywords)
		TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Keyword));

	// numbers
	for(const auto& Number : Numbers)
		TokenizerRules.Emplace(FSyntaxTokenizer::FRule(Number));

	return MakeShareable(new FPythonSyntaxMarshaller(FSyntaxTokenizer::Create(TokenizerRules), InSyntaxTextStyle));
}

FPythonSyntaxMarshaller::~FPythonSyntaxMarshaller()
{

}

void FPythonSyntaxMarshaller::ParseTokens(const FString& SourceString, FTextLayout& TargetTextLayout, TArray<FSyntaxTokenizer::FTokenizedLine> TokenizedLines)
{
	enum class EParseState : uint8
	{
		None,
		LookingForDoubleQuoteString,
		LookingForSingleQuoteString,
		LookingForSingleLineComment,
		LookingForMultiLineComment,
	};
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(TokenizedLines.Num());

	// Parse the tokens, generating the styled runs for each line
	EParseState ParseState = EParseState::None;
	for(const FSyntaxTokenizer::FTokenizedLine& TokenizedLine : TokenizedLines) {
		TSharedRef<FString> ModelString = MakeShareable(new FString());
		TArray< TSharedRef< IRun > > Runs;

		if(ParseState == EParseState::LookingForSingleLineComment)
			ParseState = EParseState::None;

		for(const FSyntaxTokenizer::FToken& Token : TokenizedLine.Tokens) {
			const FString TokenText = SourceString.Mid(Token.Range.BeginIndex, Token.Range.Len());
			const FTextRange ModelRange(ModelString->Len(), ModelString->Len() + TokenText.Len());
			ModelString->Append(TokenText);

			FRunInfo RunInfo(TEXT("SyntaxHighlight.PY.Normal"));
			FTextBlockStyle TextBlockStyle = SyntaxTextStyle.NormalTextStyle;

			const bool bIsWhitespace = FString(TokenText).TrimEnd().IsEmpty();
			if(!bIsWhitespace) {
				bool bHasMatchedSyntax = false;
				if(Token.Type == FSyntaxTokenizer::ETokenType::Syntax) {
					if(ParseState == EParseState::None && TokenText == TEXT("\"")) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.String");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
						ParseState = EParseState::LookingForDoubleQuoteString;
						bHasMatchedSyntax = true;
					} else if(ParseState == EParseState::LookingForDoubleQuoteString && TokenText == TEXT("\"")) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Normal");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
						ParseState = EParseState::None;
					} else if(ParseState == EParseState::None && TokenText == TEXT("\'")) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.String");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
						ParseState = EParseState::LookingForSingleQuoteString;
						bHasMatchedSyntax = true;
					} else if(ParseState == EParseState::LookingForSingleQuoteString && TokenText == TEXT("\'")) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Normal");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
						ParseState = EParseState::None;
					} else if(ParseState == EParseState::None && TokenText == TEXT("#")) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Comment");
						TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
						ParseState = EParseState::LookingForSingleLineComment;
					} else if(ParseState == EParseState::None && (TokenText == TEXT("\"\"\"") || TokenText == TEXT("'''"))) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Comment");
						TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
						ParseState = EParseState::LookingForMultiLineComment;
					} else if(ParseState == EParseState::LookingForMultiLineComment && (TokenText == TEXT("\"\"\"") || TokenText == TEXT("'''"))) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Comment");
						TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
						ParseState = EParseState::None;
					}else if(ParseState == EParseState::None && TChar<WIDECHAR>::IsAlpha(TokenText[0])) {
						// [space or line-start][not alphanumeric][TokenText][any][not alphanumeric][space, color or line-end]
						FString PatternString = FString::Format(TEXT("(^|\\s|^[\\._a-zA-Z0-9]){0}(^[_a-zA-Z0-9]|\\s|:|$)"), { TokenText });
						// Expand range by 1 to check if this is a whole word match
						FString Extended = SourceString.Mid(Token.Range.BeginIndex - 1, TokenText.Len() + (Token.Range.BeginIndex > 0 ? 2 : 1));
						FRegexPattern Pattern(PatternString);
						FRegexMatcher Matcher(Pattern, Extended);
						if ( Matcher.FindNext() ) {
							RunInfo.Name = TEXT("SyntaxHighlight.PY.Keyword");
							TextBlockStyle = SyntaxTextStyle.KeywordTextStyle;
							ParseState = EParseState::None;
						}
					} else if(ParseState == EParseState::None && !TChar<WIDECHAR>::IsAlpha(TokenText[0])) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Operator");
						TextBlockStyle = SyntaxTextStyle.OperatorTextStyle;
						ParseState = EParseState::None;
					}
				}
				
				// It's possible that we fail to match a syntax token if we're in a state where it isn't parsed
				// In this case, we treat it as a literal token
				if(Token.Type == FSyntaxTokenizer::ETokenType::Literal || !bHasMatchedSyntax) {
					if(ParseState == EParseState::LookingForDoubleQuoteString) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.String");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
					} else if(ParseState == EParseState::LookingForSingleQuoteString) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.String");
						TextBlockStyle = SyntaxTextStyle.StringTextStyle;
					} else if(ParseState == EParseState::LookingForSingleLineComment) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Comment");
						TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
					} else if(ParseState == EParseState::LookingForMultiLineComment) {
						RunInfo.Name = TEXT("SyntaxHighlight.PY.Comment");
						TextBlockStyle = SyntaxTextStyle.CommentTextStyle;
					}
				}

				TSharedRef< ISlateRun > Run = FSlateTextRun::Create(RunInfo, ModelString, TextBlockStyle, ModelRange);
				Runs.Add(Run);
			} else {
				RunInfo.Name = TEXT("SyntaxHighlight.PY.WhiteSpace");
				TSharedRef< ISlateRun > Run = FWhiteSpaceTextRun::Create(RunInfo, ModelString, TextBlockStyle, ModelRange, 4);
				Runs.Add(Run);
			}
		}

		LinesToAdd.Emplace(MoveTemp(ModelString), MoveTemp(Runs));
	}

	TargetTextLayout.AddLines(LinesToAdd);
}

FPythonSyntaxMarshaller::FPythonSyntaxMarshaller(TSharedPtr< FSyntaxTokenizer > InTokenizer, const FSyntaxTextStyle& InSyntaxTextStyle)
	: FSyntaxHighlighterTextLayoutMarshaller(MoveTemp(InTokenizer))
	, SyntaxTextStyle(InSyntaxTextStyle)
{
}

int32 FPythonSyntaxMarshaller::GetFontSize() const
{
	return SyntaxTextStyle.NormalTextStyle.Font.Size;
}
void FPythonSyntaxMarshaller::SetFontSize( int32 Size )
{
	if ( Size < 6 || Size > 108 ) return;
	SyntaxTextStyle.NormalTextStyle.Font.Size = Size;;
	SyntaxTextStyle.OperatorTextStyle.Font.Size = Size;;
	SyntaxTextStyle.KeywordTextStyle.Font.Size = Size;;
	SyntaxTextStyle.StringTextStyle.Font.Size = Size;;
	SyntaxTextStyle.NumberTextStyle.Font.Size = Size;;
	SyntaxTextStyle.CommentTextStyle.Font.Size = Size;
	MakeDirty();
}