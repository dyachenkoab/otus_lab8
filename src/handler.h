#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include <boost/container/set.hpp>
#include <boost/container/string.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include "strategy.h"

namespace fs = boost::filesystem;
namespace co = boost::container;
using boost::regex;
using boost::smatch;
using boost::container::string;

class Request
{
protected:
    Request() {};

public:
    virtual ~Request() { }
    virtual bool passThrough(const string &fileName) = 0;
};

class ExcludedRequest : public Request
{
public:
    ~ExcludedRequest() = default;
    ExcludedRequest(const string &s) : s(s)
    {
        split(s, m_excluded);
    };
    bool passThrough(const string &fileName) override
    {
        for (auto &x : m_excluded) {
            if (boost::icontains(fileName, x)) {
                return false;
            }
        }
        return true;
    };

private:
    string s;
    co::set<string> m_excluded;
};

class SizeRequest : public Request
{
public:
    ~SizeRequest() = default;
    SizeRequest(int size) : m_size(size) {};
    bool passThrough(const string &fileName) override
    {
        fs::path file(fileName.c_str());

        if (!fs::is_regular_file(file)) {
            return false;
        }

        if (m_size > fs::file_size(file)) {
            return false;
        }

        return true;
    };

private:
    boost::uintmax_t m_size;
};

class MaskRequest : public Request
{
public:
    ~MaskRequest() = default;
    MaskRequest(const string &s) : mask(s) {};
    bool passThrough(const string &fileName) override
    {
        fs::path file(fileName.c_str());
        boost::regex expr { mask.data() };
        return boost::regex_search(file.filename().c_str(), expr);
    };

private:
    string mask;
};

#endif // HANDLER_H
