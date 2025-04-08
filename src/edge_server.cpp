#include "edge_server.h"
#include "event.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

// Assuming objectSizes is defined globally
extern std::map<ObjectID, int> objectSizes;

// Assuming objectSizes is defined globally or passed appropriately
extern std::map<ObjectID, Location> objectLocations;

EdgeServer::EdgeServer(ServerID id, 
    int cacheLimit, 
    std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* queue, 
    const std::string& ruleFile)
    : serverId(id), edgeCacheSizeLimit(cacheLimit), currentEdgeCacheSize(0), eventQueue(queue), currentTime(0.0), associationRuleFile(ruleFile) {
        loadAssociationRules();
    }

bool EdgeServer::loadAssociationRules() {
    std::ifstream file(associationRuleFile);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open association rule file: " << associationRuleFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;
        while (std::getline(ss, segment, ',')) {
            parts.push_back(segment);
        }

        if (parts.size() == 3) {
            std::string antecedent = parts[0];
            std::string consequent = parts[1];
            double confidence = std::stod(parts[2]);

            associationRules[antecedent][consequent] = confidence;
        } else {
            std::cerr << "Warning: Skipping invalid line in association rule file: " << line << std::endl;
        }
    }
    file.close();
    std::cout << "Loaded " << associationRules.size() << " antecedent rules." << std::endl;
    return true;
}

void EdgeServer::handleCloudResponse(Timestamp time, ObjectID objectId, DeviceID deviceId) {
    currentTime = time;
    int objectSize = getObjectSize(objectId);
    if (objectSize > edgeCacheSizeLimit) {
        std::cout << time << ": Edge " << serverId << " cannot cache object " << objectId << " (size " << objectSize << " exceeds limit " << edgeCacheSizeLimit << ")." << std::endl;
        eventQueue->push(new DeviceResponseEvent(time, deviceId, objectId)); // Still respond to the device
        return;
    }

    if (edgeCache.find(objectId) == edgeCache.end()) {
        while (currentEdgeCacheSize + objectSize > edgeCacheSizeLimit && !edgeCache.empty()) {
            evictLRUObject();
        }
        if (currentEdgeCacheSize + objectSize <= edgeCacheSizeLimit) {
            edgeCache[objectId] = time;
            currentEdgeCacheSize += objectSize;
            std::cout << time << ": Edge " << serverId << " received and cached object " << objectId << " (size " << objectSize << ") from cloud (for device " << deviceId << "). Current cache size: " << currentEdgeCacheSize << std::endl;
        } else {
            std::cout << time << ": Edge " << serverId << " received object " << objectId << ", but no space to cache." << std::endl;
        }
        eventQueue->push(new DeviceResponseEvent(time, deviceId, objectId));
        // Trigger prefetching logic here (e.g., based on this new arrival)
        triggerPrefetching(time);
    } else {
        edgeCache[objectId] = time;
        std::cout << time << ": Edge " << serverId << " re-received object " << objectId << ". Updated access time." << std::endl;
        eventQueue->push(new DeviceResponseEvent(time, deviceId, objectId));
    }
}

void EdgeServer::triggerPrefetching(Timestamp time) {
    currentTime = time;
    // Implement your prefetching algorithm here, considering object sizes
    // Example: If object 3 is to be prefetched
    ObjectID prefetchObjectId = 3;
    int prefetchObjectSize = getObjectSize(prefetchObjectId);

    std::cout << time << ": Edge " << serverId << " considers prefetching object " << prefetchObjectId << " (size " << prefetchObjectSize << ")." << std::endl;

    if (edgeCache.find(prefetchObjectId) == edgeCache.end() && prefetchObjectSize <= edgeCacheSizeLimit) {
        while (currentEdgeCacheSize + prefetchObjectSize > edgeCacheSizeLimit && !edgeCache.empty()) {
            evictLRUObject();
        }
        if (currentEdgeCacheSize + prefetchObjectSize <= edgeCacheSizeLimit) {
            std::cout << time << ": Edge " << serverId << " initiates prefetching for object " << prefetchObjectId << "." << std::endl;
            eventQueue->push(new CloudRequestEvent(time, serverId, prefetchObjectId, -1)); // Device ID -1 indicates it's a prefetch
        } else {
            std::cout << time << ": Edge " << serverId << " cannot prefetch object " << prefetchObjectId << " due to insufficient cache space." << std::endl;
        }
    }
}

void EdgeServer::evictLRUObject() {
    if (edgeCache.empty()) return;

    ObjectID lruObjectId;
    Timestamp minTime = currentTime;
    double maxDistance = 0.0; // Evict the furthest object
    ObjectID furthestObjectId;

    for (const auto& pair : edgeCache) {
        if (pair.second < minTime) {
            minTime = pair.second;
            lruObjectId = pair.first;
        }
        // Check distance for eviction
        double distance = calculateDistance(simulationDevices->at(1)->location, objectLocations[pair.first]);
        if (distance > maxDistance) {
            maxDistance = distance;
            furthestObjectId = pair.first;
        }
    }

    // Choose which object to evict (e.g., furthest or LRU)
    ObjectID objectToEvict = furthestObjectId; // Example: Evict furthest
    if (edgeCache.find(objectToEvict) == edgeCache.end()) {
        objectToEvict = lruObjectId; // Evict LRU if furthest not found
    }

    int evictedSize = getObjectSize(objectToEvict);
    edgeCache.erase(objectToEvict);
    currentEdgeCacheSize -= evictedSize;
    std::cout << currentTime << ": Edge " << serverId << " evicted object " << objectToEvict << " (size " << evictedSize << ") due to cache full. New cache size: " << currentEdgeCacheSize << std::endl;
}

