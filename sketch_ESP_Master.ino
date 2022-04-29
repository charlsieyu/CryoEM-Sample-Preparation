#include <esp_now.h>
#include <WiFi.h>

// RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xAC, 0x67, 0xB2, 0x38, 0xA8, 0x60};

// Structure to receive data
// Must match the sender structure
typedef struct struct_message {
  float temp1, temp2, temp3;
  float humid1, humid2, humid3;
  float objectTemp, ambientTemp;
} struct_message;

// String to send command
String sdcommand;

// Create a struct_message called receivingData
struct_message receivingData;

// callback function that will be executed when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receivingData, incomingData, sizeof(receivingData));
  Serial.print("Bytes received: ");
  Serial.println(len);

  Serial.print("DHT1: Temperature = ");
  Serial.print(receivingData.temp1);
  Serial.print("°C  Humidity = ");
  Serial.print(receivingData.humid1);
  Serial.println("%");

  Serial.print("DHT2: Temperature = ");
  Serial.print(receivingData.temp2);
  Serial.print("°C  Humidity = ");
  Serial.print(receivingData.humid2);
  Serial.println("%");

  Serial.print("DHT3: Temperature = ");
  Serial.print(receivingData.temp3);
  Serial.print("°C  Humidity = ");
  Serial.print(receivingData.humid3);
  Serial.println("%");
  
  Serial.print("IR: Ambient temperature = ");
  Serial.print(receivingData.ambientTemp);
  Serial.print("°C  Object temperature = ");
  Serial.print(receivingData.objectTemp);
  Serial.println("°C");
}

 esp_now_peer_info_t peerInfo;
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }else
  Serial.println("Success initializing ESP-NOW");
  
  // Once ESPNow is successfully Init, we will register for CB to get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  
  if (Serial.available()) {
    sdcommand = Serial.readStringUntil('\n');
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sdcommand, sizeof(sdcommand)); // Send message via ESP-NOW
    if (result == ESP_OK) {
      Serial.print("Sent with success");
    }
    else {
      Serial.print("Error sending the data");
    }
  }
  delay(1000);

}
