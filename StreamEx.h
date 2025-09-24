#pragma once
/**
 * @file StreamEx.h
 * @brief Declarations for lightweight utilities to validate/trim/convert C-strings
 *        to typed values (Arduino-friendly, no heavy POSIX deps).
 *
 * @note Define STREAMEX_STRING_CAP before including this header to adjust the
 *       internal small string buffer size (default = 32 including NUL).
 */

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

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

namespace StreamEx
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
