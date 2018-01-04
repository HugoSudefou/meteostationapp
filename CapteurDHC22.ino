#include "DHT.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define DHTPIN 4     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22   // there are multiple kinds of DHT sensors


DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "Sudefou";
const char* password = "82B515427B1";

char* topic = "test";
char* server = "192.168.43.226";


void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void setup() {
  Serial.begin(115200);
  delay(10);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("initialize serial: ");
  // Wait for serial to initialize.
  while(!Serial) { 
    delay(500);
    Serial.print(".");
  }
  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);

  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);
    
    if (client.publish(topic, "hello from ESP8266")) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running DHT!");
  Serial.println("-------------------------------------");

}

int timeSinceLastRead = 0;
int timeSinceLastDistCalc = 0;
void loop() {

  // Report every 2 seconds.
  if(timeSinceLastRead > 2000) {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(sensorValue)) {
      Serial.println("Failed to read from DHT sensor!");
      timeSinceLastRead = 0;
      return;
    }

    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);
    
    String payload;
    payload = "Humidite : ";
    payload += h;
    
    if(h > 40) {
      payload += ", NON";
    }else{
      payload += ", OK ";
    }
    
    payload += "Temperature : ";
    payload += t;

    if(t > 25.30) {
      payload += ", NON";
    }else{
      payload += ", OK";
    }
    
    payload += "Lumiere : ";
    payload += sensorValue;    
    
    
    if (client.connected()){
      Serial.print("Sending payload: ");
      Serial.println(payload);
      
      if (client.publish(topic, (char*) payload.c_str())) {
        Serial.println("Publish ok");
      }
      else {
        Serial.println("Publish failed");
      }
    }
    timeSinceLastRead = 0;
  }
  
  delay(100);
  timeSinceLastRead += 100;
  timeSinceLastDistCalc += 100;
  
    //Serial.print(distance);
    //Serial.println("Centimeter:");
}
