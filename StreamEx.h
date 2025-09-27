#pragma once
/**
 * @file StreamEx.h
 * @brief Lightweight parsing utilities and a buffered Arduino-like I/O helper (no Stream inheritance).
 *
 * @details
 * The library exposes:
 * - A set of safe string/number parsing helpers in ::StreamEx_utility.
 * - A zero-allocation (no internal malloc/new) buffered I/O class ::StreamEx that
 *   exposes Arduino-like methods using user-supplied TX/RX buffers (no virtual base).
 *
 * Design goals:
 * - **Deterministic memory**: caller owns buffers; class never allocates.
 * - **Small footprint**: optional overloads for std::string / Arduino String can be
 *   disabled at compile time to reduce flash/RAM.
 * - **Clear errors**: operations set an ::StreamExError you can query/reset.
 */

#include <Arduino.h>      ///< Arduino core (Print/Stream base, String type, millis, etc.)
#include <stdint.h>       ///< Fixed-width integer types
#include <stddef.h>       ///< size_t, nullptr_t

/**
 * @def STREAMEX_ENABLE_STD_STRING
 * @brief Enables overloads that accept/return `std::string`.
 *
 * @note Turning this **on** can increase binary size (pulls parts of libstdc++).
 *       Keep **off** for smallest builds on MCUs.
 * @see STREAMEX_ENABLE_ARDUINO_STRING
 */
#ifndef STREAMEX_ENABLE_STD_STRING
  #define STREAMEX_ENABLE_STD_STRING 0
#endif

/**
 * @def STREAMEX_ENABLE_ARDUINO_STRING
 * @brief Enables overloads that accept/return Arduino `String`.
 *
 * @note Safer/smaller than std::string on most Arduino cores, but still uses heap.
 *       Keep **off** for fully static, lowest-risk builds.
 * @see STREAMEX_ENABLE_STD_STRING
 */
#ifndef STREAMEX_ENABLE_ARDUINO_STRING
  #define STREAMEX_ENABLE_ARDUINO_STRING 0
#endif

#if STREAMEX_ENABLE_STD_STRING
  #include <string>      ///< std::string support (optional)
#endif

/**
 * @def STREAMEX_STRING_CAP
 * @brief Capacity (including terminating NUL) of the inline scratch string buffer
 *        inside ::dataValueUnion::stringValue.
 *
 * @note Define this before including the header to customize capacity.
 */
#ifndef STREAMEX_STRING_CAP
  #define STREAMEX_STRING_CAP 32  ///< Capacity for dataValueUnion::stringValue (includes '\0')
#endif

/**
 * @enum dataTypeEnum
 * @brief Enumeration of supported scalar/string/boolean value kinds for parsing/formatting.
 */
enum dataTypeEnum
{
    noneType,   ///< No type / unspecified
    uint8Type,  ///< Unsigned 8-bit integer
    uint16Type, ///< Unsigned 16-bit integer
    uint32Type, ///< Unsigned 32-bit integer
    uint64Type, ///< Unsigned 64-bit integer
    int8Type,   ///< Signed 8-bit integer
    int16Type,  ///< Signed 16-bit integer
    int32Type,  ///< Signed 32-bit integer
    int64Type,  ///< Signed 64-bit integer
    floatType,  ///< 32-bit float
    doubleType, ///< 64-bit double
    charType,   ///< Single character
    stringType, ///< C-string (NUL-terminated)
    boolType    ///< Boolean (true/false or 0/1)
};

/**
 * @union dataValueUnion
 * @brief Holds a parsed numeric/boolean value or a small inline C-string.
 *
 * @var dataValueUnion::uint8Value  Unsigned 8-bit value
 * @var dataValueUnion::uint16Value Unsigned 16-bit value
 * @var dataValueUnion::uint32Value Unsigned 32-bit value
 * @var dataValueUnion::uint64Value Unsigned 64-bit value
 * @var dataValueUnion::int8Value   Signed 8-bit value
 * @var dataValueUnion::int16Value  Signed 16-bit value
 * @var dataValueUnion::int32Value  Signed 32-bit value
 * @var dataValueUnion::int64Value  Signed 64-bit value
 * @var dataValueUnion::floatValue  32-bit float value
 * @var dataValueUnion::doubleValue 64-bit double value
 * @var dataValueUnion::stringValue Small inline string buffer (capacity = STREAMEX_STRING_CAP)
 * @var dataValueUnion::boolValue   Boolean value
 *
 * @note `stringValue` is intended as a scratch/output buffer for small strings.
 *       For larger strings, pass an external buffer to the converters.
 */
