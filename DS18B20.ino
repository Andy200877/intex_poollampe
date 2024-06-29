#include "arduino_secrets.h"

void readSensors() {
  DEBUG_P("Lese Sensoren...");
  sensorWerte.clear();

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
  DS18B20.requestTemperatures();
  numDevices = DS18B20.getDeviceCount();
  DEBUG_S("Anzahl der Sensoren: ");
  DEBUG_P(numDevices);
  int i = 1;

  for (JsonPair sensorEntry : doc.as<JsonObject>()) {
    const char* sensor_id = sensorEntry.key().c_str();
    JsonObject sensor = sensorEntry.value().as<JsonObject>();
    float offset = sensor["offset"].as<float>();
    String beschreibung = sensor["description"].as<String>();

    // Bereinige die Sensor-ID (entferne nicht alphanumerische Zeichen)
    String sensor_id_clean = String(sensor_id);
    removeNonAlphaNumeric(sensor_id_clean);

    // Sensoradresse in OneWire-Format konvertieren
    DeviceAddress sensorAddress;
    stringToSensorAddress(sensor_id, sensorAddress);

    // Temperatur abfragen
    
    float temperature = DS18B20.getTempC(sensorAddress);

    // Temperatur mit Offset berechnen
    float calculatedTemperature = temperature + offset;

    // Temperaturwert in String konvertieren
    char tempString[8];
    dtostrf(calculatedTemperature, 6, 2, tempString);
    
    DEBUG_S("Sensor ");
    DEBUG_S(i);
    DEBUG_S("/");
    DEBUG_S(numDevices);
    DEBUG_S(" : ");
    DEBUG_S(beschreibung);
    DEBUG_S(" : ");
    DEBUG_S(tempString);
    DEBUG_P(" Â°C");


    if (mqtt_active) {
      // Temperatur Nachricht Ã¼ber MQTT verÃ¶ffentlichen
      String mqttTopic = poolLampPrefix + "/" + macAddr + "/" + sensor_id_clean + "/temp";

      if (!publishMessage(mqttTopic.c_str(), tempString)) {
        DEBUG_S("Fehler beim VerÃ¶ffentlichen auf ");
        DEBUG_P(mqttTopic);
      }

      // UID Nachricht Ã¼ber MQTT verÃ¶ffentlichen
      mqttTopic = poolLampPrefix + "/" + macAddr + "/" + sensor_id_clean + "/uid";
      if (!publishMessage(mqttTopic.c_str(), sensor_id_clean.c_str())) {
        DEBUG_S("Fehler beim VerÃ¶ffentlichen auf ");
        DEBUG_P(mqttTopic);
      }
    }

    char sensorId[17];
    snprintf(sensorId, sizeof(sensorId), "%02X%02X%02X%02X%02X%02X%02X%02X", 
        sensorAddress[0], sensorAddress[1], sensorAddress[2], sensorAddress[3],
        sensorAddress[4], sensorAddress[5], sensorAddress[6], sensorAddress[7]);
    

    JsonObject sensorWeb = sensorWerte.createNestedObject(sensorId);
    sensorWeb["description"] = beschreibung;
    sensorWeb["temperature"] = tempString;

    
    i++;
    if (i == 21) { break; } // Nur die ersten 20 Sensoren verarbeiten
  }
}




