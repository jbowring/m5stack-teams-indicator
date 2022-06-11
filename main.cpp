#include <iostream>
#include "Graph.h"
#include "ClientID.h"

int main() {
    Graph graph(clientId);
    Presence presence = graph.get_presence();

    std::cout << "Availability: " << presence.availability << std::endl;
    std::cout << "Activity: " << presence.activity << std::endl;

    return 0;
}
