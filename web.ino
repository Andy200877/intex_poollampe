void handlewebseiten() {
  server.on("/data", HTTP_GET, handleData);
  server.on("/setcolor", HTTP_POST, handleFormSubmitsetcolor);
  server.on("/submittemp", HTTP_POST, handleFormSubmittemp);
  server.on("/datamqtt", HTTP_GET, handleDatamqtt);
  server.on("/submitmqtt", HTTP_POST, handleFormSubmitmqtt);
  server.on("/info", HTTP_GET, handleInfo);
  server.on("/submitcolors", HTTP_POST, handleFormSubmitcolors);
  server.on("/submitsync", HTTP_POST, handleFormSubmitsync);
  server.on("/datawifi", HTTP_GET, handleDatawifi);
  server.on("/listwifi", HTTP_GET, handleDatawifib);
  server.on("/submitwifi", HTTP_POST, handleFormSubmitwifi);
  server.on("/neustart", handleNeustart);
  server.on("/submitstart", HTTP_POST, handleFormSubmitstart);
  server.on("/debug/on", handleDebugOn); // Wenn /debug/on aufgerufen wird
  server.on("/debug/off", handleDebugOff); // Wenn /debug/off aufgerufen wird
  server.on("/gpio", HTTP_GET, handleDatagpio);
  server.on("/submitgpio", HTTP_POST, handleFormSubmitgpio);
}

void handleNeustart() {
  DEBUG_P("ESP wurde Ã¼ber das Webinterface neu gestartet...");
  ESP.restart();
}


void handleFormSubmitstart() {
  if (server.method() == HTTP_POST) {      // ÃberprÃ¼fen, ob die Anfrage eine POST-Anfrage ist
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();

    // JSON-Daten parsen
    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }

    startinnen = doc["selectIn"];
    startaussen = doc["selectOut"];

    prefs.putInt("startinnen", startinnen);
    prefs.putInt("startaussen", startaussen);

    server.send(200, "text/plain", "Einstellungen gespeichert");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}




void handleFormSubmitwifi() {
  if (server.method() == HTTP_POST) {
    String jsonStr = server.arg("plain");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);

    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }


    // Die Werte aus dem JSON-Dokument extrahieren
    wlanmode = int(doc["wlanauswahl"]);
    apssid = doc["apssid"].as<String>();
    appsk = doc["appsk"].as<String>();
    apchannel = int(doc["apch"]);
    apip = doc["apip"].as<String>();
    apgw = doc["apgw"].as<String>();
    apnetmask = doc["apsub"].as<String>();
    stassid = doc["ssid"].as<String>();
    stapsk = doc["psk"].as<String>();

    if (wlanmode == 1) {
      // AP-MODE
      prefs.putString("apssid", apssid);
      prefs.putString("appsk", appsk);
      prefs.putInt("apchannel", apchannel);
      prefs.putString("apip", apip);
      prefs.putString("apnetmask", apnetmask);
      prefs.putString("apgw", apgw);
    }

    if (wlanmode == 2) {
      // STA-MODE
      prefs.putString("stassid", stassid);
      prefs.putString("stapsk", stapsk);
    }

    prefs.putInt("wlanmode", wlanmode);

    

    server.send(200, "text/plain", "Einstellungen gespeichert. Neustart nÃ¶tig.");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}


void handleDatagpio() {
  StaticJsonDocument<200> doc;
  doc["S1"] = buttonS1;
  doc["S2"] = buttonS2;
  doc["DS18B20"] = DS18B20Pin;
  doc["TimeDS18B20"] = DS18B20Duration;
  doc["TimeWlan"] = WlanDuration;
  doc["build"] = build;

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}


void handleFormSubmitgpio() {
  if (server.method() == HTTP_POST) {
    String jsonStr = server.arg("plain");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);

    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }


    // Die Werte aus dem JSON-Dokument extrahieren
    buttonS1 = int(doc["S1"]);
    buttonS2 = int(doc["S2"]);
    DS18B20Pin = int(doc["DS18B20"]);
    DS18B20Duration = int(doc["TimeDS18B20"]);
    WlanDuration = int(doc["TimeWlan"]);

    prefs.putInt("buttonS1", buttonS1);
    prefs.putInt("buttonS2", buttonS2);
    prefs.putInt("DS18B20Pin", DS18B20Pin);
    prefs.putInt("DS18B20Duration", DS18B20Duration);
    prefs.putInt("WlanDuration", WlanDuration);

    server.send(200, "text/plain", "Einstellungen gespeichert. Neustart nÃ¶tig.");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}





void handleDatawifi() {
  StaticJsonDocument<200> doc;
  doc["wlanmode"] = wlanmode;
  doc["apssid"] = apssid;
  doc["appsk"] = appsk;
  doc["apchannel"] = apchannel;
  doc["apip"] = apip;
  doc["apnetmask"] = apnetmask;
  doc["apgw"] = apgw;
  doc["stassid"] = stassid;
  doc["stapsk"] = stapsk;
  doc["build"] = build;

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}






