/////////////////////////////////////////////////////////////////////
// Filename: EEInputBox.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EEInputBox.h"

#include <vkcore/vulkanTools.h>

#include "coretools/AutoComplete.h"
#include "coretools/IHandler.h"
#include "EEApplication.h"


GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine)
	: EETextBox(pFontEngine)
	, m_clearOnReturn(true)
{}

GFX::EEInputBox::EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const& cinfo)
	: EETextBox(pFontEngine,  {
														EEstring(cinfo.prefix + cinfo.text)
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
	, m_clearOnReturn(cinfo.clearOnReturn)
{
	// If commands were specified create an autocompeter for them
	if (!cinfo.cmdList.empty()) {
		m_pAutoCompleter = new ::CORETOOLS::EEAutoComplete(cinfo.cmdList);
	}
}

GFX::EEInputBox::~EEInputBox()
{
	RELEASE_S(m_pAutoCompleter);
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
		m_curAutoCompletes.clear();	//< Text changed so new autocomplete results needed

	/// Simple backspace to remove single char or with ctrl all chars
	} else if (i_pApp->KeyHit(EE_KEY_BACKSPACE) && i_text.size() > m_prefix.size()) {
		i_previousText = i_text;
		if (i_pApp->KeyPressed(EE_KEY_LEFT_CONTROL) || i_pApp->KeyPressed(EE_KEY_RIGHT_CONTROL)) {
			i_text = m_prefix;
		} else {
			i_text.erase(i_text.end() - 1);
		}
		i_changes |= TEXT_CHANGE;
		m_curAutoCompletes.clear();	//< Text changed so new autocomplete results needed

	/// Auto complete the current text
	} else if (i_pApp->KeyHit(EE_KEY_TAB) && i_text.size() > m_prefix.size() && m_pAutoCompleter) {
		
		// If we have no currently auto complete suggestions get some
		if (m_curAutoCompletes.empty()) {
			EEstring cont(i_text.begin() + m_prefix.size(), i_text.end());
			m_curAutoCompletes = m_pAutoCompleter->MultiComplete(cont);
			m_autoCompleteCmdIndex = 0u;
			if (!m_curAutoCompletes.empty()) {
				i_text = m_prefix + m_curAutoCompletes[0u].get();
			}

		// ... otherwise query through them
		} else {
			m_autoCompleteCmdIndex = ++m_autoCompleteCmdIndex % m_curAutoCompletes.size();
			i_text = m_prefix + m_curAutoCompletes[m_autoCompleteCmdIndex].get();
		}
		i_changes |= TEXT_CHANGE;

	/// On return let the registered handlers the text passed in
	} else if (i_pApp->KeyHit(EE_KEY_ENTER) && i_text.size() > m_prefix.size() && m_handlers.size()) {
		EEstring cont(i_text.begin() + m_prefix.size(), i_text.end());
		for (auto const& curHandle : m_handlers) curHandle->Handle(cont.c_str());
		if (m_clearOnReturn) SetText(m_prefix);
	}

	EETextBox::Update();
}

void GFX::EEInputBox::AddHandler(::CORETOOLS::ITextHandler* h)
{
	if (!h) {
		EE_PRINT("[EEINPUTBOX] Tried to add invalid text handler!\n");
		return;
	}
	m_handlers.push_back(h);
}

void GFX::EEInputBox::RemoveHandler(::CORETOOLS::ITextHandler* h)
{
	auto el = std::find(m_handlers.begin(), m_handlers.end(), h);
	if (el != m_handlers.end()) m_handlers.erase(el);
	else {
		EE_PRINT("[EEINPUTBOX] Handler that was tried to be removed from input box wasn't found!\n");
	}
}

void GFX::EEInputBox::SetCommandList(::CORETOOLS::CmdList const& cmds)
{
	if (m_pAutoCompleter) {
		m_pAutoCompleter->SetCommandList(cmds);
	} else {
		m_pAutoCompleter = new ::CORETOOLS::EEAutoComplete(cmds);
	}
	assert(m_pAutoCompleter);
}

void GFX::EEInputBox::AddCommandToList(::CORETOOLS::Cmd const& cmd)
{
	if (!m_pAutoCompleter) {
		EE_PRINT("[EEINPUTBOX] Tried to add a command without settings a commandlist first!\n");
		return;
	}
	m_pAutoCompleter->AddCommandToList(cmd);
}

void GFX::EEInputBox::Clear()
{
	SetText(m_prefix);
}

void GFX::EEInputBox::SetFocused(bool focus)
{
	m_focused = focus;
}

bool GFX::EEInputBox::GetFocused() const
{
	return m_focused;
}
