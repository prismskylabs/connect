#include "domain-types.h"

namespace prism
{
namespace connect
{

class Metadata::Impl
{
public:
    Impl()
    {
    }

    Impl(const Impl& other)
        : nameTypeMap_(other.nameTypeMap_)
        , nameIntMap_(other.nameIntMap_)
        , nameDoubleMap_(other.nameDoubleMap_)
        , nameStringMap_(other.nameStringMap_)
    {
    }

    void setValue(const string& name, int32_t value);
    void setValue(const string& name, const string& value);
    void setValue(const string& name, double value);

    bool getValue(const string& name, int32_t& value) const;
    bool getValue(const string& name, string& value) const;
    bool getValue(const string& name, double& value) const;

    typedef Metadata::NameTypeMap NameTypeMap;

    const NameTypeMap& getNameTypeMap() const
    {
        return nameTypeMap_;
    }

private:
    typedef map<string, int32_t> NameIntMap;
    typedef map<string, double> NameDoubleMap;
    typedef map<string, string> NameStringMap;

    NameTypeMap nameTypeMap_;
    NameIntMap nameIntMap_;
    NameDoubleMap nameDoubleMap_;
    NameStringMap nameStringMap_;
};

Metadata::Metadata()
    : pImpl_(new Impl())
{
}

Metadata::Metadata(const Metadata& other)
    : pImpl_(new Impl(*other.pImpl_.get()))
{
}

Metadata::~Metadata()
{
}

void Metadata::setValue(const string& name, int32_t value)
{
    pImpl_->setValue(name, value);
}

void Metadata::setValue(const string& name, const string& value)
{
    pImpl_->setValue(name, value);
}
void Metadata::setValue(const string& name, double value)
{
    pImpl_->setValue(name, value);
}

bool Metadata::getValue(const string& name, int32_t& value) const
{
    return pImpl_->getValue(name, value);
}

bool Metadata::getValue(const string& name, string& value) const
{
    return pImpl_->getValue(name, value);
}

bool Metadata::getValue(const string& name, double& value) const
{
    return pImpl_->getValue(name, value);
}

const Metadata::NameTypeMap&Metadata::getNameTypeMap() const
{
    return pImpl_->getNameTypeMap();
}

void Metadata::Impl::setValue(const string& name, int32_t value)
{
    NameTypeMap::iterator it = nameTypeMap_.find(name);

    // if value with such name exists, but has different type, then
    // remove it fromt the value container
    if (it != nameTypeMap_.end())
    {
        switch (it->second)
        {
        case Metadata::DOUBLE:
            nameDoubleMap_.erase(name);
            break;

        case Metadata::STRING:
            nameStringMap_.erase(name);
            break;
        }
    }

    nameTypeMap_[name] = Metadata::INTEGER;
    nameIntMap_[name] = value;
}

void Metadata::Impl::setValue(const string& name, const string& value)
{
    NameTypeMap::iterator it = nameTypeMap_.find(name);

    if (it != nameTypeMap_.end())
    {
        switch (it->second)
        {
        case Metadata::DOUBLE:
            nameDoubleMap_.erase(name);
            break;

        case Metadata::INTEGER:
            nameIntMap_.erase(name);
            break;
        }
    }

    nameTypeMap_[name] = Metadata::STRING;
    nameStringMap_[name] = value;
}

void Metadata::Impl::setValue(const string& name, double value)
{
    NameTypeMap::iterator it = nameTypeMap_.find(name);

    // if value with such name exists, but has different type, then
    // remove it fromt the value container
    if (it != nameTypeMap_.end())
    {
        switch (it->second)
        {
        case Metadata::INTEGER:
            nameIntMap_.erase(name);
            break;

        case Metadata::STRING:
            nameStringMap_.erase(name);
            break;
        }
    }

    nameTypeMap_[name] = Metadata::DOUBLE;
    nameDoubleMap_[name] = value;
}

bool Metadata::Impl::getValue(const string& name, int32_t& value) const
{
    NameTypeMap::const_iterator cit = nameTypeMap_.find(name);

    if (cit != nameTypeMap_.end()  &&  cit->second == Metadata::INTEGER)
    {
        value = nameIntMap_.find(name)->second;
        return true;
    }

    return false;
}

bool Metadata::Impl::getValue(const string& name, string& value) const
{
    NameTypeMap::const_iterator cit = nameTypeMap_.find(name);

    if (cit != nameTypeMap_.end()  &&  cit->second == Metadata::STRING)
    {
        value = nameStringMap_.find(name)->second;
        return true;
    }

    return false;
}

bool Metadata::Impl::getValue(const string& name, double& value) const
{
    NameTypeMap::const_iterator cit = nameTypeMap_.find(name);

    if (cit != nameTypeMap_.end()  &&  cit->second == Metadata::DOUBLE)
    {
        value = nameDoubleMap_.find(name)->second;
        return true;
    }

    return false;
}

}
}
