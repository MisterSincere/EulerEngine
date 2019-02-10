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
	class EETextBox : public EERectangle
	{
	public:
		EETextBox(EEFontEngine* pFontEngine, std::string const& text, EEFont font,
			float charSize = { 30.f }, EEPoint32F const& pos = { 0.0f, 0.0f }, EEColor const& textColor = { .9f, .9f, .9f, 1.f });
		EETextBox(EETextBox const&) = delete;
		EETextBox(EETextBox&&) = delete;
		~EETextBox();

		void Update() override;

		void SetFont(EEFont font); // TODO
		void SetCharacterSize(float size); // TODO
		void SetPadding(float left, float top, float right, float bottom, bool expand = false);
		void SetTextColor(EEColor const& textColor);
		void SetText(std::string const& newText);
		void SetVisibility(bool isVisible) override;

		std::string GetText();

		EETextBox& operator=(EETextBox const&) = delete;
		EETextBox& operator=(EETextBox&&) = delete;

	private:
		EEFontEngine* m_pFontEngine;
		EEColor m_textColor;
		float m_characterSize;
		float m_paddingLeft{ 0.0f }, m_paddingTop{ 0.0f }, m_paddingRight{ 0.0f }, m_paddingBottom{ 0.0f };

		std::string m_text;
		std::string m_previousText;
		std::string m_textAutoWrapped;

		EEFont m_font;
		EEText m_renderText;
	};
}