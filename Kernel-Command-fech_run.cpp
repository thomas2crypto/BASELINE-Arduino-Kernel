

class KernelCommand {
public:
  /**
     * Führt einen einzelnen Befehls-String aus und liefert die Antwort als String zurück.
     * Voraussetzung: Serial.begin(...) wurde bereits in setup() aufgerufen.
     */
  String Setup_Dynamik(const String& command) {
    String output;

    if (command == "Start-USB-Mass") {
      output += "USB Mass Storage Initialization started...\n";

      pinMode(USB_POWER_PIN, OUTPUT);
      digitalWrite(USB_POWER_PIN, HIGH);
      output += "USB-A port powered on.\n";

      output += "Attempting to connect to USB mass storage...\n";
      unsigned long startTime = millis();
      while (!msd.connect()) {
        if (millis() - startTime > 10000) {  // Timeout nach 10 Sekunden
          output += "Error: USB drive not found, timeout.\n";
          return output;
        }
        delay(500);
      }

      output += "Mounting USB drive... ";
      int err = usb.mount(&msd);
      if (err) {
        output += "Mount error: " + String(err) + "\n";
        return output;
      }
      output += "successful.\n";

      DIR* test = opendir("/usb");
      if (test) {
        output += "Directory /usb opened successfully.\n";
        closedir(test);
      } else {
        output += "ERROR: Unable to open /usb directory!\n";
      }
    } else if (command == "Enable-WLAN") {
      output += "WiFi Initialization started...\n";

      if (WiFi.status() == WL_NO_MODULE) {
        output += "Communication with WiFi module failed!\n";
        return output;
      }
      if (fixed_wifi) {
        WiFi.setHostname("GIGA_R1");
        unsigned long startTime = millis();
        while (status != WL_CONNECTED) {
          if (millis() - startTime > 15000) {  // 15 Sekunden Timeout
            output += "Error: Timeout connecting to WiFi\n";
            return output;
          }
          output += "Connecting to SSID: " + ssid + "\n";
          status = WiFi.begin(ssid, pass);
          delay(3000);
        }
      } else {
        connectToWiFi();
      }
      if (no_networks_avan) {
        output += "Network connected.\n";
        output += getCurrentNetworkString();  // Muss von dir definiert werden, z.B. als String-Rückgabe-Funktion
        output += getWifiDataString();        // idem
      }
    } else if (command == "Enable-USB-Host") {
      output += "Starting USB Host...\n";
      // USB Host initialisieren hier...
    } else if (command == "Enable-Login") {
      output += "Login enabled. Starting authentication...\n";
      while (!isAuthenticated) {
        output += "Please enter password:\n";
        String loginBuffer = "";

        // Achtung: Bei interner String-Rückgabe ist hier serielles Warten schwierig.
        // Du müsstest das eventuell asynchron regeln oder auslagern.
        // Für den Demo-Case:
        while (true) {
          if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
              loginBuffer.trim();
              if (loginBuffer == CLI_PASSWORD) {
                isAuthenticated = true;
                output += "Access granted. Welcome, " CLI_USERNAME ".\n";
                break;
              } else {
                output += "Incorrect password. Please try again:\n";
                loginBuffer = "";
              }
            } else {
              loginBuffer += c;
            }
          }
        }
      }
    } else {
      output += "Unknown initialization command: " + command + "\n";
    }

    return output;
  }

  String executeCommand(const String& rawMessage) {
    String message = rawMessage;
    message.trim();
    message.toUpperCase();

    String output;

    if (message.startsWith("SET ")) {
      output = cmdSet(message);

    } else if (message.startsWith("GET ")) {
      output = cmdGet(message);

    } else if (message.startsWith("PIN ON ")) {
      output = cmdPinOn(message);

    } else if (message.startsWith("PIN OFF ")) {
      output = cmdPinOff(message);

    } else if (message.startsWith("READ DI")) {
      output = cmdReadDI(message);

    } else if (message.startsWith("ANALOG ")) {
      output = cmdAnalog(message);

    } else if (message == "HELP") {
      output = cmdHelp();

    } else if (message.startsWith("REPEAT ")) {
      output = cmdRepeat(message);

    } else if (message.startsWith("BLINK ")) {
      output = cmdBlink(message);

    } else if (message.startsWith("TONE ")) {
      output = cmdTone(message);

    } else if (message == "CLEAR") {
      output = cmdClear();

    } else if (message.startsWith("ECHO")) {
      output = cmdEcho(message);

    } else if (message == "PING") {
      output = cmdPing();

    } else if (message == "IP") {
      output = cmdIP();

    } else if (message == "MAC") {
      output = cmdMAC();

    } else if (message == "RSSI") {
      output = cmdRSSI();

    } else if (message == "GATEWAY") {
      output = cmdGateway();

    } else if (message == "DNS") {
      output = cmdDNS();

    } else if (message == "STATUS") {
      output = cmdStatus();

    } else if (message.startsWith("FILE ")) {
      output = cmdFile(message);

    } else if (message == "RESTART") {
      re_setup = true;
      output = "System is restarting...";

    } else if (message == "EN-WIFI") {
      output = cmdToggle("wlan");

    } else if (message == "EN-FIXED-WIFI") {
      output = cmdToggle("fixed_wifi");

    } else if (message == "EN-USB-MASS") {
      output = cmdToggle("USBDRIVE");

    } else if (message == "EN-USB-GENERIC") {
      output = cmdToggle("USB_GENERIC");

    } else if (message == "EN-STARTUP-CONFIG") {
      output = cmdToggle("startup_config_enabled");

    } else if (message == "SHOW-CONFIG") {
      output = cmdShowConfig();

    } else if (message == "SYSINFO") {
      output = cmdSysInfo();

    } else if (message == "UPTIME") {
      output = cmdUptime();

    } else if (message == "LIST-PROGS") {
      output = cmdListProgs();

    } else if (message.startsWith("RUN ")) {
      output = cmdRun(message);

    } else {
      output = "Unknown command: " + message + "\nType 'HELP' for assistance.";
    }

    return output;
  }

