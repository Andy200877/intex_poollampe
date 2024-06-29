void connectWifi()
{                            // Funktionsaufruf "connectWifi();" muss im Setup eingebunden werden.
  if (wlanmode == 1) {  // 1 = AP
    IPAddress tempapip;
    IPAddress tempapgw;
    IPAddress tempapnetmask;
    tempapip.fromString(apip);
    tempapgw.fromString(apgw);
    tempapnetmask.fromString(apnetmask);
    WiFi.softAPConfig(tempapip, tempapgw, tempapnetmask);

    DEBUG_S("SSID: ");
    DEBUG_P(apssid);
    DEBUG_S(" PSK: ");
    DEBUG_P(appsk);
    DEBUG_S("  IP: ");
    DEBUG_P(tempapip);
    DEBUG_S("  GW: ");
    DEBUG_P(tempapgw);
    DEBUG_S("Netm: ");
    DEBUG_P(tempapnetmask);

    mqtt_active = false;
    mqtt_ha = false;

    boolean result = WiFi.softAP(apssid.c_str(), appsk.c_str(), apchannel, 0);
    if (result) {
      DEBUG_P("Accesspoint erstellt");
    } else {
      DEBUG_P("Fehler beim erstellen des Accesspoints");
      ESP.restart();
    }
  }

  if (wlanmode == 2) {  // 2 = STA
    int wlanlogincount = 0;
    // DEBUG VERBINDE MIT
    WiFi.begin(stassid.c_str(), stapsk.c_str());
    WiFi.setAutoReconnect(true);
    
    WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event) {
      Serial.println("WiFi disconnected. Reconnecting...");
      WiFi.reconnect(); // Automatischer Reconnect-Versuch
    });
    
    DEBUG_S ("Verbinde mit :  ");
    DEBUG_S (stassid);

    
    while (WiFi.status() != WL_CONNECTED) {
      DEBUG_S (".");
      wlanlogincount++;
      delay(1000);

      if (wlanlogincount == 10) {
        DEBUG_P("");
        DEBUG_P("Verbindung zum AP fehlgeschlagen ! Starte neu...");
        ESP.restart();
      }
    }
    DEBUG_P("Verbindung erfolgreich");
    DEBUG_P("IP: " + WiFi.localIP().toString());
    macAddr = WiFi.macAddress();
    macAddr.replace(":", ""); // Remove colons from the MAC address
  }
}


String getPhyModeAsString(uint8_t phyMode) {
  switch (phyMode) {
    case WIFI_PHY_MODE_11B: return "11b";
    case WIFI_PHY_MODE_11G: return "11g";
    case WIFI_PHY_MODE_11N: return "11n";
    default: return "Unknown";
  }
}
