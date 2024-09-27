// Include libraries for LCD display, DHT sensor, and Bluetooth communication
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "BluetoothSerial.h"

// Error message if Bluetooth is not enabled during compilation
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth serial object and variables for received data
BluetoothSerial SerialBT;
int received;
char receivedChar;

// Define constants for characters used in Bluetooth commands
const char SM = '1';
const char GW = '2';
const char TH = '3';

// Define pin connected to the LED
const int LEDpin = 23;

// Define pin connected to the Pump Relay
const int relay = 26;

// Define pin connected to the DHT sensor
#define DHT11PIN 4

// Create a DHT object for interacting with the sensor
DHT dht(DHT11PIN, DHT11);

// Define pins used for the ultrasonic sensor
const int trigPin = 5;
const int echoPin = 18;

// Define constants for speed of sound and conversion factor (optional)
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

// Variables used for distance measurement
long duration;
float distanceCm;

// Variables used for soil moisture reading
int _moisture, sensor_analog;
const int sensor_pin = A0;

// Define LCD dimensions
int lcdColumns = 16;
int lcdRows = 2;

// Create a LiquidCrystal object for controlling the LCD display
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Define static message and scrolling message for the LCD
String messageStatic = "C Project";
String messageToScroll = "SMART PLANT MONITER";

// Function to scroll text on the LCD display
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i = 0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

// Function to read temperature and humidity from the DHT sensor
void read_DHT(){
  String humi = (String)dht.readHumidity();
  String temp = (String)dht.readTemperature();
  messageToScroll = "Temperature: " + temp + "'c ; Humidity: " + humi;
  SerialBT.println(messageToScroll);
  Serial.println(messageToScroll);
  scrollText(1, messageToScroll, 300, lcdColumns);
}

// Function to read soil moisture value
void read_SoilM(){
  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog / 4095.00) * 100 ) );
  messageToScroll = "Moisture = " + (String)_moisture + "%";
  SerialBT.println(messageToScroll);
  Serial.println(messageToScroll);
  scrollText(1, messageToScroll, 300, lcdColumns);  
}

//Functon to mesure soil moisture and control water Pump through erelay
void soilM_for_waterpump(){
  sensor_analog = analogRead(sensor_pin);
  _moisture = ( 100 - ( (sensor_analog / 4095.00) * 100 ) );
  if (_moisture<30){
    digitalWrite(relay, LOW);
  }
  else if(_moisture>50){
  digitalWrite(relay, HIGH);
  }
}

// Function to measure distance using the ultrasonic sensor
void read_US(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = 100-(duration * SOUND_SPEED/2);
  messageToScroll = "The hight of the plant " + (String)distanceCm;
  SerialBT.println(messageToScroll);
  Serial.println(messageToScroll);
  scrollText(1, messageToScroll, 300, lcdColumns);
}

// Function to initialize components (LCD, serial, Bluetooth, etc.)
void setup(){
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  dht.begin();
  SerialBT.begin("IOT_project");
  SerialBT.println("Use to following Numbers to control the device\n 1: soil moisture\n 2: hight of the plant \n 3: humidity and Temperature");
  scrollText(1, messageToScroll, 250, lcdColumns);
}

// Function that runs repeatedly to monitor, display, and control
void loop(){
  soilM_for_waterpump();
  lcd.setCursor(0, 0);
  lcd.print(messageStatic);
  SerialBT.println("ready to recive command: ");
  receivedChar = (char)SerialBT.read();
  
  if (Serial.available()) {
    SerialBT.write(Serial.read());  
  }
  if (SerialBT.available()) {
    SerialBT.print("Received:");
    SerialBT.println(receivedChar);   
    Serial.print("Received:");
    Serial.println(receivedChar);
    
    if(receivedChar == SM) {
      read_SoilM();
    }
    if(receivedChar == GW) {
      read_US();
    }    
    if(receivedChar == TH) {
      read_DHT(); 
    }
    else{
      SerialBT.print("the command ID is not found");
    }    
    SerialBT.print("wait till the device is ready to recive");
  }
  soilM_for_waterpump();
  delay(10);
}