union dataValueUnion
{
    uint8_t  uint8Value;
    uint16_t uint16Value;
    uint32_t uint32Value;
    uint64_t uint64Value;
    int8_t   int8Value;
    int16_t  int16Value;
    int32_t  int32Value;
    int64_t  int64Value;
    float    floatValue;
    double   doubleValue;
    char     stringValue[STREAMEX_STRING_CAP];
    bool     boolValue;
};

/**
 * @namespace StreamEx_utility
 * @brief Safe string utilities and type parsers/formatters used by StreamEx and user code.
 */
namespace StreamEx_utility
{
/**
 * @brief Bounded strlen for systems lacking `strnlen`.
 * @param str    Pointer to C-string (may be nullptr).
 * @param maxlen Maximum number of bytes to scan.
 * @return Number of characters before NUL (in [0..maxlen]).
 */
size_t safe_strnlen(const char* str, size_t maxlen);

/**
 * @brief ASCII tolower (unsigned-char safe).
 * @param c Input character.
 * @return Lowercased character (ASCII).
 */
char tolow(char c);

/**
 * @brief Case-insensitive equality for ASCII tokens.
 * @param a First C-string (nullable → false).
 * @param b Second C-string (nullable → false).
 * @return true if equal ignoring case; else false.
 */
bool iequal(const char* a, const char* b);

/**
 * @brief Test if string is a number of the form [+|-]?digits[.digits]? with ≥1 digit.
 * @param s C-string.
 * @return true on numeric form; false otherwise.
 */
bool isNumber(const char* s);

/**
 * @brief Test if string is an integer of the form [+|-]?digits+.
 * @param s C-string.
 * @return true if integer; false otherwise.
 */
bool isInteger(const char* s);

/**
 * @brief Test if string is an unsigned integer of the form [+]?digits+.
 * @param s C-string.
 * @return true if unsigned integer; false otherwise.
 */
bool isUInteger(const char* s);

/**
 * @brief Trim ASCII spaces from both ends of a C-string in place.
 * @param buf     Buffer containing a NUL-terminated string. If nullptr, no-op.
 * @param maxSize Optional buffer capacity. If non-zero, forces `buf[maxSize-1]=0`
 *                before trimming to guarantee termination.
 */
void trimString(char* buf, uint32_t maxSize = 0);

/**
 * @brief Function to split a string by a delimiter in two section and return a splited strings.
 * @param firstSection is the first part of splited string.
 * @param secondSection is the second part of splited string.
 * @return true if splited succeeded.
 */
bool splitString(const char* data, char delimiter, char* firstSection = nullptr, char* secondSection = nullptr);

/**
 * @brief Function to check if a string is empty or contains only spaces.
 * @return true if succeeded.
 */
bool isWhitespaceOnly(const char* data);

/**
 * @brief Function to validate a string has the expected sections that splited by ',' character.
 * @note if any sections be empty space it returns false.
 * @return true if succeeded.
 */
bool validateRow(const char* data, size_t expectedColumnCount);

// --- Typed validators ---

/** @brief Validate 0..255 range. */
bool isUInt8 (const char* s);
/** @brief Validate 0..65535 range. */
bool isUInt16(const char* s);
/** @brief Validate 32-bit unsigned integer. */
bool isUInt32(const char* s);
/** @brief Validate 64-bit unsigned integer. */
bool isUInt64(const char* s);

/** @brief Validate −128..127 range. */
bool isInt8 (const char* s);
/** @brief Validate −32768..32767 range. */
bool isInt16(const char* s);
/** @brief Validate 32-bit signed integer. */
bool isInt32(const char* s);
/** @brief Validate 64-bit signed integer. */
bool isInt64(const char* s);

/** @brief Validate float grammar/value. */
bool isFloat (const char* s);
/** @brief Validate double grammar/value. */
bool isDouble(const char* s);

/**
 * @brief Validate boolean (accepts "true"/"false" any case or "0"/"1").
 * @param s C-string.
 * @return true if boolean token; else false.
 */
bool isBoolean(const char* s);

// ----- Converters (return false on parse/range error) -----
bool stringToUint8 (const char* s, uint8_t*  out);
bool stringToUint16(const char* s, uint16_t* out);
bool stringToUint32(const char* s, uint32_t* out);
bool stringToUint64(const char* s, uint64_t* out);

bool stringToInt8 (const char* s, int8_t*  out);
bool stringToInt16(const char* s, int16_t* out);
bool stringToInt32(const char* s, int32_t* out);
bool stringToInt64(const char* s, int64_t* out);

bool stringToFloat (const char* s, float*  out);
bool stringToDouble(const char* s, double* out);

/**
 * @brief Validate that a string @p data matches the requested @p type.
 * @param data Input C-string (nullable → false).
 * @param type Desired data type.
 * @return true if value is syntactically/semantically valid for @p type.
 */
bool checkValueType(const char *data, dataTypeEnum type);

/**
 * @brief Parse a C-string to a typed numeric/boolean/string value.
 * @param str  Input C-string.
 * @param num  Output union to fill on success.
 * @param type Desired type.
 * @return true on success; false if parse/range check fails.
 */
bool stringToNumber(const char* str, dataValueUnion* num, dataTypeEnum type);

/**
 * @brief Convert a typed value into a C-string.
 * @param out     Destination buffer to write a NUL-terminated string.
 * @param outCap  Capacity of @p out in bytes (must be >0).
 * @param value   Source union to serialize.
 * @param type    Type of @p value to serialize.
 *
 * @note On truncation, output is still NUL-terminated.
 */
void dataValueToString(char *out, size_t outCap, const dataValueUnion& value, dataTypeEnum type);

} // namespace StreamEx_utility

