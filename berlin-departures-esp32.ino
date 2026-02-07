#include "bvg_connect.h"
#include "display.h"

// Station IDs, get them from
// https://petstore.swagger.io/?url=https%3A%2F%2Fv6.bvg.transport.rest%2F.well-known%2Fservice-desc%0A#/default/get_stops
#define LEO "900009102"
#define LEO_DIR "900009104" // DIR is the next stop on the line in that direction
#define AMRUMER "900009101"
#define AMRUMER_DIR "900001201"
#define AMRUMER_SEESTR "900009105"
#define AMRUMER_SEESTR_DIR "900009103"

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

        std::vector<int> mins;

        if (getDepartureMinutes("subway", LEO, LEO_DIR, 9, mins))
            displayMinutes(0, mins);

        if (getDepartureMinutes("subway", AMRUMER, AMRUMER_DIR, 9, mins))
            displayMinutes(1, mins);

        if (getDepartureMinutes("tram", AMRUMER_SEESTR, AMRUMER_SEESTR_DIR, 3, mins))
            displayMinutes(2, mins);
    }
}