void searchAndSaveSensorData() {
  File file = LittleFS.open("/sensor_data.json", "r+");
  bool newfile = false;

  // Wenn die Datei nicht vorhanden ist, erstellen wir sie
  if (!file) {
    DEBUG_P("JSON-Sensoren nicht gefunden. Neue Datei wird erstellt.");
    file = LittleFS.open("/sensor_data.json", "w+");
    if (!file) {
      DEBUG_P("Fehler beim Erstellen der JSON-Sensoren.");
      return;
    }
    file.close();  // Datei nach dem Schreiben schlieÃen
    jsonnewSensor = true;
    newfile = true;
    saveSensorDataToFile();  // Standarddaten speichern
  } else {
    file.close();  // Datei schlieÃen, um sicherzustellen, dass sie nicht blockiert ist
  }

  // JSON-Daten aus der Datei lesen
  file = LittleFS.open("/sensor_data.json", "r");
  if (!file) {
    DEBUG_P("Fehler beim Ãffnen der JSON-Sensoren zum Lesen.");
    return;
  }
  DeserializationError errorjson = deserializeJson(sensorData, file);
  file.close();  // Datei nach dem Lesen schlieÃen
  if (errorjson) { 
    DEBUG_P("Fehler beim Lesen der JSON-Sensoren.");
    return;
  }

  // IDs der Sensoren auslesen und Ã¼berprÃ¼fen
  DS18B20.requestTemperatures();  // Temperaturwerte abfragen
  
  numDevices = DS18B20.getDeviceCount();
  DEBUG_S("Anzahl der Sensoren: ");
  DEBUG_P(numDevices);

  
  // Iteriere durch alle Sensoren in der JSON-Datei
  for (JsonPair sensorEntry : sensorData.as<JsonObject>()) {
    String sensorID = sensorEntry.key().c_str();
    bool found = false;

    // ÃberprÃ¼fen, ob der Sensor in der aktuellen Liste gefunden wird
    for (int i = 0; i < numDevices; ++i) {
      DeviceAddress sensorAddress;
      DS18B20.getAddress(sensorAddress, i);
      String currentSensorID = "";

      // Konvertiere die Sensoradresse in eine lesbare Zeichenfolge
      for (uint8_t j = 0; j < 8; j++) {
        currentSensorID += String(sensorAddress[j], HEX);
        if (j < 7) currentSensorID += ":";
      }

      if (sensorID == currentSensorID) {
        found = true;
        break;
      }
    }

    // Wenn der Sensor nicht gefunden wird, entferne ihn aus der JSON-Datei
    if (!found) {
      DEBUG_P("Sensor nicht mehr gefunden. Entferne Eintrag aus der JSON-Datei.");
      sensorData.remove(sensorID);
      jsonnewSensor = true;
    }
  }

  // HinzufÃ¼gen neuer Sensoren zur JSON-Datei
  for (int i = 0; i < numDevices; ++i) {
    DeviceAddress sensorAddress;
    DS18B20.getAddress(sensorAddress, i);
    String sensorID = "";

    // Konvertiere die Sensoradresse in eine lesbare Zeichenfolge
    for (uint8_t j = 0; j < 8; j++) {
      sensorID += String(sensorAddress[j], HEX);
      if (j < 7) sensorID += ":";
    }


    // ÃberprÃ¼fen, ob der Sensor bereits in der JSON-Datei vorhanden ist
    if (!sensorData.containsKey(sensorID)) {
      DEBUG_P("Neue Sensor-ID gefunden. FÃ¼ge Eintrag hinzu.");
      // Sensor-ID nicht vorhanden, Offsetwert und Beschreibung hinzufÃ¼gen
      JsonObject newSensor = sensorData.createNestedObject(sensorID);
      newSensor["description"] = sensorID;
      newSensor["offset"] = 0.0;
      newfile = false;
      jsonnewSensor = true;
    }
  }

  // JSON-Daten in die Datei schreiben
  saveSensorDataToFile();
}

void saveSensorDataToFile() {
  if (jsonnewSensor) {
    File file = LittleFS.open("/sensor_data.json", "w+");
    if (!file) {
      DEBUG_P("Fehler beim Ãffnen der JSON-Sensoren zum Schreiben.");
      return;
    }

    serializeJson(sensorData, file);
    file.close();
    DEBUG_P("JSON-Sensoren wurden erfolgreich gespeichert.");
    jsonnewSensor = false;
  } else {
    DEBUG_P("JSON-Sensoren nicht gespeichert, da keine Aenderung");
  }
}

void stringToSensorAddress(const char* str, DeviceAddress sensorAddress) {
    char *token;
    char *rest = (char*)str;
    int i = 0;

    // Durchlaufe den String und teile ihn an den Doppelpunkten auf
    while ((token = strtok_r(rest, ":", &rest)) != NULL && i < 8) {
        // Konvertiere den Hexadezimal-String in einen Integer-Wert
        sensorAddress[i] = strtol(token, NULL, 16);
        i++;
    }
}

void removeNonAlphaNumeric(String &str) {
  for (int i = 0; i < str.length(); i++) {
    if (!isAlphaNumeric(str.charAt(i))) {
      str.remove(i, 1);
      i--;
    }
  }
}

