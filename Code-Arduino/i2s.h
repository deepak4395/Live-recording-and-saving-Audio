#include <driver/i2s.h>

void setupMic() {
  Serial.println("Configuring I2S...");
  esp_err_t err;
  i2s_driver_uninstall((i2s_port_t)I2S_PORT_NUMBER);
  // The I2S config as per the example
  i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t  (I2S_SAMPLE_BITS), // could only get it to work with 32bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // although the SEL config should be left, it seems to transmit on right
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 64,                           // number of buffers
    .dma_buf_len = 1024,                     // samples per buffer
    .use_apll = 1
  };
  // The pin config as per the setup
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,  // IIS_SCLK
    .ws_io_num = I2S_WS,   // IIS_LCLK
    .data_out_num = I2S_PIN_NO_CHANGE,// IIS_DSIN
    .data_in_num = I2S_SD  // IIS_DOUT
  };
  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install((i2s_port_t)I2S_PORT_NUMBER, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin((i2s_port_t)I2S_PORT_NUMBER, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  if (I2S_CHANNEL_NUM == 2)
  {
    err = i2s_set_clk((i2s_port_t)I2S_PORT_NUMBER, I2S_SAMPLE_RATE, (i2s_bits_per_sample_t)I2S_SAMPLE_BITS, I2S_CHANNEL_STEREO);
    if (err != ESP_OK) {
      Serial.printf("Failed setting CLK: %d\n", err);
      while (true);
    }
  }
  Serial.println("I2S driver installed.");
}

void i2s_adc_data_scale(uint8_t * d_buff, uint8_t* s_buff, uint32_t len) {
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 2) {
    dac_value = (((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0])));
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048;
  }
}

void i2sRawData() {
  int flash_wr_size = 0;
  size_t bytes_read;
  char* i2s_read_buff = (char*) calloc(I2S_READ_LENGTH, sizeof(char));
  flash_write_buff = (uint8_t*) calloc(I2S_READ_LENGTH, sizeof(char));
  i2s_read((i2s_port_t)I2S_PORT_NUMBER, (void*) i2s_read_buff, I2S_READ_LENGTH, &bytes_read, portMAX_DELAY);
  Serial.println("*************** Recording Start ***************");
  long timeRecord = millis();
  while (flash_wr_size < FLASH_RECORD_SIZE) {
    //read data from I2S bus, in this case, from ADC.
    i2s_read((i2s_port_t)I2S_PORT_NUMBER, (char*) i2s_read_buff, I2S_READ_LENGTH, &bytes_read, portMAX_DELAY);
    //save original data from I2S(ADC) into flash.
    i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, I2S_READ_LENGTH);
    flash_wr_size += I2S_READ_LENGTH;
    // Send binary packet
    webSocket.sendBIN(0, (uint8_t *)flash_write_buff, I2S_READ_LENGTH);
    writeFile();
  }
  Serial.println("Record Loop time: ");
  Serial.println((millis() - timeRecord) / 1000);
  Serial.println("*************** Recording Done. ***************");
  free(i2s_read_buff);
  i2s_read_buff = NULL;
  free(flash_write_buff);
  flash_write_buff = NULL;
}
