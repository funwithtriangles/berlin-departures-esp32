#ifndef BVG_CONNECT_H
#define BVG_CONNECT_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "credentials.h"

#define LEO 900009102

void initWifi()
{
    Serial.println();
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
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
}

void getDepartureMinutes()
{
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(10000); // 10 seconds timeout
    HTTPClient http;

    // const char* keys[] = {"Transfer-Encoding"};
    // http.collectHeaders(keys, 1);

    http.useHTTP10(true);

    String url = "https://v6.bvg.transport.rest/stops/900009102/departures?duration=10&linesOfStops=false&remarks=false&language=en";
    http.begin(client, url);

    int httpCode = http.GET();
    if (httpCode <= 0) {
        Serial.print("HTTP GET failed: ");
        Serial.println(http.errorToString(httpCode));
        http.end();
        return;
    }
    // Stream& rawStream = http.getStream();
    // ChunkDecodingStream decodedStream(rawStream);
    // Stream& response = http.header("Transfer-Encoding") == "chunked" ? decodedStream : rawStream;

    // Serial.println(http.header("Transfer-Encoding"));

    // JsonDocument doc;

    // DeserializationError error = deserializeJson(doc, response);

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
    // Optionally log the payload
    Serial.println("--- Raw JSON payload ---");
    Serial.println(payload);
    Serial.println("------------------------");
    // Parse JSON from the String
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
        Serial.print(lineName);
        Serial.print(" -> ");
        Serial.print(direction);
        Serial.print(" | Platform: ");
        Serial.print(platform);
        Serial.print(" | When: ");
        Serial.println(when);
    }
    Serial.println("------------------\n");
}

#endif
