/**
 * @file StreamEx.cpp
 * @brief Definitions for StreamEx utilities and the Arduino-like buffered I/O class (no inheritance).
 */
#include "StreamEx.h"

#include <limits.h>     // INT*_MIN/INT*_MAX
#include <algorithm>    // std::min
#include <ctype.h>      // isspace, tolower
#include <string.h>     // memcpy, memmove, memset, strlen
#include <stdlib.h>     // strtoul, strtoull, strtoll, strtof, strtod
#include <stdio.h>      // snprintf

namespace StreamEx_utility
{

size_t safe_strnlen(const char* str, size_t maxlen)
{
    if (!str) return 0;
    size_t n = 0;
    while (n < maxlen && str[n] != '\0') ++n;
    return n;
}

char tolow(char c) { return (char)tolower((unsigned char)c); }

bool iequal(const char* a, const char* b)
{
    if (!a || !b) return false;
    while (*a && *b)
    {
        if (tolow(*a) != tolow(*b)) return false;
        ++a; ++b;
    }
    return *a == '\0' && *b == '\0';
}

bool isNumber(const char* s)
{
    if (!s || *s == '\0') return false;
    if (*s == '+' || *s == '-') ++s;
    bool digit=false, dot=false;
    for (; *s; ++s)
    {
        if (*s >= '0' && *s <= '9') { digit = true; continue; }
        if (*s == '.' && !dot)      { dot = true;  continue; }
        return false;
    }
    return digit;
}

bool isInteger(const char* s)
{
    if (!s || *s == '\0') return false;
    if (*s == '+' || *s == '-') ++s;
    if (*s == '\0') return false;
    for (; *s; ++s) if (*s < '0' || *s > '9') return false;
    return true;
}

bool isUInteger(const char* s)
{
    if (!s || *s == '\0' || *s == '-') return false;
    if (*s == '+') ++s;
    if (*s == '\0') return false;
    for (; *s; ++s) if (*s < '0' || *s > '9') return false;
    return true;
}

void trimString(char* buf, uint32_t maxSize)
{
    if (!buf) return;

    if (maxSize) buf[maxSize - 1] = '\0';
    uint32_t len = maxSize ? (uint32_t)safe_strnlen(buf, maxSize) : (uint32_t)strlen(buf);
    if (len == 0) return;

    uint32_t start = 0;
    int32_t end = (int32_t)len - 1;

    while (start <= (uint32_t)end && isspace((unsigned char)buf[start])) ++start;
    if ((int32_t)start > end) { buf[0] = '\0'; return; }
    while (end >= (int32_t)start && isspace((unsigned char)buf[end])) --end;

    // Compact left without overlapping strcpy
    uint32_t out = 0;
    for (uint32_t i = start; i <= (uint32_t)end; ++i) buf[out++] = buf[i];
    buf[out] = '\0';
}

// ---------- Validators (typed) ----------

bool isUInt8 (const char* s){ if(!isUInteger(s)) return false; char* e=nullptr; unsigned long v=strtoul(s,&e,10); return e && *e=='\0' && v<=0xFFUL; }
bool isUInt16(const char* s){ if(!isUInteger(s)) return false; char* e=nullptr; unsigned long v=strtoul(s,&e,10); return e && *e=='\0' && v<=0xFFFFUL; }
bool isUInt32(const char* s){ if(!isUInteger(s)) return false; char* e=nullptr; unsigned long v=strtoul(s,&e,10); return e && *e=='\0'; }
bool isUInt64(const char* s){ if(!isUInteger(s)) return false; char* e=nullptr; (void)strtoull(s,&e,10);          return e && *e=='\0'; }

bool isInt8 (const char* s){ if(!isInteger(s)) return false; char* e=nullptr; long long v=strtoll(s,&e,10); return e && *e=='\0' && v>=INT8_MIN  && v<=INT8_MAX; }
bool isInt16(const char* s){ if(!isInteger(s)) return false; char* e=nullptr; long long v=strtoll(s,&e,10); return e && *e=='\0' && v>=INT16_MIN && v<=INT16_MAX; }
bool isInt32(const char* s){ if(!isInteger(s)) return false; char* e=nullptr; long long v=strtoll(s,&e,10); return e && *e=='\0' && v>=INT32_MIN && v<=INT32_MAX; }
bool isInt64(const char* s){ if(!isInteger(s)) return false; char* e=nullptr; (void)strtoll(s,&e,10);        return e && *e=='\0'; }

bool isFloat (const char* s){ if(!isNumber(s)) return false; char* e=nullptr; (void)strtof (s,&e); return e && *e=='\0'; }
bool isDouble(const char* s){ if(!isNumber(s)) return false; char* e=nullptr; (void)strtod (s,&e); return e && *e=='\0'; }

bool isBoolean(const char* s)
{
    if (!s) return false;
    if ((s[0]=='0' && s[1]=='\0') || (s[0]=='1' && s[1]=='\0')) return true;
    // case-insensitive "true"/"false"
    return iequal(s, "true") || iequal(s, "false");
}

// ---------- Conversions ----------

bool stringToUint8 (const char* s, uint8_t*  out){ if(!s||!out) return false; char* e=nullptr; unsigned long v=strtoul (s,&e,10); if(!(e&&*e=='\0')||v>0xFFUL)   return false; *out=(uint8_t)v;  return true; }
bool stringToUint16(const char* s, uint16_t* out){ if(!s||!out) return false; char* e=nullptr; unsigned long v=strtoul (s,&e,10); if(!(e&&*e=='\0')||v>0xFFFFUL) return false; *out=(uint16_t)v; return true; }
bool stringToUint32(const char* s, uint32_t* out){ if(!s||!out) return false; char* e=nullptr; unsigned long v=strtoul (s,&e,10); if(!(e&&*e=='\0'))            return false; *out=(uint32_t)v; return true; }
bool stringToUint64(const char* s, uint64_t* out){ if(!s||!out) return false; char* e=nullptr; unsigned long long v=strtoull(s,&e,10); if(!(e&&*e=='\0'))      return false; *out=(uint64_t)v; return true; }

bool stringToInt8 (const char* s, int8_t*  out){ if(!s||!out) return false; char* e=nullptr; long long v=strtoll(s,&e,10); if(!(e&&*e=='\0')||v<INT8_MIN  || v>INT8_MAX)  return false; *out=(int8_t)v;  return true; }
bool stringToInt16(const char* s, int16_t* out){ if(!s||!out) return false; char* e=nullptr; long long v=strtoll(s,&e,10); if(!(e&&*e=='\0')||v<INT16_MIN || v>INT16_MAX) return false; *out=(int16_t)v; return true; }
bool stringToInt32(const char* s, int32_t* out){ if(!s||!out) return false; char* e=nullptr; long long v=strtoll(s,&e,10); if(!(e&&*e=='\0')||v<INT32_MIN || v>INT32_MAX) return false; *out=(int32_t)v; return true; }
bool stringToInt64(const char* s, int64_t* out){ if(!s||!out) return false; char* e=nullptr; long long v=strtoll(s,&e,10); if(!(e&&*e=='\0'))                       return false; *out=(int64_t)v; return true; }

bool stringToFloat (const char* s, float*  out){ if(!s||!out) return false; char* e=nullptr; float  v=strtof(s,&e);  if(!(e&&*e=='\0')) return false; *out=v; return true; }
bool stringToDouble(const char* s, double* out){ if(!s||!out) return false; char* e=nullptr; double v=strtod(s,&e);  if(!(e&&*e=='\0')) return false; *out=v; return true; }

bool checkValueType(const char *data, dataTypeEnum type)
{
    switch (type)
    {
        case uint8Type:  return isUInt8 (data);
        case uint16Type: return isUInt16(data);
        case uint32Type: return isUInt32(data);
        case uint64Type: return isUInt64(data);
        case int8Type:   return isInt8  (data);
        case int16Type:  return isInt16 (data);
        case int32Type:  return isInt32 (data);
        case int64Type:  return isInt64 (data);
        case floatType:  return isFloat (data);
        case doubleType: return isDouble(data);
        case charType:   return true;
        case stringType: return true;
        case boolType:   return isBoolean(data);
        default:         return false;
    }
}

bool stringToNumber(const char* str, dataValueUnion* num, dataTypeEnum type)
{
    if (!str || !num) return false;

    switch (type)
    {
        case uint8Type:  return stringToUint8 (str, &num->uint8Value);
        case uint16Type: return stringToUint16(str, &num->uint16Value);
        case uint32Type: return stringToUint32(str, &num->uint32Value);
        case uint64Type: return stringToUint64(str, &num->uint64Value);

        case int8Type:   return stringToInt8 (str, &num->int8Value);
        case int16Type:  return stringToInt16(str, &num->int16Value);
        case int32Type:  return stringToInt32(str, &num->int32Value);
        case int64Type:  return stringToInt64(str, &num->int64Value);

        case floatType:  return stringToFloat (str, &num->floatValue);
        case doubleType: return stringToDouble(str, &num->doubleValue);

        case boolType:
            num->boolValue = (str[0]=='1' && str[1]=='\0') || iequal(str,"true");
            return true;

        case stringType:
        {
            size_t n = safe_strnlen(str, STREAMEX_STRING_CAP - 1);
            memcpy(num->stringValue, str, n);
            num->stringValue[n] = '\0';
            return true;
        }

        case charType:
            num->uint8Value = (uint8_t)(str && *str ? *str : '\0');
            return true;

        default: return false;
    }
}

void dataValueToString(char *out, size_t outCap,
                       const dataValueUnion& value, dataTypeEnum type)
{
    if (!out || outCap == 0) return;

    switch (type)
    {
        case uint8Type:  snprintf(out, outCap, "%u",   (unsigned)value.uint8Value);            break;
        case uint16Type: snprintf(out, outCap, "%u",   (unsigned)value.uint16Value);           break;
        case uint32Type: snprintf(out, outCap, "%lu",  (unsigned long)value.uint32Value);      break;
        case uint64Type: snprintf(out, outCap, "%llu", (unsigned long long)value.uint64Value); break;

        case int8Type:   snprintf(out, outCap, "%d",   (int)value.int8Value);                  break;
        case int16Type:  snprintf(out, outCap, "%d",   (int)value.int16Value);                 break;
        case int32Type:  snprintf(out, outCap, "%ld",  (long)value.int32Value);                break;
        case int64Type:  snprintf(out, outCap, "%lld", (long long)value.int64Value);           break;

        case floatType:  snprintf(out, outCap, "%g",   (double)value.floatValue);              break;
        case doubleType: snprintf(out, outCap, "%g",   (double)value.doubleValue);             break;

        case boolType:   snprintf(out, outCap, "%s",   value.boolValue ? "true" : "false");    break;
        case stringType: snprintf(out, outCap, "%s",   value.stringValue);                     break;
        case charType:   snprintf(out, outCap, "%c",   (char)value.uint8Value);                break;

        default:         snprintf(out, outCap, "%s",   "Unsupported Type");                    break;
    }
}

} // namespace StreamEx_utility


