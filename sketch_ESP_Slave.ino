#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define DHTPIN1 5
#define DHTPIN2 17
#define DHTPIN3 16
#define DHTTYPE DHT11

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);

// RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x10, 0x52, 0x1C, 0x68, 0xEE, 0xBC};

// Structure to send data
// Must match the receiver structure
typedef struct struct_message {
  float temp1, temp2, temp3;
  float humid1, humid2, humid3;
  float objectTemp, ambientTemp;
} struct_message;

// String to receive command
String rdcommand;

// Create a struct_message called sendingData
struct_message sendingData;

// callback function that will be executed when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rdcommand, incomingData, sizeof(rdcommand));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Content received: ");
  Serial.println(rdcommand);
}

esp_now_peer_info_t peerInfo;

void setup(){
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  
  // Initialize sensors
  dht1.begin();
  dht2.begin();
  dht3.begin();
  mlx.begin();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for CB to get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
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
void loop(){
  sendingData.temp1 = dht1.readTemperature();
  sendingData.humid1 = dht1.readHumidity();
  sendingData.temp2 = dht2.readTemperature();
  sendingData.humid2 = dht2.readHumidity();
  sendingData.temp3 = dht3.readTemperature();
  sendingData.humid3 = dht3.readHumidity();
  sendingData.ambientTemp = mlx.readAmbientTempC();
  sendingData.objectTemp = mlx.readObjectTempC();

  if (rdcommand == "ALL") {
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendingData, sizeof(sendingData)); // Send message via ESP-NOW
      if (result == ESP_OK) {
        Serial.print("Sent with success");
      }
      else {
        Serial.print("Error sending the data");
      }
    } else {
    }
  
  delay(3000);
}
