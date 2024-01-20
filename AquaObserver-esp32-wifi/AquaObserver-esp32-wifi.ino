// ESP32-C3-DevKitM-1

#include <WiFi.h>
#include <PubSubClient.h>

#define PORT 5000
#define SPEED_OF_SOUND 0.0343 // cm/us
#define MEASUREMENTS_IN_BATCH 60

// WiFi Config
const char* ssid       = "YOUR_SSID";
const char* password   = "YOUR_PASS";

// MQTT Broker Config(Choose between IP adress od domain)
IPAddress server(192, 168, 1, 103);
// const char* server = "mybroker.example.com";

WiFiClient espWiFiClient;
PubSubClient client(espWiFiClient);

// MQTT Client Config
char *clientName = "aqua1";
char *onlineTopic = "aqua1/online";
char *critLvlTopic = "aqua1/critLvl";
char *waterLvlTopic = "aqua1/wtrLvl";
char *depthCalibrationTopic = "aqua1/calibrate";

// Func Proto
// -- MQTT client
void callback(char*, byte*, unsigned int);
void reconnect();
// -- WiFi
void setupWiFi();

// Pins used
int pinTrig1 = 8;
int pinEcho1 = 7;

int ledPin1 = 10;

// Global var
bool mqttIsConnected = false;
bool wifiIsConnected = false;

unsigned long measureTiming;
int timeBetweenMeasurements = 1000; // ms
short int measurements[MEASUREMENTS_IN_BATCH];
int measurementPointer = 0;

float criticalValue = 10;     // %

int totalContainerDepth = 100; // cm


//////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  setupWiFi();

  delay(250);

  // Setup MQTT broker params
  client.setServer(server, PORT);
  client.setCallback(callback);

  // Setup HC-SR04 pins
  pinMode(pinTrig1, OUTPUT);
  pinMode(pinEcho1, INPUT);

  delay(250);

  pinMode(ledPin1, OUTPUT);

  measureTiming = millis();
}

void loop() {
  if(!client.connected()) mqttIsConnected = false; // Ask for reconnection if disconnected

  // Measure water level every timeBetweenMeasurements milliseconds
  if((millis() - measureTiming) > timeBetweenMeasurements){

    float currentWaterLevel = measureWaterLevel(pinTrig1, pinEcho1); // cm or -1 if error
    
    Serial.println("Water level("+ String(measurementPointer) +"): " + String(currentWaterLevel) + "cm");

    float waterLevelPercent = currentWaterLevel / totalContainerDepth; // cm -> %
    waterLevelPercent *= 100;

    if(currentWaterLevel < 0){
      measurements[measurementPointer] = -1;
    }else{
      measurements[measurementPointer] = (waterLevelPercent <= 100)?
                                         ((short int)waterLevelPercent):
                                         (100);
    }

    measurementPointer++;

    if(measurementPointer >= MEASUREMENTS_IN_BATCH){
      publishWaterLevel();
      measurementPointer = 0;
    }

    if (waterLevelPercent < criticalValue) {
      digitalWrite(ledPin1, HIGH);
      Serial.println("LED: ON");
    }else{
      digitalWrite(ledPin1, LOW);
    }

    measureTiming = millis();
  }

  // Loop if connected, else try reconnecting
  if(WiFi.status() != WL_CONNECTED){
    if(wifiIsConnected){
      wifiIsConnected = false;

      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(ssid);

      WiFi.begin(ssid, password);
    }

  }else if(WiFi.status() == WL_CONNECTED && wifiIsConnected == false){
    wifiIsConnected = true;

    Serial.println();
    Serial.print("Connected! Got IP: ");
    Serial.print(WiFi.localIP());
  }else{
    if(mqttIsConnected){
      client.loop();  
    }else{
      reconnect();
    }
  }
}

void setupWiFi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! Got IP: ");
  Serial.print(WiFi.localIP());
  
  wifiIsConnected = true;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("---------------------------------");
  Serial.print("Topic: ");
  Serial.print(topic);

  Serial.println();
  Serial.print("Payload(" + String(length) + "): ");

  char localPayload[length];

  for(int i = 0; i < length; i++){
    localPayload[i] = (char)*(payload + i);
  }

  String payloadString = String(localPayload);

  Serial.print(payloadString);

  Serial.println();

  callbackHandler(String(topic), payloadString);

}

