#ifndef HOSTGRAPHCLIENT_AUTH_H
#define HOSTGRAPHCLIENT_AUTH_H

#include <WString.h>
#include <WiFi.h>

class Auth {
public:
    explicit Auth(WiFiClient &client, String client_id);
    void authenticate();
    void set_refresh_token(const String& token);
    String get_access_token();

private:
    String client_id, access_token, refresh_token;
    WiFiClient& wifi_client;

    void authenticate_user();
    void refresh_access_token();
    void oauth_code_grant_flow(const String &postFields);
};

#endif //HOSTGRAPHCLIENT_AUTH_H
