/*
 * Original code by http://github.com/benjojo
 * Mucked around with by http://github.com/lexicality
 * MIT License
 */
#include <DueFlashStorage.h>
#include <Arduino.h>

uint32_t id[4];
bool success;

#define RADIO_SERIAL Serial
#define RADIO_SERIAL_BAUD 115200

//------------------------------------------------------------------------------
/** calibration factor for delayMS */
#define CAL_FACTOR (F_CPU/7000)
/** delay between led error flashes
 * \param[in] millis milliseconds to delay
 */
static void delayMS(uint32_t millis) {
  uint32_t iterations = millis * CAL_FACTOR;
  int i;
  for(i = 0; i < iterations; ++i) {
    asm volatile("nop\n\t");
  }
}
//------------------------------------------------------------------------------

void setup() {
	success = (flash_init(FLASH_ACCESS_MODE_128, 4) == FLASH_RC_OK) &&
			  (flash_read_unique_id(id, 4) == FLASH_RC_OK);

	// Setup radio communitcation
	RADIO_SERIAL.begin(RADIO_SERIAL_BAUD);
	// Setup radio pins
	pinMode(SRF_AT_COMMAND, OUTPUT);
	pinMode(SRF_SLEEP, OUTPUT);
	digitalWrite(SRF_SLEEP, LOW);
	delay(100);

	digitalWrite(SRF_AT_COMMAND, LOW);
	RADIO_SERIAL.println("ATZD3");  // output format <payload>|<rssi>
	RADIO_SERIAL.println("ATPK3A"); // 58byte packet length
	RADIO_SERIAL.println("ATCN00"); // Channel
	RADIO_SERIAL.println("ATAC");   // apply
	RADIO_SERIAL.flush();
	delay(100);
	digitalWrite(SRF_AT_COMMAND, HIGH);


	SerialUSB.begin(115200);
	delay(250);
}


void loop() {
	uint8_t availableBytes, i;
	byte packet[256], outPacket[58];
	while(true) {
		availableBytes = RADIO_SERIAL.available();
		if (availableBytes > 0) {
			pinMode(PIN_LED_RXL, OUTPUT);
			digitalWrite(PIN_LED_RXL, 1);
			if (availableBytes > 255) {
				SerialUSB.print(":( ");
				SerialUSB.print(availableBytes - 255, DEC);
				SerialUSB.println(" excess bytes available");
				availableBytes = 255;
			}
			memset(packet, 0, 256);
			RADIO_SERIAL.readBytes(packet, availableBytes);

			SerialUSB.print("Incoming Radio Message ");
			SerialUSB.print(availableBytes);
			SerialUSB.println(" bytes long");
			SerialUSB.print("Hex: ");
			for (i = 0; i < availableBytes; i++) {
				SerialUSB.print( packet[i], HEX );
			}
			SerialUSB.print("\r\n");
			SerialUSB.flush();
			SerialUSB.println("Raw characters:");
			for (i = 0; i < availableBytes; i++) {
				SerialUSB.write( packet[i] );
			}
			SerialUSB.println("\r\n.\r\n"); // Double newline
			SerialUSB.flush();
			digitalWrite(PIN_LED_RXL, 0);
		}

		availableBytes = SerialUSB.available();
		if (availableBytes > 0) {
			pinMode(PIN_LED_TXL, OUTPUT);
			digitalWrite(PIN_LED_TXL, 1);
			if (availableBytes > 58) {
				availableBytes = 58;
			}
			memset(outPacket, '@', 58);
			SerialUSB.readBytes(outPacket, availableBytes);
			SerialUSB.print("Sending Package: ");
			for (i = 0; i < 58; i++) {
				SerialUSB.write( outPacket[i] );
			}
			SerialUSB.print("\r\n");

			RADIO_SERIAL.write(outPacket, 58);
			RADIO_SERIAL.flush();
			digitalWrite(PIN_LED_TXL, 0);
		}
	}
}