// ###########################################################################################################
//                                        StreamEx class
// ###########################################################################################################


StreamEx::StreamEx(char* txBuffer, uint32_t txBufferSize, char* rxBuffer, uint32_t rxBufferSize)
: errorCode(StreamExError::None),
_txBuffer(txBuffer), _rxBuffer(rxBuffer),
_txBufferSize(txBufferSize), _rxBufferSize(rxBufferSize),
_txPosition(0), _rxPosition(0)
{
    // Null-terminate the remaining buffer (optional for string usage)
    if (_txBuffer && _txBufferSize) memset(_txBuffer, 0, _txBufferSize);
    if (_rxBuffer && _rxBufferSize) memset(_rxBuffer, 0, _rxBufferSize);
}

StreamEx::~StreamEx() { /* no-op (no ownership) */ }

void StreamEx::setTxBuffer(char* txBuffer, uint32_t txBufferSize)
{
    _txBuffer      = txBuffer;
    _txBufferSize  = txBufferSize;
    _txPosition    = 0;
    if (_txBuffer && _txBufferSize) memset(_txBuffer, 0, _txBufferSize);
}

void StreamEx::setRxBuffer(char* rxBuffer, uint32_t rxBufferSize)
{
    _rxBuffer      = rxBuffer;
    _rxBufferSize  = rxBufferSize;
    _rxPosition    = 0;
    if (_rxBuffer && _rxBufferSize) memset(_rxBuffer, 0, _rxBufferSize);
}

