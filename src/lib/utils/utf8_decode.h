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

/** 
 * Converts an utf8 string to unicode 
 * 
 * @param input the input utf8 string 
 * @param output the output buffer
 * @param maxLength 
 * 
 * @return true if the conversion completed without errors
 */
uint8_t utf8StringToUnicodeN(const uint8_t *input, uint32_t *output, uint32_t maxLength);

#endif //UTF8_DECODE_H 
