#include <iostream>
#include <unistd.h>
#include "Graph.h"
#include "ClientID.h"

int main() {
    Graph graph(clientId);
    graph.authenticate();

    while(true) {
        Presence presence = graph.get_presence();

        std::cout << "Availability: " << presence.availability << "\tActivity: " << presence.activity << std::endl;
        sleep(3);
    }
    return 0;
}
