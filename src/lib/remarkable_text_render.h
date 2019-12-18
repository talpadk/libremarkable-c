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

/** 
 * Renders a text string to the frame buffer
 * 
 * @param screen the frame buffer to render to
 * @param font the face / font to use
 * @param text utf-8 string containing the text to render
 * @param x x coordinate of the text base line start
 * @param y y coordinate of the text base line start
 * @param doDraw if false the function dos not render any pixels (can be used to obtain the bounding box before rendering)
 * @param boundingBox if non NULL the bounding box information will be returned here 
 */
void remarkable_text_render_simple(remarkable_framebuffer *screen, FT_Face font, const char *text, int16_t x, int16_t y, uint8_t doDraw, TextBoundingBoundingBox *boundingBox);

/** 
 * Renders a text string to the frame buffer
 * 
 * @param screen the frame buffer to render to
 * @param font the face / font to use
 * @param text utf-8 string containing the text to render
 * @param x x coordinate of the text base line start
 * @param y y coordinate of the text base line start
 * @param doDraw if false the function dos not render any pixels (can be used to obtain the bounding box before rendering)
 * @param boundingBox if non NULL the bounding box information will be returned here 
 * @param alphaIndex the index 0-15 of the colour that should be transparent (-1 for no colour), 0 for normal transparent rendering as the true type library uses 0 for "paper" and 255 for 100% ink
 * @param lut the colour look up table that determines font colour, use lut16Inverse_ for black on white rending
 */
void remarkable_text_render_simple_alpha_colour(remarkable_framebuffer *screen, FT_Face font, const char *text, int16_t x, int16_t y, uint8_t doDraw, TextBoundingBoundingBox *boundingBox, int16_t alphaIndex, uint16_t lut[16]);


#endif //REMARKABLE_TEXT_RENDER_H 
