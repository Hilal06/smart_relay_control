#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <RTClib.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include <coredecls.h>
#include "webpage.h"

// Pin Definitions for Wemos D1 Mini
#define RELAY1_PIN 12 // D6
#define RELAY2_PIN 13 // D7
#define I2C_SDA 4     // D2
#define I2C_SCL 5     // D1

struct RelayConfig {
    bool state = false;
    bool sched_en = false;
    int on_hour = 0, on_min = 0;
    int off_hour = 0, off_min = 0;
};

RelayConfig r1, r2;
String wifi_ssid = "";
String wifi_pass = "";

RTC_DS3231 rtc;
ESP8266WebServer server(80);
bool ntp_synced = false;
bool rtc_found = false;

// Timezone (WIB is UTC+7 by default, change if needed)
const char* ntpServer = "pool.ntp.org";
const char* tzInfo = "WIB-7"; 

// Function Prototypes
void loadConfig();
void saveConfig();
void applyRelays();
void setupWiFi();
void checkSchedule();
String formatTime(int h, int m);

void time_is_set_cb() {
    if (rtc_found) {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        // Update RTC with NTP time
        rtc.adjust(DateTime(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
        ntp_synced = true;
        Serial.println("NTP time synchronized and RTC updated!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n--- Smart Relay Booting ---");

    // Initialize Pins
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);

    // Initialize I2C and RTC
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        rtc_found = false;
    } else {
        rtc_found = true;
        if (rtc.lostPower()) {
            Serial.println("RTC lost power, let's set the time!");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }

    // Initialize LittleFS
    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
    } else {
        loadConfig();
    }
    applyRelays();

    // Setup WiFi
    setupWiFi();

    // Setup NTP
    settimeofday_cb(time_is_set_cb);
    configTzTime(tzInfo, ntpServer);

    // Web Server Routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/api/status", HTTP_GET, []() {
        StaticJsonDocument<512> doc;
        
        if (rtc_found) {
            DateTime now = rtc.now();
            char timeStr[24];
            sprintf(timeStr, "%04d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
            doc["time"] = timeStr;
        } else {
            doc["time"] = "RTC Error";
        }
        
        doc["ntp_synced"] = ntp_synced;
        
        JsonObject r1Obj = doc.createNestedObject("relay1");
        r1Obj["state"] = r1.state;
        r1Obj["sched_en"] = r1.sched_en;
        r1Obj["on_time"] = formatTime(r1.on_hour, r1.on_min);
        r1Obj["off_time"] = formatTime(r1.off_hour, r1.off_min);

        JsonObject r2Obj = doc.createNestedObject("relay2");
        r2Obj["state"] = r2.state;
        r2Obj["sched_en"] = r2.sched_en;
        r2Obj["on_time"] = formatTime(r2.on_hour, r2.on_min);
        r2Obj["off_time"] = formatTime(r2.off_hour, r2.off_min);

        doc["wifi_ssid"] = wifi_ssid;

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });

    server.on("/api/relay", HTTP_POST, []() {
        if (server.hasArg("plain") == false) {
            server.send(400, "text/plain", "Body not received");
            return;
        }
        StaticJsonDocument<200> doc;
        deserializeJson(doc, server.arg("plain"));
        
        int r_num = doc["relay"];
        bool state = doc["state"];
        
        if (r_num == 1) r1.state = state;
        if (r_num == 2) r2.state = state;
        
        applyRelays();
        saveConfig();
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/schedule", HTTP_POST, []() {
        if (server.hasArg("plain") == false) {
            server.send(400, "text/plain", "Body not received");
            return;
        }
        StaticJsonDocument<256> doc;
        deserializeJson(doc, server.arg("plain"));
        
        int r_num = doc["relay"];
        bool en = doc["en"];
        String on_time = doc["on"].as<String>();
        String off_time = doc["off"].as<String>();
        
        int on_h = on_time.substring(0, 2).toInt();
        int on_m = on_time.substring(3, 5).toInt();
        int off_h = off_time.substring(0, 2).toInt();
        int off_m = off_time.substring(3, 5).toInt();
        
        if (r_num == 1) {
            r1.sched_en = en; r1.on_hour = on_h; r1.on_min = on_m; r1.off_hour = off_h; r1.off_min = off_m;
        } else if (r_num == 2) {
            r2.sched_en = en; r2.on_hour = on_h; r2.on_min = on_m; r2.off_hour = off_h; r2.off_min = off_m;
        }
        
        saveConfig();
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/wifi", HTTP_POST, []() {
        if (server.hasArg("plain") == false) {
            server.send(400, "text/plain", "Body not received");
            return;
        }
        StaticJsonDocument<256> doc;
        deserializeJson(doc, server.arg("plain"));
        
        String new_ssid = doc["ssid"].as<String>();
        String new_pass = doc["pass"].as<String>();
        
        if (new_ssid.length() > 0) {
            wifi_ssid = new_ssid;
            if (new_pass.length() > 0) wifi_pass = new_pass;
            saveConfig();
            server.send(200, "application/json", "{\"status\":\"rebooting\"}");
            delay(1000);
            ESP.restart();
        } else {
            server.send(400, "application/json", "{\"status\":\"error\"}");
        }
    });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
    
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 10000) { // Check schedule every 10 seconds
        lastCheck = millis();
        checkSchedule();
    }
}

