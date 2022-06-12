#include <iostream>
#include <unistd.h>
#include "Graph.h"
#include "cJSON.h"

static char write_buffer[CURL_MAX_WRITE_SIZE];

static size_t write_callback(char *ptr, __attribute__((unused)) size_t size, size_t nmemb, __attribute__((unused)) void *userdata) {
    memcpy(write_buffer, ptr, nmemb);
    write_buffer[nmemb+1] = '\0';
    return nmemb;
}

Graph::Graph(const std::string& client_id) : auth(client_id) {
    this->curl = curl_easy_init();
    curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_URL, "https://graph.microsoft.com/v1.0/me/presence");
}

class RequestFailed : public std::exception {};

void Graph::authenticate() {
    this->auth.authenticate();
}

void Graph::authenticate(const std::string& refresh_token) {
    this->auth.set_refresh_token(refresh_token);
    this->auth.authenticate();
}

Presence Graph::get_presence() {
    return get_presence(true);
}

Presence Graph::get_presence(bool auto_reauthenticate) {
    struct curl_slist *list = nullptr;
    list = curl_slist_append(list, ("Authorization: Bearer " + this->auth.get_access_token()).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    CURLcode res = curl_easy_perform(this->curl);
    curl_slist_free_all(list);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    cJSON * responseJSON = nullptr;
    Presence presence;

    bool success = false;
    while(!success) {
        try {
            if (res != CURLE_OK) {
                std::cout << "Error: presence GET returned curl code " << res << std::endl;
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
            } else if (http_code < 200 || 299 < http_code) {
                std::cout << "Error: presence GET returned HTTP code " << http_code << std::endl;
                throw RequestFailed();
            }

            responseJSON = cJSON_Parse(write_buffer);
            if (responseJSON == nullptr) {
                throw RequestFailed();
            }

            cJSON *error = cJSON_GetObjectItemCaseSensitive(responseJSON, "error");
            if (error != nullptr && cJSON_IsString(error)) {
                std::cout << "Got error from presence request: " << error->valuestring << std::endl;
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

            cJSON_Delete(responseJSON);
            success = true;
        } catch (RequestFailed &requestFailed) {
            cJSON_Delete(responseJSON);
            sleep(2);
        }
    }

    return presence;
}
