#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(81);

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload);
        // Test if parsing succeeds.
        if (error) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
          //broadCastPID.attach(1, send_sensor);
          return;
        }
        COMMANDS_RECORD = doc["COMMANDS_RECORD"]; // 1-Start or 0-Stop
        I2S_SAMPLE_RATE = doc["I2S_SAMPLE_RATE"]; // 96000 sample/sec
        I2S_SAMPLE_BITS = doc["I2S_SAMPLE_BITS"]; // 16    bit/sample
        RECORDINGS_TIME = doc["RECORDINGS_TIME"]; // 10    sec
        I2S_CHANNEL_NUM = doc["I2S_CHANNEL_NUM"]; // 01    Mono
        I2S_PORT_NUMBER = doc["I2S_PORT_NUMBER"]; // 00    I2S_0
        Serial.println("*************** Recive Command ***************");
        Serial.printf("COMMANDS_RECORD : %d \r\n", COMMANDS_RECORD);
        Serial.printf("I2S_SAMPLE_RATE : %d \r\n", I2S_SAMPLE_RATE);
        Serial.printf("I2S_SAMPLE_BITS : %d \r\n", I2S_SAMPLE_BITS);
        Serial.printf("RECORDINGS_TIME : %d \r\n", RECORDINGS_TIME);
        Serial.printf("I2S_CHANNEL_NUM : %d \r\n", I2S_CHANNEL_NUM);
        Serial.printf("I2S_PORT_NUMBER : %d \r\n", I2S_PORT_NUMBER);
        Serial.println("**********************************************");
        FLASH_RECORD_SIZE = (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORDINGS_TIME);
      }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void wsSetup() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void wsLoop() {
  webSocket.loop();
}
