#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
// Global NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Parses an ISO8601 timestamp (e.g. "2024-01-29T19:32:00+01:00") and returns minutes from now as int
int getMinutesFromNow(const char* when) {
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    int year, month, day, hour, min, sec, tzh = 0, tzm = 0;
    char tzsign = '+';
    // Try to parse with timezone offset
    int n = sscanf(when, "%4d-%2d-%2dT%2d:%2d:%2d%c%2d:%2d",
        &year, &month, &day, &hour, &min, &sec, &tzsign, &tzh, &tzm);
    if (n < 6) return -1; // parse error
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    // Convert to UTC by subtracting the offset
    time_t eventTime = mktime(&tm);
    if (n == 9) {
        int offset = tzh * 3600 + tzm * 60;
        if (tzsign == '+') eventTime -= offset;
        else eventTime += offset;
    }
    if (eventTime == -1) return -1;
    time_t now = time(NULL);
    Serial.print("Now: ");
    Serial.println(ctime(&now));
    int diff = (int)difftime(eventTime, now) / 60;
    return diff;
}
#ifndef BVG_CONNECT_H
#define BVG_CONNECT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "credentials.h"

#define LEO 900009102
#define TIMEZONE_OFFSET 3600 // Berlin time offset in seconds

void initWifi()
{
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    // Set a static DNS server (Google DNS) for better stability
    // Use WiFi.config to set only DNS, keeping IP/gateway/subnet dynamic (DHCP)
    WiFi.config(IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(0,0,0,0), IPAddress(8,8,8,8));
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    wifi_mode_t mode = WiFi.getMode();
    Serial.print("Current WiFi mode: ");
    Serial.println(mode == WIFI_STA ? "WIFI_STA" : mode == WIFI_AP ? "WIFI_AP" : "WIFI_AP_STA");



    // Test connection to google.com using WiFiClientSecure
    Serial.print("Testing connection to google.com... ");
    WiFiClientSecure client;
    client.setInsecure();
    if (!client.connect("www.google.com", 443)) {
        Serial.println("Failed!");
    } else {
        Serial.println("Success!");
        client.stop();
    }

    // Start and sync NTP client
    timeClient.begin();
    timeClient.setTimeOffset(0); // Use UTC, let ISO8601 parsing handle timezone
    while (!timeClient.update()) {
        delay(100);
    }
    // Set system time from NTP (UTC)
    time_t epochTime = timeClient.getEpochTime();
    struct timeval tv = { epochTime, 0 };
    settimeofday(&tv, nullptr);
}

void getDepartureMinutes()
{
    Serial.print("Fetching departures from BVG API... ");
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    http.useHTTP10(true);

    String url = "https://v6.bvg.transport.rest/stops/900009102/departures?direction=900009104&duration=45&bus=false&linesOfStops=false&remarks=false&language=en";
    http.begin(client, url);

    int httpCode = http.GET();
    if (httpCode <= 0) {
        Serial.print("HTTP GET failed: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return;
    }
    
    // Read the entire response into a String
    String payload;
    Stream& responseStream = http.getStream();
    unsigned long startMillis = millis();
    while (http.connected() && (millis() - startMillis < 10000)) {
        while (responseStream.available()) {
            char c = responseStream.read();
            payload += c;
        }
    }
    http.end();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }
    JsonArray departures = doc["departures"].as<JsonArray>();
    Serial.println("\n--- Departures ---");
    for (JsonObject departure : departures) {
        const char* platform = departure["platform"] | "N/A";
        const char* when = departure["when"] | "N/A";
        const char* lineName = departure["line"]["name"] | "?";
        const char* direction = departure["direction"] | "?";
        int minutes = getMinutesFromNow(when);
        Serial.print(lineName);
        Serial.print(" -> ");
        Serial.print(direction);
        Serial.print(" | Platform: ");
        Serial.print(platform);
        Serial.print(" | When: ");
        Serial.print(when);
        Serial.print(" | Minutes from now: ");
        Serial.println(minutes);
    }
    Serial.println("------------------\n");
}

#endif
