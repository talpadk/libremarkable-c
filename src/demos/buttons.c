///#exe
///#link fb
///#link refresh
///#link serde


#include "lib.h"

#include "utf8_decode.h"
#include <string.h>

#include "remarkable_lut.h"
#include "remarkable_blit.h"
#include "remarkable_text_render.h"
#include "remarkable_buttons.h"

uint16_t lutGrey_[16];

RemarkableButtons oldState_;

void drawButtons(remarkable_framebuffer *screen, FT_Face font){
  int16_t xPos = 590;
  int16_t yPos = 400;
  TextBoundingBoundingBox bBox;
  uint16_t *colour;

  if (oldState_.power != buttonState_.power){
    xPos = 590;
    yPos = 400;
    colour = lutGrey_;
    if (buttonState_.power) { colour = lut16Inverse_; }  
    remarkable_text_render_simple_alpha_colour(screen, font, "POWER", xPos, yPos, 1, &bBox, -1, colour);
    remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bBox.yMin,bBox.xMin,  bBox.yMax-bBox.yMin,bBox.xMax-bBox.xMin);
  }

  if (oldState_.wakeUp != buttonState_.wakeUp){
    xPos = 565;
    yPos = 660;
    colour = lutGrey_;
    if (buttonState_.wakeUp) { colour = lut16Inverse_; }  
    remarkable_text_render_simple_alpha_colour(screen, font, "WAKE UP", xPos, yPos, 1, &bBox, -1, colour);
    remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bBox.yMin,bBox.xMin,  bBox.yMax-bBox.yMin,bBox.xMax-bBox.xMin);
  }

  
  if (oldState_.left != buttonState_.left){
    xPos = 100;
    yPos = 1500;
    colour = lutGrey_;
    if (buttonState_.left) { colour = lut16Inverse_; }  
    remarkable_text_render_simple_alpha_colour(screen, font, "LEFT", xPos, yPos, 1, &bBox, -1, colour);
    remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bBox.yMin,bBox.xMin,  bBox.yMax-bBox.yMin,bBox.xMax-bBox.xMin);
  }


  if (oldState_.home != buttonState_.home){
    xPos = 600;
    yPos = 1500;
    colour = lutGrey_;
    if (buttonState_.home) { colour = lut16Inverse_; }  
    remarkable_text_render_simple_alpha_colour(screen, font, "HOME", xPos, yPos, 1, &bBox, -1, colour);
    remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bBox.yMin,bBox.xMin,  bBox.yMax-bBox.yMin,bBox.xMax-bBox.xMin);
  }


  if (oldState_.right != buttonState_.right){
    xPos = 1150;
    yPos = 1500;
    colour = lutGrey_;
    if (buttonState_.right) { colour = lut16Inverse_; }  
    remarkable_text_render_simple_alpha_colour(screen, font, "RIGHT", xPos, yPos, 1, &bBox, -1, colour);
    remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bBox.yMin,bBox.xMin,  bBox.yMax-bBox.yMin,bBox.xMax-bBox.xMin);
  }

  memcpy(&oldState_, &buttonState_, sizeof(RemarkableButtons));
}

int main(void){
  remarkable_framebuffer *screen = remarkable_framebuffer_init("/dev/fb0");
  FT_Library  libFT;
  FT_Face     font;

  remarkable_lut16Init();
  remarkable_lut_create(lutGrey_, 255, 100);
  remarkable_buttons_init();
  oldState_.left   = -1;
  oldState_.right  = -1;
  oldState_.home   = -1;
  oldState_.power  = -1;
  oldState_.wakeUp = -1;

  
  if (FT_Init_FreeType(&libFT)){
    printf("Freetype lib init failed!\n");
    return 1;
  }

  if (FT_New_Face(libFT, "/usr/share/fonts/ttf/noto/NotoSerif-Bold.ttf", 0, &font)){
    printf("Error while loading font");
    return 1;
  }
  
  if (FT_Set_Char_Size(font, 1000, 0, 226, 226)){
    printf("Failed to set font size");
    return 1;
  }

  remarkable_framebuffer_fill(screen, REMARKABLE_BRIGHTEST);
  remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, 0,0, 1872,1404);

  while (1){
    drawButtons(screen, font);
    remarkable_buttons_animate();
  }
  
  return 0;
}
