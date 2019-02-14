/////////////////////////////////////////////////////////////////////
// Filename: EETextBox.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "EERectangle.h"

#include <string>

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
namespace GFX {
	class EEFontEngine;
	typedef uint32_t* EEFont;
	typedef uint32_t* EEText;
}

#define ACQUIRE_RECT_INFO(valPtr)	(reinterpret_cast<::GFX::EERectangleCreateInfo const*>(	\
																		reinterpret_cast<char const*>(valPtr)						\
																		+ offsetof(::GFX::EETextBoxCreateInfo, position)			\
																	))

namespace GFX
{
	struct EETextBoxCreateInfo {
		std::string						text						{ "hello world" };
		EEFont								font						{ nullptr };
		float									characterSize		{ 30.f };
		bool									adjustBoxSize		{ true };
		EEColor								textColor				{ .9f, .9f, .9f, 1.f };
		EEPadding							padding					{ 0.f, 0.f, 0.f, 0.f };
		EEPoint32F						position				{ 0.f, 0.f };
		EECenterFlags					positionFlags		{ NONE };
		EERect32F							size						{ 200.f, 200.f };
		EEColor								backgroundColor	{ 0.f, 0.f, 0.f, 1.f };
		bool									enableHover			{ false };
		EEColor								hoverColor			{ 0.f, 0.f, 0.f, 0.f };
		bool									enableActive		{ false };
		EEColor								activeColor			{ 0.f, 0.f, 0.f, 0.f };
		bool									visibility			{ true };
	};

	class EETextBox : public EERectangle
	{
	public:
		EETextBox(EEFontEngine* pFontEngine);
		EETextBox(EEFontEngine* pFontEngine, EETextBoxCreateInfo const& cinfo);
		EETextBox(EETextBox const&) = delete;
		EETextBox(EETextBox&&) = delete;
		virtual ~EETextBox();

		virtual void Update() override;

		void SetFont(EEFont font); // TODO
		void SetCharacterSize(float size); // TODO
		void SetPadding(float left, float top, float right, float bottom, bool expand = false);
		void SetTextColor(EEColor const& textColor);
		void SetText(std::string const& newText);
		void SetVisibility(bool isVisible) override;

		std::string GetText();

		EETextBox& operator=(EETextBox const&) = delete;
		EETextBox& operator=(EETextBox&&) = delete;

	protected:
		EEFontEngine* i_pFontEngine;
		EEColor				i_textColor;
		float					i_characterSize;
		float					i_paddingLeft{ 0.0f }, i_paddingTop{ 0.0f }, i_paddingRight{ 0.0f }, i_paddingBottom{ 0.0f };

		std::string i_text;
		std::string i_previousText;
		std::string i_textWrapped;

		EEFont i_font;
		EEText i_renderText;

		bool i_textBoxCreated{ false };
	};
}