#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

#define FREQUENCY     439987500 // 439.9875 MHz
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

#define SERIAL_BAUD   38400

#define RFM69_CS      10
#define RFM69_IRQ     2
#define RFM69_IRQN    0  // Pin 2 is IRQ 0!
#define RFM69_RST     9

#define PTT_LED       8
#define STATUS_LED    7

#define FLASH_DELAY   32000U

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

String message = "";
boolean messageComplete = false;

bool led = false;
uint32_t count = 0U;

void setup() {
  while (!Serial); // wait until serial console is open, remove if not tethered to computer
  Serial.begin(SERIAL_BAUD);

  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(RF69_433MHZ,0,0);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(5); // power output ranges from 0 (5dBm) to 31 (20dBm)

  // Set to ham POCSAQ frequency
  radio.setFrequency(FREQUENCY);

  // Disable encryption
  radio.encrypt(0);
  message.reserve(512); // reserve 512 bytes just to be sure

  // Initialize LED pins
  pinMode(PTT_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
}

void loop() {
  count++;
  if (count > FLASH_DELAY) {
    digitalWrite(STATUS_LED, led ? LOW : HIGH);
    led = !led;
    count = 0U;
  }
  if (messageComplete) {
    char cmessage[512];
    digitalWrite(PTT_LED, HIGH);
    message.toCharArray(cmessage,message.length() + 1);
    radio.send(0, cmessage, strlen(cmessage), 0);
    message = "";
    messageComplete = false;
    digitalWrite(PTT_LED, LOW);
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar != 0x17) { // end of transmission
      message += (inChar ^= 0xFF); // RFM69 needs the data inverted, hence XOR 0xFF
    }
    else {
      messageComplete = true;
    }
  }
}
