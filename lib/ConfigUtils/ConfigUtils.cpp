#include <LittleFS.h>
#include <ArduinoJson.h>
#include "ConfigUtils.h"

bool readJsonFile(const String& path, JsonDocument& doc) {
    File file = LittleFS.open(path, "r");
    if (file) {
        DeserializationError result = deserializeJson(doc, file);
        file.close();
        return result == DeserializationError::Ok;
    }
    return false;
}

bool writeJsonFile(const String& path, const JsonDocument& doc) {
    File file = LittleFS.open(path, "w");
    if (file) {
        const size_t size = serializeJson(doc, file);
        file.close();
        return size > 0;
    }
    return false;
}

bool ConfigUtils::loadJSON(
        const FieldHandle* handles, size_t count, void* obj, const String& filePath) {
    DynamicJsonDocument doc(1024);
    if (!readJsonFile(filePath, doc)) {
        return false;
    }
    for(size_t i = 0; i < count; ++i) {
        const FieldHandle& f = handles[i];
        if (f.type == FieldType::StringT) {
            if (doc.containsKey(f.name))
                f.ref<String>(obj) = doc[f.name].as<String>();
        }
        else if (f.type == FieldType::IntT) {
            if (doc.containsKey(f.name))
                f.ref<int>(obj) = doc[f.name];
        }
        else if (f.type == FieldType::FloatT) {
            if (doc.containsKey(f.name))
                f.ref<float>(obj) = doc[f.name];
        }
        else if (f.type == FieldType::IPAddressT) {
            if (doc.containsKey(f.name))
                f.ref<IPAddress>(obj).fromString(doc[f.name].as<String>());
        }
    }
    #ifdef CONFIG_UTILS_JSON_TRACE
        Serial.print(F("JSON Document capacity usage: "));
        Serial.print(doc.memoryUsage());
        Serial.print(F(" of "));
        Serial.println(doc.capacity());
    #endif
    return true;
}

bool ConfigUtils::saveJSON(
        const FieldHandle* handles, size_t count, void* obj, const String& filePath) {
    DynamicJsonDocument doc(1024);
    for(size_t i = 0; i < count; ++i) {
        const FieldHandle& f = handles[i];
        if (f.type == FieldType::StringT) {
            doc[f.name] = f.ref<String>(obj);
        }
        else if (f.type == FieldType::IntT) {
            doc[f.name] = f.ref<int>(obj);
        }
        else if (f.type == FieldType::FloatT) {
            doc[f.name] = f.ref<float>(obj);
        }
        else if (f.type == FieldType::IPAddressT) {
            doc[f.name] = f.ref<IPAddress>(obj).toString();
        }
    }
    #ifdef CONFIG_UTILS_JSON_TRACE
        Serial.printf("JSON Document capacity usage: %u of %u\n", doc.memoryUsage(), doc.capacity());
    #endif
    return writeJsonFile(filePath, doc);
}

bool ConfigUtils::setField(const FieldHandle* handles, size_t count, void* obj, const String& name, const String& value) {
    for(size_t i = 0; i < count; ++i) {
        const FieldHandle& f = handles[i];
        if (f.name == name) {
            switch (f.type) {
                case FieldType::StringT:
                    f.ref<String>(obj) = value;
                    return true;
                case FieldType::IntT:
                    f.ref<String>(obj) = value.toInt();
                    return true;
                case FieldType::FloatT:
                    f.ref<String>(obj) = value.toFloat();
                    return true;
                case FieldType::IPAddressT:
                    f.ref<IPAddress>(obj).fromString(value);
                    return true;
            }
        }
    }
    return false;
}

void ConfigUtils::printFields(const FieldHandle* handles, size_t count, void* obj) {
    for(size_t i = 0; i < count; ++i) {
        const FieldHandle& f = handles[i];
        Serial.print(f.name);
        Serial.print(": ");
        if (f.type == FieldType::StringT) {
            Serial.println(f.ref<String>(obj));
        }
        else if (f.type == FieldType::IntT) {
            Serial.println(f.ref<int>(obj));
        }
        else if (f.type == FieldType::FloatT) {
            Serial.println(f.ref<float>(obj));
        }
        else if (f.type == FieldType::IPAddressT) {
            Serial.println(f.ref<IPAddress>(obj));
        }
    }
}

String getStringPartCsv(const String& str, unsigned int part, char separator) {
    for(unsigned int pt = 0, beg = 0, end; beg < str.length(); beg = end + 1) {
        int pos = str.indexOf(separator, beg);
        end = (pos >= 0) ? pos : str.length();
        if (pt++ == part) {
            return str.substring(beg, end);
        }
    }
    return String();
}