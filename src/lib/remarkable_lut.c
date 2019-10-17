#include "remarkable_lut.h"

uint16_t lut16_[16];

void remarkable_lut16Init(void){
  for (uint16_t i = 0; i < 16; i++) {
    uint16_t tmp = i<<1;
    uint16_t value = (tmp<<11)|(tmp<<6)|tmp; //lower bit is left 0 (two lower for G)
    lut16_[i] = value;
  }
}