void StreamEx::clearTxBuffer() 
{
    if (_txBuffer && _txBufferSize) memset(_txBuffer, 0, _txBufferSize);
    _txPosition = 0;
}

void StreamEx::clearRxBuffer() 
{
    if (_rxBuffer && _rxBufferSize) memset(_rxBuffer, 0, _rxBufferSize);
    _rxPosition = 0;
}

// ----- internal helpers -----

void StreamEx::_dropFrontTx(uint32_t n){
    if (!_txBuffer || _txPosition == 0 || n == 0) return;
    if (n >= _txPosition) { _txPosition = 0; _txBuffer[0] = '\0'; return; }
    memmove(_txBuffer, _txBuffer + n, _txPosition - n);
    _txPosition -= n;
    _txBuffer[_txPosition] = '\0';
}

void StreamEx::_dropFrontRx(uint32_t n){
    if (!_rxBuffer || _rxPosition == 0 || n == 0) return;
    if (n >= _rxPosition) { _rxPosition = 0; _rxBuffer[0] = '\0'; return; }
    memmove(_rxBuffer, _rxBuffer + n, _rxPosition - n);
    _rxPosition -= n;
    _rxBuffer[_rxPosition] = '\0';
}

// ----- append / pop APIs -----

bool StreamEx::writeTxBuffer(const char* data, uint32_t dataSize) 
{
    if (dataSize > _txBufferSize) { errorCode = StreamExError::BufferOverflow; return false; }

    memcpy(_txBuffer, data, dataSize); // Copy data to TX buffer
    _txPosition = dataSize;

    if (_txBuffer && _txBufferSize) {
        const uint32_t term = (_txPosition < _txBufferSize) ? _txPosition : (_txBufferSize - 1);
        _txBuffer[term] = '\0';
    }

    return true;
}

