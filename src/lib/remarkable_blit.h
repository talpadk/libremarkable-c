#ifndef REMARKABLE_BLIT_H
#define REMARKABLE_BLIT_H

#include "lib.h"
#include <stdint.h>

/** 
 * A simplistic blit that dos not clip the graphics to the screen 
 * Uses the lut16_ for conversion, which must be initialized.
 * 
 * @param screen the screen to blit to
 * @param gfx a pointer to the grey scale graphics 
 * @param xPos the x position to start the blit from
 * @param yPos the y position to start the blit from
 * @param width the width of the graphics
 * @param height the height of the graphics
 */
void remarkable_fastBlit(remarkable_framebuffer *screen, const uint8_t *gfx, uint16_t xPos, uint16_t yPos, uint16_t width, uint16_t height);

void remarkable_blit(remarkable_framebuffer *screen, const uint8_t *gfx, int16_t xPos, int16_t yPos, int16_t width, int16_t height, uint8_t inverse);

void remarkable_blitRect(remarkable_framebuffer *screen, uint8_t colour, int16_t xStartPos, uint16_t yStartPos, int16_t xStopPos, uint16_t yStopPos);

#endif //REMARKABLE_BLIT_H 
