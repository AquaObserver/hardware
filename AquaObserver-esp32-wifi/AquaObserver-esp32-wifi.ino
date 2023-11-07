#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid       = "YOUR_SSID";
const char* password   = "YOUR_PASS";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

}

void loop() {
  // put your main code here, to run repeatedly:

}
