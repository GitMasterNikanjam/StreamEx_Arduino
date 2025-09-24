/**
 * @file StreamEx.cpp
 * @brief Definitions for StreamEx utilities (Arduino-friendly).
 */
#include "StreamEx.h"

#include <limits.h>
#include <float.h>
#include <limits>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace StreamEx
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

} // namespace StreamEx
