#include <Arduino.h>

const uint8_t FreeIcons24pt7Bitmaps[] PROGMEM = {
  0x20, 0x7E, 0x04, 0x13, 0xFF, 0xC8, 0x0F, 0xC3, 0xF0, 0x1E, 0x00, 0x78,
  0x3A, 0x00, 0x5C, 0x71, 0x7E, 0x8E, 0xE3, 0xFF, 0xC7, 0xC7, 0xC3, 0xE3,
  0x0E, 0x24, 0x70, 0x1C, 0x18, 0x38, 0x18, 0xFF, 0x18, 0x01, 0xE7, 0x80,
  0x03, 0x81, 0xC0, 0x03, 0x18, 0xC0, 0x02, 0x3C, 0x40, 0x04, 0x7E, 0x20,
  0x08, 0x7E, 0x10, 0x10, 0x3C, 0x08, 0x20, 0x18, 0x04, 0x00, 0x7E, 0x00,
  0x03, 0xFF, 0xC0, 0x0F, 0xC3, 0xF0, 0x1E, 0x00, 0x78, 0x38, 0x00, 0x1C,
  0x70, 0x7E, 0x0E, 0xE3, 0xFF, 0xC7, 0xC7, 0x81, 0xE3, 0x0E, 0x00, 0x70,
  0x1C, 0x18, 0x38, 0x18, 0xFF, 0x18, 0x01, 0xE7, 0x80, 0x03, 0x81, 0xC0,
  0x03, 0x18, 0xC0, 0x00, 0x3C, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00,
  0x00, 0x3C, 0x00, 0x00, 0x18, 0x00
};

const GFXglyph FreeIcons24pt7Glyphs[] PROGMEM = {
  {     0,   0,   0,  12,    0,    1 },   // 0x20 ' '
  {     0,  24,  19,  26,    1,  -18 },   // 0x21 '!'
  {    57,   0,  12,  16,    2,  -32 },   // 0x22 '"'
  {    57,  24,  19,  26,    1,  -18 },   // 0x23 '#'
  {   114,   0,  41,  26,    1,  -39 },   // 0x24 '$'
  {   114,   0,  22,  28,    1,  -33 }    // 0x25 '%'
};

const GFXfont FreeIcons24pt7 PROGMEM = {(uint8_t *)FreeIcons24pt7Bitmaps,
                                        (GFXglyph *)FreeIcons24pt7Glyphs, 0x20, 0x25,        56};
// Approx. 8136 bytes