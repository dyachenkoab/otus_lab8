#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/string.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/utility/value_init.hpp>
#include <boost/crc.hpp>

#include "strategy.h"
#include "handler.h"

namespace fs = boost::filesystem;
namespace co = boost::container;

class Processor
{

    struct FileInfo
    {
        fs::ifstream *ifs;
        co::string fileName;
    };

    using EqualPrefixFiles = co::vector<FileInfo>;
    using GroupedPrefixes = co::vector<EqualPrefixFiles>;

public:
    Processor() = default;
    ~Processor();

    void scanLevel(const int level);
    void blockSize(const size_t size);
    void hashAlgo(const co::string &hash);
    void scanDirs(const co::string &data);
    void pushRequest(unique_ptr<Request> request);

    void checkDirs();

private:
    void fillFileList(const fs::path &fPath);
    GroupedPrefixes groupByNextBlock(const EqualPrefixFiles &files) const;
    uint32_t hashBulk(fs::ifstream *fstream) const;
    void print(const GroupedPrefixes &groups);

    EqualPrefixFiles m_files;

    co::vector<co::string> m_included;
    co::vector<unique_ptr<Request>> m_requests;
    unique_ptr<iStrategy> hashStrategy;
    int m_level = 0;
    size_t m_blockSize = 0;
};

#endif // PROCESSOR_H
