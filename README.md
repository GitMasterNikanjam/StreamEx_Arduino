# StreamEx

A **lightweight buffered I/O helper** and **string/number utility toolkit** for Arduino and embedded C++.
Unlike `Stream`, this library does **not** rely on inheritance or virtual functions.
You provide the TX/RX buffers, and `StreamEx` manages them deterministically — no hidden heap allocation.

---

## ✨ Features

* **Zero dynamic allocation**: Caller owns buffers, no `malloc/new` inside.
* **Arduino-like API**: `write()`, `read()`, `available()`, `peek()`, `flush()`.
* **Deterministic sliding-window buffers**: On overflow, oldest data is dropped.
* **Safe parsing utilities**:

  * Check if strings are numbers, integers, booleans, etc.
  * Convert strings into typed numbers (`uint8_t`, `int32_t`, `float`, …).
  * Format values back to strings.
* **Clear error reporting**: Each API sets a `StreamExError`.
* **Optional overloads** for `std::string` and Arduino `String` (compile-time flags).
* **Convenience overloads** for writing C-string literals without casts.

---

## 📦 Installation

1. Copy or clone this repository into your Arduino `libraries/` folder:

   ```bash
   cd ~/Documents/Arduino/libraries
   git clone https://github.com/yourname/StreamEx_Arduino.git
   ```
2. Restart the Arduino IDE.
3. You should now see **StreamEx** available in **File → Examples**.

---

## ⚙️ Configuration

At the top of `StreamEx.h`, you can toggle features:

```cpp
#define STREAMEX_ENABLE_STD_STRING     0   // enable std::string overloads
#define STREAMEX_ENABLE_ARDUINO_STRING 0   // enable Arduino String overloads
#define STREAMEX_STRING_CAP           32   // capacity of inline stringValue buffer
```

Keep both string overloads **off** for the smallest builds on MCUs.

---

## 🚀 Basic Example

```cpp
#include <StreamEx.h>

char txBuf[64];
char rxBuf[64];

// Create a StreamEx with caller-provided buffers
StreamEx myStream(txBuf, sizeof(txBuf), rxBuf, sizeof(rxBuf));

void setup() {
  Serial.begin(115200);

  // Write to TX buffer
  myStream.write("Hello TX World!", 15);

  // Pop data out of TX into Serial
  char out[32];
  if (myStream.popAllTxBuffer(out, sizeof(out))) {
    Serial.print("Popped TX: ");
    Serial.println(out);
  }

  // Push incoming data into RX
  myStream.pushBackRxBuffer("Incoming RX Data", 16);

  // Read back RX one byte at a time
  while (myStream.available()) {
    int b = myStream.read();
    Serial.write(b);
  }
}

void loop() {
  // Nothing here
}
```

Output:

```
Popped TX: Hello TX World!
Incoming RX Data
```

---

## 🧰 Utility Example

```cpp
#include <StreamEx.h>
using namespace StreamEx_utility;

void setup() {
  Serial.begin(115200);

  const char* s = "12345";
  if (isUInt16(s)) {
    uint16_t v;
    if (stringToUint16(s, &v)) {
      Serial.print("Parsed: ");
      Serial.println(v);
    }
  }

  dataValueUnion val;
  val.int32Value = -42;
  char buf[16];
  dataValueToString(buf, sizeof(buf), val, int32Type);
  Serial.print("Formatted: ");
  Serial.println(buf);
}

void loop() {}
```

Output:

```
Parsed: 12345
Formatted: -42
```

---

## 📖 API Highlights

### Error Codes (`StreamExError`)

* `None` – No error.
* `NullData` – Null pointer passed.
* `BufferOverflow` – Not enough room; oldest data truncated.
* `SizeZero` – Zero size passed where >0 required.
* `NotEnoughData` – Requested more than available.

### Key Methods

* `write(const uint8_t*, size_t)` – Append bytes to TX.
* `write(const char*, size_t)` – **Convenience overload for string literals**.
* `available()` – Bytes available in RX.
* `read()` – Pop one byte from RX.
* `peek()` – Inspect first RX byte without removing.
* `pushBackTxBuffer(...)`, `pushBackRxBuffer(...)` – Append to buffers.
* `popAllTxBuffer(...)`, `popAllRxBuffer(...)` – Drain buffers.
* `clearTxBuffer()`, `clearRxBuffer()` – Reset buffers.

---

## 🔧 Design Notes

* No `Stream` inheritance → no vtable, smaller code, deterministic.
* Caller fully controls buffer memory and lifetime.
* Internally uses `memcpy`/`memmove` for efficient shifting.
* Overflow automatically drops **oldest** data (sliding window).
* Compatible with **Arduino Uno, Mega, Due, ESP32, STM32**, and any C++11/14 toolchain.

---

## 📜 License

MIT License – free to use, modify, and distribute.

---
