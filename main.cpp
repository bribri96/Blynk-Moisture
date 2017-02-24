/*
1,500 years ago, everybody knew that the Earth was the center of the universe.
500 years ago, everybody knew that the Earth was flat.
And 15 minutes ago, you knew that humans were alone on this planet.
Imagine what you'll know tomorrow.
Agent K.

#Muse, Madness
*/


#define myName "/plant/file.bin" //Name for OTA update
#define version "0.0.4" //Firmware version

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>//library for OTA update

#include <SimpleTimer.h>

#include "ADS1115.h"//library for I2C adc

/*
io13 resistor divider enable
*/
const int pinPart = 13;

ADS1115 adc0(ADS1115_DEFAULT_ADDRESS);


char auth[] = "3b84b9dfaedc4d7ca2f851d1796cf959";//blynk auth
char ssid[] = "Bill Wi the Science Fi";//wifi ssid
char pass[] = "t36wk67zzksmiy966996";//wifi key
const char server_upd[] = "192.168.1.124";//server for OTA update
const char server_blynk[] = "192.168.1.124";//blynk server
const int port_upd = 8090;//port for OTA update
const int port_blynk = 8080;//blynk http api port
const int port_telegram = 8091;//telegram port

SimpleTimer timer;

int sendData(float, String);//function to send float trough http api
int sendData(int, String);//function to send int trough http api
int sendData(String, String);//function to send string trough http api
int sendVersion(String);//function to send firmware version trough http api
void sendlowbat();//function to send low bat alert to telegram chat

void spedisciDati();//function to send data to blynk server
void aggiornaDormi();//function to search update and deepSleep

const long deltat = 1000*15;//max time to be awake
const long deltat2 = 1000*1;//delay beetwen send data to server

const long tsleep = 1000*1000*60*5;//time to sleep (5 min..)
//const long tsleep = 1000*1000*1;//time to sleep (1 sec.)

void setup(){

  pinMode(pinPart, OUTPUT);//setting up pin for voltage divider
  digitalWrite(pinPart, HIGH);
  delay(0);

  Wire.begin();//init i2c bus

  IPAddress ip(192, 168, 1, 131); //IP Address
  IPAddress gateway(192, 168, 1, 254); // set gateway to match your network
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);//setting up wifi fixed ip

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  WiFi.mode(WIFI_STA); //starting wifi
  WiFi.begin(ssid, pass);//starting wifi

  while (WiFi.status() != WL_CONNECTED)  {
    delay(10);
    Serial.println(WiFi.status());
  }//wait until wifi is connected

  Serial.println(adc0.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");

  adc0.initialize(); // initialize ADS1115 16 bit A/D chip

  // We're going to do single shot sampling
  adc0.setMode(ADS1115_MODE_SINGLESHOT);

  // Slow things down so that we can see that the "poll for conversion" code works
  adc0.setRate(ADS1115_RATE_860);

  // Set the gain (PGA) +/- 6.144v
  // Note that any analog input must be higher than –0.3V and less than VDD +0.3
  adc0.setGain(ADS1115_PGA_6P144);
  // ALERT/RDY pin will indicate when conversion is ready

  timer.setInterval(deltat2, spedisciDati);//setting up timer
  timer.setInterval(deltat, aggiornaDormi);//setting up timer


}

void loop() {
  spedisciDati();//overriding timer for fast transmission
  timer.run();

}

int sendData(float value, String pin){
  HTTPClient http;
  String payload = String("[\n") + value + String("\n]");
  Serial.println("\nstart:");
  Serial.println(http.begin(server_blynk, port_blynk, String("/") + auth + String("/pin/") + pin));
  http.addHeader("Content-Type", "application/json");//String("/") + auth + String("/pin/") + pin
  int codice_ritorno = http.sendRequest("PUT",(uint8_t *) payload.c_str(), payload.length());
  Serial.println(codice_ritorno);
  Serial.println(http.writeToStream(&Serial));
  http.end();
  Serial.println();
  return codice_ritorno;
}

