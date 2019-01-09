/////////////////////////////////////////////////////////////////////
// Filename: EEFontEngine.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "EEFontEngine.h"

#include "vkcore/vulkanTools.h"
#include "EEApplication.h"

using namespace DirectX;

// Useful defines
#define MAX(x, y) ((x > y) ? x : y)
#define MIN(x, y) ((x < y) ? x : y)
#define LAST_ELEMENT(vec) (vec[vec.size() - 1])

GFX::EEFontEngine::EEFontEngine(EEApplication* pApp, uint32_t amountTexts)
	: m_pApp(pApp)
{
	assert(pApp);

	// Initialize free type
	FT_Error error = FT_Init_FreeType(&m_library);
	if (error) {
		EE_PRINT("[EEFONTENGINE] Failed to initialize free type!\n");
		EE::tools::warning("[EEFONTENGINE] Failed to initialize free type!\n");
		return;
	}

	// Shader
	{
		// Descriptors
		std::vector<EEDescriptorDesc> descriptors(3);
		descriptors[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[0].shaderStage = EE_SHADER_STAGE_VERTEX;
		descriptors[0].binding = 0u;

		descriptors[1].type = EE_DESCRIPTOR_TYPE_SAMPLER;
		descriptors[1].shaderStage = EE_SHADER_STAGE_FRAGMENT;
		descriptors[1].binding = 1u;

		descriptors[2].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptors[2].shaderStage = EE_SHADER_STAGE_FRAGMENT;
		descriptors[2].binding = 2u;

		// Vertex Input
		std::vector<EEShaderInputDesc> shaderInputs(2);
		shaderInputs[0].location = 0u;
		shaderInputs[0].format = EE_FORMAT_R32G32_SFLOAT;
		shaderInputs[0].offset = offsetof(VertexInput, position);

		shaderInputs[1].location = 1u;
		shaderInputs[1].format = EE_FORMAT_R32G32_SFLOAT;
		shaderInputs[1].offset = offsetof(VertexInput, texCoord);

		EEVertexInput vertexInput;
		vertexInput.amountInputs = uint32_t(shaderInputs.size());
		vertexInput.pInputDescs = shaderInputs.data();
		vertexInput.inputStride = sizeof(VertexInput);

		std::string vert = EE_ASSETS_DIR("shader/fontVert.spv");
		std::string frag = EE_ASSETS_DIR("shader/fontFrag.spv");
		EEShaderCreateInfo shaderCInfo;
		shaderCInfo.vertexFileName = vert.c_str();
		shaderCInfo.fragmentFileName = frag.c_str();
		shaderCInfo.amountObjects = amountTexts;
		shaderCInfo.shaderInputType = EE_SHADER_INPUT_TYPE_CUSTOM;
		shaderCInfo.pVertexInput = &vertexInput;
		shaderCInfo.amountDescriptors = uint32_t(descriptors.size());
		shaderCInfo.pDescriptors = descriptors.data();
		shaderCInfo.pPushConstant = nullptr;
		shaderCInfo.is2DShader = EE_TRUE;
		shaderCInfo.wireframe = EE_FALSE;
		shaderCInfo.clockwise = EE_TRUE;
		m_shader = m_pApp->CreateShader(shaderCInfo);
	}

	m_isCreated = EE_TRUE;
}

GFX::EEFontEngine::~EEFontEngine()
{
	if (m_isCreated) {
		// Release all current texts
		for (size_t i = 0u; i < m_currentTexts.size(); i++) {
			m_pApp->ReleaseObject(m_currentTexts[i]->object);
			m_pApp->ReleaseMesh(m_currentTexts[i]->mesh);
			delete m_currentTexts[i];
			delete m_iCurrentTexts[i];
		}
		m_currentTexts.~vector();
		m_iCurrentTexts.~vector();

		// Release all current fonts
		for (size_t i = 0u; i < m_currentFonts.size(); i++) {
			m_pApp->ReleaseTexture(m_currentFonts[i]->texture);
			delete m_currentFonts[i];
			delete m_iCurrentFonts[i];
		}
		m_currentFonts.~vector();
		m_iCurrentFonts.~vector();

		// Release the shader
		m_pApp->ReleaseShader(m_shader);

		// Now tell free type to shut down
		FT_Done_FreeType(m_library);
	}
}

GFX::EEFont GFX::EEFontEngine::CreateFont(char const* fileName, char const* charSet)
{
	// Allocate memory for the font internal details
	EEInternFont* pFont = new EEInternFont;
	assert(pFont);

	// Read in the font file using the free type library
	FT_Error error = FT_New_Face(m_library, fileName, 0, &pFont->face);
	if (error == FT_Err_Unknown_File_Format) {
		delete pFont;
		EE_PRINT("[EEFONTENGINE] The font file could noe be opened. Its format is probably not supported by freetype!\n");
		EE::tools::warning("[EEFONTENGINE] The font file could noe be opened. Its format is probably not supported by freetype!\n");
		return nullptr;
	} else if (error) { //< some other error
		delete pFont;
		EE_PRINT("[EEFONTENGINE] Could not read in font file!\n");
		EE::tools::warning("[EEFONTENGINE] Could not read in font file!\n");
		return nullptr;
	}

	// Set the size of the font face
	error = FT_Set_Pixel_Sizes(pFont->face, 256, 256);
	if (error) {
		delete pFont;
		EE_PRINT("[EEFONTENGINE] Failed to set up font (size)!\n");
		EE::tools::warning("[EEFONTENGINE] Failed to set up font (size)!\n");
		return nullptr;
	}

	// Acquire informations about the passed in character set
	size_t numChars = strlen(charSet);
	uint32_t fontImgWidth{ 0u }, fontImgHeight{ 0u }, glyphIndex{ 0u };
	uint32_t maxTopBearingY{ 0u }, maxBelowBearingY{ 0u };

	// Initialize fonts' max values so the checks during the loop can be made
	pFont->maxLetterWidth = 0u;

	FT_GlyphSlot glyph = pFont->face->glyph;
	uint32_t letterWidth;
	// Get the bitmap per character
	for (size_t i = 0u; i < numChars; i++) {
		glyphIndex = FT_Get_Char_Index(pFont->face, charSet[i]);
		error = FT_Load_Glyph(pFont->face, glyphIndex, FT_LOAD_DEFAULT);
		if (error) continue;
		error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
		if (error) continue;

		// Store informations about the position of this character in the final image for the shader
		letterWidth = glyph->bitmap.width + 2*glyph->bitmap_left;
		pFont->letterDetails[charSet[i]] = {
			fontImgWidth,
			letterWidth,
			glyph->bitmap.rows,
			uint32_t(glyph->bitmap_left),
			uint32_t(glyph->bitmap_top)
		};

		// Update the final font image dimensions
		pFont->maxLetterWidth = MAX(letterWidth, pFont->maxLetterWidth);
		fontImgWidth += letterWidth;
		maxTopBearingY = MAX((uint32_t)glyph->bitmap_top, maxTopBearingY);
		maxBelowBearingY = MAX((uint32_t)glyph->bitmap.rows - MIN(glyph->bitmap.rows, (uint32_t)glyph->bitmap_top), maxBelowBearingY);
	}

	fontImgHeight = maxTopBearingY + maxBelowBearingY;

	// We got the informations now we can allocate memory and fill it with the image data
	unsigned char* fontImageData = new unsigned char[fontImgWidth * fontImgHeight];
	memset(fontImageData, 0, sizeof(unsigned char) * fontImgWidth * fontImgHeight);
	Letter curLetter; //< Stores the current letter to read
	for (uint32_t i = 0u; i < numChars; i++) {
		glyphIndex = FT_Get_Char_Index(pFont->face, charSet[i]);
		error = FT_Load_Glyph(pFont->face, glyphIndex, FT_LOAD_DEFAULT);
		if (error) continue;
		error = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
		if (error) continue;

		// We dont need a try catch since we know it was added before (same charset)
		curLetter = pFont->letterDetails.at(charSet[i]);

		// Write the image data of the character to the correct position in the final fontImageData
		for (uint32_t y = 0u; y < pFont->face->glyph->bitmap.rows; y++) {
			for (uint32_t x = 0u; x < pFont->face->glyph->bitmap.width; x++) {
				unsigned char curVal = pFont->face->glyph->bitmap.buffer[y * pFont->face->glyph->bitmap.width + x];
				fontImageData[(y + maxTopBearingY - curLetter.bearingY) * fontImgWidth + curLetter.offsetX + curLetter.bearingX + x] = curVal;
			}
		}
	}

	// Store the texture size
	pFont->width = fontImgWidth;
	pFont->height = fontImgHeight;

	// Create the font texture
	EETextureCreateInfo textureCInfo;
	textureCInfo.pData = fontImageData;
	textureCInfo.extent = { fontImgWidth, fontImgHeight };
	textureCInfo.unnormalizedCoordinates = EE_TRUE;
	textureCInfo.enableMipMapping = EE_TRUE;
	textureCInfo.format = EE_FORMAT_R8_UNORM;
	pFont->texture = m_pApp->CreateTexture(textureCInfo);

	delete[] fontImageData;
	fontImageData = nullptr;

	// Store the intern font struct and the new handle to it
	EE_INVARIANT(m_currentFonts.size() == m_iCurrentFonts.size());
	m_currentFonts.push_back(pFont);
	m_iCurrentFonts.push_back(new uint32_t(uint32_t(m_currentFonts.size() - 1)));
	EE_INVARIANT(m_currentFonts.size() == m_iCurrentFonts.size());

	// Return the new created handle
	return LAST_ELEMENT(m_iCurrentFonts);
}

void GFX::EEFontEngine::ReleaseFont(EEFont & font)
{
	// @TODO
}

GFX::EEText GFX::EEFontEngine::RenderText(EEFont font, char* text, EEPoint32F const& position, float size, EEColor const& color)
{
	assert(text);

	// Get the font
	EEInternFont* curFont = m_currentFonts[*font];

	// Allocate memory for the internal text details
	EEInternText* pText = new EEInternText;
	assert(pText);

	// Store color and font
	pText->color = { color.r, color.g, color.b, color.a };
	pText->pFont = curFont;

	// Get the mesh for the text
	std::vector<VertexInput> vertices;
	std::vector<uint32_t> indices;
	if (!ComputeMeshAccToFont(curFont, text, vertices, indices)) return nullptr;
	pText->mesh = m_pApp->CreateMesh(vertices.data(), sizeof(VertexInput) * vertices.size(), indices);

	// Create the ubos
	pText->vertexBuffer = m_pApp->CreateBuffer(sizeof(VertexUBO));
	pText->fragmentBuffer = m_pApp->CreateBuffer(sizeof(FragmentUBO));

	// Create the object
	std::vector<EEObjectResourceBinding> bindings(3);
	bindings[0].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].binding = 0u;
	bindings[0].resource = pText->vertexBuffer;

	bindings[1].type = EE_DESCRIPTOR_TYPE_SAMPLER;
	bindings[1].binding = 1u;
	bindings[1].resource = curFont->texture;

	bindings[2].type = EE_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[2].binding = 2u;
	bindings[2].resource = pText->fragmentBuffer;
	pText->object = m_pApp->CreateObject(m_shader, pText->mesh, bindings);

	// Initialize fragment buffer
	m_pApp->UpdateBuffer(pText->fragmentBuffer, &(pText->color));

	EERect32U wExtent = m_pApp->GetWindowExtent();
	XMMATRIX world = XMMatrixTransformation2D(
		{ 0.0f, 0.0f }, 0.0f, { size, size }, //< Scale to desired size
		{ 0.0f, 0.0f }, 0.0f,										//< No rotation supported until now
		{ -(wExtent.width / 2.0f) + position.x, -(wExtent.height / 2.0f) + position.y } //< Translate depending on position
	);
	XMStoreFloat4x4(&(pText->world), world);

	// Store the created text details and its index/handle
	EE_INVARIANT(m_currentTexts.size() == m_iCurrentTexts.size());
	m_currentTexts.push_back(pText);
	m_iCurrentTexts.push_back(new uint32_t(uint32_t(m_currentTexts.size() - 1)));
	EE_INVARIANT(m_currentTexts.size() == m_iCurrentTexts.size());

	// Return the handle to the created text
	return LAST_ELEMENT(m_iCurrentTexts);
}

