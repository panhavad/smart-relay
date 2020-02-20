// GPIO where the DS18B20 is connected to
const int oneWireBus = 2;
#define RELAY 5
#define LED 2

//BLYNK pin define
#define TEMP_PIN V1
#define WARNING_INTERVAL_PIN V2
#define MIN_TEMP_PIN V3
#define MAX_TEMP_PIN V4
#define WARNING_INTERVAL_VALUE_PIN V5
#define RELAY_STATUS_PIN V0

//Blynk
#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

//esp
#include <ESP8266WiFi.h>
#include <EEPROM.h>

//temp sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
WidgetLED relay_led(V0);

int interval = 0;
int min_temp = 0;
int max_temp = 0;
int warning_interval_value = 0;
int start_indicator = 1;

//Blynk config
char auth[] = "y-clyEm3FG-9jUOmSZgDg6uagBV-b61t"; //token from Blynk application
char ssid[] = "Borey R-J37"; //wifi name
char pass[] = ""; //wifi password

void setup(void) {
  // Start the Serial Monitor
  Serial.begin(115200);
  // Start the DS18B20 sensor
  sensors.begin();

  pinMode(RELAY, OUTPUT); //allow power to ralay pin
  pinMode(LED, OUTPUT); //allow power to ralay pin

  Serial.println("--Smart Relay Project!--"); //Test the serial monitor

  EEPROM.begin(32);//start the eeprom
  Blynk.begin(auth, ssid, pass);//connect to blynk cloud server
}


BLYNK_WRITE(V3) {
  min_temp = param.asInt(); // Gets the value stored in V2 as an integer
  digitalWrite(RELAY, LOW); //reset relay state everytime treshold change to prevent relay error
  relay_led.off(); //reset LED state
  interval = 0;
}


BLYNK_WRITE(V4) {
  max_temp = param.asInt(); // Gets the value stored in V2 as an integer
  digitalWrite(RELAY, LOW); //reset relay state everytime treshold change to prevent relay error
  relay_led.off(); //reset LED state
  interval = 0;
}


BLYNK_WRITE(V5) {
  warning_interval_value = param.asInt(); // Gets the value stored in V2 as an integer
}


void loop(void) {
  Blynk.run(); //start blynk

  //get data from EEPROM on start
  if (start_indicator == 1) {
    //read value rom eeprom
      min_temp = EEPROM.read(0);
      max_temp = EEPROM.read(1);
      warning_interval_value = EEPROM.read(3);
    //rewrite the value from eeprom to blynk
      Blynk.virtualWrite(MIN_TEMP_PIN, min_temp);
      Blynk.virtualWrite(MAX_TEMP_PIN, max_temp);
      Blynk.virtualWrite(WARNING_INTERVAL_VALUE_PIN, warning_interval_value);
    //debuging mode
      start_indicator = 0;
      Serial.print("EEPROM Min ---- ");
      Serial.println(min_temp);
      Serial.print("EEPROM Max ---- ");
      Serial.println(max_temp);
      Serial.print("EEPROM Warning Interval ---- ");
      Serial.println(warning_interval_value);
  }

  static float tempValue;
  sensors.requestTemperatures(); 
  tempValue = sensors.getTempCByIndex(0);

  Serial.print("Temp value: ");
  Serial.println(tempValue, 2);
  Serial.println(min_temp);
  Serial.println(max_temp);

  EEPROM.put(0, min_temp);
  EEPROM.put(1, max_temp);
  EEPROM.put(3, warning_interval_value);
  EEPROM.commit();

  if (tempValue <= min_temp) //open relay = stop operation
  {
    digitalWrite(RELAY, LOW);
    relay_led.off();
    interval = 0;
  }else if(tempValue >= max_temp){
    digitalWrite(RELAY, HIGH);
    relay_led.on();
    Serial.println(interval);
    Serial.println(warning_interval_value);
    interval += 1;
    if (interval == warning_interval_value * 60) {
      Blynk.notify("Warning!! relay was ON for: " + String(interval) + "mins, operation will be stop automatically.");
      interval = 0;
      digitalWrite(RELAY, LOW);
      relay_led.off();
    }
  }

  Blynk.virtualWrite(WARNING_INTERVAL_PIN, interval);
  Blynk.virtualWrite(TEMP_PIN, tempValue);

  //blinking every loop
  digitalWrite(LED, digitalRead(LED) ^ 1);

  delay(1000);
}
