/*
 * Project MobileController
 * Author: Chet Zema
 * Date: August 31, 2025
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "ir.h"
#include "rem.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

int func_motor_start(String data) {
  return 1;
}

int func_motor_end(String data) {
  return 1;
}

// setup() runs once, when the device is first turned on
void setup() {
  ir_setup();

  Particle.function("ir_dump", func_ir_dump);
  Particle.function("ir_erase", func_ir_erase);
  Particle.function("ir_recv", func_ir_recv);
  Particle.function("ir_send", func_ir_send);
  Particle.function("rem_send", func_rem_send);
  Particle.function("motor_start", func_motor_start);
  Particle.function("motor_end", func_motor_end);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  ir_loop();
}