void GFX::EEFontEngine::ReleaseText(EEText& text)
{
	// @TODO
}

EEBool32 GFX::EEFontEngine::ChangeText(EEText text, char* newText)
{
	if (!text) {
		EE_PRINT("[EEFONTENGINE] Text handle that was passed into ChangeText was nullptr!\n");
		return EE_FALSE;
	}
	
	// Compute new mesh
	std::vector<VertexInput> vertices;
	std::vector<uint32_t> indices;
	ComputeMeshAccToFont(m_currentTexts[*text]->pFont, newText, vertices, indices);

	m_pApp->UpdateMesh(m_currentTexts[*text]->mesh, vertices.data(), sizeof(VertexInput) * vertices.size(), indices);

	return EE_TRUE;
}

void GFX::EEFontEngine::ChangeTextColor(EEText text, EEColor const& newColor)
{
	if (!text) {
		EE_PRINT("[EEFONTENGINE] Text handle that was passed into ChangeTextColor was nullptr!\n");
		return;
	}

	EEInternText* pText = m_currentTexts[*text];

	// store new color as xmfloat4 and update the actual buffer
	pText->color = { newColor.r, newColor.g, newColor.b, newColor.a };
	m_pApp->UpdateBuffer(pText->fragmentBuffer, &(pText->color));
}

