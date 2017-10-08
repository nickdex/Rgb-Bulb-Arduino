#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#define PubNub_BASE_CLIENT WiFiClient
//#define PUBNUB_DEBUG
#define PUBNUB_DEFINE_STRSPN_AND_STRNCASECMP
#include <PubNub.h>

const char* ssid = "Web";
const char* password = "simple#9";

#define BLUE_PIN  5
#define GREEN_PIN 4
#define RED_PIN   2

char pubkey[] = "pub-c-a7a7bf57-7428-4acf-9feb-95aa76439442";
char subkey[] = "sub-c-28e8300e-7e94-11e7-a179-1e66dc778462";
char channel[] = "pi-house";

void flash() {
  digitalWrite(RED_PIN, HIGH);
  delay(100);
  digitalWrite(RED_PIN, LOW);
  delay(100);

  digitalWrite(GREEN_PIN, HIGH);
  delay(100);
  digitalWrite(GREEN_PIN, LOW);
  delay(100);

  digitalWrite(BLUE_PIN, HIGH);
  delay(100);
  digitalWrite(BLUE_PIN, LOW);
  delay(100);
}

void checkAndSetInRange(int* color)
{
  if (*color < 0) *color = 0;
  else if (*color > 255) *color = 255;
}

void setDefaultIfNotInRange(int* red, int* green, int* blue)
{
  checkAndSetInRange(red);
  checkAndSetInRange(green);
  checkAndSetInRange(blue);
}

void setLedColors(int red, int green, int blue)
{
  setDefaultIfNotInRange(&red, &green, &blue);

  analogWrite(RED_PIN, 1023 * red / 255);
  analogWrite(GREEN_PIN, 1023 * green / 255);
  analogWrite(BLUE_PIN, 1023 * blue / 255);
}

void setup() {
  Serial.begin(9600);
  delay(10);

  // prepare LEDS
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  flash();

  // Connect to WiFi network
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

  PubNub.begin(pubkey, subkey);
  Serial.println("PubNub set up");
}

const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + 50;
DynamicJsonBuffer jsonBuffer(bufferSize);

void subscribeMessage() {
  Serial.println("waiting for a message (subscribe)");
  PubSubClient *pclient = PubNub.subscribe(channel);
  if (!pclient) {
    Serial.println("subscription error");
    delay(1000);
    return;
  }
  char json[57];
  byte i = 0;
  while(pclient->wait_for_data()) {
    char c = pclient->read();
    if (!(c == '[' || c == ']')) {
      json[i++] = c;
    }
  }
  json[i] = 0;
  if (i == 0) return;

  pclient->stop();
  Serial.println();

  Serial.println(json);
  JsonObject& root = jsonBuffer.parseObject(json);
  JsonObject& value = root["value"];
  if (!value.success()) {
      Serial.println("Parsing failed");
      return;
  }

  int red = value["red"];
  int green = value["green"];
  int blue = value["blue"];

  setLedColors(red, green, blue);
  
}

void loop() {

  subscribeMessage();

  delay(10);

}
