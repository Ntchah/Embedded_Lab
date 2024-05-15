#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DHT.h"
#include <SoftwareSerial.h>
#define LM73_ADDR 0x4D
#define LED 12
#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
long last = 0;
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial testSerial(D7, D8); // RX, TX
int bufferSize = 4;
char buffer[4];
char v[4];
int bufferIndex = 0;
const char* ssid = "Gongpob";
const char* password = "gonggong";
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "979a6474-de79-4a07-8753-df6e6dd11b8e";
const char* mqtt_username = "QaYJiLZS3imHSJ1VNByWWeKmCArw4Vkf";
const char* mqtt_password = "iGvLj1TxdmNQkBF2ijPtqBvqe1Fu6xhV";

float temperature = 0;
float Humidity = 0;
int analog_value = 0;
char msg[100];
long lastMsg = 0;
long lastReadSeat = 0;
String val;
int nig = 0;

WiFiClient espClient;
PubSubClient client(espClient);

float readTemperature() {
    if (millis() - last > 2000){
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      return t;
    }
  return temperature;
}


float readHumidity() {
     if (millis() - last > 2000){
     // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
        last = millis();
          return h;
    }
    return Humidity;
}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting NETPIE2020 connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("NETPIE2020 connected");
      client.subscribe("@msg/led");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);

  if(String(topic) == "@msg/led") {
    if (message == "on"){
      digitalWrite(LED,0);
      client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"on\"}}");
      Serial.println("LED ON");
    }
    else if (message == "off") {
      digitalWrite(LED,1);
      client.publish("@shadow/data/update", "{\"data\" : {\"led\" : \"off\"}}");
      Serial.println("LED OFF");
    }
  }
}

void setup() {
  Serial.begin(9600);
  testSerial.begin(9600);
  dht.begin();
  digitalWrite(16, HIGH);
  Serial.println("Starting...");
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  temperature = readTemperature();
  Humidity = readHumidity();
  String place = "NECTEC";
    // delay(250);
    // digitalWrite(16, LOW);
    // delay(250);

    while (testSerial.available()) {    
      char receivedChar = testSerial.read();
        if (bufferIndex < bufferSize - 1) {
          if (receivedChar == '\n' || receivedChar == '\r') break;
          buffer[bufferIndex++] = receivedChar;
        } else {
          bufferIndex = 0;
          if (receivedChar == '\n' || receivedChar == '\r') break;
           // Reset buffer index on overflow
        }
      
    }
    val = buffer;
    if (val[0] != '1') {
  char tmp[3];
  tmp[0] = val[1];
  tmp[1] = val[2];
  tmp[2] = '\0'; // Null-terminate the string
  val = String(tmp); // Convert tmp to String and assign to val
}
     String seatData = "{\"data\": {\"place\": \"" + val + "\"}}";
     char seatMsg[50];
      seatData.toCharArray(seatMsg, (seatData.length() + 1));
    client.publish("@shadow/data/update2", seatMsg);

    bufferIndex = 0;
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    String data = "{\"data\": {\"temperature\":" + String(temperature) + ", \"Humidity\":" + String(Humidity) + ", \"place\": \"" +  val + "\"}}";
    Serial.println(data);
    data.toCharArray(msg, (data.length() + 1));
    client.publish("@shadow/data/update", msg);
    nig = 0;
  }
  delay(1); 
}
