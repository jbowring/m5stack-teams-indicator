//
// Created by Joel on 11/06/2022.
//

#include <iostream>
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

    this->auth.set_access_token("eyJ0eXAiOiJKV1QiLCJub25jZSI6ImY2U3o3cjBpRzFodWViN2tEWVRFRWJIckZVd3VOVXFkanRzdjE3eWFQWjgiLCJhbGciOiJSUzI1NiIsIng1dCI6ImpTMVhvMU9XRGpfNTJ2YndHTmd2UU8yVnpNYyIsImtpZCI6ImpTMVhvMU9XRGpfNTJ2YndHTmd2UU8yVnpNYyJ9.eyJhdWQiOiIwMDAwMDAwMy0wMDAwLTAwMDAtYzAwMC0wMDAwMDAwMDAwMDAiLCJpc3MiOiJodHRwczovL3N0cy53aW5kb3dzLm5ldC8zY2JhOTljZC1hMDYxLTQ2YzAtODY3Mi1jZWE4NzlkNTc4ZTYvIiwiaWF0IjoxNjU0OTYwNDMzLCJuYmYiOjE2NTQ5NjA0MzMsImV4cCI6MTY1NDk2NTg2MiwiYWNjdCI6MCwiYWNyIjoiMSIsImFpbyI6IkFWUUFxLzhUQUFBQUhZd0tQRVAwUDR2SDlWRzloTURWelVFYStvZktHTUxLc1hVL1hFS2pSZk5zbVJBQmtQVVBqQjhtVG1oc1hQZkEvdUYvWnhnLzdBQWJTWUxkeWNteFZZM2N3UkZreklvSUl5Tzk4Q09oMDBFPSIsImFtciI6WyJwd2QiLCJtZmEiXSwiYXBwX2Rpc3BsYXluYW1lIjoiR3JhcGggUHl0aG9uIHF1aWNrIHN0YXJ0IiwiYXBwaWQiOiI3NDAwYTNlMi0zZmQ4LTRiOTYtOGRiZS1iY2Y1MWE0YzdlOGYiLCJhcHBpZGFjciI6IjAiLCJmYW1pbHlfbmFtZSI6IkJvd3JpbmciLCJnaXZlbl9uYW1lIjoiSm9lbCIsImlkdHlwIjoidXNlciIsImlwYWRkciI6IjgyLjI3LjE4MC4xMjEiLCJuYW1lIjoiSm9lbCBCb3dyaW5nIiwib2lkIjoiYWFmNmMxNGUtZTc3ZC00NDBmLWJhMTktNjlkZjFlNDQ3NDAzIiwib25wcmVtX3NpZCI6IlMtMS01LTIxLTE1NDcxNjE2NDItODU0MjQ1Mzk4LTY4MjAwMzMzMC0xODE0MSIsInBsYXRmIjoiMTQiLCJwdWlkIjoiMTAwMzIwMDBEQUFFNTc0NiIsInJoIjoiMC5BVjRBelptNlBHR2d3RWFHY3M2b2VkVjQ1Z01BQUFBQUFBQUF3QUFBQUFBQUFBQmVBRmMuIiwic2NwIjoib3BlbmlkIFByZXNlbmNlLlJlYWQgcHJvZmlsZSBVc2VyLlJlYWQgZW1haWwiLCJzdWIiOiI4MThSR0tJbFZCaUJyczBSbU9vTzV5OVBycmFJZnZTNzFMSUpTV0U0bGJFIiwidGVuYW50X3JlZ2lvbl9zY29wZSI6IkVVIiwidGlkIjoiM2NiYTk5Y2QtYTA2MS00NmMwLTg2NzItY2VhODc5ZDU3OGU2IiwidW5pcXVlX25hbWUiOiJKb2VsLkJvd3JpbmdAdHRwLmNvbSIsInVwbiI6IkpvZWwuQm93cmluZ0B0dHAuY29tIiwidXRpIjoiMWxvdVMwWTU2RWkyMHhKNS05d3VBQSIsInZlciI6IjEuMCIsIndpZHMiOlsiYjc5ZmJmNGQtM2VmOS00Njg5LTgxNDMtNzZiMTk0ZTg1NTA5Il0sInhtc19zdCI6eyJzdWIiOiJRRUE1RnlRQmdqZjVuOGxrV0dNRFg3V2Z0M2VUY2NFWGp6eDhkWDJ1UU5zIn0sInhtc190Y2R0IjoxNDQ1NjE1NTI1fQ.JN5y7-a7DRr0dkttuxJ5kbabsnrD6L4gNooMAbWTNbOvsLH3Xb4NRUrT0_Z1V7u05kra1DpDpe-u8-B120KNayMA4oe6GanWX8L9YL3nZJOOsU-GIFSx_jGGIVGy73HCi8rnzyVgI70reAcbGZVnC3OWzb50ySMA-3JtpsR-tFmZMZ6RDQFiNloQskz6Bn2Um6_tPVXekxcSFm0cdp7EZ0B8JmWZSvuQ9yVC-p8BeAT_Kpmom-dgKAZHahlYKq24UZ7Zfd1sQR0jwETkS7pu6TegNDFRhG_rs8yHCkPD_kHGKi-VlRgpBXsdu-ZZMPQsST3r_RirAB4QktE-oPhilA");
    this->authenticated = true;
}

void Graph::set_refresh_token(const std::string& refresh_token) {
    this->auth.set_refresh_token(refresh_token);
}

class RequestFailed : public std::exception {};

Presence Graph::get_presence() {
    if(!this->authenticated) {
        this->auth.authenticate();
        this->authenticated = true;
    }

    struct curl_slist *list = nullptr;
    list = curl_slist_append(list, ("Authorization: Bearer " + this->auth.get_access_token()).c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_URL, "https://graph.microsoft.com/v1.0/me/presence");

    CURLcode res = curl_easy_perform(this->curl);
    curl_slist_free_all(list);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    cJSON * responseJSON = nullptr;
    Presence presence;

    try {
        if (res != CURLE_OK) {
            std::cout << "Error: presence GET returned curl code " << res << std::endl;
            throw RequestFailed();
        }

        if (http_code == 401) {
           // Access token expired
           this->authenticated = false;
           return this->get_presence();
        } else if(http_code != 200) {
            std::cout << "Error: presence GET returned HTTP code " << http_code << std::endl;
            throw RequestFailed();
        }

        responseJSON = cJSON_Parse(write_buffer);
        if (responseJSON == nullptr) {
            throw RequestFailed();
        }

        cJSON * error = cJSON_GetObjectItemCaseSensitive(responseJSON, "error");
        if (error != nullptr) {
            // TODO: better error handling
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
    } catch (...) {
        // TODO
        cJSON_Delete(responseJSON);
    }

    return presence;
}
