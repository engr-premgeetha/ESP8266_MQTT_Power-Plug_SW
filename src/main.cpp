#include <Arduino.h>
#include <PubSubClient.h> 
#include <ESP8266WiFi.h>

//#define DEBUG

#define RELAY 13
#define SW 12

WiFiClient WIFI_CLIENT;
PubSubClient MQTT_CLIENT;


int lamp = 0;
bool lamp_S = false;

String msgStr = "";     // MQTT message buffer

unsigned long debounceDuration = 50; // millis
unsigned long lastlastmqtt_buttonStateChanged = 0;


//WIFI
const char* ssid = "Mycelium";             //User Name
const char* password = "#WiFiMushroom";    //Password
//MQTT
const char* mqttServer = "192.168.0.141"; //MQTT URL
const char* mqttUserName = "";            // MQTT username
const char* mqttPwd = "";                 // MQTT password
const char* clientID = "";                // client id username+0001
const char* topic = "/UPSTAIRS/BEDSIDE/LAMP";           //publish topic

// What to do when it receives the data. 
void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic;
  String message = "";
  for (unsigned int i=0; i< length; i++) {
    message = message + (char)payload[i]; 
  }

  // Print the message to the serial port

  #ifdef DEBUG
    Serial.print("Message received: Topic: ");
    Serial.print(topicStr);
    Serial.print("   ");
    Serial.print("Message: ");
    Serial.println(message);
  #endif

  if(message == "ON"){
    lamp_S = true;
  }
  else if(message == "OFF"){
    lamp_S = false;
  }
    
}


void publish(String pub_msg){
  msgStr = pub_msg;

  byte arrSize = msgStr.length() + 1;
  char msg[arrSize];
  Serial.print("PUBLISH DATA:");
  Serial.println(msgStr);
  msgStr.toCharArray(msg, arrSize);
  MQTT_CLIENT.publish("/UPSTAIRS/BEDSIDE/LAMP/STATUS", msg);
  MQTT_CLIENT.loop(); // Check Subscription. 
 
}

void reconnect() {
  MQTT_CLIENT.setServer(mqttServer, 1883);  
  MQTT_CLIENT.setClient(WIFI_CLIENT);

  // Trying connect with broker.
  while (!MQTT_CLIENT.connected()) {
    if (MQTT_CLIENT.connect(clientID, mqttUserName, mqttPwd)){
      Serial.println("Trying to connect.");
      MQTT_CLIENT.connect(""); // it isn't necessary..
      MQTT_CLIENT.subscribe(topic); // HERE SUBSCRIBE.
      // Wait to try to reconnect again...
      delay(500);
    }
  }
  Serial.println("Connected to MQTT");
  Serial.print("Subscribed to ");
  Serial.println(topic);
}

void setup() {

  pinMode(RELAY, OUTPUT_OPEN_DRAIN);
  digitalWrite(RELAY, HIGH);
  pinMode(SW, INPUT_PULLUP);
  yield();

  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println();
  #endif

  Serial.print("Connecting with ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("WiFi conected. IP: ");
  Serial.println(WiFi.localIP());
  yield();

  // Setting Callback.
  MQTT_CLIENT.setCallback(callback); 

  publish("OFF");
}

void loop() {
  if (!MQTT_CLIENT.connected()) {
   reconnect(); 
  }
  MQTT_CLIENT.loop();

  if(digitalRead(SW) == LOW){
    delay(50);
    if(digitalRead(SW) == LOW){
      while(!digitalRead(SW));
      lamp_S = !lamp_S; 
      if(lamp_S == true)
        publish("ON");
      else if(lamp_S == false)
        publish("OFF");
    }
  }
    
  if(lamp_S == true){
    digitalWrite(RELAY, LOW);
  }
  else if(lamp_S == false){
    digitalWrite(RELAY, HIGH);
  }
}
  
 