private:
  // Key/Value Storage
  static const int MAX_ITEMS = 20;
  String keys[MAX_ITEMS] = { "" };
  String values[MAX_ITEMS] = { "" };

  // Flags
  bool re_setup = false;
  bool wlan = true;
  bool fixed_wifi = false;
  bool USBDRIVE = false;
  bool USB_GENERIC = false;
  bool startup_config_enabled = false;
  bool SHA256 = false;

  // Helper-Methoden für jede Command-Gruppe
  String cmdSet(const String& msg) {
    int keyStart = 4;
    int keyEnd = msg.indexOf(' ', keyStart);
    int valueStart = keyEnd + 1;
    if (keyEnd > 0 && valueStart < msg.length()) {
      String key = msg.substring(keyStart, keyEnd);
      String val = msg.substring(valueStart);
      return storeKeyValue(key, val);
    }
    return "Syntax error in SET command.";
  }

  String cmdGet(const String& msg) {
    String key = msg.substring(4);
    return getKeyValue(key);
  }

  String cmdPinOn(const String& msg) {
    int pin = msg.substring(7).toInt();
    digitalWrite(pin, HIGH);
    return "Pin " + String(pin) + " set to HIGH";
  }
  String cmdPinOff(const String& msg) {
    int pin = msg.substring(8).toInt();
    digitalWrite(pin, LOW);
    return "Pin " + String(pin) + " set to LOW";
  }
  String cmdReadDI(const String& msg) {
    int pin = msg.substring(8).toInt();
    return digitalRead(pin) ? "HIGH" : "LOW";
  }
  String cmdAnalog(const String& msg) {
    int pinStart = 7;
    int pinEnd = msg.indexOf(' ', pinStart);
    int valStart = pinEnd + 1;
    if (pinEnd > 0 && valStart < msg.length()) {
      int pin = msg.substring(pinStart, pinEnd).toInt();
      int val = msg.substring(valStart).toInt();
      if (val >= 0 && val <= 255) {
        analogWrite(pin, val);
        return "Analog value " + String(val) + " written to pin " + String(pin);
      }
      return "Invalid value. Allowed range: 0 - 255.";
    }
    return "Syntax error. Usage: ANALOG <PIN> <VALUE>";
  }

  String cmdHelp() {
    // Gleicher Text wie zuvor
    String help =
      "========================================\n"
      "          Available Commands         \n"
      "========================================\n"
      "[ System ]\n"
      "  HELP                - Displays this help message\n"
      "  SYSINFO             - Shows information about the system\n"
      "  UPTIME              - Shows the current uptime\n"
      "  RESTART             - Reinitializes system without full reboot\n"
      "  STATUS              - Shows WiFi connection status\n\n"
      "[ WiFi & Network ]\n"
      "  EN-WIFI             - Enables WiFi functionality\n"
      "  EN-FIXED-WIFI       - Connect always to a specified network\n"
      "  IP                  - Shows IP address of current WiFi\n"
      "  MAC                 - Shows MAC address of current WiFi\n"
      "  RSSI                - Shows signal strength of current WiFi\n"
      "  DNS                 - Shows current DNS server (usually router)\n\n"
      "[ USB & Storage ]\n"
      "  EN-USB-MASS         - Enables support for USB thumb drives\n"
      "  EN-USB-GENERIC      - Enables support for Generic-USB Devices\n"
      "  FILE               - Access USB-Massstorage device\n\n"
      "[ Data Storage ]\n"
      "  SET <KEY> <VALUE>   - Stores data\n"
      "  GET <KEY>           - Reads data\n\n"
      "[ Programs & Scripts ]\n"
      "  LIST-PROGS          - Lists all available programs\n"
      "  RUN <name>          - Runs a program\n"
      "  REPEAT <N> <TEXT>   - Repeats text N times\n\n"
      "[ Pins & IO ]\n"
      "  PIN ON/OFF <N>      - Turns pin ON or OFF\n"
      "  READ DI <PIN>       - Reads digital input pin\n"
      "  ANALOG <PIN> <VAL>  - Sets PWM (0-255) on pin\n"
      "  TONE <PIN> <FREQ> <DUR(ms)> - Plays tone on pin\n\n"
      "[ Miscellaneous ]\n"
      "  ECHO <String>       - Returns the input string\n"
      "  CLEAR               - Clears the command line\n"
      "  SHA256 <String>     - SHA256 hash with ECCX08 chip\n\n"
      "========================================\n"
      "  GitHub: https://github.com/thomas2crypto/OS-for-the-Arduino-Giga-R1-Wifi-\n"
      "========================================\n";
    return help;
  }

  String cmdRepeat(const String& msg) {
    int spacePos = msg.indexOf(' ', 7);
    int count = msg.substring(7, spacePos).toInt();
    String txt = msg.substring(spacePos + 1);
    String out;
    for (int i = 0; i < count; ++i) out += txt + "\n";
    return out;
  }
  String cmdBlink(const String& msg) {
    int pin = msg.substring(7, 9).toInt();
    int duration = msg.substring(10).toInt();
    digitalWrite(pin, HIGH);
    delay(duration);
    digitalWrite(pin, LOW);
    delay(duration);
    return "Pin " + String(pin) + " blinked for " + String(duration) + " ms.";
  }
  String cmdTone(const String& msg) {
    int sp1 = msg.indexOf(' ', 5);
    int sp2 = msg.indexOf(' ', sp1 + 1);
    if (sp1 > 0 && sp2 > 0) {
      int pin = msg.substring(5, sp1).toInt();
      int freq = msg.substring(sp1 + 1, sp2).toInt();
      int dur = msg.substring(sp2 + 1).toInt();
      tone(pin, freq, dur);
      delay(dur);
      noTone(pin);
      return "Tone on pin " + String(pin) + " @" + String(freq) + "Hz for " + String(dur) + "ms";
    }
    return "Syntax: TONE <PIN> <FREQUENCY> <DURATION(ms)>";
  }
  String cmdClear() {
    String out;
    for (int i = 0; i < 100; ++i) out += "\n";
    return out;
  }
  String cmdEcho(const String& msg) {
    return msg.substring(4);
  }

  // Netzwerkbefehle
  String cmdPing() {
    return (WiFi.status() == WL_CONNECTED) ? "PING OK - IP: " + WiFi.localIP().toString() : "PING FAIL - WiFi disconnected";
  }
  String cmdIP() {
    return "IP Address: " + WiFi.localIP().toString();
  }
  String cmdMAC() {
    return "MAC Address: " + WiFi.macAddress();
  }
  String cmdRSSI() {
    return "Signal Strength: " + String(WiFi.RSSI()) + " dBm";
  }
  String cmdGateway() {
    return "Gateway: " + WiFi.gatewayIP().toString();
  }
  String cmdDNS() {
    return "DNS: " + WiFi.dnsIP().toString();
  }
  String cmdStatus() {
    return "Status: " + String((WiFi.status() == WL_CONNECTED) ? "Connected" : "Not connected") + ", IP: " + WiFi.localIP().toString();
  }

  // FILE-Befehle
  String cmdFile(const String& msg) {
    String args = msg.substring(5);
    args.trim();
    if (args == "LIST") return fileList();
    if (args.startsWith("READ ")) return fileRead(args.substring(5));
    if (args.startsWith("WRITE ")) {
      int sp = args.indexOf(' ', 6);
      if (sp < 0) return "Syntax error: FILE WRITE <filename> <text>";
      return fileWrite(args.substring(6, sp), args.substring(sp + 1));
    }
    if (args.startsWith("DELETE ")) return fileDelete(args.substring(7));
    return "Invalid FILE command.";
  }
  String fileList() {
    String out;
    DIR* d = opendir("/usb");
    if (!d) return "Error: Unable to open directory.";
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
      if (!(e->d_type & DT_DIR)) out += String(e->d_name) + "\n";
    }
    closedir(d);
    return out;
  }
  String fileRead(const String& fn) {
    String full = "/usb" + fn;
    FILE* f = fopen(full.c_str(), "r");
    if (!f) return "Error: File not found - " + fn;
    String out;
    char buf[129];
    size_t r;
    while ((r = fread(buf, 1, 128, f)) > 0) {
      buf[r] = 0;
      out += String(buf);
    }
    fclose(f);
    return out;
  }
  String fileWrite(const String& fn, const String& txt) {
    FILE* f = fopen((String("/usb") + fn).c_str(), "w");
    if (!f) return "Error: Unable to open file for writing - " + fn;
    fwrite(txt.c_str(), 1, txt.length(), f);
    fclose(f);
    return "File written: " + fn;
  }
  String fileDelete(const String& fn) {
    String full = "/usb" + fn;
    return (remove(full.c_str()) == 0) ? "File deleted: " + fn : "Error: Failed to delete - " + fn;
  }

  // Toggle-Flags
  String cmdToggle(const String& name) {
    if (name == "wlan") wlan = !wlan;
    else if (name == "fixed_wifi") fixed_wifi = !fixed_wifi;
    else if (name == "USBDRIVE") USBDRIVE = !USBDRIVE;
    else if (name == "USB_GENERIC") USB_GENERIC = !USB_GENERIC;
    else if (name == "startup_config_enabled") startup_config_enabled = !startup_config_enabled;
    return name + (getFlag(name) ? " enabled, please RESTART" : " disabled");
  }
  bool getFlag(const String& name) {
    if (name == "wlan") return wlan;
    if (name == "fixed_wifi") return fixed_wifi;
    if (name == "USBDRIVE") return USBDRIVE;
    if (name == "USB_GENERIC") return USB_GENERIC;
    if (name == "startup_config_enabled") return startup_config_enabled;
    return false;
  }

  String cmdShowConfig() {
    return "Current-Configuration:\n" + " Wifi: " + String(wlan ? "enabled" : "disabled") + "\n" + " USB: " + String(USBDRIVE ? "enabled" : "disabled") + "\n" + " Startup-Config: " + String(startup_config_enabled ? "true" : "false");
  }

  // System & Uptime
  int getFreeMemory() {
    mbed_stats_heap_t stats;
    mbed_stats_heap_get(&stats);
    return stats.reserved_size - stats.current_size;
  }
  String cmdSysInfo() {
    String out = "System Information:\n";
    out += "Board         : Arduino GIGA R1 WiFi\n";
    out += "Firmware Ver. : 0.2.1\n";
    out += String("Build Date    : ") + __DATE__ + " " + __TIME__ + "\n";
    out += String("Git Commit    : ") + GIT_HASH + "\n";
    out += String("CPU Clock     : ") + String(SystemCoreClock / 1000000) + " MHz\n";
    out += String("Free RAM      : ") + String(getFreeMemory() / 1024) + " KB\n";
    return out;
  }
  String cmdUptime() {
    unsigned long ms = millis();
    unsigned long s = ms / 1000;
    unsigned days = s / 86400;
    unsigned hours = (s / 3600) % 24;
    unsigned mins = (s / 60) % 60;
    unsigned secs = s % 60;
    return String("System Uptime: ") + String(days) + "d " + String(hours) + "h " + String(mins) + "m " + String(secs) + "s";
  }

  String cmdListProgs() {
    // Programms::listAll packen wir in einen Buffer
    // Hier vereinfachtes Beispiel
    String out;
    Programms::listAll([&](const char* name) {
      out += String(name) + "\n";
    });
    return out;
  }
  String cmdRun(const String& msg) {
    String pname = msg.substring(4);
    pname.trim();
    if (pname.length() == 0) return "[ERROR] Provide program name";
    pname.toLowerCase();
    // Programms::run gibt keine String zurück, wir fangen Ausgaben nicht ab
    Programms::run(pname.c_str(), Serial);
    return "Program started: " + pname;
  }

  // Key/Value intern
  String storeKeyValue(const String& k, const String& v) {
    for (int i = 0; i < MAX_ITEMS; i++) {
      if (keys[i] == k || keys[i].length() == 0) {
        keys[i] = k;
        values[i] = v;
        return "Stored: " + k + " = " + v;
      }
    }
    return "Storage full!";
  }
  String getKeyValue(const String& k) {
    for (int i = 0; i < MAX_ITEMS; i++) {
      if (keys[i] == k) return k + " = " + values[i];
    }
    return "Key not found.";
  }
};
