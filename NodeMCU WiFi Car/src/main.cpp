#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFiClient.h>

#define RIGHT_BACKWARD D1
#define RIGHT_FORWARD D2
#define LEFT_BACKWARD D5
#define LEFT_FORWARD D6

#define ON HIGH
#define OFF LOW
#define ONE_SECOND 1000

#define MAX_ARGUMENTS 4

ESP8266WiFiMulti wifiMulti; // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

AsyncWebServer server(80); // Create a webserver object that listens for HTTP request on port 80

struct Command
{
    String func = "";
    String args[MAX_ARGUMENTS] = {};
    int argCount = 0;
    struct Command *nextCommand = NULL;
};

void stop(char wheel, AsyncWebServerRequest *request)
{
    Serial.printf("stop(%c)\n", wheel);
    if (wheel == 'A')
    {
        digitalWrite(LEFT_BACKWARD, HIGH);
        digitalWrite(LEFT_FORWARD, HIGH);
        digitalWrite(RIGHT_BACKWARD, HIGH);
        digitalWrite(RIGHT_FORWARD, HIGH);
        // delay(100);
        // digitalWrite(LEFT_BACKWARD, LOW);
        // digitalWrite(LEFT_FORWARD, LOW);
        // digitalWrite(RIGHT_BACKWARD, LOW);
        // digitalWrite(RIGHT_FORWARD, LOW);
    }
    else if (wheel == 'L')
    {
        digitalWrite(LEFT_BACKWARD, HIGH);
        digitalWrite(LEFT_FORWARD, HIGH);
        // delay(100);
        // digitalWrite(LEFT_BACKWARD, LOW);
        // digitalWrite(LEFT_FORWARD, LOW);
    }
    else if (wheel == 'R')
    {
        digitalWrite(RIGHT_BACKWARD, HIGH);
        digitalWrite(RIGHT_FORWARD, HIGH);
        // delay(100);
        // digitalWrite(RIGHT_BACKWARD, LOW);
        // digitalWrite(RIGHT_FORWARD, LOW);
    }
    else
    {
        String errorMsg = "Bad Request: stop() wheel should be 'A', 'L', or 'R'. Received '" + String(wheel) + "'";
        Serial.println(errorMsg);
        if (request != NULL)
        {
            request->send(503, "text/plain", errorMsg);
        }
    }

    if (request != NULL)
    {
        request->send(200);
    }
}

void move(char wheel, char direction, float speed, AsyncWebServerRequest *request)
{
    Serial.printf("move(%c, %c, %f)\n", wheel, direction, speed);
    int pwm = (int)(speed * PWMRANGE);
    Serial.printf("pwm = %d\n", pwm);

    digitalWrite(LEFT_FORWARD, LOW);
    digitalWrite(RIGHT_FORWARD, LOW);
    digitalWrite(LEFT_BACKWARD, LOW);
    digitalWrite(RIGHT_BACKWARD, LOW);

    if (direction == 'F')
    {
        if (wheel == 'A')
        {
            analogWrite(LEFT_FORWARD, pwm);
            analogWrite(RIGHT_FORWARD, pwm);
        }
        else if (wheel == 'L')
        {
            analogWrite(LEFT_FORWARD, pwm);
        }
        else if (wheel == 'R')
        {
            analogWrite(RIGHT_FORWARD, pwm);
        }
        else
        {
            stop('A', NULL);
            String errorMsg = "Bad Request: move() wheel should be 'A', 'L', or 'R'. Received '" + String(wheel) + "'";
            Serial.println(errorMsg);
            if (request != NULL)
            {
                request->send(503, "text/plain", errorMsg);
            }
        }
    }
    else if (direction == 'B')
    {
        if (wheel == 'A')
        {
            analogWrite(LEFT_BACKWARD, pwm);
            analogWrite(RIGHT_BACKWARD, pwm);
        }
        else if (wheel == 'L')
        {
            analogWrite(LEFT_BACKWARD, pwm);
        }
        else if (wheel == 'R')
        {
            analogWrite(RIGHT_BACKWARD, pwm);
        }
        else
        {
            stop('A', NULL);
            String errorMsg = "Bad Request: move() wheel should be 'A', 'L', or 'R'. Received '" + String(wheel) + "'";
            Serial.println(errorMsg);
            if (request != NULL)
            {
                request->send(503, "text/plain", errorMsg);
            }
        }
    }
    else
    {
        stop('A', NULL);
        String errorMsg = "Bad command";
        Serial.println(errorMsg);
        if (request != NULL)
        {
            request->send(503, "text/plain", errorMsg);
        }
    }

    if (request != NULL)
    {
        request->send(200);
    }
}

