#include "bvg_connect.h"
#include "display.h"

void setup()
{
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);

    // Connect to WiFi and test connection
    initWifi();

    // Initialize display and draw UI
    initDisplay();
}

void loop()
{
    // Test connection every 30 seconds
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 30000)
    {
        lastCheck = millis();

        Serial.print("Testing connection... ");
        HTTPClient http;
        http.begin("http://www.google.com");
        int httpCode = http.GET();
        if (httpCode > 0)
        {
            Serial.print("OK (HTTP ");
            Serial.print(httpCode);
            Serial.println(")");
        }
        else
        {
            Serial.print("Failed: ");
            Serial.println(http.errorToString(httpCode));
        }
        http.end();
    }
}
