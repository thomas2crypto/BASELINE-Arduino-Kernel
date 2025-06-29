#include <Arduino.h>
#include <string.h>
#include <Wire.h>
#include <vector>
#include <ECCX08.h>
#include "progs_CLI.cpp"

namespace Programms {

// === Configuration constants ===
const int MAX_PROGRAMS = 20;  // Maximum number of programs that can be registered
const int MAX_NAME_LEN = 20;  // Maximum length of a program name

// === Type definition ===
typedef void (*ProgramFunc)();  // Function pointer type for programs

// === Internal storage for registered programs ===
static char programNames[MAX_PROGRAMS][MAX_NAME_LEN];  // Array to hold program names
static ProgramFunc programFuncs[MAX_PROGRAMS];         // Array to hold function pointers
static int programCount = 0;                           // Counter for how many programs are registered

void testFloatDoublePrecision();
void runI2C_CLI();
void SERIAL_passthrough();
void USB_INFO();
bool registerProgram();
void init();
void listAll();
bool run();
};