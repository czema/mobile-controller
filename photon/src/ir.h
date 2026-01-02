#ifndef IR_H
#define IR_H

#include "Particle.h"

constexpr int IR_RECV_PIN = D6;
constexpr int IR_MAX_DATA = 64;
constexpr int IR_EEPROM_ADDR = 0;

typedef struct {
  uint8_t type;
  uint8_t bits;
  uint32_t value;
} IRdata;

int func_ir_dump(String data);
int func_ir_erase(String data);
int func_ir_recv(String data);
int func_ir_send(String data);
void ir_setup();
void ir_loop();

#endif // IR_H
