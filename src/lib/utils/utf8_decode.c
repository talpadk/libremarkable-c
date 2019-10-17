#include "utf8_decode.h"

uint32_t utf8StringLength(const char *string){
  uint32_t count = 0;

  while (*string != 0) {
    if (*string & 1<<7){
      //Non 7 bit char
      if ((*string & 0b11100000) == 0b11000000){
	if (((string[1] & 0b11000000) == 0b10000000)){
	  string += 2;
	}
	else { break; }
      }
      else if ((*string & 0b11110000) == 0b11100000){
	if (((string[1] & 0b11000000) == 0b10000000) &&
	    ((string[2] & 0b11000000) == 0b10000000)){
	  string += 3;
	}
	else { break; }
      }
      else if ((*string & 0b11111000) == 0b11110000){
	if (((string[1] & 0b11000000) == 0b10000000) &&
	    ((string[2] & 0b11000000) == 0b10000000) &&
	    ((string[3] & 0b11000000) == 0b10000000)){
	  string += 4;
	}
	else { break; }
      }
      else {
	break; //Unexpected first byte
      }
    }
    else {
      string++;
    }
    count++;
  }

  return count;
}

uint32_t utf8CharToUnicode(const uint8_t *input, uint32_t *output){
    if (*input & 1<<7){
      //Non 7 bit char
      if ((*input & 0b11100000) == 0b11000000){
	if (((input[1] & 0b11000000) == 0b10000000)){
	  *output =
	    (((uint32_t)(input[0]&0b00011111))<<6) |
	    (((uint32_t)(input[1]&0b00111111)));
	  return 2;
	}
	else { *output = 0; return 0;}
      }
      else if ((*input & 0b11110000) == 0b11100000){
	if (((input[1] & 0b11000000) == 0b10000000) &&
	    ((input[2] & 0b11000000) == 0b10000000)){
	  *output =
	    (((uint32_t)(input[0]&0b00001111))<<12) |
	    (((uint32_t)(input[1]&0b00111111))<<6)  |
	    (((uint32_t)(input[2]&0b00111111)));
	  return 3;
	}
	else { *output = 0; return 0; }
      }
      else if ((*input & 0b11111000) == 0b11110000){
	if (((input[1] & 0b11000000) == 0b10000000) &&
	    ((input[2] & 0b11000000) == 0b10000000) &&
	    ((input[3] & 0b11000000) == 0b10000000)){
	  *output =
	    (((uint32_t)(input[0]&0b00000111))<<18) |
	    (((uint32_t)(input[1]&0b00111111))<<12) |
	    (((uint32_t)(input[2]&0b00111111))<<6)  |
	    (((uint32_t)(input[3]&0b00111111)));
	  return 4;
	}
	else { *output = 0; return 0; }
      }
      else {
	*output = 0; return 0;
      }
    }
    else {
      *output = (uint32_t)*input;
      return 1;
    }
}
