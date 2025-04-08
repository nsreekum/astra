#include "ar_device.h"
#include "event.h"
#include <iostream>
#include <algorithm>

// Assuming objectSizes is defined globally or passed appropriately
extern std::map<ObjectID, int> objectSizes;

ARDevice::ARDevice(DeviceID id, int cacheLimit, std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>* queue, Location initialLocation)
    : deviceId(id), localCacheSizeLimit(cacheLimit), currentLocalCacheSize(0), eventQueue(queue), currentTime(0.0), location(initialLocation){}

    void ARDevice::requestObject(Timestamp time, ObjectID objectId) {
        currentTime = time;
        std::cout << time << ": Device " << deviceId << " requests object " << objectId << " at (" << location.first << ", " << location.second << ")" << std::endl;
        interactedObjects.insert(objectId);
        if (localCache.find(objectId) == localCache.end()) {
            eventQueue->push(new EdgeRequestEvent(time, deviceId, objectId));
        } else {
            std::cout << time << ": Device " << deviceId << " found object " << objectId << " in local cache." << std::endl;
            localCache[objectId] = time;
        }
    }

void ARDevice::receiveObject(Timestamp time, ObjectID objectId) {
    currentTime = time;
    int objectSize = getObjectSize(objectId);
    if (objectSize > localCacheSizeLimit) {
        std::cout << time << ": Device " << deviceId << " cannot cache object " << objectId << " (size " << objectSize << " exceeds limit " << localCacheSizeLimit << ")." << std::endl;
        return;
    }

    if (localCache.find(objectId) == localCache.end()) {
        // Object not in cache, add it
        while (currentLocalCacheSize + objectSize > localCacheSizeLimit && !localCache.empty()) {
            evictLRUObject();
        }
        if (currentLocalCacheSize + objectSize <= localCacheSizeLimit) {
            localCache[objectId] = time;
            currentLocalCacheSize += objectSize;
            std::cout << time << ": Device " << deviceId << " received and cached object " << objectId << " (size " << objectSize << "). Current cache size: " << currentLocalCacheSize << std::endl;
        }
    } else {
        // Object already in cache, update last access time
        localCache[objectId] = time;
        std::cout << time << ": Device " << deviceId << " re-received object " << objectId << ". Updated access time." << std::endl;
    }
}

void ARDevice::move(Timestamp time, Location newLocation) {
    currentTime = time;
    std::cout << time << ": Device " << deviceId << " moved from (" << location.first << ", " << location.second << ") to (" << newLocation.first << ", " << newLocation.second << ")" << std::endl;
    location = newLocation;
    // You might want to trigger a prefetching update or cache invalidation here
    // based on the new location. For simplicity, we'll leave that for later.
}

void ARDevice::evictLRUObject() {
    if (localCache.empty()) return;

    ObjectID lruObjectId;
    Timestamp minTime = currentTime; // Initialize with a future time to ensure the first element is smaller

    for (const auto& pair : localCache) {
        if (pair.second < minTime) {
            minTime = pair.second;
            lruObjectId = pair.first;
        }
    }

    int evictedSize = getObjectSize(lruObjectId);
    localCache.erase(lruObjectId);
    currentLocalCacheSize -= evictedSize;
    std::cout << currentTime << ": Device " << deviceId << " evicted object " << lruObjectId << " (size " << evictedSize << ") due to cache full. New cache size: " << currentLocalCacheSize << std::endl;
}

int ARDevice::getObjectSize(ObjectID objectId) {
    if (objectSizes.count(objectId)) {
        return objectSizes[objectId];
    }
    return 0; // Default size if not found
}