#ifndef UTF8_DECODE_H
#define UTF8_DECODE_H

#include <stdint.h>

/** 
 * Returns the number of valid utf8 characters found in a string
 * 
 * @param string the utf8 string to get the length for, any illegal entries will cause the counting to stop
 * 
 * @return the number of characters in the string
 */
uint32_t utf8StringLength(const char *string);

/** 
 * Converts a single utf8 character to unicode
 * 
 * @param input pointer to the utf8 data
 * @param output pointer where the resulting unicode character is to be stored
 * 
 * @return the number of bytes consumed, 0 if an error was encountred
 */
uint32_t utf8CharToUnicode(const uint8_t *input, uint32_t *output);



#endif //UTF8_DECODE_H 
