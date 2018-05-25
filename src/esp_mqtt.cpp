/**
 * @Author: Val
 * @Date:   2018-03-18T19:13:16+01:00
 * @Project: MQTT_CLIENT
 * @Last modified by:
 * @Last modified time: 2018-05-23T23:48:43+02:00
 */

/* Here ESP32 will keep 3 roles:
1/ read data from DHT11/DHT22 sensor
2/ control led on-off
3/ control solenoids with delay
So it willpublish temperature/humidity topic and scribe topic bulb on/off
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>
#include <U8x8lib.h>
#include <EEPROM.h>
#include <DallasTemperature.h>
#include <dht.h>
#include "GravityTDS.h"

// the OLED used
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

/* change it with ssid-password */
const char* ssid = "Camion wifi nsa";
const char* password = "vibestyle92";

/* this is the IP of PC/raspberry where MQTT Server is installed
on Wins use "ipconfig"
on Linux use "ifconfig" to get its IP address */
const char* mqtt_server = "192.168.1.11";

/* define pins */
#define DHTPIN 21
#define DHTPIN1 13
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 22 // DS18B20 pin
#define TDSPIN 37

/* define pins */
float txValue = 0;
const int EV1 = 2; // Pin for Solenoid 1
const int EV2 = 4; // Pin for Solenoid 2
const int EV3 = 16; // Pin for Solenoid 3
const int EV4  = 17; // pin for relay
const int AC  = 15; // pin for relay
const int PH = 38;

/* Define PH */
float calibration = 0.1; //change this value to calibrate

/* Init gravity sensor*/
GravityTDS gravityTds;

/* Init DHT sensors */
dht dht0;
dht dht1;

float temperature = 0;
float humidity = 0;
float temperature1 = 0;
float humidity1 = 0;
float analogTemp = 0; // Dallas temperature sensor
float tdsValue = 0; // TDS
float phValue = 0; // PH sensor

/* initialization of Dallas temperature sensor */
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

/* topics */
#define CAPT_TOPIC    "box/capt"
#define LIGHT_TOPIC     "box/trigger" /* 1=on, 0=off */

long lastMsg = 0;
char msg[100];

/* counter to wait cycles execution*/
int cpt=0;

void receivedCallback(char* topic, byte* payload, unsigned int length) {

  Serial.println("Msg received: ");
  Serial.println(topic);
  Serial.print("payload: ");

  /* truncate message to avoid special characters in string */
  char payloadTruncate[length];

  for (int i = 0; i < length; i++) {
    payloadTruncate[i] = (char)payload[i];
  }

  /* to detect end of string  */
  payloadTruncate[length] = '\0';

  Serial.println(payloadTruncate);
  u8x8.drawString(0, 3,payloadTruncate);

  /* truncate message*/
  char* event = strtok(payloadTruncate, ";");

  /* first argument */
  std::string eventString(event);
  event = strtok(NULL, ";");

  if (eventString.find("EV") != -1) {
    u8x8.drawString(0, 7, "EV ON");
  }

  /* CODE FOR SOLENOID & AC TRIGGER */
  if (eventString.find("EV1") != -1) {
    digitalWrite(EV1, HIGH);
    delay(1000 * atoi(event)); // wait 1 sec
    digitalWrite(EV1, LOW);
  }
  if (eventString.find("EV2") != -1) {
    digitalWrite(EV2, HIGH);
    delay(1000 * atoi(event)); // wait 1 sec
    digitalWrite(EV2, LOW);
  }
  if (eventString.find("EV3") != -1) {
    digitalWrite(EV3, HIGH);
    delay(1000 * atoi(event)); // wait 1 sec
    digitalWrite(EV3, LOW);
  }
  if (eventString.find("EV4") != -1) {
    digitalWrite(EV4, HIGH);
    delay(1000 * atoi(event)); // wait 1 sec
    digitalWrite(EV4, LOW);

  }
  if (eventString.find("ACON") != -1) {
    digitalWrite(AC, LOW);
  }
  if (eventString.find("ACOFF") != -1) {
    digitalWrite(AC, HIGH);
  }

  Serial.println(event);

  /* clear sceen */
  u8x8.clearLine(7);

}

void mqttconnect() {

  /* Loop until reconnected */
  while (!client.connected() && (cpt < 5) ) {
      Serial.print("MQTT connecting ...");
      u8x8.println("MQTT connecting ...");

      /* client ID */
      String clientId = "ESP32Client";

      /* connect now */
      if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        u8x8.println("connected");

        /* subscribe topic with default QoS 0*/
        client.subscribe(LIGHT_TOPIC);

        u8x8.clearDisplay();

      } else {

        Serial.print("failed, status code =");
        Serial.print(client.state());
        Serial.println("try again in 5 seconds");
        u8x8.println("failed, status code =");
        u8x8.println(client.state());
        u8x8.println("try again in 5 seconds");
        /* Wait 5 seconds before retrying */
        delay(5000);
        /* 5 retries max */
        cpt++;
      }
    }
}

