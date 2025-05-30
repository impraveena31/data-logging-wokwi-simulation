#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SD.h>
#include <SPI.h>

#define CS_PIN 5  // Chip Select pin for SD card

Adafruit_MPU6050 mpu;
QueueHandle_t dataQueue;

// Task Handles
TaskHandle_t Task1, Task2;

// Initial position tracking
float initialX, initialY, initialZ;
bool initialPositionSet = false;

int cycleCount = 0; // Track cycle number

void mpuTask(void *pvParameters);
void sdTask(void *pvParameters);
void returnToInitialPosition();
void stopSystem();
void printStoredDataWithCycles();

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22); // ESP32: SDA = 21, SCL = 22

    // Initialize MPU6050
    if (!mpu.begin()) {
        Serial.println("[ERROR] MPU6050 not found!");
        while (1);
    }
    Serial.println("[OK] MPU6050 connected!");

    // Initialize SD card
    if (!SD.begin(CS_PIN)) {
        Serial.println("[ERROR] SD card initialization failed!");
        while (1);
    }
    Serial.println("[OK] SD card initialized.");

    // Create FreeRTOS queue
    dataQueue = xQueueCreate(10, sizeof(String));

    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(mpuTask, "MPU6050 Task", 4096, NULL, 1, &Task1, 0);
    xTaskCreatePinnedToCore(sdTask, "SD Card Task", 4096, NULL, 2, &Task2, 1);
}

void loop() {
    // Empty loop since FreeRTOS handles tasks
}

// Task 1: Read MPU6050 Data
void mpuTask(void *pvParameters) {
    sensors_event_t a, g, temp;
    int count = 0;  // Counter to simulate error after a few cycles

    while (1) {
        Serial.println("[TASK 1] MPU6050 Task Running...");
        cycleCount++;  // Increment cycle number

        // Read sensor data
        mpu.getEvent(&a, &g, &temp);

        // 🛑 Simulate sensor error after 5 readings
        if (count == 5) {
            a.acceleration.x = NAN; // Simulate invalid reading
        }
        count++;

        // Validate sensor data (check for NaN or extreme values)
        if (isnan(a.acceleration.x) || isnan(a.acceleration.y) || isnan(a.acceleration.z) ||
            isnan(g.gyro.x) || isnan(g.gyro.y) || isnan(g.gyro.z) ||
            abs(a.acceleration.x) > 100 || abs(a.acceleration.y) > 100 || abs(a.acceleration.z) > 100) {
            Serial.println("[ERROR] Sensor data invalid! Stopping system...");
            
            // Print stored data with cycle numbers
            printStoredDataWithCycles();
            
            returnToInitialPosition();
            stopSystem();
        }

        // Store initial position if not already set
        if (!initialPositionSet) {
            initialX = a.acceleration.x;
            initialY = a.acceleration.y;
            initialZ = a.acceleration.z;
            initialPositionSet = true;
            Serial.println("[INFO] Initial position set.");
        }

        // Format sensor data with cycle number
        String dataString = "Cycle " + String(cycleCount) + ": " +
                            String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z) + "," +
                            String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z);

        // Send data to queue
        if (xQueueSend(dataQueue, &dataString, portMAX_DELAY) == pdPASS) {
            Serial.println("[TASK 1 -> QUEUE] Data sent: " + dataString);
        } else {
            Serial.println("[TASK 1 ERROR] Queue full, data lost!");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

// Task 2: Write Data to SD Card
void sdTask(void *pvParameters) {
    String receivedData;
    while (1) {
        if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            File dataFile = SD.open("/mpu_data.txt", FILE_APPEND);
            if (dataFile) {
                dataFile.println(receivedData);
                dataFile.close();
                Serial.println("[TASK 2 -> SD] Data saved: " + receivedData);
            } else {
                Serial.println("[TASK 2 ERROR] Error opening file!");
            }
        }
    }
}

// Function to move back to initial position
void returnToInitialPosition() {
    Serial.println("==================================");
    Serial.println("[SYSTEM ERROR] Moving back to initial position...");
    Serial.print("Returning to: X="); Serial.print(initialX);
    Serial.print(" Y="); Serial.print(initialY);
    Serial.print(" Z="); Serial.println(initialZ);

    // Simulating returning process (actual movement control code depends on hardware)
    vTaskDelay(pdMS_TO_TICKS(2000));

    Serial.println("[INFO] System returned to initial position.");
}

// Function to print all stored data with cycle numbers
void printStoredDataWithCycles() {
    Serial.println("==================================");
    Serial.println("[DATA LOG] Stored Data with Cycle Numbers:");

    File dataFile = SD.open("/mpu_data.txt", FILE_READ);
    if (!dataFile) {
        Serial.println("[ERROR] Cannot open file!");
        return;
    }

    // Read and print all data
    int entryNumber = 1;
    while (dataFile.available()) {
        String line = dataFile.readStringUntil('\n');
        Serial.print("[ENTRY "); Serial.print(entryNumber); Serial.print("] ");
        Serial.println(line);
        entryNumber++;
    }
    dataFile.close();

    Serial.println("==================================");
}

// Function to stop all tasks and halt system
void stopSystem() {
    Serial.println("[INFO] Stopping all tasks...");
    vTaskDelete(Task1);
    vTaskDelete(Task2);
    Serial.println("[FINAL] SYSTEM HALTED.");
    while (1);  // Halt system
}

