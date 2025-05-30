🧪 **How the Simulation Works**
This project simulates an ESP32-based real-time sensor data logger using FreeRTOS in Wokwi. The ESP32 reads data from an MPU6050 (accelerometer + gyroscope) and logs it to a virtual SD card.

The simulation demonstrates real-time parallel tasking, error handling, and file system operations using two FreeRTOS tasks running on separate cores.

**🧰 Components Used**
ESP32 Dev Module (dual-core)

MPU6050 (I2C communication)

SD Card Module (SPI communication)

FreeRTOS (built-in in ESP32 Arduino core)

Wokwi simulator
![image](https://github.com/user-attachments/assets/7a6045cd-0a99-4d4f-8ad7-d17f00d1afee)

**🧩 Task Breakdown**
1️⃣ readSensorTask (Core 0):
Reads accelerometer and gyroscope values from the MPU6050 using I2C.

Checks for invalid values (like NaN) to detect sensor issues.

On valid data:

Packages it into a struct and sends it to a FreeRTOS queue.

On error:

Prints error to Serial.

Deletes the task using vTaskDelete.

Simulates returning to initial position.

2️⃣ logDataTask (Core 1):
Waits for sensor data from the queue.

Appends the received values to a file on the SD card (/mpu_data.txt).

Also prints the logged values to Serial for debugging.

**🔁 Simulation Flow**
Start-up: Initializes I2C, SD card, MPU6050.

Task creation: Two FreeRTOS tasks are pinned to separate cores.

MPU6050 reads:

Every 100ms, readSensorTask samples acceleration & gyro data.

If valid, sends it to the logDataTask via queue.

If invalid, prints an error and stops further sampling.

Logging:

logDataTask receives the data from the queue.

Opens the file in append mode.

Writes data with a timestamp (cycle count) to SD card.

Serial Output:

Both tasks print to Serial to show system status and debug info.
![image](https://github.com/user-attachments/assets/cb314a58-d7c8-44dd-a3a3-8d39af8d678d)

**📈 What You See in Wokwi**
Serial Monitor shows:

Sensor readings from readSensorTask.

Logging confirmation from logDataTask.

Any sensor error events (e.g., "Invalid data received!").

SD card file grows with each logged entry.

If sensor fails (simulated by NaNs), logging halts gracefully.

**⚙️ Technical Highlights**
FreeRTOS multitasking with xTaskCreatePinnedToCore

Thread-safe communication using xQueueSend / xQueueReceive

File I/O using Arduino SD.h library

Error handling using isnan() and vTaskDelete()

