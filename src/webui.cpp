#include <WebServer.h>
#include <DateTime.h>
#include "webui.h"
#include "Meteo.h"
#include "Logger.h"

extern Meteo meteo;
extern Logger logger;

WebServer webServer(80);

bool checkAuthentified() {
    if (webServer.authenticate(AUTH_USER, AUTH_PASS)) {
        Serial.println(F("WebUI - Authentication GRANTED"));
        return true;
    }
    Serial.println(F("WebUI - Authentication requested..."));
    webServer.requestAuthentication(BASIC_AUTH, "METEO-LOGGER-AUTH-REALM", F("Authentication failed"));
    return false;
}

void webServerOnIndexPage() {
    String html(F(R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <title>Meteo Logger</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            font-family: Consolas;
            font-size: 1.05em;
            letter-spacing: -0.04em;
        }
        h1 {
            margin: 1.5rem 0;
            font-size: 2.5rem;
            text-align: center;
        }
        h4 {
            max-width: calc(var(--width) + 2.5rem);
            margin: 0;
            border-bottom: 1px solid #888;
            font-size: 1.5rem;
        }
        .container {
            max-width: 400px;
            margin: 0 auto;
            padding: 0.5rem;
        }
	    .value {
            margin: 0;
            margin-bottom: 0.5rem;
            color: #4285f4;
            text-align: center;
            font-size: 75px;
        }
        .param {
            text-align: center;
            margin: 0.5rem 0;
            font-size: 1.1rem;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Meteo Logger</h1>
        <h4>Temperature, °C</h4>
        <div class="value">{TEMPC}</div>
        <h4>Humidity, %</h4>
        <div class="value">{HUMID}</div>
        <h4>Pressure, mm</h4>
        <div class="value">{PRESS}</div>
        <h4>Details</h4>
        <div class="param">{DATEX}</div>
        <div class="param">Measurement: {MEASX}</div>
        <div class="param"><a href="logger-setup">Logger Setup</a></div>
    </div>
</body>
</html>
)""""));
    html.replace(F("{TEMPC}"), String((meteo.temp1 + meteo.temp2) / 2));
    html.replace(F("{HUMID}"), String(meteo.humid));
    html.replace(F("{PRESS}"), String(meteo.press, 1));
    html.replace(F("{DATEX}"), String(DateTime(meteo.timex).toString("%H:%M:%S %a, %b %d, %Y"))); //19:02:43 Sat, Sep 13, 2025
    html.replace(F("{MEASX}"), String(meteo.measx));
    webServer.send(200, F("text/html"), html);
}

void webServerOnLoggerSetup() {
    String html(F(R""""(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="icon" href="data:,">
    <title>Logger Setup</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            font-family: Consolas;
            font-size: 1.05em;
            letter-spacing: -0.04em;
        }
        h1 {
            margin: 1.5rem 0;
            font-size: 2.5rem;
            text-align: center;
        }
        .container {
            max-width: 400px;
            margin: 0 auto;
            padding: 0.5rem;
        }
        label span {
            display: inline-block;
            width: 95%;
        }
        label input[type=text] {
            font-size: 1em;
            padding: 0.1em 0.1em;
            margin: 0.5em 0;
            width: 95%;
        }
        input[type=submit] {
            font-size: 1em;
            padding: 0.1em 1em;
            margin: 0.5em;
            float: right;
        }
    </style>
</head>
<body>
    <h1>Logger Setup</h1>
    <div class="container">
    <form method="post" action="/logger-setup">
        <label>
            <span>Sensing period:</span>
            <input type="text" name="sensing" value="{SENSING}" required><br>
        </label>
        <label>
            <span>Logging period:</span>
            <input type="text" name="logging" value="{LOGGING}" required><br>
        </label>
        <label>
            <span>Logging request:</span>
            <input type="text" name="request" value="{REQUEST}" required><br>
        </label>
        <input type="submit" value="Submit">
    </form>
    </div>
</body>
</html>
)""""));
    html.replace(F("{SENSING}"), String(meteo.sensingPeriod));
    html.replace(F("{LOGGING}"), String(logger.loggingPeriod));
    html.replace(F("{REQUEST}"), logger.loggingRequest);
    webServer.send(200, F("text/html"), html);
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
    Serial.print(F("Handling client "));
    Serial.print(webServer.client().remoteIP());
    Serial.print(F(", 404 Resource not found: "));
    Serial.println(webServer.uri());
    webServer.send(404, F("text/plain"), F("Not found"));
};

void initWebServer() {
    webServer.on(F("/"), HTTP_GET, webServerOnIndexPage);
    webServer.on(F("/logger-setup"), HTTP_GET, webServerOnLoggerSetup);
    webServer.on(F("/logger-setup"), HTTP_POST, webServerOnLoggerSetupPOST);
    webServer.onNotFound(webServerOnNotFound);
    webServer.begin();
}

void handleWebServer() {
    if (WiFi.isConnected())
        webServer.handleClient();
}