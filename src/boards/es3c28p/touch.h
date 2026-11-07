#ifndef __TOUCH_H__
#define __TOUCH_H__

#include "FT6336.h"

#define TOUCH_FT6336
#define TOUCH_FT6336_SCL 15
#define TOUCH_FT6336_SDA 16
#define TOUCH_FT6336_INT 17
#define TOUCH_FT6336_RST 18

#define TOUCH_MAP_X1 0
#define TOUCH_MAP_X2 240
#define TOUCH_MAP_Y1 0
#define TOUCH_MAP_Y2 320

extern int touch_last_x;
extern int touch_last_y;

void touch_init(unsigned short int w, unsigned short int h, unsigned char r);
bool touch_touched(void);
bool touch_has_signal(void);
bool touch_released(void);

#endif // __TOUCH_H__