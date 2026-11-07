#include "touch.h"
#include <Arduino.h>

int touch_last_x = 0;
int touch_last_y = 0;

static unsigned short int width = 0;
static unsigned short int height = 0;
static unsigned char rotation = 0;

FT6336 ts = FT6336(TOUCH_FT6336_SDA, TOUCH_FT6336_SCL, TOUCH_FT6336_INT, TOUCH_FT6336_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

void touch_init(unsigned short int w, unsigned short int h, unsigned char r) {
  width = w; 
  height = h;
  rotation = r;
  
  ts.begin();
  ts.setRotation(r);
}

bool touch_touched(void) {
  ts.read();
  if (ts.isTouched) {
    // El chip FT6336 tiene un area fisica fija de X: 0..240 e Y: 0..320.
    // Dependiendo de la rotacion seleccionada en pantalla, mapeamos y rotamos las coordenadas correspondientemente.
    if (rotation == 0) {
      touch_last_x = map(ts.points[0].x, 0, 240, 0, width - 1);
      touch_last_y = map(ts.points[0].y, 0, 320, 0, height - 1);
    } 
    else if (rotation == 1) {
      // Rotacion de 90 grados a la derecha + correccion de 180 grados de inversion:
      // Mapeamos los ejes de forma invertida para corregir la orientacion del tactil.
      touch_last_x = map(ts.points[0].y, 0, 320, 0, width - 1);
      touch_last_y = map(ts.points[0].x, 240, 0, 0, height - 1);
    } 
    else if (rotation == 2) {
      // Rotacion de 180 grados:
      // Ambos ejes invertidos
      touch_last_x = map(ts.points[0].x, 240, 0, 0, width - 1);
      touch_last_y = map(ts.points[0].y, 320, 0, 0, height - 1);
    } 
    else if (rotation == 3) {
      // Rotacion de 270 grados:
      touch_last_x = map(ts.points[0].y, 320, 0, 0, width - 1);
      touch_last_y = map(ts.points[0].x, 0, 240, 0, height - 1);
    }
    return true;
  } else {
    return false;
  }
}

bool touch_has_signal(void) {
  return true;
}

bool touch_released(void) {
  return true;
}