### <center>Intex Poollampe</center>
<img src="/img/Web.png" width="150"></center>


**Benötigte Hardware**
- Nodemcu8266
- 4 kurze Kabel
- DS18B20 Sensor (optional)
- 4,7k Ohm Widerstand (optional für DS18B20)


**Anschluss des Nodemcu8266 an die Lampe**
Zuerst müssen die 4 Kreuzschrauben aus dem Aussenteil der Lampe entfernt werden.
Danach lässt sich der Lampendeckel abheben.
Stelle nun die Verbindungen wie in den Bildern beschrieben her.

- Platine V- an GND des ESP
- Platine V+ an VIN des ESP
- Platine S1 an ESP D1
- Platine S2 an ESP D2

Falls Du die erste Version der Poollampe nutzt, bitte die Verkabelung anpassen, da der D3 Anschluss am ESP nicht mehr verwendet wird.
<img src="/img/nodemcu-vin.png" width="150">
<img src="/img/S1S2.jpg" width="150">
<img src="/img/hardware.jpg" width="150">


**DS18B20 Sensoren**
Für den Anschluss der DS18B20 Sensoren kann der Anschluss D5,D6 oder D7 verwendet werden. Spannungsversorgung des DS18B20 an 3.3V des ESP. GND des Sensors an GND des ESP. Zwischen Datenleitung und 3.3V bitte noch einen 4,7kOhm Widerstand einsetzen.

Insgesamt unterstützt die Software bis zu 20 Sensoren, welche individuel benannt werden können. Der Name ist mit der SensorID verknüpft.

Beim ersten Start erstellt die Lampe einen Wlan Accesspoint mit dem Namen **Poollampe**. Das WiFi Passwort dazu lautet : **12345678**
Nachdem Ihr Euch mit dem ESP verbunden habt, könnt Ihr über einen Browser über die IP : **192.168.4.1** die Weboberfläche der Lampe aufrufen.

Die Lampe kann "Standalone" also ohne Verbindung zu einem Router verwendet werden. Natürlich dann ohne MQTT / Smarthome anbindung.

Um den vollen Umfang zu nutzen, sollest Du in den WiFi Einstellungen der Lampe dein WLAN eintragen.

Das Menü ist eigentlich selbsterklärend.

In dieser Version der Lampe gibt es folgende Neuerungen : 
- Home Assistant MQTT Discovery
- "Farbe syncronisieren" => Falls der ESP mal nach einem Neustart die falsche Farbe anzeigt, kann es dort korrigiert werden.
- Anpassbare GPIO Ports
- Filebrowser
- Update des Flashe und Filesystems über Web


**Flashen**
Zum Flashen empfehle ich das [Flash Download Tool von Espressif Systems](https://www.espressif.com/en/support/download/other-tools "Flash Download Tool") 
Als Startadresse der Flashfirmware muss 0x000000 angegeben werden.
Die Adresse für die littlefs.bin ist 0x200000.

Im LittleFS sind sämtliche Configurationen und die Webseiten gespeichert, bei einem neuen Flashen oder eines Formatierens des Filesystems macht der ESP wieder einen AccessPoint auf und Ihr müsst Eure Einstellungen anpassen.

**Reset**
Ein Reset der Wlan Konfiguration ist möglich.
Dazu die Tasten S1 und S2 an der Lampe gedrückt halten und den die Lampe an das Stromnetz anschließen. Nach ca. 5 Sekunden könnt Ihr die Tasten loslassen und Euch wieder mit den Standart Logindaten mit dem Accesspoint der Lampe verbinden.

**Debug**
Standartmässig sind alle Debug Informationen über die Serielle Schnittstelle des ESP deaktiviert.
Zum einschalten der Debug Meldung muss nur folgender Link aufgerufen werden : http://IP-Adresse/debug/on
Oder zum Ausschalten der Debug Meldungen http://IP-Adresse/debug/off