void handleDatawifib() {
  int numNetworks = WiFi.scanNetworks();
  DynamicJsonDocument jsonDoc(1024);
  JsonArray networks = jsonDoc.createNestedArray("networks");

  for (int i = 0; i < numNetworks; i++) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
  }

  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

void handleFormSubmitsync() {
  if (server.method() == HTTP_POST) {      // ÃberprÃ¼fen, ob die Anfrage eine POST-Anfrage ist
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();


    // JSON-Daten parsen
    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }

    aktfarbeinnen = doc["selectIn"];
    sollfarbeinnen = doc["selectIn"];
    aktfarbeaussen = doc["selectOut"];
    sollfarbeaussen = doc["selectOut"];
    
    if (mqtt_active) {
      String stat = poolLampPrefix + "/" + macAddr + "/innen/stat_t";
      publishMessage(stat.c_str(), incolor[aktfarbeinnen].c_str());
      stat = poolLampPrefix + "/" + macAddr + "/aussen/stat_t";
      publishMessage(stat.c_str(), outcolor[aktfarbeaussen].c_str());
    }




    server.send(200, "text/plain", "Farben sind synchronisiert");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}


void handleFormSubmitsetcolor() {
  if (server.method() == HTTP_POST) {      // ÃberprÃ¼fen, ob die Anfrage eine POST-Anfrage ist
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();

    // JSON-Daten parsen
    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }

    if (doc.containsKey("aussen")) {
      sollfarbeaussen = doc["aussen"];
      DEBUG_P("Key 'aussen' found in JSON data");
    }

    if (doc.containsKey("innen")) {
      sollfarbeinnen = doc["innen"];
      DEBUG_P("Key 'innen' found in JSON data");
    }

    server.send(200, "text/plain", "Ok... neue Farbe erhalten");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}

void handleFormSubmitcolors() {
  if (server.method() == HTTP_POST) {      // ÃberprÃ¼fen, ob die Anfrage eine POST-Anfrage ist
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();

    // JSON-Daten parsen
    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }

    incolor[0] = doc["innen1"].as<String>();
    incolor[1] = doc["innen2"].as<String>();
    incolor[2] = doc["innen3"].as<String>();
    incolor[3] = doc["innen4"].as<String>();

    outcolor[0] = doc["aussen1"].as<String>();
    outcolor[1] = doc["aussen2"].as<String>();
    outcolor[2] = doc["aussen3"].as<String>();
    outcolor[3] = doc["aussen4"].as<String>();
    outcolor[4] = doc["aussen5"].as<String>();
    outcolor[5] = doc["aussen6"].as<String>();
    outcolor[6] = doc["aussen7"].as<String>();
    
    prefs.putString("incolor1", incolor[0]);
    prefs.putString("incolor2", incolor[1]);
    prefs.putString("incolor3", incolor[2]);
    prefs.putString("incolor4", incolor[3]);
    prefs.putString("outcolor1", outcolor[0]);
    prefs.putString("outcolor2", outcolor[1]);
    prefs.putString("outcolor3", outcolor[2]);
    prefs.putString("outcolor4", outcolor[3]);
    prefs.putString("outcolor5", outcolor[4]);
    prefs.putString("outcolor6", outcolor[5]);
    prefs.putString("outcolor7", outcolor[6]);

    server.send(200, "text/plain", "Einstellungen gespeichert");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}

