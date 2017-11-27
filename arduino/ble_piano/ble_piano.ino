/*********************************************************************
*  Script     : ble_piano
*  Author     : Etienne Gobeli & Robin Christen
*  Date       : 11/12/2017
*  Description: Can recive and record frequencies over bluetooth 
*               and plays them on the arduino with a speaker
*********************************************************************
*
* Usage: 
*   Step 1: Start the ble_piano application on the Arduino
*   Step 2: Go to our frontend (gobeli.github.io/blue-pi)
*   Step 3: Connect to 'Blue-pi'
*   Step 4: Start playing
*   
* To record a meoldie:
*   Step 1: Press the record button on the Arduino (Record LED on).
*   Step 6: Play your melodie
*   Step 7: Press play button on the Arduino for replaying your recorded melody or
*   Step 8: Press the record button again to stop the recording (Record LED off)
*   Step 9: Reply your melodiy until you're ready for recording again.
* 
*********************************************************************/

#include <Arduino.h>

// Bluetooth includes
#include "Adafruit_BluefruitLE_SPI.h"
#include "BluefruitConfig.h"

/*=========================================================================
    BLUETOOTH APPLICATION SETTINGS

    FACTORYRESET_ENABLE     Perform a factory reset when running this sketch
                            SET TO 1 FOR TESTING
                            SET TO 0 FOR PRODUCTION
                            
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features    
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE        0
    #define MINIMUM_FIRMWARE_VERSION   "0.7.0"
/*=========================================================================*/


/* Hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* defining used pins */
const int buzzerPin = 9;
const int recordButtonPin = 3;
const int playButtonPin = 10;
const int ledPin = 2;

/* How long a tone should sound */
const int tone_duration = 200;

/* recive frequency in this var */
int32_t charid_string;

/* tone array for recorded tones */
int tones[100];

/* boolean if recording */
boolean isRecording = false;

/**************************************************************************
    Plays the recorded tones
**************************************************************************/
void play() {
  int i = 0;
  // loop trough available tones
  while (tones[i] != 0) {
    tone(buzzerPin, tones[i], tone_duration); // play tone
    i = i + 1;
    delay(tone_duration-25); // delay less than a tone duration for allowing long tones
  }
}


/**************************************************************************
    Push a tone to the tones array
**************************************************************************/
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

/**************************************************************************
    BLE Connected
**************************************************************************/
void connected(void) {
  pushTone(262);
  pushTone(523);
  play();
  memset(tones, 0, sizeof(tones));
  Serial.println( F("Connected") );
}

/**************************************************************************
    BLE Disconnected
**************************************************************************/
void disconnected(void) {
  pushTone(523);
  pushTone(262);
  play();
  memset(tones, 0, sizeof(tones));
  Serial.println( F("Disconnected") );
}


/**************************************************************************
    Push a tone to the tones array
**************************************************************************/
void receiveTone(int32_t chars_id, uint8_t data[], uint16_t len) {

  Serial.print("Playing Tone: ");
  Serial.println(atoi((char*)data));

  // checks if we are in recording modus and saves tone if we are
  if(isRecording == true) {
    pushTone(atoi((char*)data));
  }

  tone(buzzerPin, atoi((char*)data), tone_duration); // play tone
  
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module 
        (this function is called automatically on startup)
*/
/**************************************************************************/
void setup(void)
{

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
  ble.sendCommandWithIntReply( F("AT+GATTADDCHAR=UUID=0x2345,PROPERTIES=0x08,MIN_LEN=1,MAX_LEN=6,DATATYPE=string,DESCRIPTION=string,VALUE=blue-pi"), &charid_string);

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
  ble.setBleGattRxCallback(charid_string, receiveTone);

  /* set pins */
  pinMode(buzzerPin, OUTPUT);
  pinMode(recordButtonPin, INPUT);
  pinMode(playButtonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // reset tones array
  memset(tones, 0, sizeof(tones));
}


/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{  

  // if Record Button Pressed
  if(digitalRead(recordButtonPin) == LOW) {
    isRecording = !isRecording; // change record state

    // if recording reset tones and activate record led
    if(isRecording == 1) {
      memset(tones, 0, sizeof(tones));
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW); // deactivate record led
    }

    delay(500);

  // if play button is pressed
  } else if(digitalRead(playButtonPin) == LOW) {
    play();
    
    delay(500);
  } else {
    ble.update(200);
  }

}