// ###############################################################################
//                  StreamEx (Arduino-like; no Stream inheritance)
// ###############################################################################

/**
 * @enum StreamExError
 * @brief Error/status codes reported by ::StreamEx operations.
 */
enum class StreamExError : int8_t
{
  None = 0,        ///< No error
  NullData,        ///< A required data pointer was null
  BufferOverflow,  ///< Not enough free space; oldest data was truncated
  SizeZero,        ///< A zero length was passed where non-zero is required
  NotEnoughData    ///< Requested more data than available
};

/**
 * @class StreamEx
 * @brief Buffered, non-allocating I/O helper with user-owned TX/RX buffers (Arduino-like API).
 *
 * @details
 * **RX path**: Feed incoming bytes using `pushBackRxBuffer()` / `writeRxBuffer()`;
 *              consumers read via `read()/available()/peek()` (non-virtual) or the pop APIs.
 *
 * **TX path**: Writers call `write()` (Arduino-like) or `pushBackTxBuffer()`; your
 *              driver retrieves bytes to send using the pop/peek helpers (e.g., `popAllTxBuffer()`).
 *
 * The class is non-allocating and does not own the memory passed as buffers.
 */
class StreamEx 
{
  public:

    /** @brief Last error recorded by any API call. */
    StreamExError errorCode;

    /**
     * @brief Construct a StreamEx using caller-provided buffers.
     * @param txBuffer      Pointer to TX buffer (may be nullptr).
     * @param txBufferSize  Size of TX buffer in bytes (0 if none).
     * @param rxBuffer      Pointer to RX buffer (may be nullptr).
     * @param rxBufferSize  Size of RX buffer in bytes (0 if none).
     *
     * The buffers (if non-null) are zero-initialized and positions set to zero.
     */
    StreamEx(char* txBuffer = nullptr, uint32_t txBufferSize = 0, char* rxBuffer = nullptr, uint32_t rxBufferSize = 0);

    /** @brief Destructor (no ownership → no deallocation). */
    ~StreamEx();

    // ---------------- User buffer management ----------------

    /**
     * @brief Assign/replace the TX buffer.
     * @param txBuffer     Pointer to caller-owned memory (may be nullptr).
     * @param txBufferSize Size in bytes for @p txBuffer.
     *
     * Resets TX position to zero and clears the buffer if non-null.
     */
    void setTxBuffer(char* txBuffer, uint32_t txBufferSize);

    /**
     * @brief Assign/replace the RX buffer.
     * @param rxBuffer     Pointer to caller-owned memory (may be nullptr).
     * @param rxBufferSize Size in bytes for @p rxBuffer.
     *
     * Resets RX position to zero and clears the buffer if non-null.
     */
    void setRxBuffer(char* rxBuffer, uint32_t rxBufferSize);
    
    /**
     * @brief Get the configured TX buffer size in bytes.
     * @return Size of the TX buffer.
     */
    uint32_t getTxBufferSize() const { return _txBufferSize; }

