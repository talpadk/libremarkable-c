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
 * Initializes the lut16
 * 
 */
void remarkable_lut16Init(void);

#endif //REMARKABLE_LUT_H 
