#include "remarkable_blit.h"

#include "remarkable_lut.h"


void remarkable_fastBlit(remarkable_framebuffer *screen, const uint8_t *gfx, uint16_t xPos, uint16_t yPos, uint16_t width, uint16_t height){
  uint16_t *pixels = (uint16_t *)screen->mapped_buffer;
  pixels += xPos + yPos*(screen->finfo.line_length>>1);

  uint16_t modulo = (screen->finfo.line_length>>1) - width;
  
  for (uint16_t y = 0; y < height; y++) {
    for (uint16_t x = 0; x < width; x++) {
      *pixels++ = lut16_[(*gfx++)>>4];
    }
    pixels +=  modulo;
  }
}

void remarkable_blit(remarkable_framebuffer *screen, const uint8_t *gfx, int16_t xPos, int16_t yPos, int16_t width, int16_t height, uint8_t inverse){
  int16_t inputModulo = 0;

  
  //=== Clipping ===
  //Y Clipping
  if (yPos < 0){
    gfx -= yPos * width;
    height += yPos;
    yPos = 0;
    if (height <= 0) { return; }
  }
  if (yPos + height > screen->vinfo.yres){
    height = (int16_t)(screen->vinfo.yres - (uint32_t)yPos);
    if (height <= 0) { return; }	
  }

  //X Clipping
  if (xPos < 0){
    gfx -= xPos;
    inputModulo -= xPos;
    width += xPos;
    xPos = 0;
    if (width < 0) { return; }
  }
  if (xPos + width > screen->vinfo.xres){
    int16_t tmp = (int16_t)(screen->vinfo.xres - (uint32_t)xPos);
    inputModulo += width-tmp;
    width = tmp;
    if (width < 0) { return; }
  }

  uint16_t *pixels = (uint16_t *)screen->mapped_buffer;
  pixels += xPos + yPos*(screen->finfo.line_length>>1);

  uint16_t modulo = (screen->finfo.line_length>>1) - width;

  if (!inverse){
    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < width; x++) {
	*pixels++ = lut16_[(*gfx++)>>4];
      }
      pixels +=  modulo;
      gfx += inputModulo;
    }
  }
  else {
    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < width; x++) {
	*pixels++ = lut16_[15-((*gfx++)>>4)];
      }
      pixels +=  modulo;
      gfx += inputModulo;
    }
  }
    
  
}

void remarkable_blitRect(remarkable_framebuffer *screen, uint8_t colour, int16_t xStartPos, uint16_t yStartPos, int16_t xStopPos, uint16_t yStopPos){
  if (xStartPos < 0) { xStartPos = 0; }
  if (yStartPos < 0) { yStartPos = 0; }
  if (xStartPos >= screen->vinfo.xres) { xStartPos = screen->vinfo.xres-1; }
  if (yStartPos >= screen->vinfo.yres) { yStartPos = screen->vinfo.yres-1; }
  
  if (xStopPos >= screen->vinfo.xres) { xStopPos = screen->vinfo.xres-1; }
  if (yStopPos >= screen->vinfo.yres) { yStopPos = screen->vinfo.yres-1; }
  if (xStopPos < xStartPos) { xStopPos = xStartPos; }
  if (yStopPos < yStartPos) { yStopPos = yStartPos; }

  uint16_t *pixels = (uint16_t *)screen->mapped_buffer;
  pixels += xStartPos + yStartPos*(screen->finfo.line_length>>1);
  uint16_t fill = lut16_[colour & 15];

  uint16_t width  = xStopPos-xStartPos+1;
  uint16_t height = yStopPos-yStartPos+1;
  uint16_t modulo = (screen->finfo.line_length>>1) - width;
  
  for (uint16_t y = 0; y < height; y++) {
    for (uint16_t x = 0; x < width; x++) {
      *pixels++ = fill;
    }
    pixels +=  modulo;
  }
  
}
		     
