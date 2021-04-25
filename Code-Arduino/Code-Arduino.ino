const char* ssid = "M 57"; //Enter SSID
const char* password = "8376918157"; //Enter Password

#define I2S_WS  12
#define I2S_SCK 14
#define I2S_SD  27

long I2S_SAMPLE_RATE = 16000;// 96000 bit/sec
int I2S_SAMPLE_BITS = 16;    // 16 bit/sample
int RECORDINGS_TIME = 15;    // 05 Sec
int I2S_CHANNEL_NUM = 01;    // 01 mono
int I2S_PORT_NUMBER = 00;    // 00 I2S_0

String filename = "/sound-";
String format = ".wav";

// No changes below this
int COMMANDS_RECORD = 0;
int counter = -1;
int FLASH_RECORD_SIZE = 0;
int I2S_READ_LENGTH = 8 * 1024; // (16*1024) = 16384
uint8_t* flash_write_buff;

#include <ArduinoJson.h>
#include "pre.h"
#include "sd.h"
#include "wifi.h"
#include "ws.h"
#include "i2s.h"

void i2scode() {
  if (COMMANDS_RECORD) {
    setupMic();
    openFile();
    i2sRawData();
    closeFile();
    COMMANDS_RECORD = 0;
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  preSetup();
  wifiSetup();
  wsSetup();
  setupSDC();
}

void loop() {
  wsLoop();
  i2scode();
}
