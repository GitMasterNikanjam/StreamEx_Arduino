#pragma once
/**
 * @file StreamEx.h
 * @brief Lightweight typed parsing helpers + a simple Stream-backed TX/RX buffer.
 *
 * This class can be used in two ways:
 *   1) As a utility namespace (StreamEx_utility) for validating/parsing C-strings.
 *   2) As a concrete Arduino Stream (StreamEx): RX = incoming bytes you feed,
 *      TX = bytes written via Stream::write() that you can later read/flush out.
 *
 * @note Define STREAMEX_STRING_CAP before including this header to adjust the
 *       inline small-string capacity used in dataValueUnion (default = 32).
 */

#include <Arduino.h>
#include <Stream.h>
#include <stdint.h>
#include <stddef.h>
#include <string>      // required for std::string

#ifndef STREAMEX_STRING_CAP
  #define STREAMEX_STRING_CAP 32  ///< Capacity for dataValueUnion::stringValue (includes '\0')
#endif

/**
 * @enum dataTypeEnum
 * @brief Value type identifier for conversions & validation.
 */
enum dataTypeEnum
{
    noneType,
    uint8Type,
    uint16Type,
    uint32Type,
    uint64Type,
    int8Type,
    int16Type,
    int32Type,
    int64Type,
    floatType,
    doubleType,
    charType,
    stringType,
    boolType
};

/**
 * @union dataValueUnion
 * @brief Union to store various scalar types and a small inline string.
 * @note stringValue is a small scratch buffer; for longer strings, use your own buffer.
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

namespace StreamEx_utility
{
/**
 * @brief Bounded strlen for environments lacking strnlen.
 * @param str     C-string (nullable).
 * @param maxlen  Max bytes to scan.
 * @return length in [0..maxlen]
 */
size_t safe_strnlen(const char* str, size_t maxlen);

/** @brief ASCII tolower (unsigned char safe). */
char tolow(char c);

/** @brief Case-insensitive equality for ASCII tokens. */
bool iequal(const char* a, const char* b);

/** @brief True if s is [+|-]?digits[.digits]? (at least one digit). */
bool isNumber(const char* s);

/** @brief True if s is [+|-]?digits+ */
bool isInteger(const char* s);

/** @brief True if s is [+]?digits+ (no minus). */
bool isUInteger(const char* s);

/**
 * @brief Trim ASCII spaces in-place from both ends.
 * @param buf      Buffer containing NUL-terminated text.
 * @param maxSize  Optional capacity; if non-zero ensures buf[maxSize-1]='\0' first.
 */
void trimString(char* buf, uint32_t maxSize = 0);

// ----- Typed validators -----
bool isUInt8 (const char* s);
bool isUInt16(const char* s);
bool isUInt32(const char* s);
bool isUInt64(const char* s);

bool isInt8 (const char* s);
bool isInt16(const char* s);
bool isInt32(const char* s);
bool isInt64(const char* s);

bool isFloat (const char* s);
bool isDouble(const char* s);

/** @brief Accepts "true"/"false" (any case) or "0"/"1". */
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
 * @brief Validate that `data` matches `type`.
 */
bool checkValueType(const char *data, dataTypeEnum type);

/**
 * @brief Parse C-string to typed value.
 * @param str   Input text.
 * @param num   Output union to fill.
 * @param type  Desired type.
 * @return true on success, false on parse/range error.
 */
bool stringToNumber(const char* str, dataValueUnion* num, dataTypeEnum type);

/**
 * @brief Convert typed value to string.
 * @param out    Destination buffer.
 * @param outCap Capacity of @p out (must be > 0).
 * @param value  Source union.
 * @param type   Value type.
 */
void dataValueToString(char *out, size_t outCap,
                       const dataValueUnion& value, dataTypeEnum type);

} // namespace StreamEx

// ###############################################################################
//                              StreamEx (concrete Stream)
// ###############################################################################

