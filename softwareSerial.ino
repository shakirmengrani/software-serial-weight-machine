#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "SSID-PASS"
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;
int toggle = 0;
IPAddress ip(192, 168, 0, 10); 
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress server(192, 168, 0, 20);
WiFiClient espClient;
PubSubClient client(espClient);
const byte rxPin = D6;
const byte txPin = D5;
char incomingByte[8];
int sCount = 7;
const char* outTopic = "<root-topic>/out";
const char* inTopic = "<root-topic>/in";
// set up a new serial object
SoftwareSerial mySerial (txPin, rxPin);
/*
 * Start Software Serial
 */

void serialSetup(){
  Serial.begin(9600);
  mySerial.begin(2400);
}

void serialValue(){
  if(mySerial.available() > 0){
    unsigned long c = mySerial.read();
    incomingByte[sCount] = (char)c;
    Serial.println((char)c);
    if(sCount == 0){
      // publish value
      Serial.println(incomingByte);
      client.publish(outTopic, incomingByte);
      Serial.println("=======================================================");
      sCount = 7;
    }else{
      sCount--;
    }
  }
}
/*
 * End Software Serial
 */

/*
 * Start Wifi & PubSub
 */


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = WiFi.macAddress();
    
    Serial.println();
    Serial.print("MAC Address: ");
    Serial.println(clientId);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      char* msg = {clientId.c_str(), " Connected"};
      client.publish(outTopic, "connected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setupWifi(){
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
    WiFi.mode(WIFI_STA);
//    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {
  char str[length];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for(int i = 0; i < length; i++){
    str[i] = (char)payload[i];
  }
  if(strcmp(str, "On") == 0){
    toggle = 1;
  }else{
    toggle = 0;
  }
  Serial.println(); 
}
 /*
 * End Wifi & PubSub
 */


void setup() {
  // put your setup code here, to run once:
    serialSetup();
    setupWifi();
    client.setServer(server, 1883);
    client.setCallback(callback);
    
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(toggle == 1){
    serialValue();
//    delay(100);
  } 
}