void spin(char direction, float speed, AsyncWebServerRequest *request)
{
    int pwm = (int)(speed * PWMRANGE);
    Serial.printf("spin(%c, %f) [pwm = %d]\n", direction, speed, pwm);
    if (direction == 'L')
    {
        digitalWrite(LEFT_FORWARD, 0);
        digitalWrite(RIGHT_BACKWARD, 0);

        analogWrite(LEFT_BACKWARD, pwm);
        analogWrite(RIGHT_FORWARD, pwm);
    }
    else if (direction == 'R')
    {
        digitalWrite(LEFT_BACKWARD, 0);
        digitalWrite(RIGHT_FORWARD, 0);

        analogWrite(LEFT_FORWARD, pwm);
        analogWrite(RIGHT_BACKWARD, pwm);
    }
    else
    {
        stop('A', NULL);
        String errorMsg = "Bad Request: spin() direction should be 'L' or 'R'. Received '" + String(direction) + "'";
        Serial.println(errorMsg);
        if (request != NULL)
        {
            request->send(503, "text/plain", errorMsg);
        }
    }

    if (request != NULL)
    {
        request->send(200);
    }
}

void jump(AsyncWebServerRequest *request)
{
    Serial.println("jump()");
    // move('A', 'B', 1.0f);
    // delay(10);
    // stop('A');
    if (request != NULL)
    {
        request->send(200);
    }
}

Command *interpret(String statement)
{
    int indexOfOpenParen = statement.indexOf('(');

    struct Command *command = NULL;
    command = new Command();
    command->func = statement.substring(0, indexOfOpenParen);
    int stopIndex = statement.length();

    int argStartIndex = indexOfOpenParen + 1;
    char openingQuote = 0;

    for (int i = indexOfOpenParen + 1; i < stopIndex; i++)
    {
        char ch = statement.charAt(i);

        if (openingQuote == 0)
        {
            // Not inside quotes (string or char)
            if (ch == '\'' || ch == '"')
            {
                // Exclude opening single or double quote
                argStartIndex = i + 1;
                openingQuote = ch;
                continue;
            }

            if (ch == ' ' || ch == '\t')
            {
                // Ignore whitespace
                argStartIndex = i + 1;
                continue;
            }

            if ((ch == '\n' || ch == ';') && i + 1 < stopIndex)
            {
                command->nextCommand = interpret(statement.substring(i + 1, stopIndex));
                break;
            }
        }

        if (ch == ',' || ch == ')' || ch == openingQuote)
        {
            String arg = statement.substring(argStartIndex, i);
            command->args[command->argCount] = arg;
            command->argCount++;

            if (ch == openingQuote)
            {
                i++;
                ch = statement.charAt(i);
            }

            if (ch == ')')
            {
                while (ch == ';' || ch == '\r')
                {
                    // Eat end of line
                    i++;
                    ch = statement.charAt(i);
                }
            }

            argStartIndex = i + 1;
            openingQuote = 0;
        }
    }

    for (int i = 0; i < command->argCount; i++)
    {
        String arg = command->args[i];
    }

    return command;
}