void applyRelays() {
    digitalWrite(RELAY1_PIN, r1.state ? HIGH : LOW);
    digitalWrite(RELAY2_PIN, r2.state ? HIGH : LOW);
}

void loadConfig() {
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return;
    }
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse config file");
        return;
    }
    
    r1.state = doc["r1_state"] | false;
    r1.sched_en = doc["r1_en"] | false;
    r1.on_hour = doc["r1_on_h"] | 0;
    r1.on_min = doc["r1_on_m"] | 0;
    r1.off_hour = doc["r1_off_h"] | 0;
    r1.off_min = doc["r1_off_m"] | 0;

    r2.state = doc["r2_state"] | false;
    r2.sched_en = doc["r2_en"] | false;
    r2.on_hour = doc["r2_on_h"] | 0;
    r2.on_min = doc["r2_on_m"] | 0;
    r2.off_hour = doc["r2_off_h"] | 0;
    r2.off_min = doc["r2_off_m"] | 0;

    wifi_ssid = doc["wifi_ssid"] | "";
    wifi_pass = doc["wifi_pass"] | "";
    
    file.close();
}

void saveConfig() {
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    StaticJsonDocument<1024> doc;
    
    doc["r1_state"] = r1.state;
    doc["r1_en"] = r1.sched_en;
    doc["r1_on_h"] = r1.on_hour; doc["r1_on_m"] = r1.on_min;
    doc["r1_off_h"] = r1.off_hour; doc["r1_off_m"] = r1.off_min;

    doc["r2_state"] = r2.state;
    doc["r2_en"] = r2.sched_en;
    doc["r2_on_h"] = r2.on_hour; doc["r2_on_m"] = r2.on_min;
    doc["r2_off_h"] = r2.off_hour; doc["r2_off_m"] = r2.off_min;

    doc["wifi_ssid"] = wifi_ssid;
    doc["wifi_pass"] = wifi_pass;

    serializeJson(doc, file);
    file.close();
}

void setupWiFi() {
    WiFi.mode(WIFI_AP_STA);
    
    // Always start SoftAP
    WiFi.softAP("SmartRelay_Setup", "12345678");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Connect to STA if configured
    if (wifi_ssid.length() > 0) {
        Serial.print("Connecting to WiFi: ");
        Serial.println(wifi_ssid);
        WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println("");
        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("Connected! IP address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("Failed to connect to WiFi STA.");
        }
    }
}

int getMinutesFromMidnight(int h, int m) {
    return h * 60 + m;
}

bool isTimeInInterval(int current, int start, int end) {
    if (start < end) {
        return (current >= start && current < end);
    } else if (start > end) { // Interval crosses midnight
        return (current >= start || current < end);
    }
    return false; // start == end
}

void checkSchedule() {
    if (!rtc_found) return;
    
    DateTime now = rtc.now();
    int current_mins = getMinutesFromMidnight(now.hour(), now.minute());
    
    bool state_changed = false;

    // Check Relay 1
    if (r1.sched_en) {
        int start_mins = getMinutesFromMidnight(r1.on_hour, r1.on_min);
        int end_mins = getMinutesFromMidnight(r1.off_hour, r1.off_min);
        bool expected_state = isTimeInInterval(current_mins, start_mins, end_mins);
        if (r1.state != expected_state) {
            r1.state = expected_state;
            state_changed = true;
        }
    }

    // Check Relay 2
    if (r2.sched_en) {
        int start_mins = getMinutesFromMidnight(r2.on_hour, r2.on_min);
        int end_mins = getMinutesFromMidnight(r2.off_hour, r2.off_min);
        bool expected_state = isTimeInInterval(current_mins, start_mins, end_mins);
        if (r2.state != expected_state) {
            r2.state = expected_state;
            state_changed = true;
        }
    }

    if (state_changed) {
        applyRelays();
        saveConfig();
        Serial.println("Schedule triggered: Relays updated.");
    }
}

String formatTime(int h, int m) {
    char buf[6];
    sprintf(buf, "%02d:%02d", h, m);
    return String(buf);
}
