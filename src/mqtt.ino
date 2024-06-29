void connectToMQTTBroker() {
    static unsigned long lastAttemptTime = 0; // Zeitstempel der letzten Verbindungsversuchs
    static int mqtt_errorcount = 0; // FehlerzÃ¤hler fÃ¼r MQTT-Verbindungen

    if (mqtt_client.connected()) {
        return;
    }

    unsigned long currentTime = millis();

    // Versuche nur alle 5 Sekunden eine Verbindung herzustellen
    if (currentTime - lastAttemptTime >= 5000 || lastAttemptTime == 0) {
        lastAttemptTime = currentTime;

        String client_id = "esp8266-poollampe-" + String(WiFi.macAddress());
        DEBUG_F("Connecting to MQTT Broker as %s.....\n", client_id.c_str());
        
        if (mqtt_client.connect(client_id.c_str(), mqtt_username.c_str(), mqtt_password.c_str(), 
            (poolLampPrefix + "/" + macAddr + "/status/stat_t").c_str(), 0, true, "Offline")) {
            DEBUG_P("Connected to MQTT broker");
            mqtt_client.publish((poolLampPrefix + "/" + macAddr + "/status/stat_t").c_str(), "Online", mqtt_retain); // Retained message to set status to "Online"
            mqttinit();
        } else {
            DEBUG_S("Failed to connect to MQTT broker, rc=");
            DEBUG_S(mqtt_client.state());
            DEBUG_P(" try again in 5 seconds");
        }
    }

    yield(); // ErmÃ¶glicht dem ESP, andere Aufgaben auszufÃ¼hren
}


void mqttinit() {
  if (mqtt_ha && mqtt_active) {
    sendHAdiscoverySelectInnen();
    sendHAdiscoverySelectAussen();
    sendHAdiscoverySensors();
    sendHAdiscoveryDiagIP();
    sendHAdiscoveryDiagRSSI();
    sendHAdiscoveryDiagMAC();
    sendHAdiscoveryDiagStatus();
  }

  if (mqtt_active) {
    publishWlanDiag();
    sendInitSelectInnen();
    sendInitSelectAussen();
  }
}

void sendHAdiscoveryDiagIP() {
  String macAddr = WiFi.macAddress();
  macAddr.replace(":", ""); // Remove colons from the MAC address
  String uniqId = poolLampPrefix + "_" + macAddr + "_IP";
  String stat = poolLampPrefix + "/" + macAddr + "/ip/stat_t";
  String webif = "http://" + WiFi.localIP().toString() + "/";
  String topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/ip/config";


  DynamicJsonDocument doc(1024);
  doc["name"] = "IP Adresse";
  doc["uniq_id"] = uniqId.c_str();
  doc["ent_id"] = uniqId.c_str();
  doc["ent_cat"] = "diagnostic";
  doc["ic"] = "mdi:wifi";
  doc["stat_t"] = stat.c_str();
  JsonObject dev = doc.createNestedObject("dev");
  dev["ids"] = macAddr.c_str();
  dev["name"] = "Intex Poollampe";
  dev["sw"] = "1.1";
  dev["mf"] = "Intex";
  dev["mdl"] = "28698";
  dev["cu"] = webif.c_str();

  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  char topicCharArray[topic.length() + 1];
  topic.toCharArray(topicCharArray, topic.length() + 1);

  if (mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n, mqtt_retain)) {
    String baseIP = WiFi.localIP().toString();
    publishMessage(stat.c_str(), baseIP.c_str());
  }
}

