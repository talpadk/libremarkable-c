#ifndef REMARKABLE_LUT_H
#define REMARKABLE_LUT_H

#include <stdint.h>


/** 
 * A look up table for 16 brightness levels from dark to bright.
 * Must be initialized before use
 * @see remarkable_lut16Init 
 * 
 */
extern uint16_t lut16_[16];

/** 
 * A look up table for 16 brightness levels from bright to dark.
 * This is useful among other things for font rendering 
 * Must be initialized before use
 * @see remarkable_lut16Init 
 * 
 */
extern uint16_t lut16Inverse_[16];

/** 
 * Initializes the lut16
 * 
 */
void remarkable_lut16Init(void);

void remarkable_lut_create(uint16_t lut[16], uint8_t fromColour, uint8_t toColour);

#endif //REMARKABLE_LUT_H 
