#include <mbed.h>
#include <mbed_stats.h>
#include <Arduino.h>
#include <SDRAM.h>
#include <WiFiInterface.h>
#include <SPI.h>
#include <WiFi.h>
#include "arduino_secrets.h"
#include "progs_CLI.h"
#include <Arduino_USBHostMbed5.h>
#include <FATFileSystem.h>
#include <DigitalOut.h>
#include <ECCX08.h>

// config.h
#ifndef CONFIG_H
#define CONFIG_H

// VERSION
#define FW_VERSION "0.3.1"
#define GIT_HASH "unknown"
extern "C" char* sbrk(int incr);

//Variablen:

// USB-A Port Power Enable Pin
const int USB_POWER_PIN = PA_15;

// Configuration flags
bool USBDRIVE = false;
bool USB_GENERIC = true;
bool wlan = false;
bool fixed_wifi = false;  // only connects to the wifi in the "arduino_secrest.h" header-file
bool enable_login = false;
bool startup_config_enabled = false;
bool SHA256 = true;

bool no_networks_avan = false;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

bool isAuthenticated = false;
String loginBuffer = "";

const int StatusLedPin = LED_BUILTIN;
int ledState = LOW;
unsigned long previousMillis = 0;
const long interval = 500;


// Flag to trigger Soft-Reset
bool re_setup = false;


// WiFi-credentials 
#define SECRET_SSID      ""
#define SECRET_PASS      ""

// CLI-Auth-Credentials 
#define CLI_USERNAME     "root"
#define CLI_PASSWORD     "test1234"

#endif
