namespace Programms {

// ================
// === Programs ===
// ================
void testFloatDoublePrecision() {
  // Größe ausgeben
  Serial.print("sizeof(float): ");
  Serial.println(sizeof(float));

  Serial.print("sizeof(double): ");
  Serial.println(sizeof(double));

  // Werte definieren
  float fVal = 1.1234567f;
  double dVal = 1.123456789012345;

  Serial.print("float Wert:  ");
  Serial.println(fVal, 7);

  Serial.print("double Wert: ");
  Serial.println(dVal, 15);

  // Differenz berechnen
  double diff = dVal - (double)fVal;
  Serial.print("Differenz double - float: ");
  Serial.println(diff, 15);
}

TwoWire* activeWire = &Wire;
void runI2C_CLI() {
  Serial.println(F("=== I2C-CLI gestartet ==="));
  Serial.println(F("Befehle: scan | read <addr> <n> | write <addr> <data...> | i2cdev <n> | exit"));

  while (true) {
    Serial.print(F("i2c> "));
    while (!Serial.available())
      ;
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.equalsIgnoreCase("exit")) {
      Serial.println(F("I2C-CLI beendet."));
      break;
    } else if (line.equalsIgnoreCase("scan")) {
      Serial.println(F("Scanning..."));
      for (byte addr = 1; addr < 127; addr++) {
        activeWire->beginTransmission(addr);
        if (activeWire->endTransmission() == 0) {
          Serial.print(F(" - Gerät gefunden bei 0x"));
          Serial.println(addr, HEX);
        }
      }
      Serial.println(F("Scan abgeschlossen."));
    } else if (line.startsWith("read")) {
      int addr, n;
      sscanf(line.c_str(), "read %x %d", &addr, &n);
      if (addr < 1 || addr > 127 || n <= 0) {
        Serial.println(F(" Ungültige Adresse oder Länge."));
        continue;
      }

      activeWire->requestFrom(addr, n);
      Serial.print(F("Empfangen von 0x"));
      Serial.print(addr, HEX);
      Serial.print(F(": "));
      while (activeWire->available()) {
        byte b = activeWire->read();
        Serial.print(F("0x"));
        if (b < 0x10) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else if (line.startsWith("write")) {
      std::vector<byte> bytes;
      char* token = strtok((char*)line.c_str() + 5, " ");
      if (!token) {
        Serial.println(F(" Keine Adresse angegeben."));
        continue;
      }

      int addr = strtol(token, nullptr, 0);
      if (addr < 1 || addr > 127) {
        Serial.println(F(" Ungültige Adresse."));
        continue;
      }

      while ((token = strtok(nullptr, " "))) {
        int val = strtol(token, nullptr, 0);
        if (val < 0 || val > 255) {
          Serial.println(F(" Ungültiger Bytewert."));
          bytes.clear();
          break;
        }
        bytes.push_back((byte)val);
      }

      if (!bytes.empty()) {
        activeWire->beginTransmission(addr);
        for (byte b : bytes) {
          activeWire->write(b);
        }
        byte result = activeWire->endTransmission();
        if (result == 0) {
          Serial.println(F(" Daten gesendet."));
        } else {
          Serial.print(F(" Fehler beim Senden. Code: "));
          Serial.println(result);
        }
      }
    } else if (line.startsWith("i2cdev")) {
      int n = line.substring(6).toInt();
      switch (n) {
        case 1: activeWire = &Wire; break;
        case 2: activeWire = &Wire1; break;
        case 4: activeWire = &Wire2; break;
        default:
          Serial.println(F(" Nur i2cdev 1, 2 oder 4 erlaubt."));
          continue;
      }
      activeWire->begin();
      Serial.print(F(" I²C-Interface auf Wire"));
      Serial.print(n);
      Serial.println(F(" gesetzt."));
    } else {
      Serial.println(F(" Unbekannter Befehl."));
    }
  }
}
void SERIAL_passthrough() {
  Serial1.begin(9600);
  bool shouldExit;
  String inputString;
  while (!shouldExit) {
    if (Serial.available()) {        // If anything comes in Serial (USB),
      Serial1.write(Serial.read());  // read it and send it out Serial1 (pins 0 & 1)
    }

    if (Serial1.available()) {       // If anything comes in Serial1 (pins 0 & 1)
      Serial.write(Serial1.read());  // read it and send it out Serial (USB)
    }

    // --- Check ob ein neuer String empfangen wurde ---
    if (Serial.available()) {
      inputString = Serial.readStringUntil('\n');  // Liest bis Enter gedrückt wird
      inputString.trim();                          // Entfernt \r\n, Leerzeichen etc.

      if (inputString.equalsIgnoreCase("exit")) {
        shouldExit = true;
        Serial.println("Exit-Befehl erkannt. Schleife wird beendet...");
      }
    }
  }
}

void USB_INFO() {

  USBDevice* dev = usb.getDevice(0);  // Erstes erkannte Gerät
  if (dev && dev->isReady()) {
    Serial.println("USB-Gerät erkannt!");

    uint16_t vid = dev->idVendor();
    uint16_t pid = dev->idProduct();

    Serial.print("Vendor ID: 0x");
    Serial.println(vid, HEX);

    Serial.print("Product ID: 0x");
    Serial.println(pid, HEX);
  }
  }
  // === Function to register a new program ===
  // name: the string name to register under
  // func: a pointer to the function representing the program
  // returns: true if registration is successful, false if the registry is full
bool registerProgram(const char* name, ProgramFunc func) {
  
    if (programCount >= MAX_PROGRAMS) return false;

    strncpy(programNames[programCount], name, MAX_NAME_LEN);
    programFuncs[programCount] = func;
    programCount++;
    return true;
  }

  // === Initialization function to be called in setup() ===
  // Registers all available programs
void init() {
    //         (name to call the programm, the name of the funktion the programm has in this class)
    registerProgram("fpu-check", testFloatDoublePrecision);
    registerProgram("i2c", runI2C_CLI);
    registerProgram("serial-pass", SERIAL_passthrough);
    registerProgram("USB-Info", USB_INFO);
  }

  // === Utility function to list all registered programs ===
  // out: output stream to print to (default: Serial)
void listAll(Stream& out = Serial) {
    out.println(F("== Registered Programs =="));
    for (int i = 0; i < programCount; i++) {
      out.println(programNames[i]);
    }
  }

  // === Executes a program by name ===
  // name: the program name to search and execute
  // out: output stream to print logs to (default: Serial)
  // returns: true if program was found and executed, false otherwise
bool run(const char* name, Stream& out = Serial) {
    for (int i = 0; i < programCount; i++) {
      if (strcmp(programNames[i], name) == 0) {
        out.print(F("[RUNNING] "));
        out.println(name);
        programFuncs[i]();  // Call the registered function
        return true;
      }
    }
    out.print(F("[ERROR] Program not found: "));
    out.println(name);
    return false;
  }
};