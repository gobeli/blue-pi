/*********************************************************************
*  Script     : ble_piano
*  Author     : Etienne Gobeli & Robin Christen
*  Date       : 11/12/2017
*  Description: Can recive and record frequencies over bluetooth 
*               and plays them on the arduino with a speaker
*********************************************************************
*
* To be continiued...
* 
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE     Perform a factory reset when running this sketch
   
                            Enabling this will put your Bluefruit LE module
                            in a 'known good' state and clear any config
                            data set in previous sketches or projects, so
                            running this at least once is a good idea.
   
                            When deploying your project, however, you will
                            want to disable factory reset by setting this
                            value to 0.  If you are making changes to your
                            Bluefruit LE device via AT commands, and those
                            changes aren't persisting across resets, this
                            is the reason why.  Factory reset will erase
                            the non-volatile memory where config data is
                            stored, setting it back to factory default
                            values.
       
                            Some sketches that require you to bond to a
                            central device (HID mouse, keyboard, etc.)
                            won't work at all with this feature enabled
                            since the factory reset will clear all of the
                            bonding data stored on the chip, meaning the
                            central device won't be able to reconnect.
                            
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features    
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE        1
    #define MINIMUM_FIRMWARE_VERSION   "0.7.0"
/*=========================================================================*/


/* Hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* defining output pin for buzzer */
const int buzzerPin = 9;
const int recordButtonPin = 3;
const int playButtonPin = 10;
const int ledPin = 2;

/* How long a tone should sound */
int tone_duration = 200;

/* recive frequenci in this var */
int32_t charid_string;

int tones[100];

boolean isRecording = false;

void play() {
  int i = 0;
  while (tones[i] != 0) {
    tone(buzzerPin, tones[i], tone_duration);
    i = i + 1;
    delay(tone_duration);
  }
}


void pushTone(int t) {
  int i;
  for (i = 0; i < 100; i = i + 1) {
    if(tones[i] == 0) {
      tones[i] = t;
      break;
    }
  }
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void connected(void) {
  Serial.println( F("Connected") );
}

void disconnected(void) {
  Serial.println( F("Disconnected") );
}


void BleGattRX(int32_t chars_id, uint8_t data[], uint16_t len) {

  Serial.print("Playing Tone: ");
  Serial.println(atoi((char*)data));

  if(isRecording == true) {
    pushTone(atoi((char*)data));
  }

  tone(buzzerPin, atoi((char*)data), tone_duration);
  
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module 
        (this function is called automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);
  Serial.println(F("           BLE Piano"));
  Serial.println(F("-------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }
  
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error( F("Callback requires at least 0.7.0") );
  }

  // add an custom service with a writable characteristic
  Serial.println( F("Adding Service 0x1234 with characterisitc 0x2345") );
  ble.sendCommandCheckOK( F("AT+GATTADDSERVICE=uuid=0x1234") );
  ble.sendCommandCheckOK( F("AT+GAPDEVNAME=Blue-Pi") );
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2345,PROPERTIES=0x08,MIN_LEN=1,MAX_LEN=6,DATATYPE=string,DESCRIPTION=string,VALUE=abc"), &charid_string);

  ble.reset();

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  
  /* Set callbacks to execute when there is an update from central device */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);
  
  /* Only one BLE GATT function should be set, it is possible to set it 
  multiple times for multiple Chars ID  */
  ble.setBleGattRxCallback(charid_string, BleGattRX);

  /* set buzzer pin */
  pinMode(buzzerPin, OUTPUT);

  pinMode(recordButtonPin, INPUT);
  pinMode(playButtonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  memset(tones, 0, sizeof(tones));
}


/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{  

  if(digitalRead(recordButtonPin) == LOW) {
    isRecording = !isRecording;

    if(isRecording == 1) {
      memset(tones, 0, sizeof(tones));
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }

    delay(500);
  
  } else if(digitalRead(playButtonPin) == LOW) {
    play();
    
    delay(500);
  } else {
    ble.update(200);
  }

}

