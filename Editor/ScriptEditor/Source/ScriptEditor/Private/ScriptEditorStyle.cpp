// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScriptEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FScriptEditorStyle::StyleInstance = NULL;

void FScriptEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FScriptEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FScriptEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ScriptEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FScriptEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ScriptEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ScriptEditor")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ScriptEditor.OpenPluginWindow", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("ScriptEditor.Execute", new IMAGE_BRUSH(TEXT("UI/Execute_40x"), Icon40x40));
	Style->Set("ScriptEditor.Save", new IMAGE_BRUSH(TEXT("UI/Save_40x"), Icon40x40));
	Style->Set("ScriptEditor.Open", new IMAGE_BRUSH(TEXT("UI/FolderOpen"), Icon40x40));

	const FSlateFontInfo Consolas10  = DEFAULT_FONT("Mono", 9);
	const FTextBlockStyle NormalText = FTextBlockStyle()
		.SetFont(Consolas10)
		.SetColorAndOpacity(FLinearColor::White)
		.SetShadowOffset(FVector2D::ZeroVector)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
		.SetHighlightShape(BOX_BRUSH("UI/TextBlockHighlightShape", FMargin(3.f / 8.f)))
		;

	// Text editor
	{
		Style->Set("TextEditor.NormalText", NormalText);

		Style->Set("SyntaxHighlight.PY.Normal", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xfff2f2f2))));// light grey
		Style->Set("SyntaxHighlight.PY.Operator", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xff9cb4d9)))); // blue
		Style->Set("SyntaxHighlight.PY.Keyword", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xffb35757)))); // red
		Style->Set("SyntaxHighlight.PY.String", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xffdbc47d)))); // yellow
		Style->Set("SyntaxHighlight.PY.Number", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xff8d7ddb)))); // purple
		Style->Set("SyntaxHighlight.PY.Comment", FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(FColor(0xff2d6b1a)))); // green

		Style->Set("TextEditor.Border", new BOX_BRUSH("UI/TextEditorBorder", FMargin(4.0f/16.0f), FLinearColor(0.02f,0.02f,0.02f,1)));

		Style->Set("TextEditor.Background", new FSlateColorBrush(FLinearColor(FColor(33, 33, 33))));
		Style->Set("Menu.Background", new FSlateColorBrush(FLinearColor(FColor(33, 33, 33))));
	}
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
#undef DEFAULT_FONT

void FScriptEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FScriptEditorStyle::Get()
{
	return *StyleInstance;
}
