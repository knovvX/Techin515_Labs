
#include <magic_wand_inferencing.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define SAMPLE_RATE_MS 10
#define CAPTURE_DURATION_MS 2000
#define FEATURE_SIZE EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE

#define BUTTON_PIN D9
#define RED_LED_PIN D1
#define GREEN_LED_PIN D2
#define BLUE_LED_PIN D3
void capture_accelerometer_data();
void run_classifier_on_captured_data();
void print_inference_result(ei_impulse_result_t result);
void run_inference();


Adafruit_MPU6050 mpu;
bool capturing = false;
unsigned long last_sample_time = 0;
unsigned long capture_start_time = 0;
int sample_count = 0;
float features[FEATURE_SIZE];

#define CONFIDENCE_THRESHOLD 100.0
// WiFi credentials 
const char* ssid = "UW MPSK";
const char* password = "/}pYj&L7@i"; // use your password here
// Server details 
const char* serverUrl = "http://10.19.57.54:5002/predict"; // Fill in with server URL; Please keep /predict

// Student identifier - set this to your UWNetID
const char* studentId = "ludiyun";

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}
/**
 * Setup WiFi connection
*/
 void setupWiFi() {
     Serial.println("Connecting to WiFi...");
     WiFi.begin(ssid, password);
     
     // Wait for connection
     while (WiFi.status() != WL_CONNECTED) {
         delay(500);
         Serial.print(".");
     }
     
     Serial.println("");
     Serial.print("Connected to ");
     Serial.println(ssid);
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());
 }

void sendRawDataToServer() {
   HTTPClient http;
   http.begin(serverUrl);
   http.addHeader("Content-Type", "application/json");

   // Build JSON array from features[]
   String jsonPayload = "{";
   jsonPayload += "\"student_id\":\"";
   jsonPayload += studentId;
   jsonPayload += "\",";
   jsonPayload += "\"features\":[";

   for (int i = 0; i < FEATURE_SIZE; i++) {
       jsonPayload += String(features[i], 6);  // 6 digits of precision
       if (i < FEATURE_SIZE - 1) {
           jsonPayload += ",";
       }
   }
   jsonPayload += "]}";

   Serial.println("\n--- Sending Raw Data to Server ---");
   Serial.println("Payload: " + jsonPayload);

   int httpResponseCode = http.POST(jsonPayload);
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);

   if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response: " + response);

      DynamicJsonDocument doc(256);
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
            const char* gesture = doc["gesture"];
            float confidence = doc["confidence"];

            Serial.println("Server Inference Result:");
            Serial.print("Gesture: ");
            Serial.println(gesture);
            Serial.print("Confidence: ");
            Serial.print(confidence);
            Serial.println("%");

            // Actuate LED based on remote gesture
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(BLUE_LED_PIN, LOW);

            if (strcmp(gesture, "O") == 0) {
                digitalWrite(RED_LED_PIN, HIGH);
            } else if (strcmp(gesture, "V") == 0) {
                digitalWrite(GREEN_LED_PIN, HIGH);
            } else if (strcmp(gesture, "Z") == 0) {
                digitalWrite(BLUE_LED_PIN, HIGH);
            }

      } else {
            Serial.print("Failed to parse server response: ");
            Serial.println(error.c_str());
      }
   } else {
      Serial.printf("Error sending POST: %s\n", http.errorToString(httpResponseCode).c_str());
   }

   http.end();
}

 /**
* @brief      Send result to server
*/
void sendGestureToServer(const char* gesture, float confidence) {
   // Create JSON payload
   String jsonPayload = "{";
   jsonPayload += "\"student_id\":";
   jsonPayload += "\"";
   jsonPayload += studentId;
   jsonPayload += "\",";
   jsonPayload += "\"gesture\":";
   jsonPayload += "\"";
   jsonPayload += gesture;
   jsonPayload += "\",";
   jsonPayload += "\"confidence\":";
   jsonPayload += confidence;
   jsonPayload += "}";
   
   Serial.println("\n--- Sending Prediction to Server ---");
   Serial.println("URL: " + String(serverUrl));
   Serial.println("Payload: " + jsonPayload);
   
   HTTPClient http;
   http.begin(serverUrl);
   http.addHeader("Content-Type", "application/json");
   
   // Send POST request
   int httpResponseCode = http.POST(jsonPayload);
   
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   
   if (httpResponseCode > 0) {
       String response = http.getString();
       Serial.println("Server response: " + response);
   } else {
       Serial.printf("Error sending POST: %s\n", http.errorToString(httpResponseCode).c_str());
   }
   
   http.end();
   Serial.println("--- End of Request ---\n");
}
void setup() {
    Serial.begin(115200);

//  set inital pin mode
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);

    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
// initialize sensor
    Serial.println("Initializing MPU6050...");
    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) delay(10);
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 initialized.");
    setupWiFi();
}


void capture_accelerometer_data() {
    if (millis() - last_sample_time >= SAMPLE_RATE_MS) {
        last_sample_time = millis();

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        if (sample_count < FEATURE_SIZE / 3) {
            int idx = sample_count * 3;
            features[idx] = a.acceleration.x;
            features[idx + 1] = a.acceleration.y;
            features[idx + 2] = a.acceleration.z;
            sample_count++;
        }

        if (millis() - capture_start_time >= CAPTURE_DURATION_MS) {
            capturing = false;
            Serial.println("Capture complete");
            run_inference();
        }
    }
}

void run_inference() {
    if (sample_count * 3 < FEATURE_SIZE) {
        Serial.println("ERROR: Not enough data");
        return;
    }

    ei_impulse_result_t result = { 0 };
    signal_t features_signal;
    features_signal.total_length = FEATURE_SIZE;
    features_signal.get_data = &raw_feature_get_data;

    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
    if (res != EI_IMPULSE_OK) {
        Serial.print("ERR: Failed to run classifier (");
        Serial.print(res);
        Serial.println(")");
        return;
    }

    print_inference_result(result);
}

void print_inference_result(ei_impulse_result_t result) {
    float max_value = 0;
    int max_index = -1;

    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > max_value) {
            max_value = result.classification[i].value;
            max_index = i;
        }
    }

    // turn off LED when inferecing for new gesture
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);

    if (max_index != -1) {
        const char* prediction = ei_classifier_inferencing_categories[max_index];
        Serial.print("Prediction: ");
        Serial.print(prediction);
        Serial.print(" (");
        Serial.print(max_value * 100);
        Serial.println("%)");

        if (strcmp(prediction, "O") == 0) {
            digitalWrite(RED_LED_PIN, HIGH);
        } else if (strcmp(prediction, "V") == 0) {
            digitalWrite(GREEN_LED_PIN, HIGH);
        } else if (strcmp(prediction, "Z") == 0) {
            digitalWrite(BLUE_LED_PIN, HIGH);
        }

        // sendGestureToServer(prediction,max_value);
        float confidence_percent = max_value * 100.0;
        
        if (confidence_percent < CONFIDENCE_THRESHOLD) {
            Serial.println("Low confidence - sending raw data to server...");
            sendRawDataToServer();
        } else {
            sendGestureToServer(prediction, confidence_percent);
        }
        
    }
}

void loop() {
    // to see if the button is pressesd
    if (!capturing && digitalRead(BUTTON_PIN) == LOW) {
        // debounce delay
        delay(50); 
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Starting gesture capture...");
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(BLUE_LED_PIN, LOW);
            sample_count = 0;
            capturing = true;
            capture_start_time = millis();
            last_sample_time = millis();
        }
    }

    if (capturing) {
        capture_accelerometer_data();
    }
}