void sendHAdiscoveryDiagRSSI() {
  String macAddr = WiFi.macAddress();
  macAddr.replace(":", ""); // Remove colons from the MAC address
  String uniqId = poolLampPrefix + "_" + macAddr + "_RSSI";
  String stat = poolLampPrefix + "/" + macAddr + "/rssi/stat_t";
  String webif = "http://" + WiFi.localIP().toString() + "/";
  String topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/rssi/config";
  char payload[10];
  snprintf(payload, sizeof(payload), "%d", WiFi.RSSI());

  DynamicJsonDocument doc(1024);
  doc["name"] = "WiFi-RSSI";
  doc["uniq_id"] = uniqId.c_str();
  doc["ent_id"] = uniqId.c_str();
  doc["ent_cat"] = "diagnostic";
  doc["ic"] = "mdi:wifi";
  doc["dev_cla"]   = "signal_strength";
  doc["unit_of_meas"] = "dBm";
  doc["stat_t"] = stat.c_str();
  JsonObject dev = doc.createNestedObject("dev");
  dev["ids"] = macAddr.c_str();
  dev["name"] = "Intex Poollampe";
  dev["sw"] = "1.1";
  dev["mf"] = "Intex";
  dev["mdl"] = "28698";
  dev["cu"] = webif.c_str();

  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  char topicCharArray[topic.length() + 1];
  topic.toCharArray(topicCharArray, topic.length() + 1);

  if (mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n),mqtt_retain) {
    publishMessage(stat.c_str(), payload);
  }
}


void sendHAdiscoveryDiagMAC() {
  String macAddr = WiFi.macAddress();
  macAddr.replace(":", ""); // Remove colons from the MAC address
  String uniqId = poolLampPrefix + "_" + macAddr + "_MAC";
  String stat = poolLampPrefix + "/" + macAddr + "/mac/stat_t";
  String webif = "http://" + WiFi.localIP().toString() + "/";
  String topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/mac/config";
  char payload[10];
  snprintf(payload, sizeof(payload), "%d", WiFi.RSSI());

  DynamicJsonDocument doc(1024);
  doc["name"] = "WiFi-MAC";
  doc["uniq_id"] = uniqId.c_str();
  doc["ent_id"] = uniqId.c_str();
  doc["ent_cat"] = "diagnostic";
  doc["ic"] = "mdi:wifi";
  doc["stat_t"] = stat.c_str();
  JsonObject dev = doc.createNestedObject("dev");
  dev["ids"] = macAddr.c_str();
  dev["name"] = "Intex Poollampe";
  dev["sw"] = "1.1";
  dev["mf"] = "Intex";
  dev["mdl"] = "28698";
  dev["cu"] = webif.c_str();

  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  char topicCharArray[topic.length() + 1];
  topic.toCharArray(topicCharArray, topic.length() + 1);

  if (mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n)) {
    publishMessage(stat.c_str(), macAddr.c_str());
  }
}

void sendHAdiscoveryDiagStatus() {
  String macAddr = WiFi.macAddress();
  macAddr.replace(":", ""); // Remove colons from the MAC address
  String uniqId = poolLampPrefix + "_" + macAddr + "_Status";
  String stat = poolLampPrefix + "/" + macAddr + "/status/stat_t";
  String webif = "http://" + WiFi.localIP().toString() + "/";
  String topic = "homeassistant/binary_sensor/" + poolLampPrefix + "_" + macAddr + "/status/config";

  DynamicJsonDocument doc(1024);
  doc["name"] = "Status";
  doc["uniq_id"] = uniqId.c_str();
  doc["ent_id"] = uniqId.c_str();
  doc["ent_cat"] = "diagnostic";
  doc["pl_on"] = "Online";
  doc["pl_off"] = "Offline";
  doc["dev_cla"] = "connectivity";
  doc["stat_t"] = stat.c_str();
  JsonObject dev = doc.createNestedObject("dev");
  dev["ids"] = macAddr.c_str();
  dev["name"] = "Intex Poollampe";
  dev["sw"] = "1.1";
  dev["mf"] = "Intex";
  dev["mdl"] = "28698";
  dev["cu"] = webif.c_str();

  char buffer[512];
  size_t n = serializeJson(doc, buffer);

  char topicCharArray[topic.length() + 1];
  topic.toCharArray(topicCharArray, topic.length() + 1);

  if (mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n, mqtt_retain)) {
    mqtt_client.publish(stat.c_str(), "Online", mqtt_retain);
  }
}



