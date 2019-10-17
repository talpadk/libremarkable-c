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



int main(void){
  remarkable_framebuffer *screen = remarkable_framebuffer_init("/dev/fb0");
  FT_Library  libFT;
  FT_Face     font;

  remarkable_lut16Init();
   
  if (FT_Init_FreeType(&libFT)){
    printf("Freetype lib init failed!\n");
    return 1;
  }

  if (FT_New_Face(libFT, "/usr/share/fonts/ttf/noto/NotoSerif-Bold.ttf", 0, &font)){
    printf("Error while loading font");
    return 1;
  }
  
  if (FT_Set_Char_Size(font, 4000, 0, 226, 226)){
    printf("Failed to set font size");
    return 1;
  }

  remarkable_framebuffer_fill(screen, REMARKABLE_BRIGHTEST);

  int16_t xPos = 100;
  int16_t yPos = 800;
  TextBoundingBoundingBox boundingBox;
  remarkable_text_render_simple(screen, font, "Hello World", xPos, yPos, 1, &boundingBox);
  remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, 0,0, 1872,1404);
  return 0;
}
