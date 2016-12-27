#include "util.h"
#include "domain-types.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace prism
{
namespace connect
{

string toJsonString(const Metadata& metadata)
{
    rapidjson::Document doc;
    doc.SetObject();
    const Metadata::NameTypeMap& nameTypeMap = metadata.getNameTypeMap();

    for (Metadata::NameTypeMap::const_iterator cit = nameTypeMap.begin();
         cit != nameTypeMap.end();
         ++cit)
    {
        rapidjson::Value value;
        rapidjson::Value name(cit->first.c_str(), doc.GetAllocator());

        switch (cit->second)
        {
        case Metadata::INTEGER:
            {
                int32_t myInt;
                metadata.getValue(cit->first, myInt);
                value = myInt;
                break;
            }

        case Metadata::DOUBLE:
            {
                double myDouble;
                metadata.getValue(cit->first, myDouble);
                value = myDouble;
                break;
            }

        case Metadata::STRING:
            {
                string myString;
                metadata.getValue(cit->first, myString);
                value.SetString(myString.c_str(), doc.GetAllocator());
                break;
            }
        } // switch

        doc.AddMember(name, value, doc.GetAllocator());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

class KvNode::Impl
{
public:
    Impl(bool isArray)
        : isArray_(isArray)
    {
    }

    typedef Metadata::NameTypeMap NameTypeMap;

    const NameTypeMap& getNameTypeMap() const
    {
        return nameTypeMap_;
    }

private:
    typedef map<string, int32_t> NameIntMap;
    typedef map<string, double> NameDoubleMap;
    typedef map<string, string> NameStringMap;
    typedef map<string, *KvNode> NameObjectMap;

    bool isArray_;
    NameTypeMap nameTypeMap_;
    NameIntMap nameIntMap_;
    NameDoubleMap nameDoubleMap_;
    NameStringMap nameStringMap_;
    NameObjectMap nameObjectMap_;
};

KvNode::KvNode(bool isArray)
    : pImpl_(new Impl(isArray))
{
}

KvNode::~KvNode()
{
    delete pImpl_;
}

}
}
