#ifndef CONNECT_SDK_UTIL_H
#define CONNECT_SDK_UTIL_H

#include "common-types.h"

namespace prism
{
namespace connect
{

class Metadata;

string toJsonString(const Metadata& metadata);

class JsonValue
{
public:
    enum {
        INTEGER,
        DOUBLE,
        STRING,
        OBJECT
    };

    JsonValue();
    JsonValue(int type); // create array of type
    JsonValue(const string& str); // parse string to create object
    ~JsonValue();

    string toString();

    void setValue(const string& name, int value);
    void setValue(const string& name, double value);
    void setValue(const string& name, const string& value);

    bool hasInt(const string& name) const;
    bool hasDouble(const string& name) const;
    bool hasString(const string& name) const;
    bool hasObject(const string& name) const;

    int getInt(const string& name) const;
    double getDouble(const string& name) const;
    string getString(const string& name) const;
    const JsonValue& getObject(const string& name) const;

    bool isArray() const;
    size_t getArraySize() const;
    int getArrayType() const;

    JsonValue& appendObject();
    JsonValue& appendArray(int type);
    void appendInt(int value);
    void appendDouble(double value);
    void appendString(const string& value);
//    JsonValue& getModifiableObject(size_t index);
//    const JsonValue& getObject(size_t index) const;
//    int getInt(size_t index) const;
//    double getDouble(size_t index) const;
//    string getString(size_t index) const;

//    int& getInt(size_t index);
//    double& getDouble(size_t index);
//    string& getString(size_t index);

    JsonValue& createObject(const string& name);
//    JsonValue& getModifiableObject(const string& name);

private:
    class Impl;
    Impl* pImpl_;
};

}
}

#endif // CONNECT_SDK_UTIL_H
