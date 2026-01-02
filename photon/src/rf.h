#ifndef RF_H
#define RF_H

#include "Particle.h"

constexpr int RF_RECV_PIN = D3;
constexpr int RF_SEND_PIN = D4;
constexpr int RF_MAX_DATA = 64;
constexpr int RF_EEPROM_ADDR = 1024; // IR data is held from 0-512, leave a gap in case it grows.

typedef struct {
  uint8_t protocol;
  uint8_t length;
  uint32_t value;
} RFdata;

int func_rf_dump(String data);
int func_rf_erase(String data);
int func_rf_recv(String data);
int func_rf_send(String data);
void rf_setup();
void rf_loop();

#endif // RF_H