void handleInfo() {
  char laufzeittext[100];
  sprintf(laufzeittext, "%uy, %ud, %02uh:%02um:%02us", LZjahr, LZtag, LZstd, LZmin, LZsek);

  JsonObject wifiInfo = sensorWerte.createNestedObject("wifiInfo");
  wifiInfo["mode"] = getPhyModeAsString(WiFi.getPhyMode());
  wifiInfo["channel"] = WiFi.channel();
  wifiInfo["ssid"] = WiFi.SSID();
  wifiInfo["bssid"] = WiFi.BSSIDstr();
  wifiInfo["rssi"] = WiFi.RSSI();
  
  wifiInfo["mac"] = WiFi.macAddress();
  wifiInfo["ip"] = WiFi.localIP().toString();
  wifiInfo["gateway"] = WiFi.gatewayIP().toString();
  wifiInfo["subnetMask"] = WiFi.subnetMask().toString();
  wifiInfo["dns1"] = WiFi.dnsIP(0).toString();
  wifiInfo["dns2"] = WiFi.dnsIP(1).toString();

  JsonObject mqttInfo = sensorWerte.createNestedObject("mqtt");
  if (mqtt_active) { mqttInfo["anaus"] = "eingeschaltet"; } else { mqttInfo["anaus"] = "ausgeschaltet"; }
  mqttInfo["host"] = mqtt_broker;
  mqttInfo["port"] = mqtt_port;
  mqttInfo["user"] = mqtt_username;
  if (mqtt_client.connected()) { mqttInfo["status"] = "verbunden"; } else { mqttInfo["status"] = "getrennt"; }

  FSInfo fs_info;
  LittleFS.info(fs_info);
  JsonObject systemInfo = sensorWerte.createNestedObject("systemInfo");
  systemInfo["espChipID"] = ESP.getChipId(); // ESP ChipID
  systemInfo["flashChipID"] = ESP.getFlashChipId(); // Flash ChipID
  systemInfo["flashChipSize"] = ESP.getFlashChipSize() / 1024; // GrÃ¶Ãe des Flash-Chips in Bytes
  systemInfo["programSize"] = ESP.getSketchSize() / 1024;
  systemInfo["freeSketchSpace"] = ESP.getFreeSketchSpace() / 1024;
  systemInfo["freeHeap"] = ESP.getFreeHeap() / 1024;
  systemInfo["filesystemSize"] = fs_info.totalBytes / 1024; // GrÃ¶Ãe des Dateisystems in Bytes
  systemInfo["usedSpace"] = fs_info.usedBytes / 1024; // Verwendeter Speicherplatz in Bytes
  systemInfo["freeSpace"] = (fs_info.totalBytes - fs_info.usedBytes)  / 1024;
  systemInfo["resetReason"] = ESP.getResetReason();

  JsonObject status = sensorWerte.createNestedObject("status");
  status["laufzeittext"] = laufzeittext;
  status["build"] = build;

    // Konvertiere JSON in einen String
  String jsonResponse;
  serializeJson(sensorWerte, jsonResponse);

  // Sende die JSON-Antwort zurÃ¼ck an den Client
  server.send(200, "application/json", jsonResponse);
}

void handleData() {
  char laufzeittext[100];
  sprintf(laufzeittext, "Uptime: %uy, %ud, %02uh, %02um, %02us", LZjahr, LZtag, LZstd, LZmin, LZsek);

  // Erstelle JSON-Daten mit den aktuellen Werten von x, y und laufzeit
  StaticJsonDocument<200> doc;
  doc["colorin"] = aktfarbeinnen;
  doc["colorout"] =  aktfarbeaussen;
  doc["in1"] = incolor[0];
  doc["in2"] = incolor[1];
  doc["in3"] = incolor[2];
  doc["in4"] = incolor[3];
  doc["out1"] = outcolor[0];
  doc["out2"] = outcolor[1];
  doc["out3"] = outcolor[2];
  doc["out4"] = outcolor[3];
  doc["out5"] = outcolor[4];
  doc["out6"] = outcolor[5];
  doc["out7"] = outcolor[6];
  doc["startaussen"] = startaussen;
  doc["startinnen"] = startinnen;
  doc["laufzeittext"] = laufzeittext;
  doc["build"] = build;

  // Konvertiere JSON in einen String
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  // Sende die JSON-Antwort zurÃ¼ck an den Client
  server.send(200, "application/json", jsonResponse);
}

void handleDatamqtt() {

  // Erstelle JSON-Daten mit den aktuellen Werten von x, y und laufzeit
  StaticJsonDocument<200> doc;
  if (mqtt_active) { doc["mqttactive"] = 1; }
  if (!mqtt_active) { doc["mqttactive"] = 0; }
  doc["mqtt_broker"] = mqtt_broker;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_username"] = mqtt_username;
  doc["mqtt_password"] = mqtt_password;
  doc["mqtttopic"] = poolLampPrefix;
  if (mqtt_retain) { doc["mqttretain"] = 1; }
  if (!mqtt_retain) { doc["mqttretain"] = 0; }  
  if (mqtt_ha) { doc["mqttha"] = 1; }
  if (!mqtt_ha) { doc["mqttha"] = 0; }
  doc["build"] = build;

  // Konvertiere JSON in einen String
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  // Sende die JSON-Antwort zurÃ¼ck an den Client
  server.send(200, "application/json", jsonResponse);
}

