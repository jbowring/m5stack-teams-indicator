#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include "M5Atom.h"

#include "WiFiNetworks.h"
#include "ClientID.h"
#include "Graph.h"

// DigiCert Global Root CA
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
"-----END CERTIFICATE-----\n";

void drawFilledCircle(CRGB colour) {
  M5.dis.fillpix(colour);
  M5.dis.drawpix(0,0,0);
  M5.dis.drawpix(0,4,0);
  M5.dis.drawpix(4,0,0);
  M5.dis.drawpix(4,4,0);
}

void drawHollowCircle(CRGB colour) {
  static uint8_t pixels[] = { 1,2,3,5,9,10,14,15,19,21,22,23 };
  static uint8_t i;

  M5.dis.fillpix(0);

  for(i = 0; i < sizeof(pixels); i++) {
    M5.dis.drawpix(pixels[i], colour);
  }
}

void drawArrow(CRGB colour) {
  static uint8_t pixels[] = { 2,8,10,11,12,13,14,18,22 };
  static uint8_t i;

  M5.dis.fillpix(0);

  for(i = 0; i < sizeof(pixels); i++) {
    M5.dis.drawpix(pixels[i], colour);
  }
}

void doNotDisturb() {
  drawFilledCircle(0xff0000);
  M5.dis.drawpix(1,2,0xffffff);
  M5.dis.drawpix(2,2,0xffffff);
  M5.dis.drawpix(3,2,0xffffff);
}

void busy() {
  drawFilledCircle(0xff0000);
}

void busyOOO() {
  drawHollowCircle(0xff0000);
}

void active() {
  drawFilledCircle(0x00ff00);
}

void activeOOO() {
  drawHollowCircle(0x00ff00);
}

void away() {
  drawFilledCircle(0xffdf00);
}

void outOfOffice() {
  drawArrow(0xff00ff);
}

WiFiMulti WiFiMulti;

void WiFiReconnect (arduino_event_t *event = nullptr) {
    Serial.print("Waiting for WiFi to connect...");
    while ((WiFiMulti.run() != WL_CONNECTED)) {
      Serial.print(".");
      delay(500);
    }
    Serial.println(" connected");
}

void setup() {
  M5.begin(true, false, true);
  delay(50);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  for(const auto& network : known_ssids) {
    WiFiMulti.addAP(network.ssid.c_str(), network.password.c_str());
  }

  WiFi.onEvent(WiFiReconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // wait for WiFi connection
  WiFiReconnect();
}

void loop() {
  Graph graph(clientId);
  Presence presence;
  graph.authenticate();

  while(true) {
    presence = graph.get_presence();

    Serial.println("availability: " + presence.availability + ", activity: " + presence.activity);

    if (presence.availability == "Available") {
      active();
    } else if (presence.availability == "Away" || presence.availability == "BeRightBack") {
      if(presence.activity == "OutOfOffice") {
        outOfOffice();
      } else {
        away();
      }
    } else if (presence.availability == "Busy") {
      busy();
    } else if (presence.availability == "DoNotDisturb") {
      doNotDisturb();
    } else {
      M5.dis.fillpix(0);
    }

    delay(250);
  }
}
