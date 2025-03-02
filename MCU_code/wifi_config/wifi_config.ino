#include <WiFiS3.h>

WiFiServer server(80);
//AP 
const char *apSSID = "WiFi_Config";
const char *apPassword = "";

void setup() {
    Serial.begin(115200);
    WiFi.beginAP(apSSID, apPassword);
    Serial.println("AP mode started, SSID: " + String(apSSID));
    Serial.println("Connect to this WiFi and go to 192.168.4.1 in your browser.");
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("Client connected");
        String request = "";
        while (client.available()) {
            char c = client.read();
            request += c;
        }

        if (request.indexOf("GET / ") >= 0) {
            sendWiFiForm(client);
        } else if (request.indexOf("GET /config?ssid=") >= 0) {
            handleWiFiConfig(client, request);
        }

        client.stop();
    }
}

String getWiFiList() {
    String wifiOptions = "";
    int numNetworks = WiFi.scanNetworks();
    String seenSSIDs[numNetworks];
    int seenCount = 0;

    for (int i = 0; i < numNetworks; i++) {
        String ssid = WiFi.SSID(i);
        //remove the same name wifi(adviod the ap route)
        bool isDuplicate = false;

        for (int j = 0; j < seenCount; j++) {
            if (seenSSIDs[j] == ssid) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate && ssid.length() > 0) {
            seenSSIDs[seenCount++] = ssid;
            wifiOptions += "<option value='" + ssid + "'>" + ssid + "</option>";
        }
    }
    return wifiOptions;
}

void sendWiFiForm(WiFiClient &client) {
    String wifiOptions = getWiFiList();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>");
    client.println("<h1>WiFi Configuration</h1>");
    client.println("<form action='/config' method='GET'>");
    client.println("Select WiFi Network: <select name='ssid'>" + wifiOptions + "</select><br>");
    client.println("WiFi Password: <input type='text' name='password'><br>");
    client.println("<input type='submit' value='Connect WiFi'>");
    client.println("</form>");
    client.println("</body></html>");
}

//select the wifi and blank the password
void handleWiFiConfig(WiFiClient &client, String request) {
    int ssidStart = request.indexOf("ssid=") + 5;
    int ssidEnd = request.indexOf("&", ssidStart);
    String ssid = request.substring(ssidStart, ssidEnd);

    int passStart = request.indexOf("password=") + 9;
    int passEnd = request.indexOf(" ", passStart);
    String password = request.substring(passStart, passEnd);

    Serial.println("Selected WiFi SSID: " + ssid);
    Serial.println("Entered WiFi Password: " + password);
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<html><head><meta charset='utf-8'><title>WiFi Setup</title></head><body>");
    client.println("<h1>WiFi Setup Successful!</h1>");
    client.println("<p>Connecting to: " + ssid + "</p>");
    client.println("</body></html>");

    delay(1000);
    WiFi.end();
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.println("Connecting to WiFi...");
    delay(5000);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi Connection Failed! Please try again.");
    }
}
