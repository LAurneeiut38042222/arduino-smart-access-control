#include <Keypad.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// Servo
Servo bariera;
const int pinServo = 10;

// Buzzer
const int pinBuzzer = 11;

// Daca buzzerul tau este pasiv, lasa true.
// Daca este activ si nu suna bine, schimba true in false.
const bool BUZZER_PASIV = true;

// Pozitii servo
int pozitieInchis = 0;
int pozitieDeschis = 90;

// Miscare lenta si lina
const int pauzaServo = 45;
const int pasServo = 1;
const int timpStabilizare = 250;

// PIN
String codCorect = "1234";
String codIntrodus = "";
String codAdmin = "9999";

const int lungimePIN = 4;

// EEPROM
const int EEPROM_MAGIC_ADDR = 0;
const int EEPROM_PIN_ADDR = 1;
const byte EEPROM_MAGIC = 78;

// Incercari
int incercariGresite = 0;
const int maxIncercari = 3;

// Timeout
unsigned long ultimulInput = 0;
const unsigned long timpTimeout = 10000;

// Idle animatie
unsigned long ultimulIdle = 0;
int ecranIdle = 0;

// Keypad
const byte ROWS = 4;
const byte COLS = 4;

char taste[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(taste), rowPins, colPins, ROWS, COLS);

void setup() {
  lcd.begin(16, 2);

  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, LOW);

  citestePinDinEEPROM();

  bariera.attach(pinServo);
  bariera.write(pozitieInchis);
  delay(700);
  bariera.detach();

  ecranPornire();
  sunetStart();
  afisareStart();
}

void loop() {
  verificaTimeout();
  animatieIdle();

  char tasta = keypad.getKey();

  if (tasta) {
    ultimulInput = millis();
    sunetTasta();

    if (tasta >= '0' && tasta <= '9') {
      adaugaCifra(tasta);
    }
    else if (tasta == 'C') {
      stergeUltimaCifra();
    }
    else if (tasta == '#') {
      verificaCod();
    }
    else if (tasta == '*') {
      verificaIntrareAdmin();
    }
    else if (tasta == 'D') {
      modService();
    }
  }
}

// ---------------- BUZZER ----------------

void beep(int frecventa, int durata) {
  if (BUZZER_PASIV) {
    tone(pinBuzzer, frecventa);
    delay(durata);
    noTone(pinBuzzer);
  } else {
    digitalWrite(pinBuzzer, HIGH);
    delay(durata);
    digitalWrite(pinBuzzer, LOW);
  }
}

void sunetTasta() {
  beep(2500, 45);
}

void sunetStart() {
  beep(1200, 100);
  delay(70);
  beep(1800, 100);
}

void sunetCorect() {
  beep(1200, 120);
  delay(70);
  beep(1700, 120);
  delay(70);
  beep(2200, 180);
}

void sunetGresit() {
  beep(400, 220);
  delay(90);
  beep(250, 300);
}

void sunetBlocare() {
  for (int i = 0; i < 3; i++) {
    beep(300, 180);
    delay(120);
  }
}

void sunetAdmin() {
  beep(1600, 100);
  delay(70);
  beep(2000, 100);
}

void sunetSetat() {
  beep(1500, 120);
  delay(70);
  beep(2000, 180);
}

// ---------------- EEPROM ----------------

void citestePinDinEEPROM() {
  if (EEPROM.read(EEPROM_MAGIC_ADDR) != EEPROM_MAGIC) {
    codCorect = "1234";
    salveazaPinInEEPROM(codCorect);
    return;
  }

  String pinSalvat = "";

  for (int i = 0; i < lungimePIN; i++) {
    char c = EEPROM.read(EEPROM_PIN_ADDR + i);

    if (c < '0' || c > '9') {
      codCorect = "1234";
      salveazaPinInEEPROM(codCorect);
      return;
    }

    pinSalvat += c;
  }

  codCorect = pinSalvat;
}

void salveazaPinInEEPROM(String pin) {
  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);

  for (int i = 0; i < lungimePIN; i++) {
    EEPROM.write(EEPROM_PIN_ADDR + i, pin[i]);
  }
}

