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

namespace GFX
{
	struct EETextBoxCreateInfo {
		std::string		text						{ "" };
		EEFont				font						{ nullptr };
		float					characterSize		{ 30.f };
		EEPoint32F		position				{ 0.f, 0.f };
		EECenterFlags positionFlags		{ NONE };
		EERect32F			size						{ 200.f, 200.f };
		EEColor				backgroundColor	{ .1f, .1f, .1f, 1.f };
		EEColor				textColor				{ .9f, .9f, .9f, 1.f };
		float					padding[4]			{ 0.f, 0.f, 0.f, 0.f };
		bool					visible					{ true };
		EEColor				hoverColor			{	-1.f, -1.f, -1.f, -1.f };
		EEColor				activeColor			{ -1.f, -1.f, -1.f, -1.f };
	};

	class EETextBox : public EERectangle
	{
	public:
		EETextBox(EEFontEngine* pFontEngine) : EETextBox(pFontEngine, EETextBoxCreateInfo()) {}
		EETextBox(EEFontEngine* pFontEngine, EETextBoxCreateInfo const& cinfo);
		EETextBox(EEFontEngine* pFontEngine, std::string const& text, EEFont font,
			float charSize = { 30.f }, EEPoint32F const& pos = { 0.0f, 0.0f }, EEColor const& textColor = { .9f, .9f, .9f, 1.f });
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
		std::string i_textAutoWrapped;

		EEFont i_font;
		EEText i_renderText;
	};
}