int sendData(String value, String pin){
  HTTPClient http;
  String payload = String("[\n") + value + String("\n]");
  Serial.println("\nstart:");
  Serial.println(http.begin(server_blynk, port_blynk, String("/") + auth + String("/pin/") + pin));
  http.addHeader("Content-Type", "application/json");//String("/") + auth + String("/pin/") + pin
  int codice_ritorno = http.sendRequest("PUT",(uint8_t *) payload.c_str(), payload.length());
  Serial.println(codice_ritorno);
  Serial.println(http.writeToStream(&Serial));
  http.end();
  Serial.println();
  return codice_ritorno;
}

int sendData(int value, String pin){
  HTTPClient http;
  String payload = String("[\n") + value + String("\n]");
  Serial.println("\nstart:");
  Serial.println(http.begin(server_blynk, port_blynk, String("/") + auth + String("/pin/") + pin));
  http.addHeader("Content-Type", "application/json");//String("/") + auth + String("/pin/") + pin
  int codice_ritorno = http.sendRequest("PUT",(uint8_t *) payload.c_str(), payload.length());
  Serial.println(codice_ritorno);
  Serial.println(http.writeToStream(&Serial));
  http.end();
  return codice_ritorno;
}

int sendVersion(String pin){
  HTTPClient http;
  String payload = String("[\n\"") + String(version) + String("\"\n]");
  Serial.println("\nstart:");
  Serial.println(http.begin(server_blynk, port_blynk, String("/") + auth + String("/pin/") + pin));
  http.addHeader("Content-Type", "application/json");//String("/") + auth + String("/pin/") + pin
  Serial.println(http.sendRequest("PUT",(uint8_t *) payload.c_str(), payload.length()));
  Serial.println(http.writeToStream(&Serial));
  http.end();
}

void sendlowbat(){
  HTTPClient http;
  Serial.println("\nstart:");
  String sendLowBatS = "/lowbat";
  sendLowBatS += myName;
  Serial.println(sendLowBatS);
  Serial.println(http.begin(server_blynk, port_telegram, sendLowBatS ));
  int codice_ritorno = http.GET();
  Serial.println(codice_ritorno);
  Serial.println(http.writeToStream(&Serial));
  http.end();
}

void spedisciDati(){
  sendData(int(random(0,100)), "V3");//send random data for purpose
  sendVersion("V2");//send firmware version to Vpin V2

  //Read analog input trough i2c adc
  adc0.setMultiplexer(ADS1115_MUX_P1_NG);//Vdivider
  float ad1 = adc0.getMilliVolts(true);
  Serial.print("A1: "); Serial.print(ad1); Serial.print("mV\t");

  adc0.setMultiplexer(ADS1115_MUX_P2_NG);
  float ad2 = adc0.getMilliVolts(true);
  Serial.print("A2: "); Serial.print(ad2); Serial.print("mV\t");

  adc0.setMultiplexer(ADS1115_MUX_P3_NG);
  float ad3 = adc0.getMilliVolts(true);
  Serial.print("A3: "); Serial.print(ad3); Serial.print("mV\t");

  adc0.setMultiplexer(ADS1115_MUX_P0_NG);//Vin
  float ad0 = adc0.getMilliVolts(true);
  Serial.print("\tA0: "); Serial.print(ad0); Serial.print("mV\t");

  if(ad0/1000 < 2.5){//if battery go under 2.5v send telegran notify
    sendlowbat();//********************************************************************************
  }

  sendData(float(ad0/1000), "V5");//send Battery voltage to blynk

  int ret_code = sendData(ad1, "V34");//send moisture to blynk
  Serial.print("codice get: ");
  Serial.println(ret_code);
  if(ret_code == 200){//if succesful send to blynk then check update
    aggiornaDormi();
  }

  Serial.printf("Connection status: %d\n", WiFi.status());
}

void aggiornaDormi(){
  if((WiFi.status() == WL_CONNECTED)) {    // wait for WiFi connection
    t_httpUpdate_return ret = ESPhttpUpdate.update(server_upd, port_upd, myName, version);

    switch(ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES\n");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK\n");
        break;
    }
  }
  sendData(int(millis()), "V1");
  ESP.deepSleep(tsleep); //go to sleep
}
