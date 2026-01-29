#ifndef BVG_CONNECT_H
#define BVG_CONNECT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "credentials.h"

void initWifi()
{
    // Connect to WiFi
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

    // Test connection to google.com
    Serial.print("Testing connection to google.com... ");
    HTTPClient http;
    http.begin("http://www.google.com");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.print("Success! HTTP code: ");
        Serial.println(httpCode);
    }
    else
    {
        Serial.print("Failed! Error: ");
        Serial.println(http.errorToString(httpCode));
    }
    http.end();
}

#endif
