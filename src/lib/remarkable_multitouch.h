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

typedef enum {
	      MULTITOUCH_EVENT_NONE = 0,
	      MULTITOUCH_EVENT_CLICK,
	      MULTITOUCH_EVENT_RELEASE,
} MultitouchEventType;

typedef struct {
  MultitouchEventType eventType;
  int32_t x;
  int32_t y;
} MultitouchEvent;

extern MultitouchSlot multitouchSlots_[MAX_MULTITOUCH_TRACKS];

void remarkable_multitouch_init(void);
void remarkable_multitouch_deinit(void);

void remarkable_multitouch_animate(void);

/** 
 * Poll this until it returns false between animations in order to detect events correctly
 * 
 * @param event struct where the event data will be written to 
 * 
 * @return true if events was detected (keep polling)
 */
uint8_t remarkable_multitouch_getNextEvent(MultitouchEvent *event);

#endif //REMARKABLE_MULTITOUCH_H 
