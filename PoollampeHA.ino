#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>
Preferences prefs;

String macAddr = WiFi.macAddress();

int wlanmode;  // 1=AP-MODE 2=STA
String apssid = "";     // ACCESSPOINT SSID
String appsk = "";      // ACCESSPOINT PSK
int apchannel;          // CH FÃR ACCESSPOINT
String apip = "";       // ACCESSPOINT IP-ADRESSE DES DISPLAY
String apnetmask = "";    // NETZWERKMASKE FÃR ACCESSPOINT 
String apgw = "";         // ACCESSPOINT GW-ADRESSE DES DISPLAY
String stassid = "";      // ROUTER VERBINDEN SSID
String stapsk = "";       // ROUTER VERBINDEN PSK
//int failedconnect;        // FEHLVERSUCHE BEIM CONNECT
int WlanDuration = 10;  // Intervall DHT LESEN 10 Sekunden

String mqtt_broker = "";
String poolLampPrefix = "";
String mqtt_username = "";
String mqtt_password = "";
int mqtt_port = 0;
bool mqtt_active = false;    // MQTT AN/AUS
bool mqtt_ha = false;       // HA auto Discover
bool mqtt_retain = false;

unsigned long LZjahr = 0;     // LAUFZEIT JAHR FÃR WEBIF LAUFZEIT
unsigned long LZtag = 0;      // TAGE
unsigned long LZstd = 0;      // STUNDEN
unsigned long LZmin = 0;      // MINUTEN
unsigned long LZsek = 0;      // SEKUNDEN
int aktfarbeaussen = 0;
int aktfarbeinnen = 0;
int sollfarbeaussen = 0;
int sollfarbeinnen = 0;
int startinnen = 0;
int startaussen = 0;
int anzahldrucks1 = 0;
int anzahldrucks2 = 0;

String incolor[4] = {"", "", "", ""};
String outcolor[7] = {"", "", "", "", "", "", ""};


bool debugausgabe = true; // Globale Variable fÃ¼r den Debugging-Status SOWIE DEBUG AUSGABE DEFINIEREN
#define DEBUG_S(...) if (debugausgabe) { Serial.print(__VA_ARGS__); }
#define DEBUG_B(...) if (debugausgabe) { Serial.begin(__VA_ARGS__); }
#define DEBUG_P(...) if (debugausgabe) { Serial.println(__VA_ARGS__); }
#define DEBUG_F(...) if (debugausgabe) { Serial.printf(__VA_ARGS__); }


int buttonS1 = D1;    // the number of the pushbutton pin
int buttonS2 = D2;    // the number of the pushbutton pin
int defaultS1 = D1;
int defaultS2 = D2;
unsigned long lastDebounceTimeS1 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTimeS2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
unsigned long longPressTime = 3000;
unsigned long buttonTimerS1 = 0;
unsigned long buttonTimerS2 = 0;
boolean buttonActiveS1 = false;
boolean longPressActiveS1 = false;
boolean buttonActiveS2 = false;
boolean longPressActiveS2 = false;
int lastButtonStateS1 = LOW;   // the previous reading from the input pin
int lastButtonStateS2 = LOW;   // the previous reading from the input pin


const char* getBuildDate() {      //BUILDDATE GENERERIEREN (FÃR VERSION IM WEBIF)
  const char* monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* date = __DATE__;
  static char buildDate[9]; // "VYYYYMMDD" plus Null-Terminierung

  // Monat analysieren
  char month[4];
  strncpy(month, date, 3);
  month[3] = '\0';
  int monthNum = 0;
  for(int i = 0; i < 12; ++i) {
    if(strcmp(month, monthNames[i]) == 0) {
      monthNum = i + 1;
      break;
    }
  }

  // Jahr, Tag und Monat extrahieren und in das gewÃ¼nschte Format zusammenstellen
  int day, year;
  sscanf(date, "%s %d %d", month, &day, &year);
  sprintf(buildDate, "v%d%02d%02d", year, monthNum, day);
  
  return buildDate;
}