void GFX::EEFontEngine::Update()
{
	// Fragment buffer gets updated if changed for the vertex buffer we do not
	// know when the ortho/baseView matrix may have changed so we update it every frame
	VertexUBO vertUbo;
	XMStoreFloat4x4(&vertUbo.ortho, m_pApp->AcquireOrthoMatrixLH());
	XMStoreFloat4x4(&vertUbo.baseView, m_pApp->AcquireBaseViewLH());

	for (size_t i = 0u; i < m_currentTexts.size(); i++) {
		vertUbo.world = m_currentTexts[i]->world;
		m_pApp->UpdateBuffer(m_currentTexts[i]->vertexBuffer, &vertUbo);
	}
}

EEBool32 GFX::EEFontEngine::ComputeMeshAccToFont(EEInternFont* pFont, char* text, std::vector<VertexInput>& vertices, std::vector<uint32_t>& indices)
{
	// Settings of the per letter dimensions
	float letterWidth{ 1.0f }, letterHeight{ 1.0f }, letterSpacing{ 1.0f }, penX{ 0.0f }, penY{ 0.0f };

	Letter curLetter; //< Stored the letter for each loop iteration
	uint32_t topLeftIndex, topRightIndex, bottomRightIndex, bottomLeftIndex; //< Stores the current indices
	for (size_t i = 0u; i < strlen(text); i++) {

		// Handle first special characters, but for every else character use the font
		if (text[i] == '\n') {
			penY += 1.0f;
			penX = 0.0f;
			continue;
		} else if (text[i] == ' ') {
			penX += .5f;
			continue;
		}

		// Try to get the desired characters' details. std::map::at throws outofrange exception
		// if the character was not defined as a key in the map
		try {
			curLetter = pFont->letterDetails.at(text[i]);
		} catch (std::out_of_range const& oor) {
			EE_PRINT("[EEFONTENGINE] Invalid character! The desired text containts at least on characters that was not defined in the charset of the font.\n%s", oor.what());
			EE::tools::warning("[EEFONTENGINE] Invalid character! The desired text containts at least on characters that was not defined in the charset of the font.\n");
			return EE_FALSE;
		}

		letterWidth = (float)curLetter.width / pFont->maxLetterWidth;
		// TOP LEFT
		vertices.push_back({ {penX, penY}, {float(curLetter.offsetX), 0.0f} });
		topLeftIndex = uint32_t(vertices.size() - 1);

		// BOTTOM LEFT
		vertices.push_back({ {penX, penY + letterHeight}, {float(curLetter.offsetX), float(pFont->height)} });
		bottomLeftIndex = uint32_t(vertices.size() - 1);

		// TOP RIGHT
		vertices.push_back({ {penX + letterWidth, penY}, {float(curLetter.offsetX + curLetter.width), 0.0f} });
		topRightIndex = uint32_t(vertices.size() - 1);

		// BOTTOM RIGHT
		vertices.push_back({ {penX + letterWidth, penY + letterHeight}, {float(curLetter.offsetX + curLetter.width), float(pFont->height)} });
		bottomRightIndex = uint32_t(vertices.size() - 1);

		// Push back the indices in clockwise order
		indices.push_back(topLeftIndex);
		indices.push_back(topRightIndex);
		indices.push_back(bottomRightIndex);
		indices.push_back(topLeftIndex);
		indices.push_back(bottomRightIndex);
		indices.push_back(bottomLeftIndex);

		// Shift pen position by the current letterwidth
		penX += letterWidth;
	}

	return EE_TRUE;
}
