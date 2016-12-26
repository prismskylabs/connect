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

}
}
