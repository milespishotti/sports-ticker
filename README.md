# ESP32 Sports Score Ticker

## Overview

The ESP32 Sports Score Ticker is an embedded systems project that retrieves live sports scores from ESPN's public API endpoints and displays them on LED matrix panels in real time.

The project was developed to gain hands-on experience with embedded systems, networking, API integration, memory management, and real-time data processing. The system is designed to run continuously while automatically updating game information throughout the day.

## Features

* Retrieves live sports data from ESPN API endpoints
* Connects to Wi-Fi using an ESP32 microcontroller
* Parses and displays real-time game information
* Displays scores on LED matrix panels
* Automatically refreshes data at regular intervals
* Monitors available memory during operation
* Recovers from low-memory conditions through automatic restart logic

## Hardware

* ESP32 microcontroller
* LED matrix display panels
* Power supply
* SPI communication interface

## Software

* PlatformIO
* C++
* ESPN API endpoints
* ArduinoJson
* LED matrix display libraries

## System Architecture

1. The ESP32 connects to a Wi-Fi network.
2. The device requests score data from ESPN endpoints.
3. JSON responses are filtered to keep only required information.
4. Relevant game data is extracted and formatted.
5. Information is displayed on the LED matrix panels.
6. The process repeats periodically to maintain current scores.

## Engineering Challenges

### Memory Management

One of the largest challenges during development was memory management on the ESP32.

The ESPN API responses contain significantly more information than is required for display. Initially, deserializing the complete JSON responses resulted in memory constraints and system instability.

To address this issue, ArduinoJson document filtering was implemented so that only the fields required for display are deserialized and stored in memory. This significantly reduced RAM usage and improved system reliability.

### Long-Term Reliability

The application continuously monitors available heap memory during operation.

If available memory drops below a defined threshold, the ESP32 automatically restarts to prevent instability and maintain long-term operation.

### Display Communication

Multiple display communication approaches were evaluated during development before settling on SPI communication for the LED matrix panels.

## What I Learned

* Embedded systems development using ESP32 hardware
* Working with HTTP requests and web APIs
* JSON parsing and selective deserialization
* Memory optimization on resource-constrained devices
* SPI communication
* Real-time data processing
* Hardware and software integration
* Debugging embedded systems
* Developing larger projects using PlatformIO

## Future Improvements

* Improve handling of game status edge cases
* Support additional leagues and sports
* Build a permanent enclosure for the display
* Add a configuration web interface
* Further optimize memory usage

## Photos

(Add photos here)

## Demo Video

(Add video link here)
