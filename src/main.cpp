#include <iostream>
#include <vector>
#include <map>

#include "ar_device.h"
#include "edge_server.h"
#include "cloud.h"
#include "event.h"
#include "simulation_engine.h" // Include the SimulationEngine header

// Define object sizes (ObjectID -> size)
std::map<ObjectID, int> objectSizes = {
    {1, 10},
    {2, 5},
    {3, 12},
    {4, 8},
    {5, 15}
};

// Define object locations
std::map<ObjectID, Location> objectLocations = {
    {1, {1.0, 2.0}},
    {2, {3.0, 4.0}},
    {3, {5.0, 6.0}},
    {4, {7.0, 8.0}},
    {5, {9.0, 10.0}}
};

int main() {
    SimulationEngine engine; // Create the SimulationEngine object

    // Create multiple devices with cache limits
    std::map<DeviceID, ARDevice*> devices;
    devices[1] = new ARDevice(1, 20, &engine.eventQueue, {0.0, 0.0}); // Initial location (0, 0)
    devices[2] = new ARDevice(2, 15, &engine.eventQueue, {2.0, 2.0});
    devices[3] = new ARDevice(3, 25, &engine.eventQueue, {4.0, 4.0});

    // Add a move event
    engine.addEvent(new ARDeviceMoveEvent(10.0, 1, {5.0, 5.0})); // Move device 1 at time 10.0

    // Create edge server with a cache limit
    std::map<ServerID, EdgeServer*> servers;
    servers[1] = new EdgeServer(1, 50, &engine.eventQueue, "association_rule.txt");
    servers[1]->simulationDevices = &engine.devices; // Set the devices map

    // Add devices and servers to the engine (if you want to manage them explicitly there)
    engine.addDevice(devices[1]);
    engine.addDevice(devices[2]);
    engine.addDevice(devices[3]);
    engine.addServer(servers[1]);

    // Inject initial events into the engine's event queue
    engine.addEvent(new UserSeesObjectEvent(0.1, 1, 1));
    engine.addEvent(new UserSeesObjectEvent(0.3, 2, 2));
    engine.addEvent(new UserSeesObjectEvent(0.5, 1, 2));
    engine.addEvent(new UserSeesObjectEvent(0.8, 3, 1));
    engine.addEvent(new UserSeesObjectEvent(1.2, 1, 1));
    engine.addEvent(new UserSeesObjectEvent(1.5, 2, 3));
    engine.addEvent(new UserSeesObjectEvent(2.0, 3, 3));
    engine.addEvent(new UserSeesObjectEvent(3.0, 1, 4));
    engine.addEvent(new UserSeesObjectEvent(4.0, 2, 1));
    engine.addEvent(new UserSeesObjectEvent(5.0, 3, 2));

    engine.run(); // Run the simulation using the engine's run method

    // The engine's run method handles cleanup of devices and servers

    return 0;
}