// ---------------- ECRANE ----------------

void ecranPornire() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SMART ACCESS");
  lcd.setCursor(0, 1);
  lcd.print(" Barrier System");
  delay(1600);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Initializare...");
  lcd.setCursor(0, 1);

  for (int i = 0; i < 16; i++) {
    lcd.print("#");
    delay(60);
  }

  delay(500);
}

void afisareStart() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART ACCESS");
  afiseazaPIN();
}

void afiseazaPIN() {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (int i = 0; i < lungimePIN; i++) {
    if (i < codIntrodus.length()) lcd.print("*");
    else lcd.print("_");
  }

  lcd.print("       ");
}

void afiseazaPINCifraVizibila(char cifra) {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (int i = 0; i < lungimePIN; i++) {
    if (i < codIntrodus.length() - 1) {
      lcd.print("*");
    } 
    else if (i == codIntrodus.length() - 1) {
      lcd.print(cifra);
    } 
    else {
      lcd.print("_");
    }
  }

  lcd.print("       ");
  delay(300);
  afiseazaPIN();
}

void animatieIdle() {
  if (codIntrodus.length() > 0) return;

  if (millis() - ultimulIdle >= 4000) {
    ultimulIdle = millis();
    lcd.clear();

    if (ecranIdle == 0) {
      lcd.setCursor(0, 0);
      lcd.print("SMART ACCESS");
      lcd.setCursor(0, 1);
      lcd.print("Astept PIN...");
      ecranIdle = 1;
    } 
    else {
      lcd.setCursor(0, 0);
      lcd.print("Sistem securizat");
      lcd.setCursor(0, 1);
      lcd.print("C=Sterge  #=OK");
      ecranIdle = 0;
    }
  }
}

// ---------------- PIN NORMAL ----------------

void adaugaCifra(char cifra) {
  if (codIntrodus.length() < lungimePIN) {
    codIntrodus += cifra;
    afiseazaPINCifraVizibila(cifra);
  } 
  else {
    lcd.setCursor(0, 0);
    lcd.print("PIN COMPLET     ");
    delay(600);
    lcd.setCursor(0, 0);
    lcd.print("SMART ACCESS    ");
  }
}

void stergeUltimaCifra() {
  if (codIntrodus.length() > 0) {
    codIntrodus.remove(codIntrodus.length() - 1);
    afiseazaPIN();
  } 
  else {
    lcd.setCursor(0, 0);
    lcd.print("NIMIC DE STERS  ");
    delay(700);
    lcd.setCursor(0, 0);
    lcd.print("SMART ACCESS    ");
  }
}

void verificaTimeout() {
  if (codIntrodus.length() > 0 && millis() - ultimulInput > timpTimeout) {
    codIntrodus = "";

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sesiune expirata");
    lcd.setCursor(0, 1);
    lcd.print("Reintrodu PIN");
    delay(1500);

    afisareStart();
  }
}

void verificaCod() {
  if (codIntrodus.length() == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PIN INVALID");
    lcd.setCursor(0, 1);
    lcd.print("Introdu PIN");
    delay(1300);
    afisareStart();
    return;
  }

  if (codIntrodus.length() < lungimePIN) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PIN INCOMPLET");
    lcd.setCursor(0, 1);
    lcd.print("Necesare 4 cifre");
    delay(1500);
    afisareStart();
    return;
  }

  if (codIntrodus == codCorect) {
    codCorectAcceptat();
  } 
  else {
    codGresit();
  }

  codIntrodus = "";
  afisareStart();
}

// ---------------- COD CORECT ----------------

void codCorectAcceptat() {
  incercariGresite = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCES PERMIS");
  lcd.setCursor(0, 1);
  lcd.print("Se deschide...");
  sunetCorect();
  delay(500);

  deschideBarieraLent();

  countdownDeschis();

  inchideBarieraLent();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCES FINALIZAT");
  lcd.setCursor(0, 1);
  lcd.print("O zi buna!");
  delay(1500);
}

