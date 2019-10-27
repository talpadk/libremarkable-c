#include "remarkable_buttons.h"

#include <linux/input.h>
#include <linux/input-event-codes.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

RemarkableButtons buttonState_;

static int eventHandle_;

void remarkable_buttons_init(void){
  buttonState_.left   = 0;
  buttonState_.right  = 0;
  buttonState_.home   = 0;
  buttonState_.power  = 0;
  buttonState_.wakeUp = 0;

  eventHandle_ = open("/dev/input/event2",O_RDONLY);
}

void remarkable_buttons_deinit(void){
  if ( eventHandle_ >= 0 ) {
    close(eventHandle_);
    eventHandle_ = -1;
  }
}


void remarkable_buttons_animate(void){
  if (eventHandle_<0) { return; }
  struct input_event inputEvent;
  
  if (read(eventHandle_, &inputEvent, sizeof(struct input_event))==sizeof(struct input_event)){
    if (inputEvent.type == EV_KEY){
      if      (inputEvent.code == KEY_HOME)   { buttonState_.home   = inputEvent.value; }
      else if (inputEvent.code == KEY_LEFT)   { buttonState_.left   = inputEvent.value; }
      else if (inputEvent.code == KEY_RIGHT)  { buttonState_.right  = inputEvent.value; }
      else if (inputEvent.code == KEY_HOME)   { buttonState_.home   = inputEvent.value; }
      else if (inputEvent.code == KEY_POWER)  { buttonState_.power  = inputEvent.value; }
      else if (inputEvent.code == KEY_WAKEUP) { buttonState_.wakeUp = inputEvent.value; }
      else {
	printf("\tUnhandled key code=%X\n", inputEvent.code);
      }
    }
  }
}
