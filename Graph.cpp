#include "Graph.h"
#include "cjson/cJSON.h"
#include <Arduino.h>

// DigiCert Global Root G2
static const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
"-----END CERTIFICATE-----\n";

Graph::Graph(const String& client_id) : auth(client_id) {
    this->https.begin(this->wifi_client, "https://graph.microsoft.com/v1.0/me/presence");
    this->wifi_client.setCACert(rootCACertificate);
}

class RequestFailed : public std::exception {};

void Graph::authenticate() {
    this->auth.authenticate();
}

Presence Graph::get_presence() {
    return get_presence(true);
}

Presence Graph::get_presence(bool auto_reauthenticate) {

    cJSON * responseJSON = nullptr;
    Presence presence;

    bool success = false;
    while(!success) {
        this->https.addHeader("Authorization", "Bearer " + this->auth.get_access_token(), false, true);
        int http_code = this->https.GET();
        try {
            if (http_code < 0) {
                Serial.print("Error: presence GET returned http code ");
                Serial.println(http_code);
                throw RequestFailed();
            }

            if (http_code == 401) {
                // Access token expired
                if (auto_reauthenticate) {
                    this->authenticate();
                    return this->get_presence(false);
                } else {
                    throw RequestFailed();
                }
            } else if (http_code == 429) {
                Serial.print("Error: presence GET rate-limited! Returned HTTP code ");
                Serial.println(http_code);
                throw RequestFailed();
            } else if (http_code < 200 || 299 < http_code) {
                Serial.print("Error: presence GET returned HTTP code ");
                Serial.println(http_code);
                throw RequestFailed();
            }

            responseJSON = cJSON_Parse(https.getString().c_str());
            if (responseJSON == nullptr) {
                throw RequestFailed();
            }

            cJSON *error = cJSON_GetObjectItemCaseSensitive(responseJSON, "error");
            if (error != nullptr && cJSON_IsString(error)) {
                Serial.print("Got error from presence request: ");
                Serial.println(error->valuestring);
                throw RequestFailed();
            }

            cJSON *availability_json = cJSON_GetObjectItemCaseSensitive(responseJSON, "availability");
            if (!cJSON_IsString(availability_json) || (availability_json->valuestring == nullptr)) {
                throw RequestFailed();
            }

            cJSON *activity_json = cJSON_GetObjectItemCaseSensitive(responseJSON, "activity");
            if (!cJSON_IsString(activity_json) || (activity_json->valuestring == nullptr)) {
                throw RequestFailed();
            }

            presence.activity = activity_json->valuestring;
            presence.availability = availability_json->valuestring;

            https.end();
            cJSON_Delete(responseJSON);
            success = true;
        } catch (RequestFailed &requestFailed) {
            https.end();
            cJSON_Delete(responseJSON);
            delay(2000);
        }
    }

    return presence;
}
