#include <IRremote.h>
#include "ir.h"

IRsend irsend;  // Hard coded to send on pin A5.
IRrecv irrecv(RECV_PIN);

decode_results results;
int recv_slot = -1;  // The current slot to store an IR value in.

void storeData(int slot, uint8_t type, uint8_t bits, uint32_t value) {
  // Write to EEPROM.
  IRdata ir = {type, bits, value};

  int addr = sizeof(IRdata) * slot;
  EEPROM.put(addr, ir); // Store
  Log.info("Stored IR data at %d (size %d * idx %d).", addr, sizeof(IRdata), slot);
}

int func_ir_dump(String data) {
  IRdata irdata[MAX_IRDATA];
  EEPROM.get(0, irdata);
  Log.info("%d bytes", sizeof(irdata));
  Log.dump(LogLevel::INFO_LEVEL, &irdata, sizeof(irdata));
  Log.printf(LogLevel::INFO_LEVEL, "\r\n");

  return 1;
}

int func_ir_erase(String data) {
  // Erase the IR data in this slot.
  int tmp = atoi(data);
  if (tmp == 255) {
    // Special code that means "erase all".
    EEPROM.clear();

    Log.warn("Cleared EEPROM.");

    return 2;
  } else if (tmp > MAX_IRDATA) {
    Log.error("func_ir_erase - Cannot store more than %d codes.", MAX_IRDATA);
    return -1;  // Cannot store more than 64 codes.
  }

  Log.warn("func_ir_erase - Clearing slot %d.", tmp);
  storeData(tmp, -1, -1, -1);

  return 1;
}

int func_ir_recv(String data) {
  // Store the slot number and activate the IR receiver.
  // The call to `decode` in `loop` will return true if a signal is received.
  // Store the type, value, and bits in memory at this slot.

  int tmp = atoi(data);
  if (tmp > MAX_IRDATA) {
    Log.error("func_ir_recv - Cannot store more than %d codes.", MAX_IRDATA);
    return -1;  // Cannot store more than 64 codes.
  }

  recv_slot = tmp;

  // Start watching for a signal.  This is non-blocking, calling `decode` will let you know if a signal was received.
  // Transmitting a signal will cause the system to stop waiting for an incoming signal.
  irrecv.enableIRIn();
  irrecv.blink13(true);

  Log.info("Watching for an IR signal, will store in slot %d.", recv_slot);

  return 1;
}

int func_ir_send(String data) {
  // Lookup the slot type/value/bits from memory.
  int slot = atoi(data);
  if (slot > MAX_IRDATA) {
    Log.error("func_ir_send - Cannot store more than %d codes.", MAX_IRDATA);
    return -1;
  }

  IRdata ir;
  int addr = sizeof(IRdata) * slot;
  EEPROM.get(addr, ir);

  if (ir.type == 0xFF) {
    Log.warn("func_ir_send - Uninitialized slot.");
    return -2;
  }

  Log.info("func_ir_send - Sending type: %d, bits: %d, value: %lu", ir.type, ir.bits, ir.value);

  // Send immediately.
  switch (ir.type) {
    case 1:
      irsend.sendNEC(ir.value, ir.bits);
      break;
    default: {
      Log.warn("Unrecognized IR type %d. Expecting 1.", ir.type);
      return -3;
    }
  }

  return 1;
}

void ir_setup() {
}

void ir_loop() {
   if (recv_slot != -1) {
    // If a function activated the IR receiver, start checking if a signal is received.
    // Store it in the designated slot.
    if (irrecv.decode(&results)) {
      Log.info("loop - IR received, type: %d, bits: %d, value: %lu", results.decode_type, results.bits, results.value);

      // Write to memory and store in EEPROM.
      storeData(recv_slot, (uint8_t)results.decode_type, (uint8_t)results.bits, results.value);

      recv_slot = -1;  // Reset
      irrecv.blink13(false);
    }
  }
}