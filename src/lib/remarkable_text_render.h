#ifndef REMARKABLE_TEXT_RENDER_H
#define REMARKABLE_TEXT_RENDER_H

///#cflags `pkg-config --cflags freetype2`
///#ldflags `pkg-config --libs freetype2`
#include <ft2build.h>
#include FT_FREETYPE_H

#include "lib.h"


typedef struct {
  int16_t xMin;
  int16_t xMax;
  int16_t yMin;
  int16_t yMax;
} TextBoundingBoundingBox;

void remarkable_text_render_simple(remarkable_framebuffer *screen, FT_Face font, const char *text, int16_t x, int16_t y, uint8_t doDraw, TextBoundingBoundingBox *boundingBox);


#endif //REMARKABLE_TEXT_RENDER_H 
