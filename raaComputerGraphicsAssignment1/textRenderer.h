#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H

#include <GL/glut.h>

#include <map>
#include <string>

// struct for character information
struct Character {
	unsigned int textureID{};	// texture ID
	unsigned int width{};		// width of the glyph
	unsigned int height{};		// height of the glyph
	int bearingX{};				// offset amount from the left
	int bearingY{};				// offset amount from the top
	int advance{};				// offset to advance to next glyph
};

class Font {
public:
	std::map<char, Character> Characters{};

	Font(const char* fontPath);
	void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b);
};

