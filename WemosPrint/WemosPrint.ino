#include <Arduino.h>
#define WIFI_SSID ""
#define WIFI_PASS ""

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// Принтер RX -> RS TX
// TTL TX -> Arduino RX

#include <SoftwareSerial.h>
SoftwareSerial uart(D1, D2);

#include "printer.h"
Printer printer(uart);

#include <GyverHTTP.h>
ghttp::Server<WiFiServer, WiFiClient> server(80);

#include "bitmaper.h"

void setup() {
    // WIFI
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    if (strlen(WIFI_SSID)) {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        uint8_t tries = 20;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            if (!--tries) break;
        }
        Serial.print("Connected: ");
        Serial.println(WiFi.localIP());
    }

    WiFi.softAP("AP ESP");
    Serial.print("AP: ");
    Serial.println(WiFi.softAPIP());

    // printer
    uart.begin(9600);
    printer.begin();
    printer.config(10, 140, 4);
    printer.print("Local IP: ");
    printer.println(WiFi.localIP());
    printer.print("AP IP: ");
    printer.println(WiFi.softAPIP());
    printer.println();
    printer.println();

    server.begin();
    server.onRequest([](ghttp::ServerBase::Request req) {
        switch (req.path().hash()) {
            case su::SH("/"):
                server.sendFile_P(bitmaper_index, sizeof(bitmaper_index), "text/html", false, true);
                break;

            case su::SH("/script.js"):
                server.sendFile_P(bitmaper_script, sizeof(bitmaper_script), "text/javascript", true, true);
                break;

            case su::SH("/style.css"):
                server.sendFile_P(bitmaper_style, sizeof(bitmaper_style), "text/css", true, true);
                break;

            case su::SH("/bitmap"): {
                uint16_t w = req.param("width");
                uint16_t h = req.param("height");
                if (w && h && req.body()) {
                    uint16_t len = ((w + 8 - 1) / 8) * h;
                    if (len != req.body().length()) break;

                    // from stream
                    printer.drawBitmap(req.body().stream, w, h);
                    printer.println();
                    printer.println();

                    // from ram
                    // uint8_t* buf = new uint8_t[len];
                    // if (buf && req.body().readBytes(buf)) {
                    //     printer.drawBitmap(buf, w, h, false);
                    //     printer.println();
                    //     printer.println();
                    // }
                    // delete[] buf;
                }
                server.send(200);
            } break;

            default:
                server.send(200);
                break;
        }
    });
}

void loop() {
    server.tick();
}