    /**
     * @brief Get the configured RX buffer size in bytes.
     * @return Size of the RX buffer.
     */
    uint32_t getRxBufferSize() const { return _rxBufferSize; }

    /**
     * @brief Get the TX buffer base pointer (caller-owned memory).
     * @return Pointer to TX buffer (may be nullptr).
     */
    const char* getTxBuffer() const { return _txBuffer; }
    
    /**
     * @brief Get the RX buffer base pointer (caller-owned memory).
     * @return Pointer to RX buffer (may be nullptr).
     */
    const char* getRxBuffer() const { return _rxBuffer; }

    /**
     * @brief Clear the TX buffer content and reset the TX write position.
     * @post `availableTx()==0`.
     */
    void clearTxBuffer();

    /**
     * @brief Clear the RX buffer content and reset the RX fill position.
     * @post `availableRx()==0`.
     */
    void clearRxBuffer();

    // ---------------- High-level append / pop APIs ----------------

    /**
     * @brief Remove a number of bytes from the **front** of the TX buffer.
     * @param dataSize Number of bytes to drop.
     * @retval true  Removed exactly @p dataSize bytes.
     * @retval false Not enough data (sets error ::StreamExError::NotEnoughData).
     */
    bool removeFrontTxBuffer(uint32_t dataSize = 1);

    /**
     * @brief Remove a number of bytes from the **front** of the RX buffer.
     * @param dataSize Number of bytes to drop.
     * @retval true  Removed exactly @p dataSize bytes.
     * @retval false Not enough data (sets error ::StreamExError::NotEnoughData).
     */
    bool removeFrontRxBuffer(uint32_t dataSize = 1);

    /**
     * @brief Overwrite TX buffer with @p data (replace all TX content).
     * @param data     Pointer to bytes to copy (must be non-null if @p dataSize>0).
     * @param dataSize Number of bytes to copy.
     * @retval true  Success; TX contains exactly @p dataSize bytes.
     * @retval false Size exceeds TX capacity (sets ::StreamExError::BufferOverflow).
     *
     * @note Buffer is NUL-terminated for convenience if space allows.
     */
    bool writeTxBuffer(const char* data, uint32_t dataSize);

    /**
     * @brief Overwrite RX buffer with @p data (replace all RX content).
     * @param data     Pointer to bytes to copy (must be non-null if @p dataSize>0).
     * @param dataSize Number of bytes to copy.
     * @retval true  Success; RX contains exactly @p dataSize bytes.
     * @retval false Size exceeds RX capacity (sets ::StreamExError::BufferOverflow).
     *
     * @note Buffer is NUL-terminated for convenience if space allows.
     */
    bool writeRxBuffer(const char* data, uint32_t dataSize);

    /**
     * @brief Append bytes to the **end** of the TX buffer (sliding-window on overflow).
     * @param data     Pointer to bytes to append (must be non-null if @p dataSize>0).
     * @param dataSize Number of bytes to append.
     * @retval true  All @p dataSize bytes appended.
     * @retval false Not all bytes fit; oldest bytes were dropped
     *              (sets ::StreamExError::BufferOverflow).
     *
     * @note One byte is reserved for NUL termination when possible.
     */
    bool pushBackTxBuffer(const char* data, uint32_t dataSize = 1);

    #if STREAMEX_ENABLE_STD_STRING
      /**
       * @brief Append a std::string to TX (optional; may increase binary size).
       * @param data Pointer to std::string.
       * @retval true  Entire string appended.
       * @retval false Truncated; oldest data dropped (overflow).
       */
      bool pushBackTxBuffer(const std::string* data);
    #endif

    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Append an Arduino String to TX (optional).
       * @param s Source String.
       * @return Success status (see ::pushBackTxBuffer(const char*,uint32_t)).
       */
      bool pushBackTxBuffer(const String& s);
    #endif

    /**
     * @brief Append bytes to the **end** of the RX buffer (sliding-window on overflow).
     * @param data     Pointer to bytes to append (must be non-null if @p dataSize>0).
     * @param dataSize Number of bytes to append.
     * @retval true  All @p dataSize bytes appended.
     * @retval false Not all bytes fit; oldest bytes were dropped
     *              (sets ::StreamExError::BufferOverflow).
     *
     * @note One byte is reserved for NUL termination when possible.
     */
    bool pushBackRxBuffer(const char* data, uint32_t dataSize = 1);

