#include <WebServer.h>
#include <LittleFS.h>
#include <DateTime.h>
#include "webui.h"
#include "DeviceConfig.h"
#include "Meteo.h"
#include "Logger.h"

#define UNCHANGED_OPTION "(unchanged)"

extern DeviceConfig cfg;
extern Meteo meteo;
extern Logger logger;

WebServer webServer(80);

bool isSubnetEquals(const IPAddress& a, const IPAddress& b, const IPAddress& mask) {
    for (uint8_t i = 0; i < 4; ++i)
        if ((a[i] & mask[i]) != (b[i] & mask[i]))
            return false;
    return true;
}

String loadPageTemplate(const String& name) {
    String data, path(F("/webui/templates/{NAME}.html"));
    path.replace(F("{NAME}"), name);
    File file = LittleFS.open(path);
    if (file) {
        data = file.readString();
        file.close();
    }
    else webServer.send(500, F("text/plain"), F("Page template loading failed"));
    return data;
}

bool checkClientLocal() {
    if (WiFi.getMode() == WIFI_MODE_AP || isSubnetEquals(
            webServer.client().remoteIP(), WiFi.localIP(), WiFi.subnetMask()))
        return true;
    else {
        String html = loadPageTemplate(F("error"));
        if (html.length()) {
            html.replace(F("{ERRNUM}"), String(403));
            html.replace(F("{ERRTXT}"), F("Forbidden"));
            webServer.send(404, F("text/html"), html);
        }
    }
    return false;
}

bool checkAuthentified() {
    if(WiFi.getMode() == WIFI_AP)
        return true;
    if (!checkClientLocal())
        return false;
    String user = getStringPartCsv(cfg.auth, 0), pass = getStringPartCsv(cfg.auth, 1);
    if (webServer.authenticate(user.c_str(), pass.c_str())) {
        #if DEBUG
        Serial.println(F("WebUI - Authentication passed"));
        #endif
        return true;
    }
    else {
        #if DEBUG
        Serial.println(F("WebUI - Authentication requested..."));
        #endif

        String html = loadPageTemplate(F("error"));
        if (html.length()) {
            html.replace(F("{ERRNUM}"), String(401));
            html.replace(F("{ERRTXT}"), F("Unauthorized"));
        }
        else html = F("Error 401: Unauthorized");
        webServer.requestAuthentication(BASIC_AUTH, "IOT-DEVICE-BASIC-AUTH-REALM", html);
    }
    return false;
}

void webServerOnIndexPage() {
    String html = loadPageTemplate(F("index"));
    if (html.length()) {
        html.replace(F("{FVERS}"), F(FIRMWARE_VERSION));
        html.replace(F("{TEMPC}"), String((meteo.temp1 + meteo.temp2) / 2));
        html.replace(F("{HUMID}"), String(meteo.humid));
        html.replace(F("{PRESS}"), String(meteo.press, 1));
        html.replace(F("{DATEX}"), String(DateTime(meteo.timex).toString("%T %a, %b %d, %Y"))); //19:02:43 Sat, Sep 13, 2025
        html.replace(F("{MEASX}"), String(meteo.measx));
        unsigned long past = millis() / 1000UL;
        html.replace(F("{UPTIM}"), String(past / 86400UL) + DateTime(past).toDateTimeUTC().toString("/%T"));

        html.replace(F("{TSUCC}"), String(logger.countSuccess));
        html.replace(F("{TFAIL}"), String(logger.countFails));
        html.replace(F("{LFAIL}"), String(logger.countLastFails));
        webServer.send(200, F("text/html"), html);
    }
}

void webServerOnDeviceSetup() {
    if (checkAuthentified()) {
        String html = loadPageTemplate(F("device-setup"));
        if (html.length()) {
            html.replace(F("{SSID}"), cfg.ssid);
            html.replace(F("{PASS}"), F(UNCHANGED_OPTION));
            html.replace(F("{HOST}"), cfg.host);

            html.replace(F("{ZONE}"), cfg.zone);
            html.replace(F("{TSNS}"), cfg.tsns);

            html.replace(F("{AUTH}"), F(UNCHANGED_OPTION));

            html.replace(F("{ADDR}"), cfg.addr.toString());
            html.replace(F("{GATE}"), cfg.gate.toString());
            html.replace(F("{MASK}"), cfg.mask.toString());
            html.replace(F("{DNS1}"), cfg.dns1.toString());
            html.replace(F("{DNS2}"), cfg.dns2.toString());

            webServer.send(200, F("text/html"), html);
        }
    }
}

