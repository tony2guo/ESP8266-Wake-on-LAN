#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
#include <CSV_Parser.h>
#include <time.h>
#include <Ticker.h>
#include <HTTPSRedirect.h>

const char *ssid = "YOUR-WIFI-SSID";
const char *pass = "YOUR-WIFI-PASSWORD";

const char *host = "script.google.com";
const int httpsPort = 443;
const String GScriptId = "YOUR-APP-ID";

WiFiUDP UDP;
NTPClient timeClient(UDP);
WakeOnLan WOL(UDP);
HTTPSRedirect *client = nullptr;
Ticker ledToggle;

void SerialRelay(int id, bool status)
{
    Serial.flush();
    delay(100);
    Serial.write(0xA0);
    Serial.write(id + 1);
    Serial.write(status);
    Serial.write(0xA1 + status + id);
    Serial.flush();
    delay(100);
}

void ToggleRelay(int id, int delay_time = 500)
{
    SerialRelay(id, true);
    delay(delay_time);
    SerialRelay(id, false);
}

void ToggleLED()
{
    static bool toggle = false;
    toggle = !toggle;
    digitalWrite(LED_BUILTIN, toggle);
}

void SetLEDPerid(float period)
{
    ledToggle.attach(period, ToggleLED);
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    SetLEDPerid(0.05);
    delay(1000);
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.print("Connecting to wifi: ");
    Serial.print(ssid);

    SetLEDPerid(0.1);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.print(WiFi.localIP());
    Serial.print(" MAC: ");
    Serial.println(WiFi.macAddress());

    SetLEDPerid(0.5);
    timeClient.begin();
    Serial.print("NTP update");
    while (timeClient.forceUpdate() == false)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    WOL.setRepeat(3, 100); // Repeat the packet three times with 100ms delay between
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
}
void loop()
{
    static int errorCount = 0;
    const int kMaxErrorCount = 5;
    static bool flag = false;
    static time_t lastGetTime = 0;
    SetLEDPerid(1);

    timeClient.update();
    Serial.print("Formatted Time: ");
    String formattedTime = timeClient.getFormattedTime();
    Serial.println(formattedTime);

    time_t epochTime = timeClient.getEpochTime();
    Serial.print("Epoch Time: ");
    Serial.println(epochTime);
    if (lastGetTime == 0)
    {
        lastGetTime = epochTime;
        return;
    }

    if (!flag)
    {
        client = new HTTPSRedirect(httpsPort);
        client->setInsecure();
        flag = true;
        // client->setPrintResponseBody(true);
        client->setContentTypeHeader("application/json");
    }

    if (client != nullptr)
    {
        if (!client->connected())
        {
            client->connect(host, httpsPort);
        }
    }
    else
    {
        Serial.println("Error creating client object!");
        errorCount = kMaxErrorCount;
    }

    if (errorCount >= kMaxErrorCount)
    {
        errorCount = 0;
        flag = false;
        delete client;
        return;
    }

    String URL = "/macros/s/" + GScriptId + "/exec?client=bot";
    if (client->GET(URL, host))
    {
        String response = client->getResponseBody();
        CSV_Parser cp(response.c_str(), /*format*/ "sL");
        // cp.print();
        char **MACs = (char **)cp["MAC"];
        long *onTimes = (long *)cp["Epoch"];
        for (int i = 0; i < cp.getRowsCount(); i++)
        {
            Serial.println(MACs[i]);
            Serial.println(onTimes[i]);
            if (epochTime >= onTimes[i] && lastGetTime < onTimes[i])
            {
                SetLEDPerid(0.05);
                if (strcmp(MACs[i], "relay0") == 0)
                {
                    Serial.println("toggle relay0");
                    ToggleRelay(0);
                }
                else if (strcmp(MACs[i], "relay1") == 0)
                {
                    Serial.println("toggle relay1");
                    ToggleRelay(1);
                }
                else
                {
                    Serial.print("Send magic packet to: ");
                    Serial.println(MACs[i]);
                    WOL.sendMagicPacket(MACs[i]);
                }
            }
        }
        lastGetTime = epochTime;
    }
    else
    {
        ++errorCount;
        Serial.print("Error-count while connecting: ");
        Serial.println(errorCount);
    }

    delay(5000);
}