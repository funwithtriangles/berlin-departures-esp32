#include "bvg_connect.h"
#include "display.h"

// Station IDs, get them from
// https://petstore.swagger.io/?url=https%3A%2F%2Fv6.bvg.transport.rest%2F.well-known%2Fservice-desc%0A#/default/get_stops
#define LEO "900009102"
#define LEO_DIR "900009104" // DIR is the next stop on the line in that direction

#define AMRUMER "900009101"
#define AMRUMER_DIR "900001201"

std::vector<int> mins;

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

        mins = getDepartureMinutes("subway", LEO, LEO_DIR);
        displayMinutes(0, mins);

        mins = getDepartureMinutes("subway", AMRUMER, AMRUMER_DIR);
        displayMinutes(1, mins);
    }
}
