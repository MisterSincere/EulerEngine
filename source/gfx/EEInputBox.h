#pragma once

#include "EETextBox.h"


namespace GFX
{

	typedef EETextBoxCreateInfo EEInputBoxCreateInfo;

	class EEInputBox : public EETextBox
	{
	public:
		EEInputBox(GFX::EEFontEngine* pFontEngine);
		EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const&);

		void Update() override;


	};
}