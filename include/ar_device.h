#ifndef AR_DEVICE_H
#define AR_DEVICE_H

#include "device_id.h"
#include "object_id.h"
#include "event.h"
#include <unordered_map> // Use unordered_map to store <ObjectID, Timestamp> for LRU
#include <unordered_set> // To store interacted objects
#include <queue>
#include <functional>

using Location = std::pair<double, double>;

class ARDevice {
public:
    DeviceID deviceId;
    int localCacheSizeLimit; // Maximum size of the local cache
    int currentLocalCacheSize;
    std::unordered_map<ObjectID, Timestamp> localCache; // Store <ObjectID, LastAccessTime> for LRU
    std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* eventQueue;
    Timestamp currentTime; // To track the current simulation time for LRU updates
    std::unordered_set<ObjectID> interactedObjects; // Keep track of interacted objects
    Location location; // ARDevice location

    ARDevice(DeviceID id, int cacheLimit, std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* queue, Location initialLocation);
    void requestObject(Timestamp time, ObjectID objectId);
    void receiveObject(Timestamp time, ObjectID objectId);
    void move(Timestamp time, Location newLocation); // Function to move the ARDevice
private:
    void evictLRUObject();
    int getObjectSize(ObjectID objectId);
};

#endif // AR_DEVICE_H