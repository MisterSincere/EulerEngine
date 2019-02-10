#include "EETextBox.h"

#include "EEFontEngine.h"




GFX::EETextBox::EETextBox(EEFontEngine* pFontEngine, EETextBoxCreateInfo const& cinfo)
	: EERectangle(pFontEngine->GetApplication(), cinfo.position, cinfo.size)
	, i_pFontEngine(pFontEngine)
	, i_textColor(cinfo.textColor)
	, i_characterSize(cinfo.characterSize)
	, i_text(cinfo.text)
	, i_font(cinfo.font)
{
	i_textAutoWrapped = i_pFontEngine->WrapText(i_font, i_text, i_characterSize,
			{ i_size.width - i_paddingLeft - i_paddingRight, i_size.height - i_paddingTop - i_paddingBottom });

	i_renderText = i_pFontEngine->RenderText(i_font, i_textAutoWrapped,
		{ i_position.x + i_paddingLeft, i_position.y + i_paddingTop }, i_characterSize, i_textColor);
}

GFX::EETextBox::EETextBox(EEFontEngine* pFontEngine, std::string const& text,
	EEFont font, float charSize, EEPoint32F const& pos, EEColor const& textColor)
	: EERectangle(pFontEngine->GetApplication(), pos)
	, i_pFontEngine(pFontEngine)
	, i_textColor(textColor)
	, i_characterSize(charSize)
	, i_text(text)
	, i_font(font)
{
	i_textAutoWrapped = i_text; // No auto wrap we adjust the box size to the maximum text width/height
	i_renderText = i_pFontEngine->RenderText(i_font, i_textAutoWrapped, i_position, charSize, textColor);

	EERect32F textSize = i_pFontEngine->GetTextDimensions(i_renderText);
	SetSize({ std::ceilf(textSize.width), std::ceilf(textSize.height) });


}

GFX::EETextBox::~EETextBox()
{
}

void GFX::EETextBox::Update()
{

	if (i_changes & POSITION_CHANGE || i_changes & PADDING_CHANGE) {
		i_pFontEngine->SetTextPosition(i_renderText, { i_position.x + i_paddingLeft, i_position.y + i_paddingTop });
	}
	if (i_changes & TEXT_CHANGE || i_changes & SIZE_CHANGE || i_changes & PADDING_CHANGE) {
		std::string newTextWrapped = i_pFontEngine->WrapText(i_font, i_text, i_characterSize,
			{ (float)i_size.width - i_paddingLeft - i_paddingRight, (float)i_size.height - i_paddingTop - i_paddingBottom });
		if (i_pFontEngine->ChangeText(i_renderText, newTextWrapped)) {
			i_textAutoWrapped = newTextWrapped;
			i_previousText = i_text;
		} else {
			i_text = i_previousText; //< Revert will only do something if text was changed because of a textchange
			EE_PRINT("[EETEXTBOX] Textbox text was not changed!\n");
		}
	}
	if (i_changes & TEXTCOLOR_CHANGE) {
		i_pFontEngine->ChangeTextColor(i_renderText, i_textColor);
	}

	EERectangle::Update(); //< Will do his part to the changes and sets them to zero
}

void GFX::EETextBox::SetFont(EEFont font)
{
	// TODO
	//i_font = font;
}

void GFX::EETextBox::SetCharacterSize(float size)
{
	// TODO
	//i_characterSize = size;
}

void GFX::EETextBox::SetPadding(float left, float top, float right, float bottom, bool expand)
{
	if (!expand && (left + right + i_characterSize >= i_size.width || top + bottom + i_characterSize >= i_size.height)) {
		EE_PRINT("[EETEXTBOX] Invalid padding option, box needs at least 'characterSize' pixels of space!\n");
		return;
	}

	i_paddingLeft = left;
	i_paddingTop = top;
	i_paddingRight = right;
	i_paddingBottom = bottom;
	i_changes |= PADDING_CHANGE;

	if (expand) {
		SetSize({ i_size.width + (uint32_t)(left + right), i_size.height + (uint32_t)(top + bottom) });
		SetPosition({ i_position.x - left, i_position.y - top });
	}
}

void GFX::EETextBox::SetTextColor(EEColor const& textColor)
{
	i_textColor = textColor;
	i_changes |= TEXTCOLOR_CHANGE;
}

void GFX::EETextBox::SetText(std::string const& newText)
{
	/*std::string newStringWrapped = i_pFontEngine->WrapText(i_font, i_text, i_characterSize,
			{ i_size.width - (uint32_t)(i_paddingLeft - i_paddingRight), i_size.height - (uint32_t)(i_paddingTop - i_paddingBottom) });

	if(!i_pFontEngine->ChangeText(i_renderText, newStringWrapped)) {
		
	} else {
		i_text = newText;
		i_textAutoWrapped = newStringWrapped;
	}*/
	i_previousText = i_text;
	i_text = newText;
	i_changes |= TEXT_CHANGE;
}

void GFX::EETextBox::SetVisibility(bool isVisible)
{
	EERectangle::SetVisibility(isVisible);
	i_pFontEngine->SetTextVisibility(i_renderText, isVisible);
}

std::string GFX::EETextBox::GetText()
{
	return i_text;
}


