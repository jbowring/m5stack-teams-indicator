# M5Stack Teams Indicator

The [M5Stack ATOM Matrix](https://shop.m5stack.com/products/atom-matrix-esp32-development-kit) is an ESP32 housed in a neat package with a 5x5 grid of LEDs:

<img src="https://user-images.githubusercontent.com/4677505/174494104-0d6bfa62-191f-42d0-a843-9d98e8a34e4c.png" width=400>

The M5Stack Teams Indicator turns this nifty little unit into a status indicator for Microsoft Teams. The ESP32 connects to a local WiFi network and queries the Microsoft Graph API directly to get the user's current status. Below are examples of what is shown on the display for different statuses:

Available | Busy | Do not disturb | Away | Out of office
:-------------------------:|:-------------------------:|:-------------------------:|:-------------------------:|:-------------------------:
<img style="vertical-align:middle" src="https://user-images.githubusercontent.com/4677505/174495210-6c30bc6c-f76d-4643-94f5-18a39d39fca5.png" width=200> | <img src="https://user-images.githubusercontent.com/4677505/174495254-958d95dd-489f-488a-b1fc-41a181b9f219.png" width=200> | <img src="https://user-images.githubusercontent.com/4677505/174495255-1dc455b4-5561-4da6-a8a7-b4fc987a966b.png" width=200> | <img src="https://user-images.githubusercontent.com/4677505/174495247-d4f9c3b9-f9ee-4154-8364-670350ab0530.png" width=200> | <img src="https://user-images.githubusercontent.com/4677505/174495258-30300084-785e-4342-a2b4-83637ff8c570.png" width=200>

## Installation
1. Follow the instructions on the [M5Stack site](https://docs.m5stack.com/en/quick_start/atom/arduino) to set up an Arduino environment with the M5Stack libraries
2. Register a new Azure application following the [Azure documentation instructions](https://docs.microsoft.com/en-us/azure/active-directory/develop/quickstart-register-app)
3. Rename `ClientID.h.example` to `ClientID.h`, replacing the default string with the client ID from the Azure application
4. Rename `WiFiNetworks.h.example` to `WiFiNetworks.h` 
and populate it with WiFi network credentials
5. Flash the M5Stack

The first time the app is run it will ask (over the serial port) for the user to open a browser and log in with a Microsoft account. If authentication succeeds, it will store the refresh token in flash and the user will not be asked to log in again, even after a power cycle.

<img width=100% alt="authentication" src="https://user-images.githubusercontent.com/4677505/174496781-db3d1bf6-4032-42a0-9712-8e39928be67a.png">
