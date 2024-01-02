#ifndef PFM2SID_RESOURCES_H_
#define PFM2SID_RESOURCES_H_

#include <cinttypes>

namespace pfm2sid::resources {

enum LCD_CHAR_RESID {
  LCD_CHAR_NOTE,
  LCD_CHAR_BEND,
  LCD_CHAR_TRI,
  LCD_CHAR_SAW,
  LCD_CHAR_SQUARE,
  LCD_CHAR_LAST
}; // enum LCD_CHAR_RESID

extern const uint8_t * const lcd_chars[LCD_CHAR_LAST];

} // namespace pfm2sid::resources

#endif // PFM2SID_RESOURCES_H_
