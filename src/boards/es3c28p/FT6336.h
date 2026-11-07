#ifndef __FT6336_WRAPPER_H__
#define __FT6336_WRAPPER_H__

#include <Arduino.h>
#include <Wire.h>

struct TouchPoint {
  uint16_t x;
  uint16_t y;
};

class FT6336 {
public:
  FT6336(uint8_t sda, uint8_t scl, uint8_t int_pin, uint8_t rst_pin, uint16_t max_x, uint16_t max_y) {
    _sda = sda;
    _scl = scl;
    _int = int_pin;
    _rst = rst_pin;
    _max_x = max_x;
    _max_y = max_y;
    isTouched = false;
    points[0] = {0, 0};
    points[1] = {0, 0};
  }

  void begin() {
    // Inicializar el bus I2C en los pines indicados
    Wire.begin(_sda, _scl);
    
    // Configurar pin de interrupción
    pinMode(_int, INPUT_PULLUP);
    
    // Resetear físicamente el chip táctil (activo en LOW)
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(200);
  }

  void setRotation(uint8_t r) {
    // La rotación se maneja mediante mapeo de coordenadas
  }

  void read() {
    isTouched = false;
    
    // Iniciar lectura en el registro TD_STATUS (0x02)
    Wire.beginTransmission(0x38);
    Wire.write(0x02);
    if (Wire.endTransmission() != 0) {
      return; // Error de comunicación I2C (sale sin bloquear)
    }
    
    // Solicitar 5 bytes (0x02 TD_STATUS, 0x03 P1_XH, 0x04 P1_XL, 0x05 P1_YH, 0x06 P1_YL)
    uint8_t read_bytes = Wire.requestFrom((uint8_t)0x38, (uint8_t)5);
    if (read_bytes < 5) {
      return; // Datos insuficientes, salir
    }
    
    uint8_t td_status = Wire.read();
    uint8_t x_h = Wire.read();
    uint8_t x_l = Wire.read();
    uint8_t y_h = Wire.read();
    uint8_t y_l = Wire.read();
    
    uint8_t touch_count = td_status & 0x0F;
    if (touch_count > 0 && touch_count <= 2) {
      isTouched = true;
      points[0].x = ((x_h & 0x0F) << 8) | x_l;
      points[0].y = ((y_h & 0x0F) << 8) | y_l;
    }
  }

  bool isTouched;
  TouchPoint points[2];

private:
  uint8_t _sda, _scl, _int, _rst;
  uint16_t _max_x, _max_y;
};

#endif // __FT6336_WRAPPER_H__