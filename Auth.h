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

private:
    std::string client_id, access_token, refresh_token;
    CURL *curl;

    void authenticate_user();
    void refresh_access_token();
    void oauth_code_grant_flow(const std::string &postFields);
};

#endif //HOSTGRAPHCLIENT_AUTH_H
