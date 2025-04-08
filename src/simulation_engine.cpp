#include "simulation_engine.h"
#include <iostream>

SimulationEngine::SimulationEngine() :
    eventQueue([](Event* a, Event* b) { return a->timestamp > b->timestamp; }) {}

void SimulationEngine::addDevice(ARDevice* device) {
    devices[device->deviceId] = device;
}

void SimulationEngine::addServer(EdgeServer* server) {
    servers[server->serverId] = server;
}

void SimulationEngine::addEvent(Event* event) {
    eventQueue.push(event);
}

void SimulationEngine::run() {
    while (!eventQueue.empty()) {
        Event* currentEvent = eventQueue.top();
        eventQueue.pop();
        currentTime = currentEvent->timestamp;
        std::cout << "Processing event at time: " << currentTime << std::endl;
        currentEvent->process(devices, servers, cloud, eventQueue);
        delete currentEvent;

        // Example of triggering prefetching periodically
        if (static_cast<int>(currentTime) % 5 == 0) {
            if (servers.count(1)) {
                servers[1]->triggerPrefetching(currentTime);
            }
        }
    }

    // Clean up allocated memory
    for (auto const& [id, device] : devices) {
        delete device;
    }
    for (auto const& [id, server] : servers) {
        delete server;
    }
}