    #if STREAMEX_ENABLE_STD_STRING
      /**
       * @brief Append a std::string to RX (optional).
       * @param data Pointer to std::string.
       * @retval true  Entire string appended.
       * @retval false Truncated; oldest data dropped (overflow).
       */
      bool pushBackRxBuffer(const std::string* data);
    
      /**
       * @brief Pop @p dataSize bytes from the **front** of TX into a std::string.
       * @param data     Destination std::string (must be non-null).
       * @param dataSize Number of bytes to pop; clamped to available.
       * @retval true  Exactly @p dataSize bytes were popped.
       * @retval false Fewer bytes were available (sets ::StreamExError::NotEnoughData).
       */
      bool popFrontTxBuffer(std::string* data, uint32_t dataSize = 1);
    #endif

    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Append an Arduino String to RX (optional).
       * @param s Source String.
       * @return Success status (see ::pushBackRxBuffer(const char*,uint32_t)).
       */
      bool pushBackRxBuffer(const String& s);
    #endif
    
    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Pop @p dataSize bytes from the **front** of TX into an Arduino String.
       * @param out      Destination String.
       * @param dataSize Number of bytes to pop; clamped to available.
       * @return Success status (false if not enough data; error set).
       */
      bool popFrontTxBuffer(String& out, uint32_t dataSize = 1);
    #endif

    /**
     * @brief Pop @p dataSize bytes from the **front** of the TX buffer into @p data.
     * @param data     Destination buffer (must be non-null).
     * @param dataSize Number of bytes to pop; clamped to available.
     * @retval true  Exactly @p dataSize bytes were popped.
     * @retval false Fewer bytes were available (sets ::StreamExError::NotEnoughData).
     */
    bool popFrontTxBuffer(char* data, uint32_t dataSize = 1);

    #if STREAMEX_ENABLE_STD_STRING
      /**
       * @brief Pop @p dataSize bytes from the **front** of RX into a std::string.
       * @param data     Destination std::string (must be non-null).
       * @param dataSize Number of bytes to pop; clamped to available.
       * @retval true  Exactly @p dataSize bytes were popped.
       * @retval false Fewer bytes were available (sets ::StreamExError::NotEnoughData).
       */
      bool popFrontRxBuffer(std::string* data, uint32_t dataSize = 1);
    #endif

    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Pop @p dataSize bytes from the **front** of RX into an Arduino String.
       * @param out      Destination String.
       * @param dataSize Number of bytes to pop; clamped to available.
       * @return Success status (false if not enough data; error set).
       */
      bool popFrontRxBuffer(String& out, uint32_t dataSize = 1);
    #endif

    /**
     * @brief Pop @p dataSize bytes from the **front** of the RX buffer into @p data.
     * @param data     Destination buffer (must be non-null).
     * @param dataSize Number of bytes to pop; clamped to available.
     * @retval true  Exactly @p dataSize bytes were popped.
     * @retval false Fewer bytes were available (sets ::StreamExError::NotEnoughData).
     */
    bool popFrontRxBuffer(char* data, uint32_t dataSize = 1);

    #if STREAMEX_ENABLE_STD_STRING
      /**
       * @brief Pop **all** available TX bytes into a std::string and clear TX.
       * @param data Destination std::string (must be non-null).
       * @return true on success.
       */
      bool popAllTxBuffer(std::string* data);
    #endif

    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Pop **all** available TX bytes into an Arduino String and clear TX.
       * @param out Destination String.
       * @return true on success.
       */
      bool popAllTxBuffer(String& out);
    #endif

    /**
     * @brief Pop **all** available TX bytes into @p data (up to @p maxSize) and clear TX.
     * @param data    Destination buffer (must be non-null).
     * @param maxSize Maximum number of bytes to copy.
     * @retval true  Copied all available or exactly @p maxSize bytes.
     * @retval false Invalid args (e.g., maxSize=0 → sets ::StreamExError::SizeZero).
     */
    bool popAllTxBuffer(char* data, uint32_t maxSize);

    #if STREAMEX_ENABLE_STD_STRING
      /**
     * @brief Pop **all** available RX bytes into a std::string and clear RX.
     * @param data Destination std::string (must be non-null).
     * @return true on success.
     */
      bool popAllRxBuffer(std::string* data);
    #endif

