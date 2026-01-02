#include "rf.h"
#include "RCSwitch.h"

RCSwitch rftrans = RCSwitch();

int rf_recv_slot = -1;  // The current slot to store a RF remote value in.

void store_rf_data(int slot, uint8_t protocol, uint8_t length, uint32_t value) {
  // Write to EEPROM.
  RFdata rf = {protocol, length, value};

  int addr = RF_EEPROM_ADDR + (sizeof(RFdata) * slot);
  EEPROM.put(addr, rf);  // Store
  Log.info("Stored RF data at %d (size %d * idx %d).", addr, sizeof(RFdata), slot);
}

int func_rf_dump(String data) {
  RFdata rfdata[RF_MAX_DATA];
  EEPROM.get(RF_EEPROM_ADDR, rfdata);
  Log.info("%d bytes", sizeof(rfdata));
  Log.dump(LogLevel::INFO_LEVEL, &rfdata, sizeof(rfdata));
  Log.printf(LogLevel::INFO_LEVEL, "\r\n");

  return 1;
}

int func_rf_erase(String data) {
  // Erase the RF data in this slot.
  int slot = atoi(data);
  if (slot == 255) {
    Log.error("Cannot use this function to clear entire EEPROM.  Use `ir_erase(255)` instead.");

    return 2;
  } else if (slot > RF_MAX_DATA) {
    Log.error("func_rf_erase - Cannot store more than %d codes.", RF_MAX_DATA);
    return -1;  // Cannot store more than 64 codes.
  }

  Log.warn("func_rf_erase - Clearing slot %d.", slot);
  store_rf_data(slot, -1, -1, -1);

  return 1;
}

int func_rf_recv(String data) {
  // Store the slot number and activate the RF receiver.
  // Store the protocol, length, and bits in memory at this slot.

  int slot = atoi(data);
  if (slot > RF_MAX_DATA) {
    Log.error("func_rf_recv - Cannot store more than %d codes.", RF_MAX_DATA);
    return -1;  // Cannot store more than 64 codes.
  }

  rf_recv_slot = slot;

  // Start watching for a signal.  This is non-blocking, calling `available` will let you know if a signal was received.
  rftrans.enableReceive();

  Log.info("Watching for an RF signal, will store in slot %d.", rf_recv_slot);

  return 1;
}

int func_rf_send(String data) {
  // Lookup the slot protocol/length/bits from memory.
  int slot = atoi(data);
  if (slot > RF_MAX_DATA) {
    Log.error("func_rf_send - Cannot store more than %d codes.", RF_MAX_DATA);
    return -1;
  }

  RFdata rf;
  int addr = RF_EEPROM_ADDR + (sizeof(RFdata) * slot);
  EEPROM.get(addr, rf);

  if (rf.protocol == 0xFF) {
    Log.warn("func_rf_send - Uninitialized slot.");
    return -2;
  }

  Log.info("func_rf_send - Sending protocol: %d, length: %d, value: %lu", rf.protocol, rf.length, rf.value);

  // Send immediately.
  rftrans.setProtocol(rf.protocol);
  rftrans.send(rf.value, rf.length);

  return 1;
}

void rf_setup() {
  // Enabling the receiver attaches an interrupt.  To prevent issues with other devices needing an interrupt, only enable while its being used.
  // This call configures the pins.  Later uses will call this without a parameter.
  pinMode(RF_RECV_PIN, INPUT_PULLDOWN);
  rftrans.enableReceive(RF_RECV_PIN);
  rftrans.disableReceive();

  // All enabling the transmitter does is configure the pins.  No point in disabling.
  rftrans.enableTransmit(RF_SEND_PIN);
}

unsigned long nextTick = 0;

void rf_loop() {
  // If we are waiting to fill a slot, and there is data available, write the data to the slot and disable the receiver.
  if (rf_recv_slot != -1) {
    if (rftrans.available()) {
      Log.info("Received RF data: protocol: %u, length: %u, delay: %u, value: %lu", rftrans.getReceivedProtocol(), rftrans.getReceivedBitlength(), rftrans.getReceivedDelay(), rftrans.getReceivedValue());

      // Write to memory and store in EEPROM.
      store_rf_data(rf_recv_slot, (uint8_t)rftrans.getReceivedProtocol(), (uint8_t)rftrans.getReceivedBitlength(), rftrans.getReceivedValue());

      rf_recv_slot = -1;  // Reset

      rftrans.resetAvailable();
      rftrans.disableReceive();
    }
  }

  if (nextTick < System.millis()) {
    rftrans.send(6925890, 24);
    nextTick = System.millis() + 5000;
    Log.info("RF sending");
  }
}