void sendHAdiscoverySelectInnen() {
    String uniqId = poolLampPrefix + "_" + macAddr + "_FarbeInnen";
    String stat = poolLampPrefix + "/" + macAddr + "/innen/stat_t";
    String cmdt = poolLampPrefix + "/" + macAddr + "/innen/cmd_t";
    String webif = "http://" + WiFi.localIP().toString() + "/";
    String topic = "homeassistant/select/" + poolLampPrefix + "_" + macAddr + "/innen/config";

    char buffer[512];
    StaticJsonDocument<512> doc;
    doc["name"] = "Farbe Innen";
    doc["uniq_id"] = uniqId.c_str();
    doc["ent_id"] = uniqId.c_str();
    doc["ic"] = "mdi:lightbulb";

    // Manuell eindeutige Farben prÃ¼fen und hinzufÃ¼gen
    JsonArray options = doc.createNestedArray("options");
    String uniqueColors[4];  // Annahme: incolor hat maximal 4 Elemente

    int index = 0;
    for (int i = 0; i < 4; ++i) {
        bool isDuplicate = false;
        for (int j = 0; j < index; ++j) {
            if (incolor[i] == uniqueColors[j]) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            uniqueColors[index++] = incolor[i];
            options.add(incolor[i]);
        }
    }

    JsonObject dev = doc.createNestedObject("dev");
    dev["ids"] = macAddr.c_str();
    dev["name"] = "Intex Poollampe";
    dev["sw"] = "1.0.0";
    dev["mf"] = "Intex";
    dev["mdl"] = "28698";
    dev["cu"] = webif.c_str();
    doc["stat_t"] = stat.c_str();
    doc["cmd_t"] = cmdt.c_str();

    serializeJson(doc, buffer);
    size_t n = serializeJson(doc, buffer);

    char topicCharArray[topic.length() + 1];
    topic.toCharArray(topicCharArray, topic.length() + 1);
    
    mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n, mqtt_retain);

}


void sendHAdiscoverySelectAussen() {
    String uniqId = poolLampPrefix + "_" + macAddr + "_FarbeAussen";
    String stat = poolLampPrefix + "/" + macAddr + "/aussen/stat_t";
    String cmdt = poolLampPrefix + "/" + macAddr + "/aussen/cmd_t";
    String webif = "http://" + WiFi.localIP().toString() + "/";
    String topic = "homeassistant/select/" + poolLampPrefix + "_" + macAddr + "/aussen/config";

    char buffer[512];
    StaticJsonDocument<512> doc;
    doc["name"] = "Farbe Aussen";
    doc["uniq_id"] = uniqId.c_str();
    doc["ent_id"] = uniqId.c_str();
    doc["ic"] = "mdi:lightbulb";

    // Manuell eindeutige Farben prÃ¼fen und hinzufÃ¼gen
    JsonArray options = doc.createNestedArray("options");
    String uniqueColors[7];  // Annahme: outcolor hat maximal 7 Elemente

    int index = 0;
    for (int i = 0; i < 7; ++i) {
        bool isDuplicate = false;
        for (int j = 0; j < index; ++j) {
            if (outcolor[i] == uniqueColors[j]) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            uniqueColors[index++] = outcolor[i];
            options.add(outcolor[i]);
        }
    }

    JsonObject dev = doc.createNestedObject("dev");
    dev["ids"] = macAddr.c_str();
    dev["name"] = "Intex Poollampe";
    dev["sw"] = "1.0.0";
    dev["mf"] = "Intex";
    dev["mdl"] = "28698";
    dev["cu"] = webif.c_str();
    doc["stat_t"] = stat.c_str();
    doc["cmd_t"] = cmdt.c_str();

    serializeJson(doc, buffer);
    size_t n = serializeJson(doc, buffer);

    char topicCharArray[topic.length() + 1];
    topic.toCharArray(topicCharArray, topic.length() + 1);
    
    mqtt_client.publish(topicCharArray, (uint8_t*)buffer, n, mqtt_retain);
}