void countdownDeschis() {
  for (int sec = 5; sec >= 1; sec--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bariera deschisa");
    lcd.setCursor(0, 1);
    lcd.print("Inchidere: ");
    lcd.print(sec);
    lcd.print("s ");
    delay(1000);
  }
}

// ---------------- COD GRESIT ----------------

void codGresit() {
  incercariGresite++;
  int ramase = maxIncercari - incercariGresite;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ACCES RESPINS");

  lcd.setCursor(0, 1);

  if (ramase > 0) {
    lcd.print("Ramase: ");
    lcd.print(ramase);
    lcd.print(" incerc.");
    sunetGresit();
    delay(1200);
  } 
  else {
    lcd.print("Limita atinsa");
    sunetBlocare();
    delay(800);
    blocheazaSistem();
  }
}

void blocheazaSistem() {
  for (int sec = 10; sec >= 1; sec--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SISTEM BLOCAT ");
    lcd.print(sec);
    lcd.print("s");

    lcd.setCursor(0, 1);
    afiseazaBaraBlocare(sec);

    delay(1000);
  }

  incercariGresite = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEM DEBLOCAT");
  lcd.setCursor(0, 1);
  lcd.print("Incearca iar");
  sunetStart();
  delay(1300);
}

void afiseazaBaraBlocare(int sec) {
  int blocuri = map(sec, 10, 1, 10, 1);

  lcd.print("[");
  for (int i = 0; i < 10; i++) {
    if (i < blocuri) lcd.print("#");
    else lcd.print(" ");
  }
  lcd.print("]");
}

// ---------------- ADMIN ----------------

void verificaIntrareAdmin() {
  if (codIntrodus == codAdmin) {
    codIntrodus = "";
    modAdmin();
  } 
  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Comanda invalida");
    lcd.setCursor(0, 1);
    lcd.print("# confirma PIN");
    sunetGresit();
    delay(1000);

    codIntrodus = "";
    afisareStart();
  }
}

void modAdmin() {
  String pinNou1 = "";
  String pinNou2 = "";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MOD ADMIN ACTIV");
  lcd.setCursor(0, 1);
  lcd.print("Setare PIN nou");
  sunetAdmin();
  delay(1600);

  pinNou1 = citestePINAdmin("Introdu PIN nou");

  if (pinNou1 == "") {
    adminAnulat();
    return;
  }

  if (pinNou1 == codCorect) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("COD IDENTIC");
    lcd.setCursor(0, 1);
    lcd.print("Alege alt PIN");
    sunetGresit();
    delay(1400);
    afisareStart();
    return;
  }

  pinNou2 = citestePINAdmin("Confirma PIN nou");

  if (pinNou2 == "") {
    adminAnulat();
    return;
  }

  if (pinNou1 != pinNou2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PIN NU COINCIDE");
    lcd.setCursor(0, 1);
    lcd.print("Reincearca");
    sunetGresit();
    delay(1400);
    afisareStart();
    return;
  }

  codCorect = pinNou1;
  salveazaPinInEEPROM(codCorect);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("COD NOU SETAT");
  lcd.setCursor(0, 1);
  lcd.print("Salvat permanent");
  sunetSetat();
  delay(1500);

  afisareStart();
}

String citestePINAdmin(String mesaj) {
  String pin = "";
  unsigned long ultimulInputAdmin = millis();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mesaj);
  afiseazaPinGeneric(pin);

  while (true) {
    char tasta = keypad.getKey();

    if (millis() - ultimulInputAdmin > timpTimeout) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Admin expirat");
      lcd.setCursor(0, 1);
      lcd.print("Revenire...");
      sunetGresit();
      delay(1100);
      return "";
    }

    if (tasta) {
      ultimulInputAdmin = millis();
      sunetTasta();

      if (tasta >= '0' && tasta <= '9') {
        if (pin.length() < lungimePIN) {
          pin += tasta;
          afiseazaPinGenericVizibil(pin, tasta);
        }
      }

      else if (tasta == 'C') {
        if (pin.length() > 0) {
          pin.remove(pin.length() - 1);
          afiseazaPinGeneric(pin);
        }
      }

      else if (tasta == '#') {
        if (pin.length() == lungimePIN) {
          return pin;
        } 
        else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("PIN INCOMPLET");
          lcd.setCursor(0, 1);
          lcd.print("Introdu 4 cifre");
          sunetGresit();
          delay(1000);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(mesaj);
          afiseazaPinGeneric(pin);
        }
      }

      else if (tasta == '*') {
        return "";
      }
    }
  }
}

