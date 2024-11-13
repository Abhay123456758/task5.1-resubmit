#include <Wire.h>
#include <BH1750.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

// WiFi Credentials
char ssid[] = "iPhone 14 pro";      // Your WiFi SSID
char pass[] = "abhay1559";          // Your WiFi Password

// IFTTT Credentials
const char* host = "maker.ifttt.com"; // IFTTT host for HTTP requests
const int port = 80;                  // Port for HTTP (80 for HTTP, 443 for HTTPS)
const String eventName = "Received_sketch"; // IFTTT event name
const String iftttKey = "cJ_qpFC2dGxVpbyjZZl3Vj"; // Your IFTTT Webhooks key

// Light Sensor Threshold
const float lightThreshold = 500.0;   // Threshold for detecting sunlight (in lux)

// BH1750 Sensor Object
BH1750 lightMeter;  // Create a BH1750 object to interact with the light sensor

// WiFi Client and HTTP Client Objects
WiFiClient wifi;    // Manages WiFi connection
HttpClient client = HttpClient(wifi, host, port); // Manages HTTP requests to IFTTT using WiFi

// State Variables
bool sunlightDetected = false; // Tracks whether sunlight is detected

void setup() {
  Serial.begin(9600);         // Start serial communication for debugging output
  
  // Start I2C communication
  Wire.begin();
  
  // Initialize the light sensor
  lightMeter.begin();
  
  // Connect to WiFi
  connectWiFi();
}

void loop() {
  // Read light level from BH1750 sensor
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");     // Print light level in lux

  // Check if light level exceeds threshold
  if (lux > lightThreshold) {    // If light exceeds threshold
    if (!sunlightDetected) {     // If sunlight was not previously detected
      sendIFTTTEvent("Sunlight Started", lux); // Send notification for sunlight started
      sunlightDetected = true;   // Update state to indicate sunlight detected
    }
  } else {                       // If light level is below threshold
    if (sunlightDetected) {      // If sunlight was previously detected
      sendIFTTTEvent("Sunlight Stopped", lux); // Send notification for sunlight stopped
      sunlightDetected = false;  // Update state to indicate sunlight not detected
    }
  }

  // Delay before the next reading
  delay(60000);  // Check every minute
}

// Function to connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) { // Attempt WiFi connection
    delay(1000);                 // Wait for 1 second before retrying
    Serial.print(".");           // Print dot to indicate progress
  }
  Serial.println("Connected!");   // Indicate successful connection
}

// Function to send an event to IFTTT with a message and lux value
void sendIFTTTEvent(String value, float lux) {
  // Prepare URL and data to be sent to IFTTT
  String url = "/trigger/" + eventName + "/with/key/" + iftttKey; // URL for triggering IFTTT event
  String postData = "{\"value1\":\"" + value + "\", \"value2\":\"" + String(lux) + "\"}"; // JSON data to send

  // Send HTTP POST request to IFTTT
  client.beginRequest();
  client.post(url);
  client.sendHeader("Content-Type", "application/json"); // Set content type to JSON
  client.sendHeader("Content-Length", postData.length()); // Send content length
  client.beginBody();
  client.print(postData);         // Send JSON data
  client.endRequest();

  // Get the response from IFTTT
  int statusCode = client.responseStatusCode(); // Read status code
  String response = client.responseBody();      // Read response body
  
  // Print response for debugging
  Serial.print("Status: ");
  Serial.println(statusCode); // Print HTTP status code
  Serial.print("Response: ");
  Serial.println(response);   // Print IFTTT response
}
