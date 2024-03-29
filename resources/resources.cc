#include "resources.h"

namespace pfm2sid::resources {

static const uint8_t lcd_char_note_data[8] = { 0x02, 0x03, 0x03, 0x02, 0x02, 0x0e, 0x1e, 0x0c };
static const uint8_t lcd_char_bend_data[8] = { 0x00, 0x00, 0x00, 0x04, 0x04, 0x0e, 0x11, 0x1f };
static const uint8_t lcd_char_tri_data[8] = { 0x00, 0x00, 0x08, 0x15, 0x02, 0x00, 0x00, 0x00 };
static const uint8_t lcd_char_saw_data[8] = { 0x00, 0x03, 0x05, 0x09, 0x11, 0x00, 0x00, 0x00 };
static const uint8_t lcd_char_square_data[8] = { 0x00, 0x1c, 0x14, 0x05, 0x07, 0x00, 0x00, 0x00 };

const uint8_t * const lcd_chars[LCD_CHAR_LAST] = {
  // LCD_CHAR_NOTE
  lcd_char_note_data, 
  // LCD_CHAR_BEND
  lcd_char_bend_data, 
  // LCD_CHAR_TRI
  lcd_char_tri_data, 
  // LCD_CHAR_SAW
  lcd_char_saw_data, 
  // LCD_CHAR_SQUARE
  lcd_char_square_data, 
};

} // namespace pfm2sid::resources