void reconnect() {
  // Try reconnecting
  Serial.print("Attempting MQTT connection...");

  delay(1000);

  // Attempt to connect
  if (client.connect(clientName)) { // Also possible to use authentication with broker with client.connect(clientName, "client username", "client password")
    Serial.println("connected");
    mqttIsConnected = true;
    // Once connected, publish an announcement...
    client.publish(onlineTopic, String(totalContainerDepth).c_str());
    // ... and resubscribe
    client.subscribe(critLvlTopic);
    client.subscribe(depthCalibrationTopic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  }
}

float measureDistance(int pinTrig, int pinEcho){
  long duration;

  digitalWrite(pinTrig, LOW);
  delayMicroseconds(5);

  // 10us impuls
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // Echo
  duration = pulseIn(pinEcho, HIGH); // Returns the duration until HIGH signal on echo pin in microseconds with a timeout of 0.1s

  // Serial.println("Duration: " + String(duration));

  duration = duration >> 1; // duration = duration/2 <- Because the echo duration is 2*duration to surface
  // Serial.println("Duration/2: " + String(duration));

  float distance = (float)SPEED_OF_SOUND * duration; // cm

  return distance;
}

float measureWaterLevel(int pinTrig, int pinEcho){
  float waterLevel = totalContainerDepth - measureDistance(pinTrig, pinEcho);

  int counter = 0;

  while ((waterLevel < -5 || waterLevel > totalContainerDepth+5) && counter < 50) {
    waterLevel = totalContainerDepth - measureDistance(pinTrig, pinEcho);
    counter++;
  }

  if (waterLevel < 0) {
    if(waterLevel > -5){
      return 0;
    }else{
      return -1;
    }
  }else if(waterLevel > totalContainerDepth){
    if(waterLevel < totalContainerDepth+5){
      return totalContainerDepth;
    }else{
      return -1;
    }
  }else{
    return waterLevel;
  }
}

void callbackHandler(String topic, String payload){
  if(topic.equals(critLvlTopic)){
    // When we receive a msg for changing the critical value
    int intPayload = payload.toInt();

    // Change the value if the new value is between 0 and maximum depth
    criticalValue = (intPayload > 0 && intPayload <= 100)?
                    (intPayload):
                    (criticalValue);

    Serial.println("cryticalValue = " + String(criticalValue));

  }else if(topic.equals(depthCalibrationTopic)){
    // When we receive a calibration topic msg
    float sum = 0;

    // Reseting measurements
    measurementPointer = 0;

    // Take 10 measurements
    for(int counter = 0; counter < 10; counter++){
      sum += measureDistance(pinTrig1, pinEcho1);
    }

    // Average and calibrate
    totalContainerDepth = (int)(sum/10);
    
    Serial.print("Container depth calibration: ");
    Serial.print(String(totalContainerDepth));
    Serial.println(" cm");

    if(client.connected() && WiFi.status() == WL_CONNECTED){
      // Publish new value to online topic so the server knows what the current value is
      client.publish(onlineTopic, String(totalContainerDepth).c_str());
    }
  }
}

void publishWaterLevel(){
  int sum = 0;
  int numOfErrors = 0;

  for(int i = 0; i < MEASUREMENTS_IN_BATCH; i++){
    if(measurements[i] < 0){
      numOfErrors++;
      continue;
    }

    sum += (measurements[i] > 100)?(100):(measurements[i]);
  }

  if(numOfErrors > (MEASUREMENTS_IN_BATCH / 2)){
    Serial.println("Measurements not published due to more than 50% being errors!");
    return;
  }

  int waterLevelForPublish = sum / (MEASUREMENTS_IN_BATCH - numOfErrors);

  waterLevelForPublish = (waterLevelForPublish >= 0 && waterLevelForPublish <= 100)?
                         (waterLevelForPublish):
                         (0);

  if(client.connected() && WiFi.status() == WL_CONNECTED){
    Serial.println("Publishing water level: " + String(waterLevelForPublish) + "%");
    client.publish(waterLvlTopic, String(waterLevelForPublish).c_str());
  }
}
