// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient by Nick ‘O Leary
// - DHT sensor library by Adafruit

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define wifi_ssid "fubar"
#define wifi_password "foobar0x1"

#define mqtt_server "192.168.2.53"
#define mqtt_user "sensor1"
#define mqtt_password "Plm21#qwp0999"

#define RoomName "office"

#define humidity_topic "home/%s/humidity"
#define temperature_topic "home/%s/temperature"
char deviceName[sizeof(temperature_topic)*2];

#define DHTTYPE DHT22
#define DHTPIN  12

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
void setup_wifi();

long randNumber;
char clientname[32];
const char * namemask="%s%d";

void setup() {
  memset(clientname,0,sizeof(clientname));
  randomSeed(analogRead(0));
  randNumber = random(300);
  sprintf(clientname,namemask, "ESP8266Client",randNumber);
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.printf("Attempting MQTT connection... with clientname=%s - ",(char *)clientname);
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(clientname, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      break;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

void loop() {

  long now = millis();
  if (now - lastMsg > 2000) {
    if (!client.connected()) {
      //Serial.print(client.state());
      reconnect();
    }
    client.loop();    
    lastMsg = now;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      sprintf(deviceName,temperature_topic,RoomName);
      client.publish(deviceName, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      sprintf(deviceName,humidity_topic,RoomName);
      client.publish(deviceName, String(hum).c_str(), true);
    }
  }
}