#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#define PubNub_BASE_CLIENT WiFiClient
//#define PUBNUB_DEBUG
#define PUBNUB_DEFINE_STRSPN_AND_STRNCASECMP
#include <PubNub.h>
#include <RGBColors.h>

const char *ssid = "Web";
const char *password = "simple#9";

#define BLUE_PIN 5
#define GREEN_PIN 4
#define RED_PIN 2

char pubkey[] = "pub-c-a7a7bf57-7428-4acf-9feb-95aa76439442";
char subkey[] = "sub-c-28e8300e-7e94-11e7-a179-1e66dc778462";
char channel[] = "pi-house";

void flash()
{
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

void checkAndSetInRange(int *color)
{
    if (*color < 0)
        *color = 0;
    else if (*color > 255)
        *color = 255;
}

void setDefaultIfNotInRange(int *red, int *green, int *blue)
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

// Set initial color
int redVal = 0;
int grnVal = 0;
int bluVal = 0;

// Initialize color variables
int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

int wait = 5;      // 10ms internal crossFade delay; increase for slower fades
int hold = 0;       // Optional hold when a color is complete, before the next crossFade
int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial
int loopCount = 60; // How often should DEBUG report?

int calculateStep(int prevValue, int endValue)
{
    int step = endValue - prevValue; // What's the overall gap?
    if (step)
    {                       // If its non-zero,
        step = 1020 / step; //   divide by 1020
    }
    return step;
}

int calculateVal(int step, int val, int i)
{

    if ((step) && i % step == 0)
    { // If step is non-zero and its time to change a value,
        if (step > 0)
        { //   increment the value if step is positive...
            val += 1;
        }
        else if (step < 0)
        { //   ...or decrement it if step is negative
            val -= 1;
        }
    }
    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255)
    {
        val = 255;
    }
    else if (val < 0)
    {
        val = 0;
    }
    return val;
}

void crossFade(struct RGB color)
{
    int R = color.r;
    int G = color.g;
    int B = color.b;

    int stepR = calculateStep(prevR, R);
    int stepG = calculateStep(prevG, G);
    int stepB = calculateStep(prevB, B);

    for (int i = 0; i <= 1020; i++)
    {
        redVal = calculateVal(stepR, redVal, i);
        grnVal = calculateVal(stepG, grnVal, i);
        bluVal = calculateVal(stepB, bluVal, i);

        setLedColors(redVal, grnVal, bluVal);

        delay(wait); // Pause for 'wait' milliseconds before resuming the loop

        if (DEBUG)
        { // If we want serial output, print it at the
            if (i == 0 or i % loopCount == 0)
            { // beginning, and every loopCount times
                Serial.print("Loop/RGB: #");
                Serial.print(i);
                Serial.print(" | ");
                Serial.print(redVal);
                Serial.print(" / ");
                Serial.print(grnVal);
                Serial.print(" / ");
                Serial.println(bluVal);
            }
            DEBUG += 1;
        }
    }
    // Update current values for next loop
    prevR = redVal;
    prevG = grnVal;
    prevB = bluVal;
    delay(hold); // Pause for optional 'wait' milliseconds before resuming the loop
}

void startCrossfade()
{
    RGB red; 
    red.r=255; red.g=0; red.b=0;

    RGB green; 
    green.g=255; green.b=0; green.r=0;

    RGB blue;
    blue.b=255;blue.g=0; blue.r=0;

    crossFade(green);
    crossFade(red);
    crossFade(blue);
}

void setup()
{
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

    while (WiFi.status() != WL_CONNECTED)
    {
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

void subscribeMessage()
{
    Serial.println("waiting for a message (subscribe)");
    PubSubClient *pclient = PubNub.subscribe(channel);
    if (!pclient)
    {
        Serial.println("subscription error");
        delay(1000);
        return;
    }
    char json[57];
    byte i = 0;
    while (pclient->wait_for_data())
    {
        char c = pclient->read();
        if (!(c == '[' || c == ']'))
        {
            json[i++] = c;
        }
    }
    json[i] = 0;
    if (i == 0)
        return;

    pclient->stop();
    Serial.println();

    Serial.println(json);
    JsonObject &root = jsonBuffer.parseObject(json);
    JsonObject &value = root["value"];
    if (!value.success())
    {
        Serial.println("Parsing failed");
        return;
    }

    int red = value["red"];
    int green = value["green"];
    int blue = value["blue"];

    setLedColors(red, green, blue);
}

void loop()
{
    //subscribeMessage();
    startCrossfade();
    
    delay(10);
}
