void leseS1() { //INNEN
  int readingS1 = digitalRead(buttonS1);
  if (readingS1 != lastButtonStateS1) { lastDebounceTimeS1 = millis(); }

  if ((millis() - lastDebounceTimeS1) > debounceDelay) {
    
    if (digitalRead(buttonS1) == LOW) {
      if (buttonActiveS1 == false) {
        buttonActiveS1 = true;
        buttonTimerS1 = millis();
      }

      if ((millis() - buttonTimerS1 > longPressTime) && (longPressActiveS1 == false)) {
        longPressActiveS1 = true;
        DEBUG_P("Longpress... S1");
        sollfarbeinnen = 0;
      }
    }

  
    if (digitalRead(buttonS1) == HIGH) {
      if (buttonActiveS1 == true) {
        if (longPressActiveS1 == true) {
          longPressActiveS1 = false;
          } else {
          DEBUG_P("Shortpress... S1");
          sollfarbeinnen++;
          if (sollfarbeinnen > 3) { sollfarbeinnen = 0; }
        }
      buttonActiveS1 = false;
      }
    }  
  }
  lastButtonStateS1 = readingS1;
}



void leseS2() { //AUSSEN
  int readingS2 = digitalRead(buttonS2);
  if (readingS2 != lastButtonStateS2) { lastDebounceTimeS2 = millis(); }

  if ((millis() - lastDebounceTimeS2) > debounceDelay) {
    if (digitalRead(buttonS2) == LOW) {
      if (buttonActiveS2 == false) {
        buttonActiveS2 = true;
        buttonTimerS2 = millis();
      }

      if ((millis() - buttonTimerS2 > longPressTime) && (longPressActiveS2 == false)) {
        longPressActiveS2 = true;
        DEBUG_P("Longpress... S2");
        sollfarbeaussen = 0;
      }
    }
  
    if (digitalRead(buttonS2) == HIGH) {
      if (buttonActiveS2 == true) {
        if (longPressActiveS2 == true) {
          longPressActiveS2 = false;
        } else {
          DEBUG_P("Shortpress... S2");
          sollfarbeaussen++;
          if (sollfarbeaussen > 6) { sollfarbeaussen = 0; }
        }
        buttonActiveS2 = false;
      }
    }
  }
  lastButtonStateS2 = readingS2;
}




void changecolorS1() { //INNEN
  if (sollfarbeinnen > aktfarbeinnen) { anzahldrucks1 = sollfarbeinnen - aktfarbeinnen; }
  if (sollfarbeinnen < aktfarbeinnen) { anzahldrucks1 = 3 - aktfarbeinnen + sollfarbeinnen + 1; }
  if (sollfarbeinnen == aktfarbeinnen) { anzahldrucks1 = 0; }

  DEBUG_S("FarbeL1 : ");
  DEBUG_S(aktfarbeinnen);
  DEBUG_S(" | neuesprogrammL1 : ");
  DEBUG_P(sollfarbeinnen);
  DEBUG_S("anzahldrucks1 : ");
  DEBUG_P(anzahldrucks1);
  
  pinMode(buttonS1, OUTPUT);

  DEBUG_P("S1 : ");
  for (int zaehler=0; zaehler<anzahldrucks1; zaehler = zaehler+1) {
    DEBUG_S(zaehler);
    DEBUG_S(". LOW|");
    digitalWrite(buttonS1, LOW);
    delay(70);
    DEBUG_P("HIGH");
    digitalWrite(buttonS1, HIGH);
    delay(70);
  }

  pinMode(buttonS1, INPUT_PULLUP);
  aktfarbeinnen = sollfarbeinnen;
  anzahldrucks1 = 0;
  if (mqtt_active) {
    DEBUG_P("Publish neue Innenfarbe...");
    String stat = poolLampPrefix + "/" + macAddr + "/innen/stat_t";
    publishMessage(stat.c_str(), incolor[aktfarbeinnen].c_str());
    stat = poolLampPrefix + "/" + macAddr + "/innen/cmd_t";
    publishMessage(stat.c_str(), outcolor[aktfarbeinnen].c_str());
  }
}


void changecolorS2() { //AUSSEN
  if (sollfarbeaussen == 0) { anzahldrucks2 = 99; }
  if (sollfarbeaussen > aktfarbeaussen) { anzahldrucks2 = sollfarbeaussen - aktfarbeaussen; }
  if ((sollfarbeaussen < aktfarbeaussen) && (sollfarbeaussen > 0 )) { anzahldrucks2 = 6 - aktfarbeaussen + sollfarbeaussen + 1; }
  if (sollfarbeaussen == aktfarbeaussen) { anzahldrucks2 = 0; }

  DEBUG_S("Farbe Aussen : ");
  DEBUG_S(aktfarbeaussen);
  DEBUG_S(" | Neue Farbe : ");
  DEBUG_P(sollfarbeaussen);
  DEBUG_S("Anzahl Tastendruck : ");
  DEBUG_P(anzahldrucks2);

  pinMode(buttonS2, OUTPUT);

  if (anzahldrucks2 == 99) {
    DEBUG_P("Mache Longpress..."); 
    DEBUG_S("LOW...");
    digitalWrite(buttonS2, LOW);
    delay(3500);
    DEBUG_P("HIGH (fertig)");
    digitalWrite(buttonS2, HIGH); 
  } else {
  
    DEBUG_P("S2 : ");
    for (int zaehler=0; zaehler<anzahldrucks2; zaehler = zaehler+1){
      DEBUG_S(zaehler);
      DEBUG_S(". LOW|");
      digitalWrite(buttonS2, LOW);
      delay(70);
      DEBUG_P("HIGH");
      digitalWrite(buttonS2, HIGH);
      delay(70);
    }
  }

  pinMode(buttonS2, INPUT_PULLUP);
  aktfarbeaussen = sollfarbeaussen;
  anzahldrucks2 = 0;
  if (mqtt_active) {
    DEBUG_P("Publish neue Aussenfarbe...");
    String stat = poolLampPrefix + "/" + macAddr + "/aussen/stat_t";
    publishMessage(stat.c_str(), outcolor[aktfarbeaussen].c_str());
    stat = poolLampPrefix + "/" + macAddr + "/aussen/cmd_t";
    publishMessage(stat.c_str(), outcolor[aktfarbeaussen].c_str());
  }
}
