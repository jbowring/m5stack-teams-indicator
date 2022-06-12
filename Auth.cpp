#include "Auth.h"
#include "cJSON.h"
#include <HTTPClient.h>
#include <Preferences.h>

static Preferences prefs;

class RetryOauthCodeGrant : public std::exception {};

class RefreshTokenExpired : public std::exception {};
class NewDeviceCodeNeeded : public std::exception {};
class OauthCodeGrantFailed : public RefreshTokenExpired, public NewDeviceCodeNeeded {};

// DigiCert Global Root CA
static const char* rootCACertificate = \
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

Auth::Auth(String client_id) : client_id(client_id) {
  this->wifi_client.setCACert(rootCACertificate);
  prefs.begin("refresh_token");
  this->refresh_token = prefs.getString("refresh_token");
  Serial.print("Got stored refresh token: ");
  Serial.println(this->refresh_token);
};

void Auth::authenticate() {
    try {
        refresh_access_token();
    } catch (RefreshTokenExpired &refreshTokenExpired) {
        authenticate_user();
        Serial.print("Refresh token: ");
        Serial.println(this->refresh_token);
    }
}

void Auth::refresh_access_token() {
    Serial.println("Refreshing access token");
    if(this->refresh_token.isEmpty()) {
        throw RefreshTokenExpired();
    }

    oauth_code_grant_flow(
            "client_id="+client_id+
            "&refresh_token="+this->refresh_token+
            "&grant_type=refresh_token"
    );
}

void Auth::oauth_code_grant_flow(const String& postFields) {
    cJSON *error;
    cJSON *authResponseJSON;
    HTTPClient https;

    bool verified = false;
    while(!verified) {
        https.begin(this->wifi_client, "https://login.microsoftonline.com/common/oauth2/v2.0/token");
        int http_code = https.POST(postFields);

        try {
            if (http_code < 0) {
                Serial.print("Error: oauth code grant flow returned http code ");
                Serial.println(http_code);
                throw RetryOauthCodeGrant();
            }

            authResponseJSON = cJSON_Parse(https.getString().c_str());

            if (authResponseJSON == nullptr) {
                throw RetryOauthCodeGrant();
            }

            error = cJSON_GetObjectItemCaseSensitive(authResponseJSON, "error");

            if(200 <= http_code && http_code < 300) {
                cJSON *access_token_json = cJSON_GetObjectItemCaseSensitive(authResponseJSON, "access_token");
                if (!cJSON_IsString(access_token_json) || (access_token_json->valuestring == nullptr)) {
                    throw RetryOauthCodeGrant();
                }

                cJSON *refresh_token_json = cJSON_GetObjectItemCaseSensitive(authResponseJSON, "refresh_token");
                if (!cJSON_IsString(refresh_token_json) || (refresh_token_json->valuestring == nullptr)) {
                    throw RetryOauthCodeGrant();
                }

                this->access_token = access_token_json->valuestring;
                this->refresh_token = refresh_token_json->valuestring;
                prefs.putString("refresh_token", this->refresh_token);

                verified = true;
            } else if (http_code >= 500) {
                throw RetryOauthCodeGrant();
            } else if (error != nullptr && cJSON_IsString(error)) {
                if (strcmp(error->valuestring, "temporarily_unavailable") == 0 ||
                    strcmp(error->valuestring, "authorization_pending") == 0
                ) {
                    throw RetryOauthCodeGrant();
                } else if (strcmp(error->valuestring, "authorization_declined") == 0) {
                    Serial.println("User declined permissions, trying again...");
                    throw OauthCodeGrantFailed();
                } else if (strcmp(error->valuestring, "bad_verification_code") == 0) {
                    Serial.println("Internal error, trying again...");
                    throw OauthCodeGrantFailed();
                } else if (strcmp(error->valuestring, "expired_token") == 0) {
                    Serial.println("Session timed out, trying again...");
                    throw OauthCodeGrantFailed();
                } else {
                    Serial.print("Got error from refresh token request: ");
                    Serial.println(error->valuestring);
                    throw OauthCodeGrantFailed();
                }
            } else {
                Serial.print("Error: oauth code grant flow returned HTTP code ");
                Serial.println(http_code);
                throw OauthCodeGrantFailed();
            }

            https.end();
            cJSON_Delete(authResponseJSON);
        } catch (RetryOauthCodeGrant &retryOauthCodeGrant) {
            https.end();
            cJSON_Delete(authResponseJSON);
            delay(2000);
            continue;
        } catch (OauthCodeGrantFailed &oauthCodeGrantFailed) {
            https.end();
            cJSON_Delete(authResponseJSON);
            throw oauthCodeGrantFailed;
        }
    }
}

void Auth::authenticate_user() {
    cJSON *auth_message, *device_code;
    cJSON *authResponseJSON;
    HTTPClient https;

    bool authenticated = false;
    while(!authenticated) {
      https.begin(this->wifi_client, "https://login.microsoftonline.com/common/oauth2/v2.0/devicecode");
      int http_code = https.POST("client_id="+this->client_id+"&scope=offline_access%20user.read%20presence.read");

        try {
            if (http_code < 0) {
                Serial.print("Error: auth POST returned http code ");
                Serial.println(http_code);
                throw NewDeviceCodeNeeded();
            }

            authResponseJSON = cJSON_Parse(https.getString().c_str());
            if (authResponseJSON == nullptr) {
                throw NewDeviceCodeNeeded();
            }

            auth_message = cJSON_GetObjectItemCaseSensitive(authResponseJSON, "message");
            if (!cJSON_IsString(auth_message) || (auth_message->valuestring == nullptr)) {
                throw NewDeviceCodeNeeded();
            }

            device_code = cJSON_GetObjectItemCaseSensitive(authResponseJSON, "device_code");
            if (!cJSON_IsString(device_code) || (device_code->valuestring == nullptr)) {
                throw NewDeviceCodeNeeded();
            }

            Serial.println(auth_message->valuestring);

            oauth_code_grant_flow(
                    "grant_type=urn:ietf:params:oauth:grant-type:device_code"
                    "&client_id="+this->client_id+
                    "&device_code="+device_code->valuestring
            );
            authenticated = true;
            https.end();
            cJSON_Delete(authResponseJSON);
        } catch (NewDeviceCodeNeeded &newAuthRequestNeeded) {
            https.end();
            cJSON_Delete(authResponseJSON);
            delay(2000);
            continue;
        }
    }
}

String Auth::get_access_token() {
    return this->access_token;
}
