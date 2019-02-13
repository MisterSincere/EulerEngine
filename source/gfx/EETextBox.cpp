#include "EETextBox.h"

#include "EEFontEngine.h"

using namespace GFX;

#define EPSILON 0.1f



EETextBox::EETextBox(EEFontEngine* pFontEngine)
	: EERectangle(pFontEngine->GetApplication())
	, i_pFontEngine(pFontEngine)
{}

EETextBox::EETextBox(EEFontEngine* pFontEngine, EETextBoxCreateInfo const& cinfo)
	: EERectangle(pFontEngine->GetApplication(), *ACQUIRE_RECT_INFO(&cinfo))
	, i_pFontEngine(pFontEngine)
	, i_textColor(cinfo.textColor)
	, i_characterSize(cinfo.characterSize)
	, i_text(cinfo.text)
	, i_font(cinfo.font)
	, i_paddingLeft(cinfo.padding.left)
	, i_paddingTop(cinfo.padding.top)
	, i_paddingRight(cinfo.padding.right)
	, i_paddingBottom(cinfo.padding.bottom)
{
	assert(i_font);

	// Get a version of the passed in text that has newline characters at positions
	// that would exceed the desired width/height if we are limited to the box size
	if (!cinfo.adjustBoxSize) {
		i_textWrapped = i_pFontEngine->WrapText(i_font, i_text, i_characterSize,
			{ i_size.width - i_paddingLeft - i_paddingRight, i_size.height - i_paddingTop - i_paddingBottom });

	// otherwise we just go with the standard text and resize the box later
	} else {
		i_textWrapped = i_text;
	}

	// Create the renderable text
	i_renderText = i_pFontEngine->RenderText(i_font, i_textWrapped, 
		{ i_position.x + i_paddingLeft, i_position.y + i_paddingTop }, i_characterSize, i_textColor);

	// Store the new size if we adjust it to the text
	if (cinfo.adjustBoxSize) {
		EERect32F textSize = i_pFontEngine->GetTextDimensions(i_renderText);
		SetSize({ textSize.width + i_paddingLeft + i_paddingRight + EPSILON, textSize.height + i_paddingTop + i_paddingBottom + EPSILON });
		// Now center the pox aligned if desired
		if (cinfo.positionFlags) SetPositionAligned(cinfo.positionFlags);
	}

	if (!cinfo.visibility) {
		i_pFontEngine->SetTextVisibility(i_renderText, EE_FALSE);
	}

	i_textBoxCreated = true;
}

EETextBox::~EETextBox()
{
	if (i_textBoxCreated) {
		i_pFontEngine->ReleaseText(i_renderText);

		i_textBoxCreated = false;
	}
}

void EETextBox::Update()
{
	if (i_changes & POSITION_CHANGE || i_changes & PADDING_CHANGE) {
		i_pFontEngine->SetTextPosition(i_renderText, { i_position.x + i_paddingLeft, i_position.y + i_paddingTop });
	}

	if (i_changes & TEXT_CHANGE || i_changes & SIZE_CHANGE || i_changes & PADDING_CHANGE) {
		std::string newTextWrapped = i_pFontEngine->WrapText(i_font, i_text, i_characterSize,
			{ i_size.width - i_paddingLeft - i_paddingRight, i_size.height - i_paddingTop - i_paddingBottom});
		if (i_pFontEngine->ChangeText(i_renderText, newTextWrapped)) {
			i_textWrapped = newTextWrapped;
			i_previousText = i_text;
		} else {
			i_text = i_previousText; //< Revert will only do something if text was changed because of a textchange
			EE_PRINT("[EETEXTBOX] Textbox text was not changed!\n");
		}
	}

	if (i_changes & TEXTCOLOR_CHANGE) {
		i_pFontEngine->ChangeTextColor(i_renderText, i_textColor);
	}

	EERectangle::Update(); //< Will do his part to the changes and sets the bitmask to zero
}





void EETextBox::SetFont(EEFont font)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING CHANGE FONT YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}
	// TODO
	//i_font = font;
}

void EETextBox::SetCharacterSize(float size)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING SET A NEW CHAR SIZE YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}
	// TODO
	//i_characterSize = size;
}

void EETextBox::SetPadding(float left, float top, float right, float bottom, bool expand)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING SET A PADDING YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}

	if (!expand && (left + right + i_characterSize >= i_size.width || top + bottom + i_characterSize >= i_size.height)) {
		EE_PRINT("[EETEXTBOX] Invalid padding option, box needs at least space for one line and one character!\n");
		return;
	}

	i_paddingLeft = left;
	i_paddingTop = top;
	i_paddingRight = right;
	i_paddingBottom = bottom;
	i_changes |= PADDING_CHANGE;

	if (expand) {
		SetSize({ i_size.width + left + right, i_size.height + top + bottom });
	}
}

void EETextBox::SetTextColor(EEColor const& textColor)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING SET A TEXT COLOR YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}

	i_textColor = textColor;
	i_changes |= TEXTCOLOR_CHANGE;
}

void EETextBox::SetText(std::string const& newText)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING SET A NEW TEXT YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}

	i_previousText = i_text;
	i_text = newText;
	i_changes |= TEXT_CHANGE;
}

void EETextBox::SetVisibility(bool isVisible)
{
	if (!i_textBoxCreated) {
		EE_PRINT("[EETEXTBOX] WHY BOTHER TO FUCKING CHANGE VISIBILITY YOU RETARD!! TEXT WASNT CREATED YET!!!!!\n");
		return;
	}

	EERectangle::SetVisibility(isVisible);
	i_pFontEngine->SetTextVisibility(i_renderText, isVisible);
}









std::string EETextBox::GetText()
{
	return i_text;
}