void execute(Command *command, AsyncWebServerRequest *request)
{
    if (command->func == "move")
    {
        // void move(char wheel, char direction, float speed)
        char wheel = command->args[0][0];
        char direction = command->args[1][0];
        float speed = atof(command->args[2].c_str());
        move(wheel, direction, speed, request);
    }
    else if (command->func == "spin")
    {
        // void spin(char direction, float speed)
        char direction = command->args[0][0];
        float speed = atof(command->args[1].c_str());
        spin(direction, speed, request);
    }
    else if (command->func == "stop")
    {
        //stop(char wheel)
        char wheel = command->args[0][0];
        stop(wheel, request);
    }
    else if (command->func == "delay" || command->func == "sleep" || command->func == "wait")
    {
        int ms = atol(command->args[0].c_str());
        delay(ms);
    }
    else if (command->func == "jump")
    {
        jump(request);
    }
    else
    {
        stop('A', NULL);
        String errorMsg = "Bad Request: No function named '" + command->func + "'";
        Serial.println(errorMsg);
        if (request != NULL)
        {
            request->send(503, "text/plain", errorMsg);
        }
    }

    if (command->nextCommand != NULL)
    {
        execute(command->nextCommand, request);
    }
    else if (request != NULL)
    {
        request->send(200);
    }
}

void handleNotFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void handleRoot(AsyncWebServerRequest *request)
{
    Serial.println("handleRoot()");
    if (LittleFS.exists("wificar.html"))
    {
        File file = LittleFS.open("wificar.html", "r");
        String html = file.readString();
        file.close();
        request->send(200, "text/html", html);
    }
    else
    {
        handleNotFound(request);
    }
}

void handleExecute(AsyncWebServerRequest *request)
{
    Serial.println("handleExecute()");
    if (!request->hasArg("statements"))
    {
        request->send(503, "text/plain", "Missing argument: statements");
    }

    String statements = request->arg("statements");
    Serial.printf("Execute: \"%s\"\n", statements.c_str());
    Command *command = interpret(statements);
    execute(command, request);
}

void handleMove(AsyncWebServerRequest *request)
{
    Serial.println("handleMove()");

    if (!request->hasArg("wheel") || !request->hasArg("direction") || !request->hasArg("speed"))
    {
        request->send(503);
        return;
    }

    Serial.printf("handleMove('%s', '%s', %s)\n", request->arg("wheel").c_str(), request->arg("direction").c_str(), request->arg("speed").c_str());
    move(request->arg("wheel")[0], request->arg("direction")[0], atof(request->arg("speed").c_str()), request);
}

void handleSpin(AsyncWebServerRequest *request)
{
    Serial.println("handleSpin()");
    if (!request->hasArg("direction") || !request->hasArg("speed"))
    {
        request->send(503);
        return;
    }

    spin(request->arg("direction")[0], request->arg("speed").toFloat(), request);
}

void handleStop(AsyncWebServerRequest *request)
{
    Serial.println("handleStop()");
    if (!request->hasArg("wheel"))
    {
        request->send(503);
        return;
    }

    stop(request->arg("wheel")[0], request);
}

void setupWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin("Not Your Network", "I love cheese 1234567890 ABCDEFG");
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/execute", HTTP_POST, handleExecute);
    server.on("/move", HTTP_POST, handleMove);
    server.on("/spin", HTTP_POST, handleSpin);
    server.on("/stop", HTTP_POST, handleStop);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
    Serial.begin(9600);
    delay(10);
    analogWriteFreq(500);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    pinMode(RIGHT_BACKWARD, OUTPUT);
    pinMode(RIGHT_FORWARD, OUTPUT);
    pinMode(LEFT_BACKWARD, OUTPUT);
    pinMode(LEFT_FORWARD, OUTPUT);

    LittleFS.begin();
    setupWiFi();

    Serial.println("");
}

void loop()
{
}
