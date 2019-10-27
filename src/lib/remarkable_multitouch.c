#include "remarkable_multitouch.h"

#include <linux/input.h>
#include <linux/input-event-codes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


MultitouchSlot multitouchSlots_[MAX_MULTITOUCH_TRACKS];
static MultitouchSlot multitouchEventBufferSlots_[MAX_MULTITOUCH_TRACKS];

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

    multitouchEventBufferSlots_[i].trackingId     = -1;
    multitouchEventBufferSlots_[i].xRaw           = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].yRaw           = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].x              = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].y              = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].pressure       = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].majorTouchAxis = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].minorTouchAxis = MULTITOUCH_UNDEFINED;
    multitouchEventBufferSlots_[i].orientation    = MULTITOUCH_UNDEFINED;

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


void remarkable_multitouch_animate(void){
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

	  multitouchEventBufferSlots_[currentSlot_].x = multitouchSlots_[currentSlot_].x; //Update the event detection buffers as well
	}
	else if (inputEvent.code == ABS_MT_POSITION_Y){
	  multitouchSlots_[currentSlot_].yRaw = inputEvent.value;
	  multitouchSlots_[currentSlot_].y    = ((1023-multitouchSlots_[currentSlot_].yRaw)*1871)/1023;
	  if (multitouchSlots_[currentSlot_].y < 0)    { multitouchSlots_[currentSlot_].y = 0; }
	  if (multitouchSlots_[currentSlot_].y > 1871) { multitouchSlots_[currentSlot_].y = 1871; }

	  multitouchEventBufferSlots_[currentSlot_].y = multitouchSlots_[currentSlot_].y; //Update the event detection buffers as well
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
	  printf("\tUnhandled multitouch code=%X\n", inputEvent.code);
	}
	*/
      }
    }
  }     
}

uint8_t remarkable_multitouch_getNextEvent(MultitouchEvent *event){
  for (uint32_t i = 0; i < MAX_MULTITOUCH_TRACKS; i++) {
    
    if (multitouchSlots_[i].trackingId != -1 && 
	multitouchEventBufferSlots_[i].trackingId == -1){
      //Touch start, wait for coordinates
      if (multitouchEventBufferSlots_[i].x != MULTITOUCH_UNDEFINED &&
	  multitouchEventBufferSlots_[i].y != MULTITOUCH_UNDEFINED){
	//Mark the event as detected
	multitouchEventBufferSlots_[i].trackingId = multitouchSlots_[i].trackingId;
	//Copy the event data
	event->eventType = MULTITOUCH_EVENT_CLICK;
	event->x = multitouchEventBufferSlots_[i].x;
	event->y = multitouchEventBufferSlots_[i].y;
	return 1;	
      }      
    }
    else if (multitouchSlots_[i].trackingId == -1 && 
	     multitouchEventBufferSlots_[i].trackingId != -1){
      //Touch end, wait for coordinates (we probably always have them by now)
      if (multitouchEventBufferSlots_[i].x != MULTITOUCH_UNDEFINED &&
	  multitouchEventBufferSlots_[i].y != MULTITOUCH_UNDEFINED){
	//Mark the event as detected
	multitouchEventBufferSlots_[i].trackingId = multitouchSlots_[i].trackingId;
	//Copy the event data
	event->eventType = MULTITOUCH_EVENT_RELEASE;
	event->x = multitouchEventBufferSlots_[i].x;
	event->y = multitouchEventBufferSlots_[i].y;
	//Invalidate the coordinates
	multitouchEventBufferSlots_[i].x = MULTITOUCH_UNDEFINED;
	multitouchEventBufferSlots_[i].y = MULTITOUCH_UNDEFINED;
	return 1;	
      }
    }
  }
  //No events found
  event->eventType = MULTITOUCH_EVENT_NONE;
  return 0;
}
