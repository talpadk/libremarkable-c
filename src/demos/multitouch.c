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
#include "remarkable_multitouch.h"

MultitouchSlot multitouchSlotsLastValues_[MAX_MULTITOUCH_TRACKS];

#define BUF_SIZE (100)

void clearRenderText(remarkable_framebuffer *screen, FT_Face font, const char *string, int16_t baseX, int16_t baseY, int16_t clearLength){
  TextBoundingBoundingBox bounds, tmp;
  remarkable_text_render_simple(screen, font, string, baseX, baseY, 0, &bounds);
  bounds.xMax = bounds.xMin+clearLength;
  remarkable_blitRect(screen, 15, bounds.xMin, bounds.yMin, bounds.xMax, bounds.yMax);
  remarkable_text_render_simple(screen, font, string, baseX, baseY, 1, &tmp);

  /*uint32_t marker = */
  remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, bounds.yMin, bounds.xMin, bounds.yMax-bounds.yMin+1, bounds.xMax-bounds.xMin+1);
  //remarkable_framebuffer_wait_refresh_marker(screen, marker);
}

void updateSlotGfx(remarkable_framebuffer *screen, FT_Face font, uint32_t bufferIndex){
  char buffer[BUF_SIZE];
  int16_t xPos;
  int16_t yPos;
  if (bufferIndex<5) {
    xPos = 10;
    yPos = 200+bufferIndex*340;
  }
  else {
    xPos = 700;
    yPos = 200+(bufferIndex-5)*340;
  }
   
  int16_t spacing = 37;
  int16_t clearSize = 500;

  MultitouchSlot *out = multitouchSlotsLastValues_ + bufferIndex;
  MultitouchSlot *in  = multitouchSlots_ + bufferIndex;
  if (in->trackingId != out->trackingId){
    snprintf(buffer, BUF_SIZE, "Slot %u  Track ID : %d", bufferIndex, in->trackingId);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->trackingId = in->trackingId;
  }
  yPos += spacing;

  if (in->xRaw != out->xRaw){
    snprintf(buffer, BUF_SIZE, "X                : %d", in->x);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->xRaw = in->xRaw;
  }
  yPos += spacing;

  if (in->yRaw != out->yRaw){
    snprintf(buffer, BUF_SIZE, "Y                : %d", in->y);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->yRaw = in->yRaw;
  }
  yPos += spacing;

  if (in->pressure != out->pressure){
    snprintf(buffer, BUF_SIZE, "Pressure         : %d", in->pressure);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->pressure = in->pressure;
  }
  yPos += spacing;

  if (in->majorTouchAxis != out->majorTouchAxis){
    snprintf(buffer, BUF_SIZE, "Major Touch Axis : %d", in->majorTouchAxis);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->majorTouchAxis = in->majorTouchAxis;
  }
  yPos += spacing;

  if (in->minorTouchAxis != out->minorTouchAxis){
    snprintf(buffer, BUF_SIZE, "Minor Touch Axis : %d", in->minorTouchAxis);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->minorTouchAxis = in->minorTouchAxis;
  }
  yPos += spacing;

  if (in->orientation != out->orientation){
    snprintf(buffer, BUF_SIZE, "Orientation      : %d", in->orientation);
    clearRenderText(screen, font, buffer,  xPos, yPos, clearSize);
    out->orientation = in->orientation;
  }
  yPos += spacing;
}

int main(void){
  remarkable_framebuffer *screen = remarkable_framebuffer_init("/dev/fb0");
  FT_Library  libFT;
  FT_Face     font;
  FT_Face     fontMono;

  remarkable_lut16Init();
  remarkable_multitouch_init();
  memset(multitouchSlotsLastValues_, 0, sizeof(multitouchSlotsLastValues_));
   
  if (FT_Init_FreeType(&libFT)){
    printf("Freetype lib init failed!\n");
    return 1;
  }

  if (FT_New_Face(libFT, "/usr/share/fonts/ttf/noto/NotoSans-Bold.ttf", 0, &font)){
    printf("Error while loading font");
    return 1;
  }

  if (FT_New_Face(libFT, "/usr/share/fonts/ttf/noto/NotoMono-Regular.ttf", 0, &fontMono)){
    printf("Error while loading font");
    return 1;
  }
  
  if (FT_Set_Char_Size(font, 3000, 0, 226, 226)){
    printf("Failed to set font size");
    return 1;
  }

  remarkable_framebuffer_fill(screen, REMARKABLE_BRIGHTEST);
  
  TextBoundingBoundingBox exitBox;
  remarkable_text_render_simple(screen, font, "EXIT", 20, 135, 1, &exitBox);

  if (FT_Set_Char_Size(fontMono, 700, 0, 226, 226)){
    printf("Failed to set font size");
    return 1;
  }

  remarkable_framebuffer_refresh(screen, UPDATE_MODE_FULL, WAVEFORM_MODE_GC16_FAST, 4096, EPDC_FLAG_USE_REMARKABLE_DITHER, 0, DRAWING_QUANT_BIT, 0, 0,0, 1872,1404);

  uint8_t done = 0;
  MultitouchEvent touchEvent;
  while (!done) {
  
    for (uint32_t i = 0; i < MAX_MULTITOUCH_TRACKS; i++) {
      updateSlotGfx(screen, fontMono, i);
    }

    remarkable_multitouch_animate();
    while (remarkable_multitouch_getNextEvent(&touchEvent)){
      //Look for a release above the exit "button"
      if (touchEvent.eventType == MULTITOUCH_EVENT_RELEASE &&
	  exitBox.xMin <= touchEvent.x &&
	  touchEvent.x <= exitBox.xMax && 
	  exitBox.yMin <= touchEvent.y &&
	  touchEvent.y <= exitBox.yMax){
	done = 1;
      }
    }
  }

  return 0;
}