/** @brief Error codes for StreamEx operations. */
enum class StreamExError : int8_t {
  None = 0,
  NullData,          ///< Provided data pointer was null.
  BufferOverflow,    ///< Not enough free space (content truncated from front).
  SizeZero,          ///< Requested size was zero (invalid).
  NotEnoughData      ///< Tried to pop/remove more than available.
};

class StreamEx : public Stream
{
  public:

    /// @brief Last error recorded by any API.
    StreamExError errorCode;

    /**
     * @brief Constructor. Initializes sizes and (optionally) supplied buffers.
     * @param txBuffer      Transmit buffer pointer (may be nullptr).
     * @param txBufferSize  Transmit buffer size.
     * @param rxBuffer      Receive buffer pointer (may be nullptr).
     * @param rxBufferSize  Receive buffer size.
     *
     * @note Buffers are **not** allocated internally. You own the memory.
     *       If non-null, they will be zeroed and used immediately.
     */
    StreamEx(char* txBuffer = nullptr, uint32_t txBufferSize = 0, char* rxBuffer = nullptr, uint32_t rxBufferSize = 0);

    /**
     * Destructor.
     */
    ~StreamEx();

    // ---------------- User buffer management ----------------

    /**
     * @brief Set transmit buffer.
     * @param txBuffer: Transmit buffer pointer.
     * @param txBufferSize: Transmit buffer size.
     */
    void setTxBuffer(char* txBuffer, uint32_t txBufferSize);

    /**
     * @brief Set receive buffer.
     * @param rxBuffer: Recieve buffer pointer.
     * @param rxBufferSize: Recieve buffer size.
     */
    void setRxBuffer(char* rxBuffer, uint32_t rxBufferSize);
    
    /**
     * @brief Return TxBuffer size.
     */
    uint32_t getTxBufferSize() const { return _txBufferSize; }

    /**
     * @brief Return RxBuffer size.
     */
    uint32_t getRxBufferSize() const { return _rxBufferSize; }

    /**
     * @brief Return TxBuffer pointer.
     */
    const char* getTxBuffer() const { return _txBuffer; }
    
    /**
     * @brief Return RxBuffer pointer.
     */
    const char* getRxBuffer() const { return _rxBuffer; }

    /**
     * @brief Clear all data on the TxBuffer.
     */
    void clearTxBuffer();

    /**
     * @brief Clear all data on the RxBuffer.
     */
    void clearRxBuffer();

    // ---------------- High-level append / pop APIs ----------------

    /**
     * @brief Remove certain number elements from front of TX buffer.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Not enough data in the buffer to remove"
     *  */
    bool removeFrontTxBuffer(uint32_t dataSize = 1);

    /**
     * @brief Remove certain number elements from front of RX buffer.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Not enough data in the buffer to remove"
     *  */
    bool removeFrontRxBuffer(uint32_t dataSize = 1);

    // /**
    //  * @brief Write data Tx buffer.
    //  * @note - Clear any previouse data on Tx buffer.
    //  * @return true if succeeded.
    //  * @note - Error code be 1 if: "Error Stream: Data size exceeds TX buffer size."
    //  */
    // bool writeTxBuffer(const char* data, uint32_t dataSize);

    // /**
    //  * @brief Write data Rx buffer.
    //  * @note - Clear any previouse data on Rx buffer.
    //  * @return true if succeeded.
    //  * @note - Error code be 1 if: "Error Stream: Data size exceeds RX buffer size."
    //  */
    // bool writeRxBuffer(const char* data, uint32_t dataSize);

    /**
     * @brief Push back certain number charecter in to TxBuffer.
     * @param data is a char array pointer that you want to push back.
     * @param dataSize is the data length for push back.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Error Stream: TX Buffer Overflow"
     */
    bool pushBackTxBuffer(const char* data, uint32_t dataSize = 1);

    /**
     * @brief Push back certain number charecter in to TxBuffer.
     * @param data is a string pointer that you want to push back.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Error Stream: TX Buffer Overflow"
     */
    bool pushBackTxBuffer(const std::string* data);

