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
	/**
	 * This struct holds informations that are required to create a input box.
	 * The struct will be initialized to valid standard values except the font,
	 * which needs to be set!
	 * The following will explain the different settings:
	 *	- cmdList:
	 *		  Holds the commands that the box will auto complete to when tab was hit
	 *  - enableDefocus:
	 *		  If set to true the box will ony take input when the box was clicked last. Note
	 *			that if set to false the box can still be de-focused with SetFocused(state)
	 *	- prefix:
	 *			A string that will always be printed in front of the input text. Clear() is a method
	 *			that will not remove the prefix but just the input text
	 *	-	clearOnReturn:
	 *			If set to true the shown text will only consist of the prefix, when return was hit
	 *	- text:
	 *			Initial text that will be printed after the prefix
	 *	- font:
	 *			The font that the printed text will use. To create a font check the EEFontEngine
	 *	- characterSize:
	 *			Height in pixels of the characters the width is different per character
	 *	- adjustBoxSize:
	 *			If true the set size will be ignored and instead a size is computed that will show
	 *			whole text (prefix + text). Note that if a padding is set afterwards this can often
	 *			conflict since the minimum width/height minus padding needs to be greated than the 
	 *			characterSize
	 *	- textColor:
	 *			The shown color of the whole text
	 *	- padding:
	 *			A struct where left / top define the texts' offset within the box and right / bottom
	 *			confine how far / down the text will be rendered within the box.
	 *			The padding must not confine the text space in a way that not one character can be printed
	 *	- position:
	 *			The top-left corner offset of the box relativ to the top-left corner of the window
	 *	-	positionFlags:
	 *			If set to HORIZONTAL the x offset defined in position will be ignored and instead the box
	 *			will be centered within the window: (windowWidth - boxWidth) / 2
	 *			VERTICAL analog to horizontal and COMPLETE meaning vertical and horizontal
	 *	- size:
	 *			The size in pixels the box will have
	 *	- backgroundColor:
	 *			Standard color of the boxs' background
	 *	- enableHover:
	 *			If set to true the background color will be replaced with the hoverColor on mouseover
	 *	- hoverColor:
	 *			The color that will be used as the boxs' background color if enableHover was set to true
	 *	- enableActive:
	 *			If set to true the background color will be replaced with the activeColor when the
	 *			box was clicked
	 *	- activeColor:
	 *			The color that will be used as the boxs' background color if enableActive was set to true
	 *	- visibility:
	 *			Indicates wether this input box will be visible after creation. Can be changed afterwards
	 *			with SetVisibility(visible)
	 **/
	struct EEInputBoxCreateInfo {
		::CORETOOLS::CmdList	cmdList					{ ::CORETOOLS::CmdList() };
		bool									enableDefocus		{ false };
		EEstring							prefix					{ STR("") };
		bool									clearOnReturn		{ true };
		EEstring							text						{ STR("") };
		EEFont								font						{ nullptr };
		float									characterSize		{ 20.f };
		bool									adjustBoxSize		{ true };
		EEColor								textColor				{ .9f, .9f, .9f, 1.f };
		EEPadding							padding					{ 0.f, 0.f, 0.f, 0.f };
		EEPoint32F						position				{ 0.f, 0.f };
		EECenterFlags					positionFlags		{ EE_CENTER_NONE };
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
		/* @brief Standard creation of an empty box */
		EEInputBox(GFX::EEFontEngine* pFontEngine);

		/* @brief Creation of an input box with the properties passed in the createinfo */
		EEInputBox(GFX::EEFontEngine* pFontEngine, GFX::EEInputBoxCreateInfo const&);

		/* @brief Frees: autocompleter */
		~EEInputBox();

		/* @brief Needs to be called, so input (when focused) / style changes can be recognized and made */
		void Update() override;

		/**
		 * Adds a handler whos Handle()-method will be called when return was hit
		 * @param pHandler		The handler that will be added to the list
		 **/
		void AddHandler(::CORETOOLS::ITextHandler* pHandler);

		/**
		 * Removes a handler from the toBeSignaled list
		 * @param pHandler		The handler that will be removed from the list
		 **/
		void RemoveHandler(::CORETOOLS::ITextHandler* pHandler);

		/**
		 * Set the list of commands that will be autocompleted
		 * @param cmds	List of commands that will be autocompleted
		 **/
		void SetCommandList(::CORETOOLS::CmdList const& cmds);

		/**
		 * Adds a single command to the autocompletion list
		 * @param cmd		The command that will be added to the list
		 **/
		void AddCommandToList(::CORETOOLS::Cmd const& cmd);

		/* @brief Clears the text to the only the prefix */
		void Clear();

		/* @brief Allows to change the focuse state of the box */
		void SetFocused(bool focus);

		/* @brief Returns the current focus state */
		bool GetFocused() const;

	private:
		EEstring m_prefix; //< inited on construct

		std::vector<::CORETOOLS::ITextHandler*>		m_handlers;

		::CORETOOLS::EEAutoComplete*							m_pAutoCompleter{ nullptr };
		std::vector<::CORETOOLS::Cmd>							m_curAutoCompletes;
		uint32_t																	m_autoCompleteCmdIndex{ 0u };

		bool m_clearOnReturn;	//< inited on construct
		bool m_focused{ false };
	};
}