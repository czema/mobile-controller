#ifndef IR_H
#define IR_H

const int RECV_PIN = D2;
const int MAX_IRDATA = 64;

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
