# ESP8266-Wake-on-LAN
ESP8266 Wake-on-LAN and relay control with Google Sheets and Google Apps Script

ESP8266 (the bot) will check the time and commands in the Google Sheet and choose to send multiple Wake-on-LAN magic packets and/or control LC-Relay-ESP01-2R-5V relay module. Users can edit "MAC address" and "on time" in the Google Sheet. If the value of "on time" is NOW(), it will be updated whenever the sheet is edited or when someone goes to the App URL.

## Dependencies
* [HTTPSRedirect](https://github.com/electronicsguy/HTTPSRedirect)
* [WakeOnLan](https://github.com/a7md0/WakeOnLan)
* [CSV-Parser-for-Arduino](https://github.com/michalmonday/CSV-Parser-for-Arduino)
* [NTPClient](https://github.com/arduino-libraries/NTPClient)

## Usage
### Google Sheets
1. Create a copy of the example [Google Sheets](https://docs.google.com/spreadsheets/d/1EcZ3VdlwAWUzbpfXQl1WoVENOWdkfZqq5p5OwH1Co6A/edit?usp=sharing).
2. Edit MAC address and time zone.
3. Deploy Google Apps Script and get your App ID and URL. Should look like this: `https://script.google.com/macros/s/YOUR-APP-ID/exec`
4. Go to your App URL to update NOW(), so the bot will know the link is clicked. [example user link](https://script.google.com/macros/s/AKfycbyfqzNhH7XDwJ45n6gDZ4jYQnBfjXLt88P5YbyiACKrFDNb-RwU1nKCP_fOrefJXpI8EA/exec)
5. Go to your App URL with `?client=bot` added to the end, to see what the bot will get. [example bot link](https://script.google.com/macros/s/AKfycbyfqzNhH7XDwJ45n6gDZ4jYQnBfjXLt88P5YbyiACKrFDNb-RwU1nKCP_fOrefJXpI8EA/exec?client=bot)

### ESP8266
1. Edit YOUR-WIFI-SSID, YOUR-WIFI-PASSWORD, and YOUR-APP-ID in [main.cpp](src/main.cpp)
2. Build and upload. ([PlatformIO](https://platformio.org/) recommended)
3. The built-in LED will blink at a perid of 0.05 seconds at startup, or when sending Wake-on-LAN magic packet/controlling relay, 0.1 seconds when connecting to WiFi, 0.5 seconds when updating time with NTP, and 1 second when waiting for changes in Google Sheets.
4. Open serial monitor to see more information.