#include "cloud.h"
#include "event.h"
#include <iostream>

void Cloud::processRequest(Timestamp time, ObjectID objectId, ServerID serverId, DeviceID deviceId, std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    double cloudLatency = 2.0; // Example latency
    std::cout << time << ": Cloud received request for object " << objectId << " from edge " << serverId << " (for device " << deviceId << "). Processing..." << std::endl;
    eventQueue.push(new EdgeResponseEvent(time + cloudLatency, serverId, objectId, deviceId));
}