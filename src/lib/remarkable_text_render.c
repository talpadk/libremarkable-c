#include "remarkable_text_render.h"

#include "remarkable_blit.h"
#include "utf8_decode.h"
#include "remarkable_lut.h"

void remarkable_text_render_simple(remarkable_framebuffer *screen, FT_Face font, const char *text, int16_t x, int16_t y, uint8_t doDraw, TextBoundingBoundingBox *boundingBox){
  remarkable_text_render_simple_alpha_colour(screen, font, text, x,y, doDraw, boundingBox, -1, lut16Inverse_);
}
void remarkable_text_render_simple_alpha_colour(remarkable_framebuffer *screen, FT_Face font, const char *text, int16_t x, int16_t y, uint8_t doDraw, TextBoundingBoundingBox *boundingBox, int16_t alphaIndex, uint16_t lut[16]){
  uint8_t done = 0;
  FT_GlyphSlot slot;

  int16_t renderX;
  int16_t renderY;

  
  if (boundingBox != 0){
    boundingBox->xMin =  32767;
    boundingBox->xMax = -32768;
    boundingBox->yMin =  32767;
    boundingBox->yMax = -32768;
  }
  
  while (!done){
    uint32_t character=0;
    uint32_t bytesConsumed;
    bytesConsumed = utf8CharToUnicode((uint8_t *)text, &character);
    if (bytesConsumed != 0 && character != 0){
      text += bytesConsumed;
      FT_UInt glyph_index = FT_Get_Char_Index(font, character);
      
      FT_Load_Glyph(font, glyph_index, FT_LOAD_DEFAULT );
      slot = font->glyph;

      renderX = x+slot->bitmap_left;
      renderY = y-slot->bitmap_top;
      if (boundingBox != 0) {
	if (boundingBox->xMin > renderX) { boundingBox->xMin = renderX; }
	if (boundingBox->yMin > renderY) { boundingBox->yMin = renderY; }
	if (boundingBox->xMax < renderX+(uint16_t)slot->bitmap.width)  { boundingBox->xMax = renderX+(uint16_t)slot->bitmap.width; }
	if (boundingBox->yMax < renderY+(uint16_t)slot->bitmap.rows)   { boundingBox->yMax = renderY+(uint16_t)slot->bitmap.rows; }
      }
      
      if (doDraw){
	FT_Render_Glyph(font->glyph, FT_RENDER_MODE_NORMAL );
	remarkable_blit(screen, slot->bitmap.buffer, renderX, renderY, slot->bitmap.width, slot->bitmap.rows, alphaIndex, lut);
      }
      x += slot->advance.x >> 6;
    }
    else {
      done = 1;
    }    
  }
}
