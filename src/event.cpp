#include "event.h"
#include "ar_device.h"
#include "edge_server.h"
#include "cloud.h"
#include <iostream>

Event::Event(Timestamp time) : timestamp(time) {} 

UserSeesObjectEvent::UserSeesObjectEvent(Timestamp time, DeviceID dId, ObjectID oId) : Event(time), deviceId(dId), objectId(oId) {}
void UserSeesObjectEvent::process(std::map<DeviceID, ARDevice*>& devices,
                                  std::map<ServerID, EdgeServer*>& servers,
                                  Cloud& cloud,
                                  std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    if (devices.count(deviceId)) {
        devices[deviceId]->requestObject(timestamp, objectId);
    }
}

EdgeRequestEvent::EdgeRequestEvent(Timestamp time, DeviceID dId, ObjectID oId) : Event(time), deviceId(dId), objectId(oId) {}
void EdgeRequestEvent::process(std::map<DeviceID, ARDevice*>& devices,
                                std::map<ServerID, EdgeServer*>& servers,
                                Cloud& cloud,
                                std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    if (servers.count(1)) { // Assuming a single edge server with ID 1
        servers[1]->handleDeviceRequest(timestamp, objectId, deviceId);
    }
}

CloudRequestEvent::CloudRequestEvent(Timestamp time, ServerID sId, ObjectID oId, DeviceID dId) : Event(time), serverId(sId), objectId(oId), requestingDeviceId(dId) {}
void CloudRequestEvent::process(std::map<DeviceID, ARDevice*>& devices,
                                 std::map<ServerID, EdgeServer*>& servers,
                                 Cloud& cloud,
                                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    cloud.processRequest(timestamp, objectId, serverId, requestingDeviceId, eventQueue);
}

DeviceResponseEvent::DeviceResponseEvent(Timestamp time, DeviceID dId, ObjectID oId) : Event(time), deviceId(dId), objectId(oId) {}
void DeviceResponseEvent::process(std::map<DeviceID, ARDevice*>& devices,
                                  std::map<ServerID, EdgeServer*>& servers,
                                  Cloud& cloud,
                                  std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    if (devices.count(deviceId)) {
        devices[deviceId]->receiveObject(timestamp, objectId);
    }
}

EdgeResponseEvent::EdgeResponseEvent(Timestamp time, ServerID sId, ObjectID oId, DeviceID dId) : Event(time), serverId(sId), objectId(oId), targetDeviceId(dId) {}
void EdgeResponseEvent::process(std::map<DeviceID, ARDevice*>& devices,
                                 std::map<ServerID, EdgeServer*>& servers,
                                 Cloud& cloud,
                                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) {
    if (servers.count(serverId)) {
        servers[serverId]->handleCloudResponse(timestamp, objectId, targetDeviceId);
    }
}