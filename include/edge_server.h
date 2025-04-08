#ifndef EDGE_SERVER_H
#define EDGE_SERVER_H

#include "server_id.h"
#include "object_id.h"
#include "device_id.h"
#include "event.h"
#include "ar_device.h"
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>

struct ObjectInfo {
    ObjectID id;
    Location location;
};

class EdgeServer {
public:
    ServerID serverId;
    int edgeCacheSizeLimit; // Maximum size of the edge cache
    int currentEdgeCacheSize;
    std::unordered_map<ObjectID, Timestamp> edgeCache; // Store <ObjectID, LastAccessTime> for LRU
    std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* eventQueue;
    Timestamp currentTime; // To track current simulation time for LRU updates

    std::vector<ObjectInfo> tempMetadata;

    // New data structure for association rules
    std::unordered_map<std::string, std::unordered_map<std::string, double>> associationRules;
    std::string associationRuleFile;

    std::map<DeviceID, ARDevice*> *simulationDevices;

    EdgeServer(ServerID id, 
        int cacheLimit, 
        std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* queue, 
        const std::string& ruleFile = "association_rule.txt");
    bool loadAssociationRules(); // Function to load rules from the file
    void handleDeviceRequest(Timestamp time, ObjectID objectId, DeviceID deviceId);
    void handleCloudResponse(Timestamp time, ObjectID objectId, DeviceID deviceId);
    void triggerPrefetching(Timestamp time); // Placeholder for prefetching logic
private:
    void evictLRUObject();
    int getObjectSize(ObjectID objectId);
    bool canCacheObject(ObjectID objectId);
    std::vector<ObjectID> getPrefetchCandidates(const std::unordered_set<ObjectID>& interactedObjects);
};

// Utility function to calculate distance between two locations
double calculateDistance(Location loc1, Location loc2) {
    double dx = loc1.first - loc2.first;
    double dy = loc1.second - loc2.second;
    return std::sqrt(dx * dx + dy * dy);
}

#endif // EDGE_SERVER_H