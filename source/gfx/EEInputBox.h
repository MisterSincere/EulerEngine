#pragma once

#include "EETextBox.h"
#include "coretools/Command.h"


#define ACQUIRE_TEXT_INFO(valPtr)	(reinterpret_cast<::GFX::EETextBoxCreateInfo const*>(	\
																		reinterpret_cast<char const*>(valPtr)						\
																		+ offsetof(::GFX::EEInputBoxCreateInfo, text)			\
																	))

namespace CORETOOLS {
	class EEAutoComplete;
}

namespace GFX
{

	struct EEInputBoxCreateInfo {
		std::string						prefix					{ "" };
		std::string						text						{ "" };
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

		void SetCommandList(::CORETOOLS::CmdList const&);
		void Clear();

	private:
		std::string m_prefix;

		::CORETOOLS::EEAutoComplete* m_pAutoCompleter{ nullptr };

		std::vector<::CORETOOLS::Cmd> m_autoCompleteCmds;
		uint32_t m_autoCompleteCmdIndex{ 0u };
	};
}