#include "EEInputBox.h"

#include "EEApplication.h"


GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine)
	: EETextBox(pFontEngine)
{}

GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const& cinfo)
	: EETextBox(pFontEngine,  {
														std::string(cinfo.prefix + cinfo.text)
														, cinfo.font
														, cinfo.characterSize
														, cinfo.adjustBoxSize
														, cinfo.textColor
														, cinfo.padding
														, cinfo.position
														, cinfo.positionFlags
														, cinfo.size
														, cinfo.backgroundColor
														, cinfo.enableHover
														, cinfo.hoverColor
														, cinfo.enableActive
														, cinfo.activeColor
														, cinfo.visibility
														})
	, m_prefix(cinfo.prefix)
{
	
}

void GFX::EEInputBox::Update()
{
	uint32_t codePoint = i_pApp->TextInput();
	if (codePoint) {
		i_previousText = i_text;
		if (codePoint <= 0x7f) {
			i_text.append(1, static_cast<char>(codePoint));
		} else if (codePoint <= 0x7ff) {
			i_text.append(1, static_cast<char>(0xc0 | ((codePoint >> 6) & 0x1f)));
			i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
		} else if (codePoint <= 0xffff) {
			i_text.append(1, static_cast<char>(0xe0 | ((codePoint >> 12) & 0x0f)));
			i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
			i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
		} else {
			i_text.append(1, static_cast<char>(0xf0 | ((codePoint >> 18) & 0x07)));
			i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 12) & 0x3f)));
			i_text.append(1, static_cast<char>(0x80 | ((codePoint >> 6) & 0x3f)));
			i_text.append(1, static_cast<char>(0x80 | (codePoint & 0x3f)));
		}
		i_changes |= TEXT_CHANGE;

	//< Simple backspace to remove single character or with lctrl all chars
	} else if (i_pApp->KeyHit(EE_KEY_BACKSPACE) && i_text.size() > m_prefix.size()) {
		i_previousText = i_text;
		if (i_pApp->KeyPressed(EE_KEY_LEFT_CONTROL)) {
			i_text = m_prefix;
		} else {
			i_text.erase(i_text.end() - 1);
		}
		i_changes |= TEXT_CHANGE;
	}

	EETextBox::Update();
}

void GFX::EEInputBox::Clear()
{
	SetText(m_prefix);
}
