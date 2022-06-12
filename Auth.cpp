#include "Auth.h"
#include <utility>
#include <iostream>
#include <curl/curl.h>
#include "cJSON.h"
#include <unistd.h>

static char write_buffer[CURL_MAX_WRITE_SIZE];

static size_t write_callback(char *ptr, __attribute__((unused)) size_t size, size_t nmemb, __attribute__((unused)) void *userdata) {
    memcpy(write_buffer, ptr, nmemb);
    write_buffer[nmemb+1] = '\0';
    return nmemb;
}

class RetryOauthCodeGrant : public std::exception {};

class RefreshTokenExpired : public std::exception {};
class NewDeviceCodeNeeded : public std::exception {};
class OauthCodeGrantFailed : public RefreshTokenExpired, public NewDeviceCodeNeeded {};

Auth::Auth(std::string client_id) : client_id(std::move(client_id)) {
    this->curl = curl_easy_init();
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, write_callback);
}

void Auth::authenticate() {
    try {
        refresh_access_token();
    } catch (RefreshTokenExpired &refreshTokenExpired) {
        authenticate_user();
        std::cout << "Refresh token: " << this->refresh_token << std::endl;
    }
}

void Auth::refresh_access_token() {
    std::cout << "Refreshing access token" << std::endl;
    if(this->refresh_token.empty()) {
        throw RefreshTokenExpired();
    }

    oauth_code_grant_flow(
            "client_id="+client_id+
            "&refresh_token="+this->refresh_token+
            "&grant_type=refresh_token"
    );
}

void Auth::oauth_code_grant_flow(const std::string& postFields) {
    cJSON *error;
    cJSON *authResponseJSON;

    bool verified = false;
    while(!verified) {
        curl_easy_setopt(this->curl, CURLOPT_URL, "https://login.microsoftonline.com/common/oauth2/v2.0/token");
        curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, postFields.c_str());
        CURLcode res = curl_easy_perform(this->curl);

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        try {
            if (res != CURLE_OK) {
                std::cout << "Error: oauth code grant flow returned curl code " << res << std::endl;
                throw RetryOauthCodeGrant();
            }

            authResponseJSON = cJSON_Parse(write_buffer);

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

                verified = true;
            } else if (http_code >= 500) {
                throw RetryOauthCodeGrant();
            } else if (error != nullptr && cJSON_IsString(error)) {
                if (strcmp(error->valuestring, "temporarily_unavailable") == 0 ||
                    strcmp(error->valuestring, "authorization_pending") == 0
                ) {
                    throw RetryOauthCodeGrant();
                } else if (strcmp(error->valuestring, "authorization_declined") == 0) {
                    std::cout << "User declined permissions, trying again..." << std::endl;
                    throw OauthCodeGrantFailed();
                } else if (strcmp(error->valuestring, "bad_verification_code") == 0) {
                    std::cout << "Internal error, trying again..." << std::endl;
                    throw OauthCodeGrantFailed();
                } else if (strcmp(error->valuestring, "expired_token") == 0) {
                    std::cout << "Session timed out, trying again..." << std::endl;
                    throw OauthCodeGrantFailed();
                } else {
                    std::cout << "Got error from refresh token request: " << error->valuestring << std::endl;
                    throw OauthCodeGrantFailed();
                }
            } else {
                std::cout << "Error: oauth code grant flow returned HTTP code " << http_code << std::endl;
                throw OauthCodeGrantFailed();
            }

            cJSON_Delete(authResponseJSON);
        } catch (RetryOauthCodeGrant &retryOauthCodeGrant) {
            sleep(2);
            cJSON_Delete(authResponseJSON);
            continue;
        } catch (OauthCodeGrantFailed &oauthCodeGrantFailed) {
            cJSON_Delete(authResponseJSON);
            throw oauthCodeGrantFailed;
        }
    }
}

void Auth::authenticate_user() {
    cJSON *auth_message, *device_code;
    cJSON *authResponseJSON;

    std::string auth_request_fields = "client_id="+this->client_id+"&scope=offline_access%20user.read%20presence.read";
    std::string auth_verification_fields;

    bool authenticated = false;
    while(!authenticated) {
        curl_easy_setopt(this->curl, CURLOPT_URL, "https://login.microsoftonline.com/common/oauth2/v2.0/devicecode");
        curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, auth_request_fields.c_str());
        CURLcode res = curl_easy_perform(this->curl);

        try {
            if (res != CURLE_OK) {
                std::cout << "Error: auth POST returned curl code " << std::to_string(res) << std::endl;
                throw NewDeviceCodeNeeded();
            }

            authResponseJSON = cJSON_Parse(write_buffer);
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

            std::cout << auth_message->valuestring << std::endl;

            oauth_code_grant_flow(
                    "grant_type=urn:ietf:params:oauth:grant-type:device_code"
                    "&client_id="+this->client_id+
                    "&device_code="+device_code->valuestring
            );
            authenticated = true;
            cJSON_Delete(authResponseJSON);
        } catch (NewDeviceCodeNeeded &newAuthRequestNeeded) {
            sleep(2);
            cJSON_Delete(authResponseJSON);
            continue;
        }
    }
}

void Auth::set_refresh_token(const std::string& token) {
    this->refresh_token = token;
}

std::string Auth::get_access_token() {
    return this->access_token;
}
