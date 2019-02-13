#include "EEInputBox.h"

#include "EEApplication.h"


GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine)
	: EETextBox(pFontEngine)
{}

GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const& cinfo)
	: EETextBox(pFontEngine, cinfo)
{

}

void GFX::EEInputBox::Update()
{
	uint32_t codePoint = i_pApp->TextInput();

	if (codePoint <= 0x7f)
	{
		i_text.append(1, static_cast<char>(codePoint));
	}
	else if (codePoint <= 0x7ff)
	{
		i_text.append(1, static_cast<char>(0xc0 | ((codePoint >> 6) & 0x1f)));
		i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
	}
	else if (codePoint <= 0xffff)
	{
		i_text.append(1, static_cast<char>(0xe0 | ((codePoint >> 12) & 0x0f)));
		i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
		i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
	}
	else
	{
		i_text.append(1, static_cast<char>(0xf0 | ((codePoint >> 18) & 0x07)));
		i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f)));
		i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
		i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
	}
	SetText(i_text);
}