#include <WiFi.h>
#include <WebServer.h>

// Replace with your network credentials
const char *ssid = "Ultra";
const char *password = "tommaso00";

// Create an instance of the web server on port 80
WebServer server(80);

const int outputPin = 27;

String gateState = "1"; // CLOSED

// Timing constants
const unsigned long buttonPressTime = 1300; // time in milliseconds to simulate a button press
const unsigned long openDuration = 10000;   // time in milliseconds to wait before changing state to OPEN
const unsigned long closeDuration = 30000;  // time in milliseconds to wait before changing state to CLOSED

// Timing variables
unsigned long previousMillis = 0; // will store last time the action was updated
bool isButtonPressed = false;     // flag for button press
bool isOpeningGate = false;       // flag for gate opening
bool isClosingGate = false;       // flag for gate closing

void setup()
{
    Serial.begin(115200);
    pinMode(outputPin, OUTPUT);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        // Use non-blocking delay to connect to WiFi
        Serial.println("Connecting to WiFi...");
        delay(100); // This delay is acceptable since it's a short one for status update only
    }
    Serial.println("Connected to WiFi as:");
    Serial.println(WiFi.localIP());

    server.on("/open", HTTP_GET, []()
              {
        Serial.println(server.client().remoteIP().toString() + " " + server.uri());
        server.send(200, "text/plain", "OK");
        isButtonPressed = true;
        previousMillis = millis(); });

    server.on("/close", HTTP_GET, []()
              {
        Serial.println(server.client().remoteIP().toString() + " " + server.uri());
        server.send(200, "text/plain", "OK");
        isClosingGate = true;
        previousMillis = millis(); });

    server.on("/getState", HTTP_GET, []()
              {
        Serial.println(server.client().remoteIP().toString() + " " + server.uri());
        server.send(200, "text/plain", gateState); });

    server.begin();
}

void loop()
{
    server.handleClient();
    unsigned long currentMillis = millis();

    if (isButtonPressed)
    {
        digitalWrite(outputPin, HIGH);
        if (currentMillis - previousMillis >= buttonPressTime)
        {
            digitalWrite(outputPin, LOW);
            isButtonPressed = false;
            isOpeningGate = true;
            previousMillis = currentMillis;
        }
    }

    if (isOpeningGate && currentMillis - previousMillis >= openDuration)
    {
        gateState = "0"; // OPEN
        isOpeningGate = false;
        isClosingGate = true;
        previousMillis = currentMillis;
    }

    if (isClosingGate && currentMillis - previousMillis >= closeDuration)
    {
        gateState = "1"; // CLOSED
        isClosingGate = false;
    }
}
