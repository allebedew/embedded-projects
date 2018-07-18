
// #include "U8glib.h"

// U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);
// char * strings[4] = {"1", "2", "3", "4"};

// void displayBegin() {
//   u8g.setHardwareBackup(u8g_backup_avr_spi);
//   u8g.setColorIndex(1);
// }

// void displayOnLoop() {
//   u8g.firstPage();  
//   do {

//     u8g.setFont(u8g_font_04b_03r);
//     u8g.drawStr(0, 8, strings[0]);
//     u8g.drawStr(0, 16, strings[1]);
//     u8g.drawStr(0, 24, strings[2]);
//     u8g.drawStr(0, 32, strings[3]);

//   //   char buf[9];
//   //   sprintf (buf, "%d", c);
//   //   u8g.drawStr(64, 24, buf);

//   } while( u8g.nextPage() );
// }

// void displayPrint(int line, char * string) {
//   strings[line] = string;
// }
