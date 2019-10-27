#include "remarkable_lut.h"

uint16_t lut16_[16];
uint16_t lut16Inverse_[16];

void remarkable_lut16Init(void){
  for (uint16_t i = 0; i < 16; i++) {
    uint16_t tmp = i<<1;
    uint16_t value = (tmp<<11)|(tmp<<6)|tmp; //lower bit is left 0 (two lower for G)
    lut16_[i] = value;
    lut16Inverse_[15-i] = value;
  }
}

void remarkable_lut_create(uint16_t lut[16], uint8_t fromColour, uint8_t toColour){
  int32_t from = ((int32_t)fromColour) << 8;
  int32_t incremment = ((((int32_t)toColour) << 8)-from)/16;
  for (uint16_t i = 0; i < 16; i++) {
    if (from < 0       ) { from = 0; }
    if (from > (255<<8)) { from = 255<<8; }
    uint16_t tmp = (((uint32_t)from) >> (8+3));
    uint16_t value = (tmp<<11)|(tmp<<6)|tmp; //lower bit is left 0 for G
    lut[i] = value;
    from += incremment;
  }
}
