#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include "event.h"
#include "ar_device.h"
#include "edge_server.h"
#include "cloud.h"
#include <map>
#include <queue>
#include <vector>
#include <functional>

class SimulationEngine {
public:
    std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>> eventQueue;
    std::map<DeviceID, ARDevice*> devices;
    std::map<ServerID, EdgeServer*> servers;
    Cloud cloud;
    Timestamp currentTime = 0.0;

    SimulationEngine(); // Add a constructor to initialize the priority queue with the comparator
    void addDevice(ARDevice* device);
    void addServer(EdgeServer* server);
    void addEvent(Event* event);
    void run();
};

#endif // SIMULATION_ENGINE_H