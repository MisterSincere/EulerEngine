#include "EEInputBox.h"

#include "coretools/Command.h"
#include "coretools/AutoComplete.h"
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
{}

GFX::EEInputBox::~EEInputBox()
{
	if (m_pAutoCompleter) {
		delete m_pAutoCompleter;
		m_pAutoCompleter = nullptr;
	}
}

void GFX::EEInputBox::Update()
{
	uint32_t codePoint = i_pApp->TextInput();

	/// Check simple text input (utf-8)
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
		m_autoCompleteCmds.clear();	//< Text changed so new autocomplete result needed

	/// Simple backspace to remove single char or with ctrl all chars
	} else if (i_pApp->KeyHit(EE_KEY_BACKSPACE) && i_text.size() > m_prefix.size()) {
		i_previousText = i_text;
		if (i_pApp->KeyPressed(EE_KEY_LEFT_CONTROL) || i_pApp->KeyPressed(EE_KEY_RIGHT_CONTROL)) {
			i_text = m_prefix;
		} else {
			i_text.erase(i_text.end() - 1);
		}
		i_changes |= TEXT_CHANGE;
		m_autoCompleteCmds.clear();	//< Text changed so new autocomplete result needed

	/// Auto complete the current text
	} else if (i_pApp->KeyHit(EE_KEY_TAB) && i_text.size() > m_prefix.size() && m_pAutoCompleter) {
		
		// If we have no currently auto complete suggestions get some
		if (m_autoCompleteCmds.empty()) {
			std::string cont(i_text.begin() + m_prefix.size(), i_text.end());
			m_autoCompleteCmds = m_pAutoCompleter->MultiComplete(cont);
			m_autoCompleteCmdIndex = 0u;
			if (!m_autoCompleteCmds.empty()) {
				i_text = m_prefix + m_autoCompleteCmds[0u].get();
			}

		// ... otherwise query through them
		} else {
			m_autoCompleteCmdIndex = ++m_autoCompleteCmdIndex % m_autoCompleteCmds.size();
			i_text = m_prefix + m_autoCompleteCmds[m_autoCompleteCmdIndex].get();
		}
		i_changes |= TEXT_CHANGE;
	}

	EETextBox::Update();
}

void GFX::EEInputBox::SetCommandList(::CORETOOLS::CmdList const& cmds)
{
	m_pAutoCompleter = new ::CORETOOLS::EEAutoComplete(cmds);
}

void GFX::EEInputBox::Clear()
{
	SetText(m_prefix);
}