int EdgeServer::getObjectSize(ObjectID objectId) {
    if (objectSizes.count(objectId)) {
        return objectSizes[objectId];
    }
    return 0; // Default size if not found
}

std::vector<ObjectID> EdgeServer::getPrefetchCandidates(const std::unordered_set<ObjectID>& interactedObjects) {
    std::unordered_map<ObjectID, double> candidateConfidences;
    std::vector<ObjectID> prefetchCandidates;

    // Create a string representation of the interacted objects (antecedent)
    std::vector<ObjectID> sortedInteracted(interactedObjects.begin(), interactedObjects.end());
    std::sort(sortedInteracted.begin(), sortedInteracted.end());
    std::string antecedentKey;
    for (size_t i = 0; i < sortedInteracted.size(); ++i) {
        antecedentKey += std::to_string(sortedInteracted[i]);
        if (i < sortedInteracted.size() - 1) {
            antecedentKey += " ";
        }
    }

    // Check for rules matching the current set of interacted objects
    if (associationRules.count(antecedentKey)) {
        for (const auto& consequentPair : associationRules[antecedentKey]) {
            ObjectID consequentId = std::stoi(consequentPair.first);
            double confidence = consequentPair.second;
            if (edgeCache.find(consequentId) == edgeCache.end()) { // Don't prefetch if already in cache
                candidateConfidences[consequentId] = confidence;
            }
        }
    }

    // If no direct match, consider rules where the antecedent is a subset
    for (const auto& rulePair : associationRules) {
        std::stringstream ss(rulePair.first);
        std::string segment;
        std::unordered_set<ObjectID> ruleAntecedent;
        while (ss >> segment) {
            ruleAntecedent.insert(std::stoi(segment));
        }

        // Check if the rule's antecedent is a subset of the interacted objects
        if (std::includes(interactedObjects.begin(), interactedObjects.end(),
                          ruleAntecedent.begin(), ruleAntecedent.end())) {
            for (const auto& consequentPair : rulePair.second) {
                ObjectID consequentId = std::stoi(consequentPair.first);
                double confidence = consequentPair.second;
                if (edgeCache.find(consequentId) == edgeCache.end()) {
                    // Aggregate confidence (you might want a more sophisticated aggregation)
                    candidateConfidences[consequentId] += confidence;
                }
            }
        }
    }

    // Filter candidates based on proximity to AR device
    double fovRadius = 10.0; // Example FoV radius
    ARDevice* requestingDevice = nullptr;
    if (simulationDevices) {
        if (simulationDevices->count(1)) { // Assuming device ID 1 for now
            requestingDevice = simulationDevices->at(1);
            for (const auto& candidatePair : candidateConfidences) {
                ObjectID candidateId = candidatePair.first;
                double distance = calculateDistance(requestingDevice->location, objectLocations[candidateId]);
                if (distance <= fovRadius) {
                    prefetchCandidates.push_back(candidateId);
                } else {
                    // Store metadata temporarily
                    ObjectInfo tempObj;
                    tempObj.id = candidateId;
                    tempObj.location = objectLocations[candidateId];
                    tempMetadata.push_back(tempObj);
                    std::cout << currentTime << ": Edge " << serverId << " storing metadata for object " << candidateId << " (distance: " << distance << ")" << std::endl;
                }
            }
        }
    }

    // Sort candidates by confidence (descending)
    std::vector<std::pair<ObjectID, double>> sortedCandidates(candidateConfidences.begin(), candidateConfidences.end());
    std::sort(sortedCandidates.begin(), sortedCandidates.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    // Select the top N candidates (e.g., top 1 for now)
    size_t numPrefetch = 1;
    for (size_t i = 0; i < std::min(numPrefetch, sortedCandidates.size()); ++i) {
        prefetchCandidates.push_back(sortedCandidates[i].first);
    }

    std::cout << currentTime << ": Edge " << serverId << " found prefetch candidates based on interacted objects: ";
    for (ObjectID id : prefetchCandidates) {
        std::cout << id << "(" << std::fixed << std::setprecision(2) << candidateConfidences[id] << ") ";
    }
    std::cout << std::endl;

    return prefetchCandidates;
}

void EdgeServer::handleDeviceRequest(Timestamp time, ObjectID objectId, DeviceID deviceId) {
    currentTime = time;
    std::cout << time << ": Edge " << serverId << " received request for object " << objectId << " from device " << deviceId << std::endl;
    if (edgeCache.find(objectId) == edgeCache.end()) {
        // Cache miss, try prefetching
        // First, get the interacted objects of the requesting device
        ARDevice* requestingDevice = nullptr;
        if (simulationDevices) { // Assuming you have a way to access the devices map
            if (simulationDevices->count(deviceId)) {
                requestingDevice = (*simulationDevices)[deviceId];
                std::vector<ObjectID> candidates = getPrefetchCandidates(requestingDevice->interactedObjects);
                for (ObjectID candidateId : candidates) {
                    int prefetchObjectSize = getObjectSize(candidateId);
                    if (prefetchObjectSize <= edgeCacheSizeLimit && edgeCache.find(candidateId) == edgeCache.end()) {
                        // Basic prefetching: just request it
                        std::cout << time << ": Edge " << serverId << " initiating prefetch for object " << candidateId << " due to device request." << std::endl;
                        eventQueue->push(new CloudRequestEvent(time, serverId, candidateId, -1));
                    }
                }
            }
        }
        eventQueue->push(new CloudRequestEvent(time, serverId, objectId, deviceId)); // Still forward the original request
    } else {
        std::cout << time << ": Edge " << serverId << " found object " << objectId << " in edge cache." << std::endl;
        edgeCache[objectId] = time;
        eventQueue->push(new DeviceResponseEvent(time, deviceId, objectId));
    }
}