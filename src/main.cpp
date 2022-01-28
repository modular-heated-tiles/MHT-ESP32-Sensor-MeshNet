/************************************************************************************ 
 * painlessMesh for ESP32 running with freeRTOS
 * Tasks:
 * meshNetwork - core 0, runs mesh network by running mesh.update frequently to keep up with the network
 * sensorMonitoring - core 1, polls sensors for data
 * 
 * 
 ***********************************************************************************/
#include "painlessMesh.h"

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

painlessMesh  mesh;

void meshNetwork( void *pvParameters );
void sensorMonitoring( void *pvParameters );

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    meshNetwork
    ,  "meshNetwork"   // A name just for humans
    ,  10000  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  0  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL 
    ,  0);

  xTaskCreatePinnedToCore(
    sensorMonitoring
    ,  "sensor monitoring"
    ,  10000 // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL 
    ,  1);

}

void loop() {

}
void meshNetwork(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  for (;;) // A Task shall never return or exit.
  {
    mesh.update();
    vTaskDelay(100); 
  }
}

void sensorMonitoring(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    // Send message
    // we might want to put a semaphore here to prevent potential issues, so far there are none
    String msg = "Hello from node ";
    msg += mesh.getNodeId();
    mesh.sendBroadcast( msg );
    vTaskDelay(1000);  
  }
}