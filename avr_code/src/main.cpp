/*

Powerblinker:

Blink LEDs as pulses from the smartmeter are received

*/

#include <RF12.h>
#include <Ports.h>
#include <avr/wdt.h>   
#include <WProgram.h>

#define DEBUG_ENABLED

#define LED1 4 // Port 1: counter1: blue
#define LED2 5 // Port 2: counter2: green
#define LED3 6 // Port 3: counter3: yellow
#define LED4 7 // Port 4: lost: red

#define NODE_ID 1 // monitor mode: pick up packets for all nodes
#define NETGROUP 42


MilliTimer led1Timer;
MilliTimer led2Timer;
MilliTimer led3Timer;
MilliTimer led4Timer;

typedef struct {
    uint32_t counter_millis;
    uint32_t active_counter;
} smartmeter_pulse_t;
smartmeter_pulse_t smartmeter_pulse;

uint32_t last_smartmeter_seq;
uint32_t last_smartmeter_millis = 0;

void printSmartmeterPulse() {
  smartmeter_pulse = *(smartmeter_pulse_t*) rf12_data;
  int seq_diff;
  int milli_diff;

  if (last_smartmeter_seq) {
    seq_diff = rf12_seq - last_smartmeter_seq;
    if (seq_diff > 1) {
      #ifdef DEBUG_ENABLED
      Serial.print("ALARM: Smartmeter LOST ");
      Serial.print(seq_diff - 1, DEC);
      Serial.print(" PULSE(S)!");
      Serial.println();
      #endif
      digitalWrite(LED4, HIGH);
      led4Timer.set(90);
    }
  }
  last_smartmeter_seq = rf12_seq;
  milli_diff = smartmeter_pulse.counter_millis - last_smartmeter_millis;
  last_smartmeter_millis = smartmeter_pulse.counter_millis;

  #ifdef DEBUG_ENABLED
  Serial.print(" Smartmeter counted pulse on meter: ");
  Serial.print(smartmeter_pulse.active_counter + 1, DEC);
  Serial.print(", millis: ");
  Serial.print(smartmeter_pulse.counter_millis);
  Serial.print(", diff: ");
  Serial.println(milli_diff);
  #endif

  if (smartmeter_pulse.active_counter == 0) {
    digitalWrite(LED1, HIGH);
    led1Timer.set(90);
  }
  if (smartmeter_pulse.active_counter == 1) {
    digitalWrite(LED2, HIGH);
    led2Timer.set(90);
  }
  if (smartmeter_pulse.active_counter == 2) {
    digitalWrite(LED3, HIGH);
    led3Timer.set(90);
  }
}

void printUnknown() {
  #ifdef DEBUG_ENABLED
  int rf12_node_id;
  rf12_node_id = 0x1F & rf12_hdr;
  Serial.print("Unknown node: id: ");
  Serial.print(rf12_node_id, DEC);
  Serial.print(", hdr: 0b");
  Serial.print(rf12_hdr, BIN);
  Serial.print(" (0x");
  Serial.print(rf12_hdr, HEX);
  Serial.print(")");
  Serial.print(", seq: ");
  Serial.print(rf12_seq, DEC);
  Serial.print(", length: ");
  Serial.println(rf12_len, DEC);
  #endif
}


void receiveRF12() {
  int rf12_node_id;
  if (rf12_recvDone() && (rf12_crc == 0)) {
    rf12_node_id = 0x1F & rf12_hdr;
    if ((rf12_node_id == 5) && (rf12_len == sizeof(smartmeter_pulse_t))) {
      printSmartmeterPulse();
    } else { // unknown node ID
      printUnknown();
    }
  }
}

void setup() {

  #ifdef DEBUG_ENABLED
  // initialize serial communication:
  Serial.begin(57600);
  Serial.println("[Alis Smartmeter - Blinker]");
  Serial.println("initializing RFM12");
  #endif

	rf12_initialize(NODE_ID, RF12_868MHZ, NETGROUP);
	rf12_encrypt(RF12_EEPROM_EKEY);

  #ifdef DEBUG_ENABLED
  Serial.println("initializing LEDs");
  #endif

  // initialize the LEDs as outputs:
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
 
  for (int i = 0; i < 2; i++) {
    delay(90);
    digitalWrite(LED1, HIGH);
    delay(90);
    digitalWrite(LED1, LOW);
    delay(90);
    digitalWrite(LED2, HIGH);
    delay(90);
    digitalWrite(LED2, LOW);
    delay(90);
    digitalWrite(LED3, HIGH);
    delay(90);
    digitalWrite(LED3, LOW);
    delay(90);
    digitalWrite(LED4, HIGH);
    delay(90);
    digitalWrite(LED4, LOW);
  }
/*
*/

  #ifdef DEBUG_ENABLED
  Serial.println("initialization done");
  wdt_enable(WDTO_1S);
  #else
  wdt_enable(WDTO_60MS); // sending a packet of 64bit length takes about ??ms
  #endif
}


void loop() {
  receiveRF12();
  if (led1Timer.poll()) {
    digitalWrite(LED1, LOW);
  }
  if (led2Timer.poll()) {
    digitalWrite(LED2, LOW);
  }
  if (led3Timer.poll()) {
    digitalWrite(LED3, LOW);
  }
  if (led4Timer.poll()) {
    digitalWrite(LED4, LOW);
  }
  wdt_reset();
  
}










// vim: expandtab sw=2 ts=2
