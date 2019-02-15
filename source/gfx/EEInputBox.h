/////////////////////////////////////////////////////////////////////
// Filename: EEInputBox.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "EETextBox.h"
#include "coretools/Command.h"

namespace CORETOOLS {
	class EEAutoComplete;
	class ITextHandler;
}

namespace GFX
{
	struct EEInputBoxCreateInfo {
		::CORETOOLS::CmdList	cmdList					{ ::CORETOOLS::CmdList() };
		std::wstring					prefix					{ L"" };
		bool									clearOnReturn		{ true };
		std::wstring					text						{ L"" };
		EEFont								font						{ nullptr };
		float									characterSize		{ 20.f };
		bool									adjustBoxSize		{ true };
		EEColor								textColor				{ .9f, .9f, .9f, 1.f };
		EEPadding							padding					{ 0.f, 0.f, 0.f, 0.f };
		EEPoint32F						position				{ 0.f, 0.f };
		EECenterFlags					positionFlags		{ NONE };
		EERect32F							size						{ 200.f, 100.f };
		EEColor								backgroundColor	{ 0.f, 0.f, 0.f, 1.f };
		bool									enableHover			{ false };
		EEColor								hoverColor			{ 0.f, 0.f, 0.f, 0.f };
		bool									enableActive		{ false };
		EEColor								activeColor			{ 0.f, 0.f, 0.f, 0.f };
		bool									visibility			{ true };
	};

	class EEInputBox : public EETextBox
	{
	public:
		EEInputBox(GFX::EEFontEngine* pFontEngine);
		EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const&);
		~EEInputBox();

		void Update() override;

		void AddHandler(::CORETOOLS::ITextHandler*);
		void RemoveHandler(::CORETOOLS::ITextHandler*);

		void SetCommandList(::CORETOOLS::CmdList const&);
		void AddCommandToList(::CORETOOLS::Cmd const&);
		void Clear();

	private:
		std::wstring m_prefix;

		::CORETOOLS::EEAutoComplete* m_pAutoCompleter{ nullptr };

		std::vector<::CORETOOLS::ITextHandler*> m_handlers;

		std::vector<::CORETOOLS::Cmd> m_autoCompleteCmds;
		uint32_t m_autoCompleteCmdIndex{ 0u };

		bool m_clearOnReturn;
	};
}