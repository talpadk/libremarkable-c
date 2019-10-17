#include "remarkable_multitouch.h"

#include <linux/input.h>
#include <linux/input-event-codes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


MultitouchSlot multitouchSlots_[MAX_MULTITOUCH_TRACKS];

static int eventHandle_;
static int32_t currentSlot_;

void remarkable_multitouch_init(void){
  for (uint32_t i = 0; i < MAX_MULTITOUCH_TRACKS; i++) {
    multitouchSlots_[i].trackingId     = -1;
    multitouchSlots_[i].xRaw           = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].yRaw           = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].x              = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].y              = MULTITOUCH_UNDEFINED;    
    multitouchSlots_[i].pressure       = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].majorTouchAxis = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].minorTouchAxis = MULTITOUCH_UNDEFINED;
    multitouchSlots_[i].orientation    = MULTITOUCH_UNDEFINED;
  }
  currentSlot_ = 0;
  eventHandle_ = open("/dev/input/event1",O_RDONLY);
}

void remarkable_multitouch_deinit(void){
  if ( eventHandle_ >= 0 ) {
    close(eventHandle_);
    eventHandle_ = -1;
  }
}


void remarkable_multitouch_getTouchEvent(void){
  if (eventHandle_<0) { return; }
  struct input_event inputEvent;
  
  if (read(eventHandle_, &inputEvent, sizeof(struct input_event))==sizeof(struct input_event)){
    if (inputEvent.type == EV_ABS){
      if (inputEvent.code == ABS_MT_SLOT){
	currentSlot_ = inputEvent.value;
      }
      else if (currentSlot_ >= 0 && currentSlot_ < MAX_MULTITOUCH_TRACKS) {
	if (inputEvent.code == ABS_MT_POSITION_X){
	  multitouchSlots_[currentSlot_].xRaw = inputEvent.value;
	  multitouchSlots_[currentSlot_].x    = ((767-multitouchSlots_[currentSlot_].xRaw)*1403)/767;
	  if (multitouchSlots_[currentSlot_].x < 0)    { multitouchSlots_[currentSlot_].x = 0; }
	  if (multitouchSlots_[currentSlot_].x > 1403) { multitouchSlots_[currentSlot_].x = 1403; }
	  
	}
	else if (inputEvent.code == ABS_MT_POSITION_Y){
	  multitouchSlots_[currentSlot_].yRaw = inputEvent.value;
	  multitouchSlots_[currentSlot_].y    = ((1023-multitouchSlots_[currentSlot_].yRaw)*1871)/1023;
	  if (multitouchSlots_[currentSlot_].y < 0)    { multitouchSlots_[currentSlot_].y = 0; }
	  if (multitouchSlots_[currentSlot_].y > 1871) { multitouchSlots_[currentSlot_].y = 1871; }
	}
	else if (inputEvent.code == ABS_MT_PRESSURE){
	  multitouchSlots_[currentSlot_].pressure = inputEvent.value; 
	}
	else if (inputEvent.code == ABS_MT_TRACKING_ID){
	  multitouchSlots_[currentSlot_].trackingId = inputEvent.value;
	}
	else if (inputEvent.code == ABS_MT_TOUCH_MAJOR){
	  multitouchSlots_[currentSlot_].majorTouchAxis = inputEvent.value;
	}
	else if (inputEvent.code == ABS_MT_TOUCH_MINOR){
	  multitouchSlots_[currentSlot_].minorTouchAxis = inputEvent.value;
	}
	else if (inputEvent.code == ABS_MT_ORIENTATION){
	  multitouchSlots_[currentSlot_].orientation = inputEvent.value;
	}
	/*else {
	  printf("\tUnhandled code=%X\n", inputEvent.code);
	}
	*/
      }
    }

  }
      
}
