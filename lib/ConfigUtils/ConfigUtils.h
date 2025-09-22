#ifndef ConfigUtils_h
#define ConfigUtils_h

#include <Arduino.h>

enum class FieldType { IntT, FloatT, StringT, IPAddressT };

class FieldHandle {
public:
    String name;
    FieldType type;
    size_t offset;

    template<typename T> T& ref(void *obj) const {
        return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(obj) + offset);
    }
};

class ConfigUtils {
public:
    static bool saveJSON(const FieldHandle* handles, size_t count, void* obj, const String& filePath);
    static bool loadJSON(const FieldHandle* handles, size_t count, void* obj, const String& filePath);
    static bool setField(const FieldHandle* handles, size_t count, void* obj, const String& name, const String& value);
    static void printFields(const FieldHandle* handles, size_t count, void* obj);
};

#endif