void handleFormSubmitmqtt() {
  if (server.method() == HTTP_POST) {      // ÃberprÃ¼fen, ob die Anfrage eine POST-Anfrage ist
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();

    // JSON-Daten parsen
    StaticJsonDocument<200> doc;
    DeserializationError errorjson = deserializeJson(doc, jsonStr);
    if (errorjson) {
      server.send(400, "text/plain", "Failed to parse JSON data");
      return;
    }

    // Die Werte aus dem JSON-Dokument extrahieren
    if (int(doc["mqttactive"]) == 0) { mqtt_active = false; }
    if (int(doc["mqttactive"]) == 1) {
      mqtt_active = true;
    }
    if (int(doc["mqtthaactive"]) == 0) { mqtt_ha = false; }
    if (int(doc["mqtthaactive"]) == 1) { mqtt_ha = true; }

    if (int(doc["mqttretain"]) == 0) { mqtt_retain = false; }
    if (int(doc["mqttretain"]) == 1) { mqtt_retain = true; }



    mqtt_broker = doc["mqttBroker"].as<String>();
    mqtt_port = int(doc["mqttPort"]);
    mqtt_username = doc["mqttUsername"].as<String>();
    mqtt_password = doc["mqttPassword"].as<String>();
    poolLampPrefix = doc["mqttTopic"].as<String>();

    prefs.putString("mqtt_broker", mqtt_broker);
    prefs.putString("mqttprefix", poolLampPrefix);
    prefs.putString("mqtt_username", mqtt_username);
    prefs.putString("mqtt_password", mqtt_password);
    prefs.putInt("mqtt_port", mqtt_port);
    prefs.putBool("mqttactive", mqtt_active);
    prefs.putBool("mqttha", mqtt_ha);
    prefs.putBool("mqttretain", mqtt_retain);
    server.send(200, "text/plain", "Einstellungen gespeichert");  // Hier kÃ¶nnen Sie eine benutzerdefinierte Antwort senden, wenn gewÃ¼nscht
  } else {
    server.send(405, "text/plain", "es ist ein Fehler aufgetreten");
  }
}



void handleFormSubmittemp() {
  if (server.method() == HTTP_POST) {
    String jsonStr = server.arg("plain");
    jsonStr.replace("\\", "");
    DEBUG_P("Received JSON data:");
    DEBUG_P(jsonStr);
    jsonStr.trim();
    
    int startIndex = jsonStr.indexOf('{');
    int endIndex = jsonStr.lastIndexOf('}');
    if (startIndex != -1 && endIndex != -1) {
      jsonStr = jsonStr.substring(startIndex, endIndex + 1);

      // Deserialisierung des JSON-Strings
      DynamicJsonDocument doc(2048);
      DeserializationError errorjson = deserializeJson(doc, jsonStr);

      if (errorjson) {
        DEBUG_S("Failed to parse JSON data: ");
        DEBUG_P(errorjson.c_str());
        server.send(400, "text/plain", "Failed to parse JSON data");
        return;
      }

      // Ãffnen oder Erstellen der Datei im LittleFS
      File file = LittleFS.open("/sensor_data.json", "w");
      if (!file) {
        DEBUG_P("Failed to open file");
        server.send(500, "text/plain", "Failed to open file");
        return;
      }

      // JSON-Daten in die Datei schreiben
      if (serializeJson(doc, file) == 0) {
        DEBUG_P("Failed to write JSON data to file");
        server.send(500, "text/plain", "Failed to write JSON data to file");
        file.close();
        return;
      }

      // Datei schlieÃen
      file.close();

      // JSON-Daten durchgehen und MQTT-Publish fÃ¼r jeden Sensor durchfÃ¼hren
      for (JsonPair sensor : doc.as<JsonObject>()) {
        String sensorID = sensor.key().c_str(); // Sensor-ID aus dem JSON-Paar extrahieren
        removeNonAlphaNumeric(sensorID);
        String topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/" + sensorID + "_temp/config";
        mqtt_client.publish(topic.c_str(), "", mqtt_retain);
        topic = "homeassistant/sensor/" + poolLampPrefix + "_" + macAddr + "/" + sensorID + "_uid/config";
        mqtt_client.publish(topic.c_str(), "", mqtt_retain);
      }      

      DEBUG_P("Einstellungen gespeichert.");
      server.send(200, "text/plain", "Einstellungen gespeichert.");
        } else {
      DEBUG_P("Failed to extract valid JSON data");
      server.send(400, "text/plain", "Failed to extract valid JSON data");
        }
  } else {
    DEBUG_P("Method Not Allowed");
    server.send(405, "text/plain", "Method Not Allowed");
  }
  if (mqtt_ha) { sendHAdiscoverySensors(); }
}


void handleDebugOn() {
  debugausgabe = true;
  prefs.putBool("debug", true);

  handleRootDebug(); // Rufen Sie die handleRoot-Funktion auf, um den aktualisierten Status anzuzeigen
}

void handleDebugOff() {
  debugausgabe = false;
  prefs.putBool("debug", false);
  handleRootDebug(); // Rufen Sie die handleRoot-Funktion auf, um den aktualisierten Status anzuzeigen
}

void handleRootDebug() {
  String message;
  if (debugausgabe) {
    message = "Debug ist aktiviert.";
  } else {
    message = "Debug ist deaktiviert.";
  }
  server.send(200, "text/plain", message); // Senden Sie die Nachricht an den Client
}