void sendHAdiscoverySensors() {
  char tempString[8]; // FÃ¼r die Konvertierung der Temperatur in String
  int i = 1; // ZÃ¤hler fÃ¼r die Anzahl der Sensoren
  String webif = "http://" + WiFi.localIP().toString() + "/"; // Webinterface URL

  DEBUG_P("Lese sensor_data.json.");

  // Laden der JSON-Datei
  File file = LittleFS.open("/sensor_data.json", "r");
  if (!file) {
    DEBUG_P("Fehler beim Ãffnen der Datei sensor_data.json");
    return;
  }

  // Lese den Inhalt der JSON-Datei
  String jsonContent;
  while (file.available()) {
    jsonContent += (char)file.read();
  }
  file.close();

  // Erstellen eines JSON-Dokuments
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, jsonContent);
  if (error) {
    DEBUG_P("Fehler beim Parsen der JSON-Daten");
    return;
  }

  // Durchlaufe alle Paare im JSON-Objekt
  for (JsonPair sensorEntry : doc.as<JsonObject>()) {
    // TemporÃ¤re Zeichenfolge fÃ¼r die Sensor-ID
    String sensor_id = sensorEntry.key().c_str();

    JsonObject sensor = sensorEntry.value().as<JsonObject>();
    String sensor_description = sensor["description"].as<String>();
    float offset = sensor["offset"].as<float>();

    // Sensoradresse fÃ¼r DS18B20 konvertieren
    DeviceAddress sensor1;
    stringToSensorAddress(sensor_id.c_str(), sensor1);

    // Temperatur abfragen
    DS18B20.requestTemperatures();
    float temperature = DS18B20.getTempC(sensor1);
    float tempberechnet = temperature + offset;

    // MAC-Adresse bereinigen (Doppelpunkte und andere Zeichen entfernen)
    removeNonAlphaNumeric(sensor_id);

    // MQTT-Topics und eindeutige IDs erstellen
    String uniqId = poolLampPrefix + "_Sensor_" + sensor_id + "_temp";
    String stat = poolLampPrefix + "/" + macAddr + "/" + sensor_id + "/temp";
    String topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/" + sensor_id +  "_temp/config";

    // JSON-Dokument fÃ¼r die Konfigurationsnachricht erstellen
    StaticJsonDocument<512> docConfig;
    docConfig["name"] = sensor_description;
    docConfig["uniq_id"] = uniqId;
    docConfig["ent_id"] = uniqId;
    docConfig["dev_cla"] = "temperature";
    docConfig["unit_of_meas"] = "Â°C";
    docConfig["ic"] = "mdi:thermometer";
    JsonObject devConfig = docConfig.createNestedObject("dev");
    devConfig["ids"] = macAddr;
    devConfig["name"] = "Intex Poollampe";
    devConfig["sw"] = "1.0.0";
    devConfig["mf"] = "Intex";
    devConfig["mdl"] = "28698";
    devConfig["cu"] = webif;
    docConfig["stat_t"] = stat;

    // JSON-Dokument in einen String konvertieren
    char bufferConfig[512];
    size_t nConfig = serializeJson(docConfig, bufferConfig);

    // MQTT-Topic und Nachricht verÃ¶ffentlichen
    if (mqtt_client.publish(topic.c_str(), (uint8_t*)bufferConfig, nConfig, mqtt_retain)) {
      dtostrf(tempberechnet, 6, 2, tempString);
      publishMessage(stat.c_str(), tempString);
    }

    // Eindeutige ID fÃ¼r die UID-Konfiguration erstellen
    uniqId = poolLampPrefix + "_Sensor_" + sensor_id + "_uid";
    stat = poolLampPrefix + "/" + macAddr + "/" + sensor_id + "/uid";
    topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/" + sensor_id +  "_uid/config";
    String nameUid = sensor_description + "_uid";

    // JSON-Dokument fÃ¼r die UID-Konfigurationsnachricht erstellen
    StaticJsonDocument<512> docUid;
    docUid["name"] = nameUid;
    docUid["uniq_id"] = uniqId;
    docUid["ent_id"] = uniqId;
    docUid["ic"] = "mdi:thermometer";
    JsonObject devUid = docUid.createNestedObject("dev");
    devUid["ids"] = macAddr;
    devUid["name"] = "Intex Poollampe";
    devUid["sw"] = "1.0.0";
    devUid["mf"] = "Intex";
    devUid["mdl"] = "28698";
    devUid["cu"] = webif;
    docUid["stat_t"] = stat;

    // JSON-Dokument in einen String konvertieren
    char bufferUid[512];
    size_t nUid = serializeJson(docUid, bufferUid);

    // MQTT-Topic und Nachricht verÃ¶ffentlichen
    if (mqtt_client.publish(topic.c_str(), (uint8_t*)bufferUid, nUid, mqtt_retain)) {
      publishMessage(stat.c_str(), sensor_id.c_str());
    }

    // ZÃ¤hler erhÃ¶hen und ggf. Schleife beenden
    i++;
    if (i == 20) { break; } // Nur die ersten 20 Sensoren verarbeiten
  }
}


