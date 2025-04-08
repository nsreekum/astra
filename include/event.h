#ifndef EVENT_H
#define EVENT_H

#include "object_id.h"
#include "device_id.h"
#include "server_id.h"
#include <functional>
#include <map>
#include <queue>
#include <vector>

class ARDevice;
class EdgeServer;
class Cloud;

using Timestamp = double;

struct Event {
    Timestamp timestamp;
    Event(Timestamp time);
    virtual void process(std::map<DeviceID, ARDevice*>& devices,
                         std::map<ServerID, EdgeServer*>& servers,
                         Cloud& cloud,
                         std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) = 0;
    virtual ~Event() {}
};

// Custom comparator for the priority queue
struct CompareEvents {
    bool operator()(Event* a, Event* b) {
        return a->timestamp > b->timestamp;
    }
};

// Concrete event classes
struct UserSeesObjectEvent : public Event {
    DeviceID deviceId;
    ObjectID objectId;
    UserSeesObjectEvent(Timestamp time, DeviceID dId, ObjectID oId);
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override;
};

struct EdgeRequestEvent : public Event {
    DeviceID deviceId;
    ObjectID objectId;
    EdgeRequestEvent(Timestamp time, DeviceID dId, ObjectID oId);
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override;
};

struct CloudRequestEvent : public Event {
    ServerID serverId;
    ObjectID objectId;
    DeviceID requestingDeviceId;
    CloudRequestEvent(Timestamp time, ServerID sId, ObjectID oId, DeviceID dId);
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override;
};

struct DeviceResponseEvent : public Event {
    DeviceID deviceId;
    ObjectID objectId;
    DeviceResponseEvent(Timestamp time, DeviceID dId, ObjectID oId);
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override;
};

struct EdgeResponseEvent : public Event {
    ServerID serverId;
    ObjectID objectId;
    DeviceID targetDeviceId;
    EdgeResponseEvent(Timestamp time, ServerID sId, ObjectID oId, DeviceID dId);
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override;
};

struct ARDeviceMoveEvent : public Event {
    DeviceID deviceId;
    Location newLocation;
    ARDeviceMoveEvent(Timestamp time, DeviceID dId, Location newLoc) : Event(time), deviceId(dId), newLocation(newLoc) {}
    void process(std::map<DeviceID, ARDevice*>& devices,
                 std::map<ServerID, EdgeServer*>& servers,
                 Cloud& cloud,
                 std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue) override {
        if (devices.count(deviceId)) {
            devices[deviceId]->move(timestamp, newLocation);
        }
    }
};

#endif // EVENT_H