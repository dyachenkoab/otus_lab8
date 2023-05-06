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

class Request
{
protected:
    Request() {};

public:
    virtual ~Request() { }
    virtual bool passThrough(const co::string &fileName) = 0;
};

class ExcludedRequest : public Request
{
public:
    ~ExcludedRequest() = default;
    ExcludedRequest(const co::string &s)
    {
        split(s, m_excluded);
    };
    bool passThrough(const co::string &fileName) override
    {
        auto is_subpath = [](const fs::path &path, const fs::path &base) {
            const auto mismatch_pair =
                std::mismatch(path.begin(), path.end(), base.begin(), base.end());
            return mismatch_pair.second == base.end();
        };

        auto path = fs::system_complete(fs::path(fileName.c_str()).remove_filename());
        for (auto &x : m_excluded) {
            if (is_subpath(path, fs::system_complete(fs::path(x.c_str())))) {
                return false;
            }
        }
        return true;
    };

private:
    co::set<co::string> m_excluded;
};

class SizeRequest : public Request
{
public:
    ~SizeRequest() = default;
    SizeRequest(int size) : m_size(size) {};
    bool passThrough(const co::string &fileName) override
    {
        fs::path file(fileName.c_str());

        if (!fs::is_regular_file(file) || fs::is_symlink(file)) {
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
    MaskRequest(const co::string &s) : mask(s) {};
    bool passThrough(const co::string &fileName) override
    {
        fs::path file(fileName.c_str());
        boost::regex expr { mask.data() };
        return boost::regex_search(file.filename().c_str(), expr);
    };

private:
    co::string mask;
};

#endif // HANDLER_H