void sendInitSelectInnen() {
    String stat = poolLampPrefix + "/" + macAddr + "/innen/stat_t";
    String cmdt = poolLampPrefix + "/" + macAddr + "/innen/cmd_t";
    publishMessage(stat.c_str(), incolor[sollfarbeinnen].c_str());
    publishMessage(cmdt.c_str(), incolor[sollfarbeinnen].c_str());
    mqtt_client.subscribe(cmdt.c_str());
}

void sendInitSelectAussen() {
    String stat = poolLampPrefix + "/" + macAddr + "/aussen/stat_t";
    String cmdt = poolLampPrefix + "/" + macAddr + "/aussen/cmd_t";
    publishMessage(stat.c_str(), outcolor[sollfarbeaussen].c_str());
    publishMessage(cmdt.c_str(), outcolor[sollfarbeaussen].c_str());
    mqtt_client.subscribe(cmdt.c_str());
}



void publishWlanDiag() {
  // PUBLISH IP-ADRESSE
  String baseIP = WiFi.localIP().toString();
  String stat = poolLampPrefix + "/" + macAddr + "/ip/stat_t";
  publishMessage(stat.c_str(), baseIP.c_str());
  
  // PUBLISH RSSI-ADRESSE
  stat = poolLampPrefix + "/" + macAddr + "/rssi/stat_t";
  char payload[10];
  snprintf(payload, sizeof(payload), "%d", WiFi.RSSI());
  publishMessage(stat.c_str(), payload);

  // PUBLISH MAC-ADRESSE
  String macAddrfull = WiFi.macAddress();
  stat = poolLampPrefix + "/" + macAddr + "/mac/stat_t";
  publishMessage(stat.c_str(), macAddrfull.c_str());
}


bool publishMessage(const char* topic, const char* payload) {
  if (!mqtt_client.connected()) {
    connectToMQTTBroker();
  }
  if (mqtt_client.publish(topic, (uint8_t*)payload, strlen(payload), mqtt_retain)) {
    DEBUG_P("Message published successfully");
    return true;
  } else {
    DEBUG_P("Message publication failed");
    return false;
  }
}




void mqttCallback(char* topic, byte* payload, unsigned int length) {
  DEBUG_S("Message arrived [");
  DEBUG_S(topic);
  DEBUG_S("] ");
  String payloadStr = "";
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
    DEBUG_S((char)payload[i]);
  }
  DEBUG_P();


  String topicStr = String(topic);
  if (topicStr.indexOf("/aussen/cmd_t") != -1) {
    // FARBE AUSSEN GEÃNDERT....
    int colorIndex = processColorChange(payloadStr, outcolor, 7);
    if (colorIndex > -1 && colorIndex < 7) {
      if (sollfarbeaussen != colorIndex) {
        sollfarbeaussen = colorIndex;
        DEBUG_S ("Neue Sollfarbe aussen per MQTT : ")
        DEBUG_P (sollfarbeaussen)
      }
    }
  }


  if (topicStr.indexOf("/innen/cmd_t") != -1) {
    // FARBE INNEN GEÃNDERT....
    int colorIndex = processColorChange(payloadStr, incolor, 4);
    if (colorIndex > -1 && colorIndex < 4) {
      if (sollfarbeinnen != colorIndex) {
        sollfarbeinnen = colorIndex;
        DEBUG_S ("Neue Sollfarbe innen per MQTT : ")
        DEBUG_P (sollfarbeinnen)
      }
    }
  }

}


int processColorChange(String color, String colors[], int num) {
  int foundIndex = -1;
  for (int i = 0; i < num; i++) {
    if (color.equals(colors[i])) {
      foundIndex = i;
      break;
    }
  }
  return foundIndex;
}