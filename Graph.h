#ifndef HOSTGRAPHCLIENT_GRAPH_H
#define HOSTGRAPHCLIENT_GRAPH_H

#include "Auth.h"
#include <WString.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

struct Presence {
    String availability, activity;
};

class Graph {
public:
    Graph(const String& client_id);
    Presence get_presence();
    void authenticate();

private:
    Auth auth;
    WiFiClientSecure wifi_client;
    HTTPClient https;

    Presence get_presence(bool auto_reauthenticate);
};

#endif //HOSTGRAPHCLIENT_GRAPH_H
