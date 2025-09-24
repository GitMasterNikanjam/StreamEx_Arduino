#include <StreamEx.h>
using namespace StreamEx;

void setup() {
  Serial.begin(115200);
  dataValueUnion v{};
  char s[] = "  -42  ";
  trimString(s, sizeof(s));
  if (checkValueType(s, int16Type) && stringToNumber(s, &v, int16Type)) {
    char out[16];
    dataValueToString(out, sizeof(out), v, int16Type);
    Serial.println(out); // -> -42
  }
}
void loop() {}
