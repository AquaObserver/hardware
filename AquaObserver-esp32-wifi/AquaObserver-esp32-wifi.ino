#include <WiFi.h>
#include <PubSubClient.h>

// WiFi Config
const char* ssid       = "YOUR_SSID";
const char* password   = "YOUR_PASS";

// MQTT Broker Config
IPAddress server(192, 168, 1, 103);
unsigned int serverPort = 5000;

WiFiClient espWiFiClient;
PubSubClient client(espWiFiClient);

// MQTT Client Config
char *clientName = "aqua1";
char *onlineTopic = "aqua1/online";
char *critLvlTopic = "aqua1/critLvl";

// Func Proto
// -- MQTT client
void callback(char*, byte*, unsigned int);
void reconnect();
// -- WiFi
void setupWiFi();

//////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  setupWiFi();

  delay(250);

  // Setup MQTT broker params
  client.setServer(server, serverPort);
  client.setCallback(callback);

  delay(250);
}

void loop() {
  if(!client.connected()) reconnect();

  client.loop();

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

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(onlineTopic,"alive");
      // ... and resubscribe
      client.subscribe(critLvlTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
