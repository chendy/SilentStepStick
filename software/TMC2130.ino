/*
  TMC2130 Example
*/

#include "SPI.h"

// Note: You also have to connect also GND, 5V and VM.
//       A connection diagram can be found in the schematics.
#define EN_PIN   7
#define DIR_PIN  8
#define STEP_PIN 9

#define CS_PIN   10
#define MOSI_PIN 11
#define MISO_PIN 12
#define SCK_PIN  13

//TMC2130 registers
#define WRITE          0x80 // write flag
#define REG_GCONF      0x00
#define REG_GSTAT      0x01
#define REG_IHOLD_IRUN 0x10
#define REG_CHOPCONF   0x6C
#define REG_COOLCONF   0x6D
#define REG_DCCTRL     0x6E
#define REG_DRVSTATUS  0x6F

uint8_t tmc_write(uint8_t cmd, uint32_t data)
{
  uint8_t s;

  digitalWrite(CS_PIN, LOW);

  s = SPI.transfer(cmd);
  SPI.transfer((data>>24UL)&0xFF)&0xFF;
  SPI.transfer((data>>16UL)&0xFF)&0xFF;
  SPI.transfer((data>> 8UL)&0xFF)&0xFF;
  SPI.transfer((data>> 0UL)&0xFF)&0xFF;

  digitalWrite(CS_PIN, HIGH);

  return s;
}

uint8_t tmc_read(uint8_t cmd, uint32_t *data)
{
  uint8_t s;

  tmc_write(cmd, 0UL); //set read address

  digitalWrite(CS_PIN, LOW);

  s = SPI.transfer(cmd);
  *data  = SPI.transfer(0x00)&0xFF;
  *data <<=8;
  *data |= SPI.transfer(0x00)&0xFF;
  *data <<=8;
  *data |= SPI.transfer(0x00)&0xFF;
  *data <<=8;
  *data |= SPI.transfer(0x00)&0xFF;

  digitalWrite(CS_PIN, HIGH);

  return s;
}

void setup()
{
  //set pins
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(MOSI_PIN, OUTPUT);
  digitalWrite(MOSI_PIN, LOW);
  pinMode(MISO_PIN, INPUT);
  digitalWrite(MISO_PIN, HIGH);
  pinMode(SCK_PIN, OUTPUT);
  digitalWrite(SCK_PIN, LOW);

  //init serial port
  Serial.begin(9600); //init serial port and set baudrate
  while(!Serial); //wait for serial port to connect (needed for Leonardo only)
  Serial.println("\nStart...");

  //init SPI
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV128);
  SPI.begin();

  //set TMC2130 config
  tmc_write(WRITE|REG_GCONF,      0x00000001UL); //voltage on AIN is current reference
  tmc_write(WRITE|REG_IHOLD_IRUN, 0x00001010UL); //IHOLD=0x10, IRUN=0x10
  tmc_write(WRITE|REG_CHOPCONF,   0x00008008UL); //native 256 microsteps

  //TMC2130 outputs on (LOW active)
  digitalWrite(EN_PIN, LOW);
}

void loop()
{
  static uint32_t last_time=0;
  uint32_t ms = millis();
  uint32_t data;
  uint8_t s;
    
  if((ms-last_time) > 1000) //run every 1s
  {
    last_time = ms;

    //show REG_GSTAT
    s = tmc_read(REG_GSTAT, &data);
    Serial.print("GSTAT:     0x0");
    Serial.print(data, HEX);
    Serial.print("\t - ");
    Serial.print("Status: 0x");
    Serial.print(s, HEX);
    if(s & 0x01) Serial.print(" reset");
    if(s & 0x02) Serial.print(" error");
    if(s & 0x04) Serial.print(" sg2");
    if(s & 0x08) Serial.print(" standstill");
    Serial.println(" ");

    //show REG_DRVSTATUS
    s = tmc_read(REG_DRVSTATUS, &data);
    Serial.print("DRVSTATUS: 0x");
    Serial.print(data, HEX);
    Serial.print("\t - ");
    Serial.print("Status: 0x");
    Serial.print(s, HEX);
    if(s & 0x01) Serial.print(" reset");
    if(s & 0x02) Serial.print(" error");
    if(s & 0x04) Serial.print(" sg2");
    if(s & 0x08) Serial.print(" standstill");
    Serial.println(" ");
  }

  //make steps
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(10);
}

