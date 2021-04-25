#include <SD.h>
#include <SPI.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setupSDC() {
  while (!SD.begin()) {
    Serial.print(".");
    delay(500);
  }
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);
}

byte header[44];
void waveHeader() {
  uint32_t fileSize = FLASH_RECORD_SIZE + 36;
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';

  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);

  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';

  header[16] = 0x10;
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;

  ///********** Type of Format  **********///
  header[20] = 0x01;
  header[21] = 0x00;

  ///**********   Num Channels  **********///
  header[22] = (I2S_CHANNEL_NUM & 0xFF);
  header[23] = ((I2S_CHANNEL_NUM >> 8) & 0xFF);

  ///**********   Smaple Rate   **********///
  header[24] = (byte)(I2S_SAMPLE_RATE & 0xFF);
  header[25] = (byte)((I2S_SAMPLE_RATE >> 8) & 0xFF);
  header[26] = (byte)((I2S_SAMPLE_RATE >> 16) & 0xFF);
  header[27] = (byte)((I2S_SAMPLE_RATE >> 24) & 0xFF);

  ///**********    Byte Rate    **********///
  uint32_t byteRate = (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8);
  header[28] = (byte)(byteRate & 0xFF);
  header[29] = (byte)((byteRate >> 8) & 0xFF);
  header[30] = (byte)((byteRate >> 16) & 0xFF);
  header[31] = (byte)((byteRate >> 24) & 0xFF);

  ///**********   Block Align   **********///
  uint16_t blockAlign = (I2S_SAMPLE_BITS * I2S_CHANNEL_NUM / 8);
  header[32] = (byte)(blockAlign & 0xFF);
  header[33] = (byte)((blockAlign >> 8) & 0xFF);

  ///********** Bits per sample **********///
  header[34] = (byte)(I2S_SAMPLE_BITS & 0xFF);
  header[35] = (byte)((I2S_SAMPLE_BITS >> 8) & 0xFF);

  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';

  ///********** Bits per sample **********///
  header[40] = (byte)(FLASH_RECORD_SIZE & 0xFF);
  header[41] = (byte)((FLASH_RECORD_SIZE >> 8) & 0xFF);
  header[42] = (byte)((FLASH_RECORD_SIZE >> 16) & 0xFF);
  header[43] = (byte)((FLASH_RECORD_SIZE >> 24) & 0xFF);
}
File currentOpenFile;
void openFile() {
  loadCounter();
  String fN = filename + String(counter) + format;
  Serial.println("Writing to file: " + fN);
  if (SD.remove(fN)) Serial.println("File removed");
  currentOpenFile = SD.open(fN.c_str(), FILE_WRITE);
  if (!currentOpenFile) {
    Serial.println("Error: Unable to open file");
  }
  listDir(SD, "/", 0);
  waveHeader();
  if (!currentOpenFile.write(header, 44))Serial.println("Failed to write");
}
void writeFile() {
  if (!currentOpenFile.write((const byte*) flash_write_buff, I2S_READ_LENGTH))
    Serial.println("failed to write");
}

void closeFile() {
  currentOpenFile.close();
  Serial.println("File Closed");
  delay(500);
  listDir(SD, "/", 0);
}
