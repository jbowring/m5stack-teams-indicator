//
// Created by Joel on 11/06/2022.
//

#ifndef HOSTGRAPHCLIENT_AUTH_H
#define HOSTGRAPHCLIENT_AUTH_H

#include <string>
#include <curl/curl.h>

class Auth {
public:
    explicit Auth(std::string client_id);
    void authenticate();
    void set_refresh_token(const std::string& token);
    std::string get_access_token();
    void set_access_token(const std::string& token);

private:
    std::string client_id, access_token, refresh_token;
    CURL *curl;

    void device_code_verification(const std::string &device_code);
    void authenticate_user();
    void refresh_access_token();
};

#endif //HOSTGRAPHCLIENT_AUTH_H
