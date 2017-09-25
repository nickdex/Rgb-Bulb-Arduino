#include <ESP8266WiFi.h>
#define PubNub_BASE_CLIENT WiFiClient
//#define PUBNUB_DEBUG
#define PUBNUB_DEFINE_STRSPN_AND_STRNCASECMP
#include <PubNub.h>

const char* ssid = "Web";
const char* password = "simple#9";

const int subLedPin = 2; // Blue
const int pubLedPin = 4; // Green

char pubkey[] = "pub-c-a7a7bf57-7428-4acf-9feb-95aa76439442";
char subkey[] = "sub-c-28e8300e-7e94-11e7-a179-1e66dc778462";
char channel[] = "pi-house";

void setup() {
  Serial.begin(9600);
  delay(10);

  // prepare LEDS
  pinMode(subLedPin, OUTPUT);
  digitalWrite(subLedPin, LOW);

  pinMode(pubLedPin, OUTPUT);
  digitalWrite(pubLedPin, LOW);

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

void flash(int ledPin) {
  /* Flash LED three times. */
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}


// Publishes A Message
void publishMessage() {
  WiFiClient *client;
  Serial.println("publishing a message");
  client = PubNub.publish(channel, "\"Hi from ESP\"");
  if (!client) {
    Serial.println("publishing error");
    delay(1000);
    return;
  }
  while(client->connected()) {
    while(client->connected() && !client->available()); //wait
    char c = client->read();
    Serial.print(c);
  }
  client->stop();
  Serial.println();
  flash(pubLedPin);
}

void subscribeMessage() {
  Serial.println("waiting for a message (subscribe)");
  PubSubClient *pclient = PubNub.subscribe(channel);
  if (!pclient) {
    Serial.println("subscription error");
    delay(1000);
    return;
  }
  while(pclient->wait_for_data()) {
    char c = pclient->read();
    Serial.print(c);
  }
  pclient->stop();
  Serial.println();
  flash(subLedPin);
}

void loop() {

//  publishMessage();

  subscribeMessage();

  delay(10000);

}
