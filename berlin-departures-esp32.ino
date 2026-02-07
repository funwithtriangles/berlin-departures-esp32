#include "bvg_connect.h"
#include "display.h"

#define LEO "900009102"
#define LEO_DIR "900009104"

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
        std::vector<int> minutes = getDepartureMinutes("subway", LEO, LEO_DIR);
        displayMinutes(0, minutes);

        Serial.print("LEO U6: ");
        for (size_t i = 0; i < minutes.size(); ++i)
        {
            Serial.print(minutes[i]);
            if (i < minutes.size() - 1)
                Serial.print(", ");
        }
        Serial.println();
        lastCheck = millis();
    }
}
