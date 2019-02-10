#include "EETextBox.h"

#include "EEFontEngine.h"




GFX::EETextBox::EETextBox(EEFontEngine* pFontEngine, std::string const& text,
	EEFont font, float charSize, EEPoint32F const& pos, EEColor const& textColor)
	: EERectangle(pFontEngine->GetApplication(), pos)
	, m_pFontEngine(pFontEngine)
	, m_textColor(textColor)
	, m_characterSize(charSize)
	, m_text(text)
	, m_font(font)
{
	m_textAutoWrapped = m_text; // No auto wrap we adjust the box size to the maximum text width/height
	m_renderText = m_pFontEngine->RenderText(m_font, m_textAutoWrapped, i_position, charSize, textColor);

	EERect32F textSize = m_pFontEngine->GetTextDimensions(m_renderText);
	SetSize({ (uint32_t)std::ceilf(textSize.width), (uint32_t)std::ceilf(textSize.height) });


}
//
//GFX::EETextBox::EETextBox(EEFontEngine* pFontEngine, std::string const& text, EEFont font, float charSize, EEColor const& textColor,
//													EEPoint32F const& pos, EERect32U const& size)
//	: EERectangle(pFontEngine->GetApplication(), pos, size)
//	, m_pFontEngine(pFontEngine)
//	, m_textColor(textColor)
//	, m_characterSize(charSize)
//	, m_text(text)
//	, m_font(font)
//{
//	// Store the text with newline characters where the end of box was reached
//	m_textAutoWrapped = m_pFontEngine->WrapText(m_font, m_text, m_characterSize,
//		{ (float)i_size.width - m_paddingLeft - m_paddingRight, (float)i_size.height - m_paddingTop - m_paddingBottom });
//	
//	m_renderText = m_pFontEngine->RenderText(m_font, m_textAutoWrapped,
//																					 { i_position.x + m_paddingLeft, i_position.y + m_paddingTop },
//																					 m_characterSize, m_textColor);
//}

GFX::EETextBox::~EETextBox()
{
}

void GFX::EETextBox::Update()
{

	if (i_changes & POSITION_CHANGE || i_changes & PADDING_CHANGE) {
		m_pFontEngine->SetTextPosition(m_renderText, { i_position.x + m_paddingLeft, i_position.y + m_paddingTop });
	}
	if (i_changes & TEXT_CHANGE || i_changes & SIZE_CHANGE || i_changes & PADDING_CHANGE) {
		std::string newTextWrapped = m_pFontEngine->WrapText(m_font, m_text, m_characterSize,
			{ (float)i_size.width - m_paddingLeft - m_paddingRight, (float)i_size.height - m_paddingTop - m_paddingBottom });
		if (m_pFontEngine->ChangeText(m_renderText, newTextWrapped)) {
			m_textAutoWrapped = newTextWrapped;
			m_previousText = m_text;
		} else {
			m_text = m_previousText; //< Revert will only do something if text was changed because of a textchange
			EE_PRINT("[EETEXTBOX] Textbox text was not changed!\n");
		}
	}
	if (i_changes & TEXTCOLOR_CHANGE) {
		m_pFontEngine->ChangeTextColor(m_renderText, m_textColor);
	}

	EERectangle::Update(); //< Will do his part to the changes and sets them to zero
}

void GFX::EETextBox::SetFont(EEFont font)
{
	// TODO
	//m_font = font;
}

void GFX::EETextBox::SetCharacterSize(float size)
{
	// TODO
	//m_characterSize = size;
}

void GFX::EETextBox::SetPadding(float left, float top, float right, float bottom, bool expand)
{
	if (!expand && (left + right + m_characterSize >= i_size.width || top + bottom + m_characterSize >= i_size.height)) {
		EE_PRINT("[EETEXTBOX] Invalid padding option, box needs at least 'characterSize' pixels of space!\n");
		return;
	}

	m_paddingLeft = left;
	m_paddingTop = top;
	m_paddingRight = right;
	m_paddingBottom = bottom;
	i_changes |= PADDING_CHANGE;

	if (expand) {
		SetSize({ i_size.width + (uint32_t)(left + right), i_size.height + (uint32_t)(top + bottom) });
		SetPosition({ i_position.x - left, i_position.y - top });
	}
}

void GFX::EETextBox::SetTextColor(EEColor const& textColor)
{
	m_textColor = textColor;
	i_changes |= TEXTCOLOR_CHANGE;
}

void GFX::EETextBox::SetText(std::string const& newText)
{
	/*std::string newStringWrapped = m_pFontEngine->WrapText(m_font, m_text, m_characterSize,
			{ i_size.width - (uint32_t)(m_paddingLeft - m_paddingRight), i_size.height - (uint32_t)(m_paddingTop - m_paddingBottom) });

	if(!m_pFontEngine->ChangeText(m_renderText, newStringWrapped)) {
		
	} else {
		m_text = newText;
		m_textAutoWrapped = newStringWrapped;
	}*/
	m_previousText = m_text;
	m_text = newText;
	i_changes |= TEXT_CHANGE;
}

void GFX::EETextBox::SetVisibility(bool isVisible)
{
	EERectangle::SetVisibility(isVisible);
	m_pFontEngine->SetTextVisibility(m_renderText, isVisible);
}

std::string GFX::EETextBox::GetText()
{
	return m_text;
}


