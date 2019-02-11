/////////////////////////////////////////////////////////////////////
// Filename: EEFontEngine.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

//////////////
// INCLUDES //
//////////////
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <vector>

/////////////////
// MY INCLUDES //
/////////////////
#include "eedefs.h"

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
class EEApplication;

namespace GFX
{
	EE_DEFINE_HANDLE(EEFont);
	EE_DEFINE_HANDLE(EEText);

	class EEFontEngine
	{
	private:
		/* @brief [ShaderSpecific] Holds the shader input values per vertex */
		struct VertexInput {
			DirectX::XMFLOAT2 position;
			DirectX::XMFLOAT2 texCoord;
		};

		/* @brief [ShaderSpecific; binding=0] Holds the content of the vertex ubo */
		struct VertexUBO {
			DirectX::XMFLOAT4X4 ortho;
			DirectX::XMFLOAT4X4 baseView;
			DirectX::XMFLOAT4X4 world;
		};

		/* @brief [ShaderSpecifig; binding=1] Holds the color of the text */
		struct FragmentUBO {
			DirectX::XMFLOAT4 textColor;
		};

		/* @brief Holds information of a letter to find it in the created font texture */
		struct Letter {
			uint32_t offsetX;
			uint32_t width;
			uint32_t height;
			uint32_t bearingX;
			uint32_t bearingY;
		};

		/* @brief Holds informations of a specific font that is necessary to render a text */
		struct EEInternFont {
			FT_Face face;
			EETexture texture;
			uint32_t width;
			uint32_t height;
			std::map<char, Letter> letterDetails;
			uint32_t maxLetterWidth;
		};

		/* @brief Holds informatios of a text that can be rendered */
		struct EEInternText {
			EEInternFont* pFont;
			EEMesh mesh;
			EEBuffer vertexBuffer;
			EEBuffer fragmentBuffer;
			EEObject object;
			float size;
			EEPoint32F position;
			EERect32F maxTextDimensions;

			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT4X4 world;
		};

	public:
		/**
		 * Constructor to intialize the font engine. After this the engine will be available for
		 * creating fonts and rendering texts.
		 *
		 * @param pApp				Pointer to the application to use
		 * @param amountTexts	The amount of texts that this engine will render (defaulted)
		 **/
		EEFontEngine(EEApplication* pApp, uint32_t amountTexts = 1000u);

		/**
		 * Destructor will release all currently active fonts and texts
		 **/
		~EEFontEngine();

		/**
		 * Reads in the fonts' style of the passed in charset
		 *
		 * @param fileName	Destination of the font file
		 * @param charSet		The characters that will be read in and made available for texts using this font
		 *
		 * @return Handle to the created font
		 **/
		EEFont CreateFont(char const* fileName, char const* charSet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZäüöß0123456789€$,.;:_-!?<>()[]");

		/**
		 * Frees the font passed in
		 * @note Before that all texts using this font need to be manually released 
		 **/
		void ReleaseFont(EEFont& font);

		/**
		 * Renders the text passed in with the desired options
		 *
		 * @param font			Handle to the font to use
		 * @param text			The text to be rendered (must only include characters from the charset)
		 * @param position	Top left corner position of the text
		 * @param size			Size of the text in pixel
		 * @param color			Color of the text
		 * @param wrapDim		Width and height borders the text will not exceed (defaults to -1 for no auto wrap)
		 *
		 * @return Handle to the created text, which can be used to modify/release the text
		 **/
		EEText RenderText(EEFont font, std::string const& text, EEPoint32F const& position, float size, EEColor const& color);

		/**
		 * Frees the text passed in.
		 **/
		void ReleaseText(EEText& text);

		/**
		 * Changes the text of the EEText passed in.
		 * @note The new text (like the old one) must not contain a char that was not in the charset
		 *
		 * @param text			Handle to the text to change
		 * @param newText		The new text that should be rendered
		 *
		 * @return Could be false if the @note was not adhered to or the vertex buffer update failed.
		 **/
		EEBool32 ChangeText(EEText text, std::string const& newText);

		/**
		 * Inserts new line characters for the text according to the font passed in where the text exceeds
		 * the dimensions passed in.
		 *
		 * @param font			The font which properties will be used
		 * @param text			The text that will be wrapped
		 * @param wrapDim		Struct where x defines the maximum x coord and y respectively
		 *
		 * @return The wrapped text
		 **/
		std::string WrapText(EEFont font, std::string const& text, float size, EERect32F const& wrapDim);

		/**
		 * Changes the color of the text passed in
		 **/
		void ChangeTextColor(EEText text, EEColor const& newColor);

		/**
		 * Changes the visibility of the object
		 **/
		void SetTextVisibility(EEText text, EEBool32 visibility) const;

		/**
		 * Changes the position of the text passed in
		 * @param text		The text which position will be changed
		 * @param pos			The new position
		 **/
		void SetTextPosition(EEText text, EEPoint32F const& pos);

		/**
		 * Changes the size in pixels of the text passed in
		 * @param text			The text to change
		 * @param charSize	New desired size in pixels of the text
		 **/
		void SetCharacterSize(EEText text, float charSize);

		/**
		 * Please call every frame so the text can adjust to i.e. resizing
		 **/
		void Update() const;

		/**
		 * @return The pixel width/height the text takes
		 **/
		EERect32F GetTextDimensions(EEText text);

		/**
		 * @return The application this font engine operates on
		 **/
		::EEApplication* GetApplication() const;

	private:
		/**
		 * Computes the mesh data for the text passed in according to the font passed in.
		 *
		 * @param font				The font that defines the style of the text
		 * @param text				The desired text to compute vertices/indices for
		 * @param verticesOut	Will be filled with the computed vertex data
		 * @param indicesOut	Will be filled with the indices according to the vertex data
		 * @param maxTextDims The maximum width and height in pixels the text needs
		 *
		 * @return Is false if a desired character was not declared to be in the charset
		 **/
		EEBool32 ComputeMeshAccToFont(
			EEInternFont*							font,
			std::string const&				text,
			std::vector<VertexInput>& verticesOut,
			std::vector<uint32_t>&		indicesOut,
			EERect32F&								maxTextDims);

		/**
		 * Goes back from the specified index and replaces the first occurence
		 * of a space char with a new line.
		 * @param text		The text that the newline will be placed in
		 * @param index		The position from where to go
		 *
		 * @return Amount of steps back that were needed to find a space
		 **/
		int InsertLineBreak(std::string& text, size_t index);

	private:
		/* @brief The application this font engine will use */
		EEApplication* m_pApp;
		/* @brief The initialized free type library that is used for reading in fonts */
		FT_Library m_library;
		/* @brief The standard shader that is used for every text */
		EEShader m_shader;

		/* @brief All created fonts that can be used and accessed by a font handle */
		std::vector<EEInternFont*> m_currentFonts;
		/* @brief The indices/handles of fonts given to the user */
		std::vector<uint32_t*> m_iCurrentFonts;
		/* @brief All created texts that can be rendered and modified by a text handle */
		std::vector<EEInternText*> m_currentTexts;
		/* @brief The indices/handles to the created texts */
		std::vector<uint32_t*> m_iCurrentTexts;

		/* @brief Indicates wether the font engine can be used */
		EEBool32 m_isCreated{ EE_FALSE };
	};
}