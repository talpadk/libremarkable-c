#ifndef REMARKABLE_BUTTONS_H
#define REMARKABLE_BUTTONS_H

#include <stdint.h>

typedef struct {
  int32_t home;
  int32_t left;
  int32_t right;
  int32_t power;
  int32_t wakeUp;
} RemarkableButtons;

extern RemarkableButtons buttonState_;

void remarkable_buttons_init(void);
void remarkable_buttons_deinit(void);

void remarkable_buttons_animate(void);

#endif //REMARKABLE_BUTTONS_H 