    #if STREAMEX_ENABLE_ARDUINO_STRING
      /**
       * @brief Pop **all** available RX bytes into an Arduino String and clear RX.
       * @param out Destination String.
       * @return true on success.
       */
      bool popAllRxBuffer(String& out);
    #endif

    /**
     * @brief Pop **all** available RX bytes into @p data (up to @p maxSize) and clear RX.
     * @param data    Destination buffer (must be non-null).
     * @param maxSize Maximum number of bytes to copy.
     * @retval true  Copied all available or exactly @p maxSize bytes.
     * @retval false Invalid args (e.g., maxSize=0 → sets ::StreamExError::SizeZero).
     */
    bool popAllRxBuffer(char* data, uint32_t maxSize);

    /**
     * @brief Number of valid bytes currently stored in TX.
     * @return Count of bytes available in TX buffer.
     */
    uint32_t availableTx() const { return _txPosition; }

    /**
     * @brief Number of valid bytes currently stored in RX.
     * @return Count of bytes available in RX buffer.
     */
    uint32_t availableRx() const { return _rxPosition; }

    // ---------------- Error helpers ----------------

    /**
     * @brief Reset ::errorCode to ::StreamExError::None.
     */
    void clearError() { errorCode = StreamExError::None; }

    /**
     * @brief Get the last error that occurred.
     * @return Current ::StreamExError value.
     */
    StreamExError lastError() const { return errorCode; }

    // ---------------- Arduino-like methods (non-virtual) ----------------

    /**
     * @brief Number of bytes available to read from the RX buffer.
     * @return Count of readable bytes currently buffered in RX.
     */
    int available();

    /**
     * @brief Read and remove one byte from the front of RX.
     * @return The byte (0..255) or -1 if RX is empty.
     */
    int read();

     /**
     * @brief Peek the next byte in RX without removing it.
     * @return The byte (0..255) or -1 if RX is empty.
     */
    int peek();                            

    /**
     * @brief Clear the TX buffer.
     * @details Interpreted as “TX delivered”: resets TX to empty.
     */
    void flush();                        

    /**
     * @brief Append one byte to TX.
     * @details Uses sliding-window semantics on overflow (oldest TX data is dropped).
     */
    size_t write(uint8_t b);                 

    /**
     * @brief Append a block of bytes to TX.
     * @param buffer Pointer to bytes.
     * @param size   Number of bytes.
     * @return Number of bytes accepted (equal to @p size on full success).
     * @details Sliding-window on overflow: if @p size exceeds free space, the oldest TX bytes are
     *          dropped to make room. A trailing NUL is written when capacity allows.
     */
    size_t write(const uint8_t* buffer, size_t size); 

    /**
     * @brief Convenience overload for writing string literals or C-strings.
     *
     * @param buffer Pointer to a NUL-terminated C-string (treated as raw bytes).
     * @param size   Number of bytes from @p buffer to append.
     * @return Number of bytes accepted (equal to @p size on success).
     *
     * @details
     * This overload exists to allow calls like:
     * @code
     *   myStream.write("Hello World", 11);
     * @endcode
     * without requiring an explicit cast to `const uint8_t*`.
     * Internally, it simply forwards to `write(const uint8_t*, size)`.
     */
    size_t write(const char* buffer, size_t size) 
    {
        return write(reinterpret_cast<const uint8_t*>(buffer), size);
    }

  private:

    // ---------- Raw buffers (caller-owned; no ownership here) ----------

    char*     _txBuffer      = nullptr;  ///< Base pointer to TX buffer memory.
    char*     _rxBuffer      = nullptr;  ///< Base pointer to RX buffer memory.
    uint32_t  _txBufferSize  = 0;        ///< Capacity in bytes of TX buffer.
    uint32_t  _rxBufferSize  = 0;        ///< Capacity in bytes of RX buffer.

    // ---------- Active lengths (number of valid bytes) ----------

    uint32_t  _txPosition    = 0;        ///< Current used length in TX buffer.
    uint32_t  _rxPosition    = 0;        ///< Current used length in RX buffer.

    // ---------- Internal helpers (buffer compaction) ----------

    /**
     * @brief Drop @p n bytes from TX front, compacting the remaining data to the start.
     * @param n Number of bytes to remove.
     */
    void _dropFrontTx(uint32_t n);

    /**
     * @brief Drop @p n bytes from RX front, compacting the remaining data to the start.
     * @param n Number of bytes to remove.
     */
    void _dropFrontRx(uint32_t n);
};