    /**
     * @brief Push back certain number charecter in to RxBuffer.
     * @param data is a char array pointer that you want to push back.
     * @param dataSize is the data length for push back.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Error Stream: RX Buffer Overflow"
     */
    bool pushBackRxBuffer(const char* data, uint32_t dataSize = 1);

    /**
     * @brief Push back certain number charecter in to RxBuffer.
     * @param data is a string pointer that you want to push back.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Error Stream: RX Buffer Overflow"
     */
    bool pushBackRxBuffer(const std::string* data);

    /**
     * @brief Pop certain number elements from front of TX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Not enough data in the buffer to pop"
     *  */
    bool popFrontTxBuffer(std::string* data, uint32_t dataSize = 1);

    /**
     * @brief Pop certain number elements from front of TX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Not enough data in the buffer to pop"
     *  */
    bool popFrontTxBuffer(char* data, uint32_t dataSize = 1);

    /**
     * @brief Pop certain number elements from front of RX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Not enough data in the buffer to pop"
     *  */
    bool popFrontRxBuffer(std::string* data, uint32_t dataSize = 1);

    /**
     * @brief Pop certain number elements from front RX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     * @note - Error code be 2 if: "Not enough data in the buffer to pop"
     *  */
    bool popFrontRxBuffer(char* data, uint32_t dataSize = 1);

    /**
     * @brief Pop all elements from front of TX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     *  */
    bool popAllTxBuffer(std::string* data);

    /**
     * @brief Pop all elements from front of TX buffer and remove them.
     * @param data: is the string that poped front.
     * @param maxSize: is the max length that can poped from front of buffer.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     *  */
    bool popAllTxBuffer(char* data, uint32_t maxSize);

    /**
     * @brief Pop all elements from front of RX buffer and remove them.
     * @param data is the string that poped front.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     *  */
    bool popAllRxBuffer(std::string* data);

    /**
     * @brief Pop all elements from front of RX buffer and remove them.
     * @param data: is the string that poped front.
     * @param maxSize: is the max length that can poped from front of buffer.
     * @return true if succeeded.
     * @note - Error code be 1 if: "Error Stream: data can not be null."
     *  */
    bool popAllRxBuffer(char* data, uint32_t maxSize);

    /**
     * @brief Return data length on TxBuffer.
     */
    uint32_t availableTx() const { return _txPosition; }

    /**
     * @brief Return data length on RxBuffer.
     */
    uint32_t availableRx() const { return _rxPosition; }

    // ---------------- Error helpers ----------------
    void          clearError() { errorCode = StreamExError::None; }
    StreamExError lastError() const { return errorCode; }

    // ---------------- Arduino Stream overrides (concrete implementation) ----------------
    // RX side is what readers consume via Stream API:
    int     available() override;                       ///< Bytes available to read from RX buffer
    int     read() override;                            ///< Pop 1 byte from RX front (or -1 if empty)
    int     peek() override;                            ///< Peek 1st RX byte (or -1 if empty)
    void    flush() override;                           ///< No-op for RX; clears TX by convention

    // TX side is what writers produce via Stream API:
    size_t  write(uint8_t b) override;                  ///< Append 1 byte to TX buffer
    size_t  write(const uint8_t* buffer, size_t size) override;  ///< Append N bytes to TX buffer

    using Print::write; // bring in other Print::write overloads

  private:

    // User-supplied raw buffers (not owned)
    char*     _txBuffer = nullptr;
    char*     _rxBuffer = nullptr;

    uint32_t  _txBufferSize = 0;
    uint32_t  _rxBufferSize = 0;

    // Occupied length in each buffer
    uint32_t  _txPosition = 0;  ///< number of valid bytes in _txBuffer
    uint32_t  _rxPosition = 0;  ///< number of valid bytes in _rxBuffer

    // Internal helpers
    void _dropFrontTx(uint32_t n);
    void _dropFrontRx(uint32_t n);
    
};