const char* build = getBuildDate();     // BUILD VERSION IN VARIABELE SPEICHERN




// ########################
// ### DS18B20 SENSOREN ###
// ########################
int DS18B20Pin = D6;
int defaultDS18B20Pin = D6;
OneWire oneWire(defaultDS18B20Pin);      // ONEWIRE
DallasTemperature DS18B20(&oneWire);  // DS18B20
DynamicJsonDocument sensorData(4096); // JSON MIT SENSOREN
bool jsonnewSensor = false;       // NEUER SENSOR BEIM START GEFUNDEN ?
float temp = 0.0;                 // VARIABLE FÃR TEMPERATURWERT Â°C
int DS18B20Duration = 10;  // Intervall DHT LESEN 10 Sekunden
int numDevices = 0;                 // ANZAHL DER SENSOREN


unsigned long lastUpdateAt = 0;
int counta = 0;   //ZÃHLER TEMPERATUR DURATION
int countb = 0;   //ZÃHLER WLAN DURATION

WiFiClient client;
PubSubClient mqtt_client(client);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

StaticJsonDocument<2048> sensorWerte;




void handleOTAUpdate()
{
    if (!LittleFS.begin())
    {
        DEBUG_P("LittleFS initialization failed!");
        return;
    }

    httpUpdater.setup(&server);
}

