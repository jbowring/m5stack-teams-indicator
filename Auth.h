#ifndef HOSTGRAPHCLIENT_AUTH_H
#define HOSTGRAPHCLIENT_AUTH_H

#include <WString.h>
#include <WiFiClientSecure.h>

class Auth {
public:
    Auth(String client_id);
    void authenticate();
    String get_access_token();

private:
    String client_id, access_token, refresh_token;
    WiFiClientSecure wifi_client;

    void authenticate_user();
    void refresh_access_token();
    void oauth_code_grant_flow(const String &postFields);
};

#endif //HOSTGRAPHCLIENT_AUTH_H