void setup() {

  Serial.begin(115200);

  /* We start by connecting to a WiFi network */
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  Serial.println();

  u8x8.println("Connecting to ");
  Serial.println(ssid);
  Serial.println("Connecting to ");
  u8x8.println(ssid);

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int cpt=0;

  /* 32x500ms = 16 sec to connect until reset */
  while ( (WiFi.status() != WL_CONNECTED) && (cpt < 32) ) {
    delay(500);
    Serial.print(".");
    u8x8.print(".");
    cpt++;
  }

  /*reset*/
  if (cpt >= 32){
    esp_restart();
  }

  /* set led as output to control AC/EV */
  pinMode(EV1, OUTPUT);
  pinMode(EV2, OUTPUT);
  pinMode(EV3, OUTPUT);
  pinMode(EV4, OUTPUT);
  pinMode(AC, OUTPUT);
  digitalWrite(AC, HIGH);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  u8x8.println("");
  u8x8.println("WiFi connected");
  u8x8.println("IP address: ");
  u8x8.println(WiFi.localIP());

  /* configure the MQTT server with IPaddress and port */
  client.setServer(mqtt_server, 1883);
  /* this receivedCallback function will be invoked
  when client received subscribed topic */
  client.setCallback(receivedCallback);



  /* initialize ppm parameters */
  gravityTds.setPin(TDSPIN);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization

}

void loop() {


  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }
  /* this function will listen for incomming
  subscribed topic-process-invoke receivedCallback */
  client.loop();



  /* we measure temperature every 3 secs
  we count until 5 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  if (now - lastMsg > 8000) {

    int measure = analogRead(PH);
    Serial.print("Measure: ");
    Serial.print(measure);

    float voltage=(float)measure*5.0/1024;
    Serial.print("\tVoltage: ");
    Serial.print(voltage, 3);

    phValue = 3.5 * voltage * calibration;
    Serial.print("\tPH: ");
    Serial.print(phValue, 3);
    Serial.print("\n");

    lastMsg = now;

    /* temperature for calibration    */
    DS18B20.requestTemperatures();
    analogTemp = DS18B20.getTempCByIndex(0);

    /* start DHT sensor 1 */
    dht0.read22(DHTPIN);

    /* read DHT11/DHT22 sensor and convert to string */
    humidity = dht0.humidity;
    temperature = dht0.temperature;

    /* start DHT sensor 2 */
    dht1.read22(DHTPIN1);

    /* read DHT11/DHT22 sensor and convert to string */
    humidity1 = dht1.humidity;
    temperature1 = dht1.temperature;


    //temperature = readTemperature();  //add your temperature sensor and read it
    gravityTds.setTemperature(analogTemp);  // set the temperature and execute temperature compensation
    gravityTds.update();  //sample and calculate
    tdsValue = gravityTds.getTdsValue();  // then get the value

    /*msg+= "TMP:XX.XX;HUM:XX"*/
    snprintf (msg, 70, "TMP:%.2f;HUM:%.2f;TMP1:%.2f;HUM1:%.2f;ATEMP:%.2f;TDS:%.2f;PH:%.2f", temperature, humidity, temperature1, humidity1,analogTemp,tdsValue,phValue);

    Serial.println(msg);

   if (!isnan(temperature) && !(temperature < 0) && !(humidity < 0)  && !(temperature1 < 0) && !(humidity1 < 0)  && !isnan(temperature1) && analogTemp < 80) {
    char part1[20];
    char part2[10];
    char part3[10];
    char part4[10];
    char part5[10];
    char part6[10];

    snprintf (part1, 20, "TMP:%.2f;%.2f",temperature,temperature1);
    snprintf (part2, 10, "HUM:%lf",humidity);
    snprintf (part3, 10, "HUM1:%lf",humidity1);
    snprintf (part4, 10, "ATEMP:%lf",analogTemp);
    snprintf (part5, 10, "TDS:%.2f",tdsValue);
    snprintf (part6, 10, "PH:%.2f",phValue);

    u8x8.drawString(0, 1, part1);
    u8x8.drawString(0, 2, part2);
    u8x8.drawString(0, 3, part3);
    u8x8.drawString(0, 4, part4);
    u8x8.drawString(0, 5, part5);
    u8x8.drawString(0, 6, part6);


      /* publish the message */
      client.publish(CAPT_TOPIC, msg);

    }
  }
}