bool StreamEx::writeRxBuffer(const char* data, uint32_t dataSize) 
{
    if (dataSize > _rxBufferSize) { errorCode = StreamExError::BufferOverflow; return false; }

    memcpy(_rxBuffer, data, dataSize); // Copy data to RX buffer
    _rxPosition = dataSize;

    if (_rxBuffer && _rxBufferSize) {
        const uint32_t term = (_rxPosition < _rxBufferSize) ? _rxPosition : (_rxBufferSize - 1);
        _rxBuffer[term] = '\0';
    }

    return true;
}

bool StreamEx::pushBackTxBuffer(const char* data, uint32_t dataSize)
{
    if (!data) { errorCode = StreamExError::NullData; return false; }
    if (!_txBuffer || _txBufferSize == 0) { errorCode = StreamExError::BufferOverflow; return false; }

    // empty space size of tx buffer.
    const uint32_t freeCap = (_txBufferSize > _txPosition) ? (_txBufferSize - _txPosition - 1) : 0;

    // Check for buffer overflow
    if (dataSize > freeCap){
        // Truncate from the front (sliding window)
        const uint32_t need = dataSize - freeCap;
        _dropFrontTx(need);
        errorCode = StreamExError::BufferOverflow;
    }

    const uint32_t canCopy = std::min<uint32_t>(dataSize, (_txBufferSize - _txPosition - 1));
    if (canCopy){
        memcpy(_txBuffer + _txPosition, data, canCopy);
        _txPosition += canCopy;
        _txBuffer[_txPosition] = '\0';
    }
    return (canCopy == dataSize);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::pushBackTxBuffer(const std::string* data)
    {
        if (!data) { errorCode = StreamExError::NullData; return false; }
        return pushBackTxBuffer(data->c_str(), (uint32_t)data->size());
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::pushBackTxBuffer(const String& s) {
        return pushBackTxBuffer(s.c_str(), (uint32_t)s.length());
    }
#endif

bool StreamEx::pushBackRxBuffer(const char* data, uint32_t dataSize)
{
    if (!data) { errorCode = StreamExError::NullData; return false; }
    if (!_rxBuffer || _rxBufferSize == 0) { errorCode = StreamExError::BufferOverflow; return false; }

    const uint32_t freeCap = (_rxBufferSize > _rxPosition) ? (_rxBufferSize - _rxPosition - 1) : 0;

    if (dataSize > freeCap){
        const uint32_t need = dataSize - freeCap;
        _dropFrontRx(need);
        errorCode = StreamExError::BufferOverflow;
    }

    const uint32_t canCopy = std::min<uint32_t>(dataSize, (_rxBufferSize - _rxPosition - 1));
    if (canCopy){
        memcpy(_rxBuffer + _rxPosition, data, canCopy);
        _rxPosition += canCopy;
        _rxBuffer[_rxPosition] = '\0';
    }
    return (canCopy == dataSize);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::pushBackRxBuffer(const std::string* data)
    {
        if (!data) { errorCode = StreamExError::NullData; return false; }
        return pushBackRxBuffer(data->c_str(), (uint32_t)data->size()); 
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::pushBackRxBuffer(const String& s) {
        return pushBackRxBuffer(s.c_str(), (uint32_t)s.length());
    }
#endif

bool StreamEx::popFrontTxBuffer(char* data, uint32_t dataSize)
{
    if (!data) { errorCode = StreamExError::NullData; return false; }
    if (dataSize == 0) { errorCode = StreamExError::SizeZero; return false; }

    if (dataSize > _txPosition){
        // clamp and signal
        dataSize = _txPosition;
        errorCode = StreamExError::NotEnoughData;
    }

    if (dataSize == 0) { data[0] = '\0'; return false; }
    memcpy(data, _txBuffer, dataSize);

    _dropFrontTx(dataSize);
    return (errorCode != StreamExError::NotEnoughData);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::popFrontTxBuffer(std::string* out, uint32_t dataSize)
    {
        if (!out) { errorCode = StreamExError::NullData; return false; }
        if (dataSize > _txPosition){
            dataSize = _txPosition;
            errorCode = StreamExError::NotEnoughData;
        }
        out->assign(_txBuffer, _txBuffer + dataSize);
        _dropFrontTx(dataSize);
        return (errorCode != StreamExError::NotEnoughData);
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::popFrontTxBuffer(String& out, uint32_t dataSize) {
        if (dataSize > _txPosition) { dataSize = _txPosition; errorCode = StreamExError::NotEnoughData; }
        out.remove(0); out.reserve(dataSize);
        char saved = _txBuffer[dataSize];
        _txBuffer[dataSize] = '\0';
        out.concat(_txBuffer);
        _txBuffer[dataSize] = saved;
        _dropFrontTx(dataSize);
        return (errorCode != StreamExError::NotEnoughData);
    }
#endif

bool StreamEx::popAllTxBuffer(char* out, uint32_t maxSize){
    if (!out) { errorCode = StreamExError::NullData; return false; }
    if (maxSize == 0) { errorCode = StreamExError::SizeZero; return false; }
    uint32_t take = std::min<uint32_t>(_txPosition, maxSize);
    memcpy(out, _txBuffer, take);
    _dropFrontTx(take);
    return (take == maxSize || _txPosition == 0);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::popAllTxBuffer(std::string* out){
        if (!out) { errorCode = StreamExError::NullData; return false; }
        out->assign(_txBuffer, _txBuffer + _txPosition);
        _dropFrontTx(_txPosition);
        return true;
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::popAllTxBuffer(String& out) {
        out.remove(0); out.reserve(_txPosition);
        out.concat(_txBuffer);
        _dropFrontTx(_txPosition);
        return true;
    }
#endif

bool StreamEx::popFrontRxBuffer(char* out, uint32_t dataSize){
    if (!out) { errorCode = StreamExError::NullData; return false; }
    if (dataSize == 0) { errorCode = StreamExError::SizeZero; return false; }
    if (dataSize > _rxPosition){
        dataSize = _rxPosition;
        errorCode = StreamExError::NotEnoughData;
    }
    if (dataSize == 0) { out[0] = '\0'; return false; }
    memcpy(out, _rxBuffer, dataSize);
    _dropFrontRx(dataSize);
    return (errorCode != StreamExError::NotEnoughData);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::popFrontRxBuffer(std::string* out, uint32_t dataSize){
        if (!out) { errorCode = StreamExError::NullData; return false; }
        if (dataSize > _rxPosition){
            dataSize = _rxPosition;
            errorCode = StreamExError::NotEnoughData;
        }
        out->assign(_rxBuffer, _rxBuffer + dataSize);
        _dropFrontRx(dataSize);
        return (errorCode != StreamExError::NotEnoughData);
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::popFrontRxBuffer(String& out, uint32_t dataSize) {
        if (dataSize > _rxPosition) { dataSize = _rxPosition; errorCode = StreamExError::NotEnoughData; }
        out.remove(0); out.reserve(dataSize);
        char saved = _rxBuffer[dataSize];
        _rxBuffer[dataSize] = '\0';
        out.concat(_rxBuffer);
        _rxBuffer[dataSize] = saved;
        _dropFrontRx(dataSize);
        return (errorCode != StreamExError::NotEnoughData);
    }
#endif

bool StreamEx::popAllRxBuffer(char* out, uint32_t maxSize){
    if (!out) { errorCode = StreamExError::NullData; return false; }
    if (maxSize == 0) { errorCode = StreamExError::SizeZero; return false; }
    uint32_t take = std::min<uint32_t>(_rxPosition, maxSize);
    memcpy(out, _rxBuffer, take);
    _dropFrontRx(take);
    return (take == maxSize || _rxPosition == 0);
}

#if STREAMEX_ENABLE_STD_STRING
    bool StreamEx::popAllRxBuffer(std::string* out){
        if (!out) { errorCode = StreamExError::NullData; return false; }
        out->assign(_rxBuffer, _rxBuffer + _rxPosition);
        _dropFrontRx(_rxPosition);
        return true;
    }
#endif

#if STREAMEX_ENABLE_ARDUINO_STRING
    bool StreamEx::popAllRxBuffer(String& out) {
        out.remove(0); out.reserve(_rxPosition);
        out.concat(_rxBuffer);
        _dropFrontRx(_rxPosition);
        return true;
    }
#endif

// ----------------------------------------------

bool StreamEx::removeFrontTxBuffer(uint32_t dataSize)
{
    if (dataSize > _txPosition) { errorCode = StreamExError::NotEnoughData; return false; }

    // Shift the remaining data in the TX buffer
    memmove(_txBuffer, _txBuffer + dataSize, _txPosition - dataSize);

    // Update the TX buffer position
    _txPosition -= dataSize;

    // Null-terminate the remaining buffer (optional for string usage)
    _txBuffer[_txPosition] = '\0';

    return true;
}

bool StreamEx::removeFrontRxBuffer(uint32_t dataSize)
{
    if (dataSize > _rxPosition) { errorCode = StreamExError::NotEnoughData; return false; }

    // Shift the remaining data in the TX buffer
    memmove(_rxBuffer, _rxBuffer + dataSize, _rxPosition - dataSize);

    // Update the TX buffer position
    _rxPosition -= dataSize;

    // Null-terminate the remaining buffer (optional for string usage)
    _rxBuffer[_rxPosition] = '\0';

    return true;
}

// ---------------- Arduino-like interface (no Stream inheritance) ----------------

int StreamEx::available() {
    return (int)_rxPosition;
}

int StreamEx::read() {
    if (_rxPosition == 0 || !_rxBuffer) return -1;
    uint8_t b = (uint8_t)_rxBuffer[0];
    _dropFrontRx(1);
    return (int)b;
}

int StreamEx::peek() {
    if (_rxPosition == 0 || !_rxBuffer) return -1;
    return (uint8_t)_rxBuffer[0];
}

void StreamEx::flush() {
    // On common Arduino drivers, Stream::flush() affects TX.
    // Here we mirror that semantic: interpret as "TX is delivered" → clear TX buffer.
    clearTxBuffer();
}

size_t StreamEx::write(uint8_t b) {
    return write(&b, 1);
}

size_t StreamEx::write(const uint8_t* buffer, size_t size) {
    if (!buffer || size == 0) { errorCode = StreamExError::SizeZero; return 0; }
    const bool ok = pushBackTxBuffer((const char*)buffer, (uint32_t)size);
    // If overflow occurred, sliding-window logic may have dropped oldest bytes;
    // return the number requested on full success, or the current TX fill otherwise.
    return ok ? size : (size_t)(_txPosition); 
}




