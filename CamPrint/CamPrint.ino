#include <Arduino.h>
#define WIFI_SSID ""
#define WIFI_PASS ""

#define BTN_PIN 15
#define USE_MIFI 0
#define SSERIAL_RX 12
#define SSERIAL_TX 13

#include <WiFi.h>

// Принтер RX -> RS TX
// TTL TX -> Arduino RX

#include "printer.h"
Printer printer(Serial1);

#include <GyverHTTP.h>
ghttp::Server<WiFiServer, WiFiClient> server(80);

#include "bilinear.h"
#include "blur.h"
#include "camera.h"
#include "camtest.h"
#include "dithering.h"
#include "edges.h"

void printFrame();

void setup() {
    // printer
    Serial1.begin(9600, SERIAL_8N1, SSERIAL_RX, SSERIAL_TX);
    printer.begin();
    printer.config(10, 140, 4);

    cam_init(FRAMESIZE_VGA, PIXFORMAT_GRAYSCALE);
    pinMode(BTN_PIN, INPUT_PULLUP);

    if (USE_MIFI) {
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
                    server.sendFile_P((const uint8_t *)index_html, sizeof(index_html) - 1, "text/html");
                    break;

                case su::SH("/printFrame"):
                    printFrame();
                    break;

                case su::SH("/getFrame"): {
                    camera_fb_t *fbj = esp_camera_fb_get();
                    esp_camera_fb_return(fbj);

                    fbj = esp_camera_fb_get();
                    if (fbj) {
                        server.sendFile(fbj->buf, fbj->len);
                        esp_camera_fb_return(fbj);
                    }
                } break;
            }
        });
    }
}

void loop() {
    server.tick();

    if (!digitalRead(BTN_PIN)) printFrame();
}

void printFrame() {
    // skip frame
    camera_fb_t *fbj = esp_camera_fb_get();
    esp_camera_fb_return(fbj);

    fbj = esp_camera_fb_get();
    if (!fbj) {
        printer.println("Camera error");
        return;
    }

    int h = 384;
    int w = h * fbj->width / fbj->height;  // 512
    uint8_t *chunks = (uint8_t *)ps_malloc(w * h / 8);
    if (!chunks) {
        esp_camera_fb_return(fbj);
        return;
    }

    uint8_t *resized = (uint8_t *)ps_malloc(w * h);
    if (!resized) {
        free(chunks);
        esp_camera_fb_return(fbj);
        return;
    }

    bilinear_interp(fbj->buf, fbj->width, fbj->height, resized, w, h);
    // blur(resized, w, h);
    // edges(resized, w, h);
    dither(resized, w, h);

    int idx = 0;
    for (int x = 0; x < w; x++) {
        int y = h;
        while (y) {
            uint8_t b = 0;
            int i = 8;
            while (i--) {
                y--;
                b <<= 1;
                b |= !resized[x + y * w];
            }
            chunks[idx++] = b;
        }
    }

    free(resized);
    printer.drawBitmap(chunks, h, w);
    printer.println();
    printer.println();

    free(chunks);
    esp_camera_fb_return(fbj);
}