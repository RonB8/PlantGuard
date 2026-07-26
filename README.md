# PlantGuard 🌱🐶

PlantGuard is an ESP32-based smart plant monitoring system that combines IoT, FreeRTOS, and AI-powered object detection to protect and monitor outdoor plants.

The system continuously measures soil moisture and sends Telegram notifications when watering is required. It also uses a distance sensor to detect nearby objects, activates an ESP32-CAM to capture an image, and leverages an AI detection service to identify dogs. When a dog is detected near the plant, the system immediately sends a Telegram alert with the detection result.

Features
🌱 Real-time soil moisture monitoring
💧 Telegram notifications when watering is needed
📏 Distance-based event detection using a VL53L0X sensor
📷 Image capture with ESP32-CAM
🤖 AI-powered dog detection
📲 Telegram alerts for dog detection events
⚡ FreeRTOS-based multitasking architecture
📡 Wi-Fi connectivity and asynchronous communication
📝 Modular, production-style Embedded software architecture
Hardware
ESP32 DevKit V1
ESP32-CAM (OV2640)
Capacitive Soil Moisture Sensor v2.0
VL53L0X Time-of-Flight Distance Sensor
Wi-Fi connection
Technologies
C++
ESP-IDF / Arduino Framework
FreeRTOS
Telegram Bot API
HTTP REST API
JSON
Embedded Systems
IoT
Computer Vision Integration
