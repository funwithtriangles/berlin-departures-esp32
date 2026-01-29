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
    // Fetch departures every 15 seconds
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck >= 15000)
    {
        lastCheck = millis();
        getDepartureMinutes();
    }
}
