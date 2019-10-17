#ifndef REMARKABLE_MULTITOUCH_H
#define REMARKABLE_MULTITOUCH_H

#include <stdint.h>


#ifndef MAX_MULTITOUCH_TRACKS
#define MAX_MULTITOUCH_TRACKS (10)
#endif

#define MULTITOUCH_UNDEFINED (-100)


typedef struct {
  int32_t trackingId;
  int32_t xRaw;
  int32_t yRaw;
  int32_t x;
  int32_t y;
  int32_t pressure;
  int32_t majorTouchAxis;
  int32_t minorTouchAxis;
  int32_t orientation;
} MultitouchSlot;

extern MultitouchSlot multitouchSlots_[MAX_MULTITOUCH_TRACKS];

void remarkable_multitouch_init(void);
void remarkable_multitouch_deinit(void);

void remarkable_multitouch_getTouchEvent(void);

#endif //REMARKABLE_MULTITOUCH_H 
