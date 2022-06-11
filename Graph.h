//
// Created by Joel on 11/06/2022.
//

#ifndef HOSTGRAPHCLIENT_GRAPH_H
#define HOSTGRAPHCLIENT_GRAPH_H

#include "Auth.h"
#include <string>

struct Presence {
    std::string availability, activity;
};

class Graph {
public:
    explicit Graph(const std::string& client_id);
    Presence get_presence();

private:
    Auth auth;
    CURL *curl;
    bool authenticated = false;

    void set_refresh_token(const std::string &refresh_token);
};

#endif //HOSTGRAPHCLIENT_GRAPH_H