void setup()
{
  pinMode(buttonS1, INPUT_PULLUP);
  pinMode(buttonS2, INPUT_PULLUP);

  Serial.begin(115200);
  setupFS();

  //LESE SETTINGS
  prefs.begin("IntexPoollampe");
  debugausgabe = prefs.getBool("debug", true);
  wlanmode = prefs.getInt("wlanmode", 1);
  if (wlanmode < 0 || wlanmode > 2) { wlanmode = 1; }
  apssid = prefs.getString("apssid", "Poollampe");
  appsk = prefs.getString("appsk", "12345678");
  apchannel = prefs.getInt("apchannel", 6);
  apip = prefs.getString("apip", "192.168.4.1");
  apnetmask = prefs.getString("apnetmask", "255.255.255.0");
  apgw = prefs.getString("apgw", "192.168.4.1");
  stassid = prefs.getString("stassid", "");
  stapsk = prefs.getString("stapsk", "");
  if (stassid == "" && wlanmode == 2) { wlanmode = 1; }

  mqtt_broker = prefs.getString("mqtt_broker", "192.168.2.110");
  poolLampPrefix = prefs.getString("mqttprefix", "Poollampe");
  mqtt_username = prefs.getString("mqtt_username", "Mqtt");
  mqtt_password = prefs.getString("mqtt_password", "Coam24611");
  mqtt_port = prefs.getInt("mqtt_port", 1883);
  mqtt_active = prefs.getBool("mqttactive", true);
  mqtt_ha = prefs.getBool("mqttha", true);
  mqtt_retain = prefs.getBool("mqttretain", false);
  incolor[0] = prefs.getString("incolor1", "Aus");
  incolor[1] = prefs.getString("incolor2", "Weiss");
  incolor[2] = prefs.getString("incolor3", "Aus");
  incolor[3] = prefs.getString("incolor4", "Farbwechsel");
  outcolor[0] = prefs.getString("outcolor1", "Aus");
  outcolor[1] = prefs.getString("outcolor2", "Weiss");
  outcolor[2] = prefs.getString("outcolor3", "GrÃ¼n");
  outcolor[3] = prefs.getString("outcolor4", "GrÃ¼nblau");
  outcolor[4] = prefs.getString("outcolor5", "Blau");
  outcolor[5] = prefs.getString("outcolor6", "Violett");
  outcolor[6] = prefs.getString("outcolor7", "Farbwechsel");
  startinnen = prefs.getInt("startinnen", 0);
  startaussen = prefs.getInt("startaussen", 0);

  buttonS1 = prefs.getInt("buttonS1", defaultS1);
  buttonS2 = prefs.getInt("buttonS2", defaultS2);
  DS18B20Pin = prefs.getInt("DS18B20Pin", defaultDS18B20Pin);
  DS18B20Duration = prefs.getInt("DS18B20Duration", 10);
  WlanDuration = prefs.getInt("WlanDuration", 30);

  oneWire = OneWire(DS18B20Pin);



  DEBUG_P("Einstellungen geladen...");
  DEBUG_S(F("preferences free:"));
  DEBUG_P(prefs.freeEntries());

  sollfarbeaussen = startaussen;
  sollfarbeinnen = startinnen;

unsigned long pressStartTime = 0;
bool buttonsPressed = false;

// ÃberprÃ¼fe, ob beide Taster gedrÃ¼ckt sind
if (digitalRead(buttonS1) == LOW && digitalRead(buttonS2) == LOW) {
  pressStartTime = millis();
  buttonsPressed = true;
  Serial.println ("Beide Tasten gedrÃ¼ckt...");
}

if (buttonsPressed) {
  while (buttonsPressed) {
    // ÃberprÃ¼fe, ob beide Taster immer noch gedrÃ¼ckt sind
    if (digitalRead(buttonS1) == LOW && digitalRead(buttonS2) == LOW) {
      // Wenn 5 Sekunden vergangen sind
      if (millis() - pressStartTime >= 2000) {
        Serial.println("Beide Taster wurden fÃ¼r 2 Sekunden gedrÃ¼ckt beim Start.");
        wlanmode = 1;
        apssid = "Poollampe";
        appsk = "12345678";
        apchannel = 6;
        apip = "192.168.4.1";
        apnetmask = "255.255.255.0";
        apgw = "192.168.4.1";
        prefs.putInt("wlanmode", wlanmode);
        prefs.putString("apssid", apssid);
        prefs.putString("appsk", appsk);
        prefs.putInt("apchannel", apchannel);
        prefs.putString("apip", apip);
        prefs.putString("apnetmask", apnetmask);
        prefs.putString("apgw", apgw);
        break;
      }
    } else {
      // Wenn einer der Taster losgelassen wird, breche die ÃberprÃ¼fung ab
      Serial.println("Nicht beide Taster fÃ¼r 2 Sekunden gedrÃ¼ckt beim Start.");
      buttonsPressed = false;
    }
  }
}


  DS18B20.begin();
//  DS18B20.requestTemperatures();
//  numDevices = DS18B20.getDeviceCount();
//  DEBUG_S("Anzahl der Sensoren: ");
//  DEBUG_P(numDevices);
  
  searchAndSaveSensorData();
  connectWifi();
  if (mqtt_active) {
    mqtt_client.setServer(mqtt_broker.c_str(), mqtt_port);
    mqtt_client.setBufferSize(1024);
    mqtt_client.setCallback(mqttCallback);
    connectToMQTTBroker();
  }




  handlewebseiten();
  handleOTAUpdate(); // Handle OTA update
  server.begin();
  readSensors();
}




void loop()
{
  leseS1();
  leseS2();

  if (aktfarbeaussen != sollfarbeaussen) { changecolorS2(); }
  if (aktfarbeinnen != sollfarbeinnen) { changecolorS1(); }
  if (!mqtt_client.connected() && mqtt_active) { connectToMQTTBroker(); }
  if (mqtt_active) { mqtt_client.loop(); }
  
  server.handleClient();

  if ((millis() - lastUpdateAt) > 1000) {
    lastUpdateAt = millis();
    counta++;
    countb++;
    LZsek++;

    if (LZsek == 60) {
      LZsek = 0;
      LZmin++;
    }
    if (LZmin == 60) {
      LZmin = 0;
      LZstd++;
    }
    if (LZstd == 24) {
      LZstd = 0;
      LZtag++;
    }
    if (LZtag == 366) {
      LZtag = 0;
      LZjahr++;
    }
  }

    if (counta >= DS18B20Duration)
    {
        counta = 0;
        readSensors();
    }

    if (countb >= WlanDuration)
    {
        countb = 0;
        if (mqtt_active) { publishWlanDiag(); }
    }

}
