#include "Kernel-Command-fech_run.cpp"
#include "config-inlcude.h"

class KernelCommand {
public:
  void update();

private:
  static const int BUFFER_SIZE = 200;
  static const int MAX_ITEMS = 200;
  char serialIn[BUFFER_SIZE];
  int serialIndex = 0;
  String receivedMessage;
  String keys[MAX_ITEMS];
  String values[MAX_ITEMS];
  bool echoEnabled = false;

  void readSerialData();
  void processMessage();
  void storeKeyValue();
  void getKeyValue();
  void printHelp();
  void playTone();
  void handleFileCommand();
  int getFreeMemory();
  void showSysInfo();
  void showUptime();



};