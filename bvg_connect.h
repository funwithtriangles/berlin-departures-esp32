#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "credentials.h"

#define TIMEZONE_OFFSET 3600 // Berlin time offset in seconds

// Global NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Converts an ISO8601 timestamp (e.g. "2024-01-29T19:32:00+01:00") to epoch seconds (UTC)
time_t getEpochFromTimestamp(const char* when) {
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
    return eventTime;
}

// Returns minutes from now for a given ISO8601 timestamp
int getMinutesFromNow(const char* when) {
    time_t eventTime = getEpochFromTimestamp(when);
    if (eventTime == -1) return -1;
    time_t now = time(NULL);
    return (eventTime - now) / 60;
}

void initWifi() {
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
    // timeClient.setTimeOffset(TIMEZONE_OFFSET); // Use UTC, let ISO8601 parsing handle timezone
    while (!timeClient.update()) {
        delay(100);
    }
    // Set system time from NTP (UTC)
    time_t epochTime = timeClient.getEpochTime();
    struct timeval tv = { epochTime, 0 };
    settimeofday(&tv, nullptr);
}

#include <vector>
bool getDepartureMinutes(const char* transportType, const char* station, const char* direction, int fromWhen, std::vector<int>& minutesList) {
    Serial.println("Fetching departures from BVG API... ");
    minutesList.clear();
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15); // 15 second TLS timeout
    HTTPClient http;

    http.useHTTP10(true);
    http.setTimeout(15000); // 15 second HTTP timeout
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);


    // List of all supported transport types
    const char* types[] = {"bus", "subway", "tram", "ferry", "express", "regional"};
    // Calculate 'when' parameter as URL-encoded ISO8601 string
    // e.g. 2026-02-07T17%3A44%3A00%2B01%3A00
    time_t now = time(NULL);
    time_t whenTs = now + fromWhen * 60; // fromWhen is in minutes
    time_t localWhenTs = whenTs + TIMEZONE_OFFSET;
    struct tm tmBuf;
    gmtime_r(&localWhenTs, &tmBuf);
    int tzH = TIMEZONE_OFFSET / 3600;
    int tzM = (TIMEZONE_OFFSET % 3600) / 60;
    char whenStr[64];
    snprintf(whenStr, sizeof(whenStr),
        "%04d-%02d-%02dT%02d%%3A%02d%%3A%02d%%2B%02d%%3A%02d",
        tmBuf.tm_year + 1900, tmBuf.tm_mon + 1, tmBuf.tm_mday,
        tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec,
        tzH, tzM);

    String url = "https://v6.bvg.transport.rest/stops/";
    url += station;
    url += "/departures?direction=";
    url += direction;
    url += "&when=";
    url += whenStr;
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i) {
        url += "&";
        url += types[i];
        url += "=";
        url += (strcmp(types[i], transportType) == 0) ? "true" : "false";
    }
    url += "&linesOfStops=false&remarks=false&duration=60&results=3";

    http.begin(client, url);
    Serial.println(url);

    int httpCode = http.GET();
    if (httpCode != 200) {
        Serial.print("HTTP GET failed, code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }
    // Read full response then parse JSON
    String payload = http.getString();
    http.end();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        return false;
    }
    JsonArray departures = doc["departures"].as<JsonArray>();
    for (JsonObject departure : departures) {
        const char* when = departure["when"] | "N/A";
        int minutes = getMinutesFromNow(when);
        minutesList.push_back(minutes);
    }
    return true;
}
