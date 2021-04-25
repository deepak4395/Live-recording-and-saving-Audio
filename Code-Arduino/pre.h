#include <Preferences.h>
Preferences preferences;

void preSetup(){
  preferences.begin("my-app", false);
}
void saveCounter(){
  preferences.putUInt("counter", counter);
}
void loadCounter(){
  counter = preferences.getUInt("counter", 0);
}
