#ifndef CLOUD_H
#define CLOUD_H

#include "object_id.h"
#include "server_id.h"
#include "device_id.h"
#include "event.h"
#include <queue>
#include <functional>

class Cloud {
public:
    void processRequest(Timestamp time, ObjectID objectId, ServerID serverId, DeviceID deviceId, std::priority_queue<Event*, std::vector<Event*>, std::function<bool(Event*, Event*)>>& eventQueue);
};

#endif // CLOUD_H