void afiseazaPinGeneric(String pin) {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (int i = 0; i < lungimePIN; i++) {
    if (i < pin.length()) lcd.print("*");
    else lcd.print("_");
  }

  lcd.print("       ");
}

void afiseazaPinGenericVizibil(String pin, char cifra) {
  lcd.setCursor(0, 1);
  lcd.print("PIN: ");

  for (int i = 0; i < lungimePIN; i++) {
    if (i < pin.length() - 1) lcd.print("*");
    else if (i == pin.length() - 1) lcd.print(cifra);
    else lcd.print("_");
  }

  lcd.print("       ");
  delay(300);
  afiseazaPinGeneric(pin);
}

void adminAnulat() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ADMIN ANULAT");
  lcd.setCursor(0, 1);
  lcd.print("PIN neschimbat");
  sunetGresit();
  delay(1200);
  afisareStart();
}

// ---------------- SERVICE MODE ----------------

void modService() {
  String pinService = "";

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SERVICE MODE");
  lcd.setCursor(0, 1);
  lcd.print("Introdu PIN");
  sunetAdmin();
  delay(1000);

  pinService = citestePINAdmin("PIN service");

  if (pinService == "") {
    afisareStart();
    return;
  }

  if (pinService != codCorect) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SERVICE RESPINS");
    lcd.setCursor(0, 1);
    lcd.print("PIN incorect");
    sunetGresit();
    delay(1200);
    afisareStart();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SERVICE ACTIV");
  lcd.setCursor(0, 1);
  lcd.print("Deschidere...");
  sunetCorect();
  delay(800);

  deschideBarieraLent();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MANUAL DESCHIS");
  lcd.setCursor(0, 1);
  lcd.print("# sau D inchide");

  unsigned long startManual = millis();

  while (true) {
    char tasta = keypad.getKey();

    if (tasta) {
      sunetTasta();
    }

    if (tasta == '#' || tasta == 'D') {
      break;
    }

    if (millis() - startManual > 15000) {
      break;
    }
  }

  inchideBarieraLent();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SERVICE FINAL");
  lcd.setCursor(0, 1);
  lcd.print("Bariera inchisa");
  delay(1500);

  afisareStart();
}

// ---------------- SERVO LENT ----------------

void deschideBarieraLent() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Deschidere lenta");
  lcd.setCursor(0, 1);
  lcd.print("[              ]");

  bariera.attach(pinServo);
  delay(200);

  for (int poz = pozitieInchis; poz <= pozitieDeschis; poz += pasServo) {
    bariera.write(poz);
    afiseazaProgresServo(poz, pozitieInchis, pozitieDeschis, '>');
    delay(pauzaServo);
  }

  bariera.write(pozitieDeschis);
  delay(timpStabilizare);
  bariera.detach();
}

void inchideBarieraLent() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inchidere lenta");
  lcd.setCursor(0, 1);
  lcd.print("[              ]");

  bariera.attach(pinServo);
  delay(200);

  for (int poz = pozitieDeschis; poz >= pozitieInchis; poz -= pasServo) {
    bariera.write(poz);
    afiseazaProgresServo(pozitieDeschis - poz, 0, pozitieDeschis - pozitieInchis, '<');
    delay(pauzaServo);
  }

  bariera.write(pozitieInchis);
  delay(timpStabilizare);
  bariera.detach();
}

void afiseazaProgresServo(int valoare, int minim, int maxim, char simbol) {
  int blocuri = map(valoare, minim, maxim, 0, 14);

  lcd.setCursor(1, 1);

  for (int i = 0; i < 14; i++) {
    if (i < blocuri) lcd.print(simbol);
    else lcd.print(" ");
  }
}