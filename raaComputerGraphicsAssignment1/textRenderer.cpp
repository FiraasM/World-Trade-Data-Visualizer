#include "textRenderer.h"

#include <iostream>
#include <GL/GL.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

Font::Font(const char* fontPath) {
	FT_Library ft{};
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "Could not initialise FreeType\n";
		return;
	}

	FT_Face face{};
	if (FT_New_Face(ft, fontPath, 0, &face)) {
		std::cerr << "Failed to load font\n";
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, 48);	// font size

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	// prevent alignment issues with fonts, otherwise could cause seg-faults

	for (unsigned char c{ 0 }; c < 128; ++c) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "Failed to load Glyph for character " << c << '\n';
			continue;
		}

		// Generate texture
		unsigned int texture{};
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_ALPHA,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);

		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Store character
		Characters[c] = {
			texture,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			face->glyph->bitmap_left,
			face->glyph->bitmap_top,
			face->glyph->advance.x
		};
	}
	
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Font::renderText(const std::string& text, float x, float y, float scale, float r, float g, float b) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);

	// Render each character
	glColor3f(r, g, b);

	for (const char& c : text) {
		Character ch{ Characters[c] };

		float xpos{ x + ch.bearingX * scale };
		float ypos{ y - (ch.height - ch.bearingY) * scale };

		float width{ ch.width * scale };
		float height{ ch.height * scale };

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f);	glVertex2f(xpos			, ypos			);
			glTexCoord2f(1.0f, 1.0f);	glVertex2f(xpos + width	, ypos			);
			glTexCoord2f(1.0f, 0.0f);	glVertex2f(xpos + width	, ypos + height	);
			glTexCoord2f(0.0f, 0.0f);	glVertex2f(xpos			, ypos + height	);
		glEnd();

		x += (ch.advance >> 6) * scale;	// Advance cursor for next glyph
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glMatrixMode(GL_MODELVIEW);
}