void webServerOnDeviceSetupPOST() {
    if (checkAuthentified()) {
        for(int i = 0; i < webServer.args(); ++i) {
            String k = webServer.argName(i), v = webServer.arg(i);
            if ((k == F("pass") || k == F("auth")) && v == F(UNCHANGED_OPTION))
                continue;
            if (v.length() > 128)
                continue;
            cfg.setField(k, v);
        }

        webServer.sendHeader(F("Location"), String(F("http://")) + webServer.hostHeader(), true);
        webServer.send(302, F("text/plain"), F("Redirecting to main page"));

        Serial.println(F("Device config updated\n=============================="));
        cfg.print();
        Serial.println(F("==============================\n"));

        // Serial.print(F("Device configuration updated. Saving... "));
        // Serial.println(cfg.save() ? F("OK") : F("FAILED"));
        // Serial.println(F("Restarting device..."));
        // ESP.restart();
    }
}

void webServerOnLoggerSetup() {
    if (checkAuthentified()) {
        String html = loadPageTemplate(F("logger-setup"));
        if (html.length()) {
            html.replace(F("{SENSING}"), String(meteo.sensingPeriod));
            html.replace(F("{LOGGING}"), String(logger.loggingPeriod));
            html.replace(F("{REQUEST}"), logger.loggingRequest);

            webServer.send(200, F("text/html"), html);
        }
    }
}

void webServerOnLoggerSetupPOST() {
    if (checkAuthentified()) {
        if (webServer.hasArg(F("sensing"))) {
            Serial.print(F("Sensing period set: "));
            Serial.println(meteo.sensingPeriod = webServer.arg(F("sensing")).toInt());
        }
        if (webServer.hasArg(F("logging"))) {
            Serial.print(F("Logging period set: "));
            Serial.println(logger.loggingPeriod = webServer.arg(F("logging")).toInt());
        }
        if (webServer.hasArg(F("request"))) {
            Serial.print(F("Logging request set: "));
            Serial.println(logger.loggingRequest = webServer.arg(F("request")));
        }
    }
    webServer.sendHeader(F("Location"), String(F("http://")) + webServer.hostHeader(), true);
    webServer.send(302, F("text/plain"), F("Redirecting to main page"));
}

void webServerOnNotFound() {
#if DEBUG
    Serial.print(F("Handling client "));
    Serial.print(webServer.client().remoteIP());
    Serial.print(F(", 404 Resource not found: "));
    Serial.println(webServer.uri());
#endif
    String html = loadPageTemplate(F("error"));
    if (html.length()) {
        html.replace(F("{ERRNUM}"), String(404));
        html.replace(F("{ERRTXT}"), F("Resource not found"));
        webServer.send(404, F("text/html"), html);
    }
};

void initWebServer() {
    webServer.on(F("/"), HTTP_GET, webServerOnIndexPage);
    webServer.on(F("/logger-setup"), HTTP_GET, webServerOnLoggerSetup);
    webServer.on(F("/logger-setup"), HTTP_POST, webServerOnLoggerSetupPOST);
    webServer.on(F("/device-setup"), HTTP_GET, webServerOnDeviceSetup);
    webServer.on(F("/device-setup"), HTTP_POST, webServerOnDeviceSetupPOST);
    webServer.serveStatic("/", LittleFS, "/webui/static/", "max-age=60"); //"max-age=3600" or "no-cache"
    webServer.onNotFound(webServerOnNotFound);
    webServer.begin();
}

void handleWebServer() {
    if (WiFi.isConnected() || (WiFi.getMode() == WIFI_AP && WiFi.softAPgetStationNum() > 0)) {
        webServer.handleClient();
    }
}

void initWebServerAP() {
    webServer.on(F("/"), HTTP_GET, webServerOnDeviceSetup);
    webServer.on(F("/device-setup"), HTTP_POST, webServerOnDeviceSetupPOST);
    webServer.serveStatic("/", LittleFS, "/webui/static/", "max-age=60");
    webServer.onNotFound(webServerOnNotFound);
    webServer.begin();
}