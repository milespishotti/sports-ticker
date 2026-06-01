# ESP32 Sports Score Ticker

## Overview

The ESP32 Sports Score Ticker is an embedded systems project that retrieves live sports scores from ESPN API endpoints and displays them across three HUB75 LED matrix panels in real time.

The project was developed to gain hands-on experience with embedded systems, networking, API integration, memory management, and real-time data processing. The system is designed to operate continuously while automatically updating game information throughout the day.

Currently, the ticker supports MLB, NFL, NBA, and NHL games, with future support planned for NCAA Football and NCAA Basketball.

---

## Features

* Retrieves live sports scores from ESPN API endpoints
* Supports MLB, NFL, NBA, and NHL
* Planned support for NCAA Football and NCAA Basketball
* Displays game information across three HUB75 LED matrix panels
* Updates sports data every 300 seconds
* Rotates displayed games every 4 seconds
* Hardware button allows league filtering
* Supports:

  * All Sports
  * NFL Only
  * NBA Only
  * MLB Only
  * NHL Only
* Filters games to display only those occurring on the current date
* Monitors available heap memory during operation
* Automatically recovers from low-memory conditions

---

## Technical Specifications

| Component                 | Specification      |
| ------------------------- | ------------------ |
| Microcontroller           | ESP32              |
| Display Type              | HUB75 LED Matrix   |
| Number of Panels          | 3                  |
| Panel Resolution          | 64×32 each         |
| Communication Protocol    | SPI                |
| Development Environment   | PlatformIO         |
| Programming Language      | C++                |
| Data Source               | ESPN API Endpoints |
| Data Refresh Interval     | 300 seconds        |
| Display Rotation Interval | 4 seconds          |
| Approximate Code Size     | 550 lines          |

---

## Hardware

* ESP32 microcontroller
* Three 64×32 HUB75 LED matrix panels
* SPI communication interface
* External power supply
* Mode-selection push button

---

## Software

* PlatformIO
* C++
* ESPN API endpoints
* ArduinoJson
* HUB75 display libraries
* Wi-Fi networking libraries

---

## System Architecture

1. The ESP32 connects to a Wi-Fi network.
2. Sports data is requested from ESPN API endpoints.
3. JSON responses are filtered and parsed.
4. Games that do not occur on the current date are removed.
5. Remaining games are organized based on the selected display mode.
6. Game information is formatted for display.
7. Scores are shown across the HUB75 LED matrix panels.
8. The display cycles through available games every 4 seconds.
9. New score data is retrieved every 300 seconds.

---

## Engineering Challenges

### Memory Management

One of the largest challenges during development was memory management on the ESP32.

The ESPN API responses contain significantly more information than is required for display. Early versions of the software experienced memory constraints due to the size of the JSON responses being deserialized.

To solve this issue, ArduinoJson document filters were implemented so that only the fields required for display are loaded into memory. This significantly reduced RAM consumption and improved system stability.

This project provided valuable experience working with resource-constrained embedded hardware and optimizing software to fit within available memory limits.

### Long-Term Reliability

The application continuously monitors available heap memory during operation.

During normal execution, free heap memory gradually decreases from approximately 190 KB to roughly 100 KB. To prevent instability or crashes, the system automatically restarts when memory falls below a predefined threshold.

This approach allows the ticker to maintain reliable long-term operation while additional memory optimizations continue to be developed.

### Display Communication

Several display communication approaches were evaluated during development before selecting SPI communication for the HUB75 display system.

Implementing and troubleshooting communication between the ESP32 and multiple LED matrix panels provided experience with hardware interfaces and real-time display control.

### Data Filtering and Processing

Another challenge involved ensuring that only relevant games were displayed.

The ESPN endpoints often contain games from multiple dates, requiring the software to determine the current date and filter out games that should not appear on the ticker.

Additional logic was implemented to organize games by sport and support user-selectable display modes.

---

## What I Learned

Through this project I gained experience with:

* Embedded systems development using ESP32 hardware
* Developing larger projects using PlatformIO
* HTTP networking and API integration
* Working with real-time sports data
* Parsing and processing JSON responses
* Selective JSON deserialization using ArduinoJson filters
* Memory optimization on resource-constrained devices
* Heap monitoring and system reliability strategies
* SPI communication
* HUB75 LED matrix interfacing
* Hardware and software integration
* Real-time data processing
* Debugging embedded systems
* Designing systems that operate continuously without user intervention

---

## Future Improvements

* Further reduce memory consumption
* Improve handling of game-status edge cases
* Build a permanent enclosure
* Add a web-based configuration interface
* Add user-configurable refresh intervals
* Expand display customization options

---

## Photos

(Add project photos here)

---

## Demo Video

(Add demo video link here)
