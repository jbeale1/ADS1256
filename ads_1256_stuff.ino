// from Matt Bilsky https://github.com/mbilsky/TeensyADS1256

void initADS() {
  attachInterrupt(ADS_RDY_PIN, DRDY_Interrupt, FALLING);

  digitalWrite(ADS_RST_PIN, LOW);
  delay(10); // LOW at least 4 clock cycles of onboard clock. 100 microseconds is enough
  digitalWrite(ADS_RST_PIN, HIGH); // now reset to default values

  delay(500);

  //now reset the ADS
  Reset();

  //let the system power up and stabilize (datasheet pg 24)
  delay(1000);
  //this enables the buffer which gets us more accurate voltage readings
  SetRegisterValue(STATUS,B00110010);

  //next set the mux register
  //we are only trying to read differential values from pins 0 and 1. your needs may vary.
  //this is the default setting so we can just reset it
  SetRegisterValue(MUX,MUX_RESET); //set the mux register
   //B00001000 for single ended measurement

  // ADCON register: 0 CLK1 CLK0 SDCS1 SDCS0 PGA2 PGA1 PGA0
  // controls D0/CLKOUT flag, Sensor Detect current source, and PGA gain settings
  // SetRegisterValue(ADCON, PGA_64); //set the adcon register
  SetRegisterValue(ADCON, PGA_1); //set the adcon register

  // SetRegisterValue(DRATE, DR_30000); //set the drate register
  SetRegisterValue(DRATE, DR2_5); //set the drate register

  delay(3000);  // settling time <2 seconds: first few readings have offset

  SendCMD(SELFCAL); //send the calibration command

  delay(1);
  waitforDRDY(); // Wait until DRDY is LOW => calibration complete

  uint8_t off0 = GetRegisterValue(OFC0);
  uint8_t off1 = GetRegisterValue(OFC1);
  uint8_t off2 = GetRegisterValue(OFC2);
  uint8_t fs0 = GetRegisterValue(FSC0);
  uint8_t fs1 = GetRegisterValue(FSC1);
  uint8_t fs2 = GetRegisterValue(FSC2);
  int32_t offsetCal = (off2<<16 & 0x00FF0000) | off1<<8 | off0;
  if (offsetCal & 0x00800000) offsetCal |= 0xFF000000;  // sign-extend from 24 bits
  
  uint32_t fsCal = (fs2<<16 & 0x00FF0000) | fs1<<8 | fs0;
  Serial.print("# Offset Cal: ");
  Serial.print(offsetCal);
  Serial.print(", Fullscale Cal: ");
  Serial.print(fsCal);
  Serial.println();
  
} // end initADS()

// ----------------------------------------------------------
//function to read a value
//this assumes that we are not changing the mux action
int32_t read_Value() {
  
  int32_t adc_val;

  waitforDRDY(); // Wait until DRDY is LOW
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1));
  digitalWriteFast(ADS_CS_PIN, LOW); //Pull SS Low to Enable Communications with ADS1247
  
  SPI.transfer(RDATA);  // Issue read data command RDATA
  delayMicroseconds(7);
  
    // assemble 3 bytes into one 32 bit word
  adc_val = ((uint32_t)SPI.transfer(NOP) << 16) & 0x00FF0000;
  adc_val |= ((uint32_t)SPI.transfer(NOP) << 8);
  adc_val |= SPI.transfer(NOP);

  digitalWriteFast(ADS_CS_PIN, HIGH);
  SPI.endTransaction();

    //  Extend a signed number
  if (adc_val & 0x800000)
    {
      adc_val |= 0xFF000000;
    }
    
  return adc_val;
}

//library files

volatile int DRDY_state = HIGH;

void waitforDRDY() {
  while (DRDY_state) {
    continue;
  }
  noInterrupts();
  DRDY_state = HIGH;
  interrupts();
}

//Interrupt function
void DRDY_Interrupt() {
  DRDY_state = LOW;
}

long GetRegisterValue(uint8_t regAdress) {
  uint8_t bufr;
  digitalWriteFast(ADS_CS_PIN, LOW);
  delayMicroseconds(10);
  SPI.transfer(RREG | regAdress); // send 1st command byte, address of the register
  SPI.transfer(0x00);     // send 2nd command byte, read only one register
  delayMicroseconds(10);
  bufr = SPI.transfer(NOP); // read data of the register
  delayMicroseconds(10);
  digitalWriteFast(ADS_CS_PIN, HIGH);
  //digitalWrite(_START, LOW);
  SPI.endTransaction();
  return bufr;

}

void SendCMD(uint8_t cmd) {
  waitforDRDY();
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with 4Mhz clock, MSB first, SPI Mode0
  digitalWriteFast(ADS_CS_PIN, LOW);
  delayMicroseconds(10);
  SPI.transfer(cmd);
  delayMicroseconds(10);
  digitalWriteFast(ADS_CS_PIN, HIGH);
  SPI.endTransaction();
}

void Reset() {
  SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with  clock, MSB first, SPI Mode1
  digitalWriteFast(ADS_CS_PIN, LOW);
  delayMicroseconds(10);
  SPI.transfer(RESET); //Reset
  delay(2); //Minimum 0.6ms required for Reset to finish.
  SPI.transfer(SDATAC); //Issue SDATAC
  delayMicroseconds(100);
  digitalWriteFast(ADS_CS_PIN, HIGH);
  SPI.endTransaction();
}

void SetRegisterValue(uint8_t regAdress, uint8_t regValue) {

  uint8_t regValuePre = GetRegisterValue(regAdress);
  if (regValue != regValuePre) {
    delayMicroseconds(10);
    waitforDRDY();
    SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE1)); // initialize SPI with SPI_SPEED, MSB first, SPI Mode1
    digitalWriteFast(ADS_CS_PIN, LOW);
    delayMicroseconds(10);
    SPI.transfer(WREG | regAdress); // send 1st command byte, address of the register
    SPI.transfer(0x00);   // send 2nd command byte, write only one register
    SPI.transfer(regValue);         // write data (1 Byte) for the register
    delayMicroseconds(10);
    digitalWriteFast(ADS_CS_PIN, HIGH);

    // problem with readback is STATUS reg changes with read-only bit0 (DRDY/ status)
/*
    if (regValue != GetRegisterValue(regAdress)) {   //Check if write was succesful
      Serial.print("Readback of Register 0x");
      Serial.print(regAdress, HEX);
      Serial.println(" different from commanded value!");
    }
    */
    SPI.endTransaction();

  }
}
