#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/utility/value_init.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/crc.hpp>
#include "strategy.h"
#include "handler.h"

using namespace boost::uuids;
namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace co = boost::container;
using boost::char_separator;
using boost::ptr_vector;
using boost::tokenizer;
using boost::container::basic_string;
using boost::container::string;
using boost::movelib::unique_ptr;

struct FileInfo
{
    fs::path path;
    co::vector<uint32_t> blocks = boost::initialized_value;
    uuid id = nil_generator()();
};

class Processor
{
    using fileInfoIterator = co::vector<FileInfo>::iterator;

public:
    Processor();
    ~Processor();

    void scanLevel(const int level);
    void blockSize(const size_t size);
    void hashAlgo(const string &hash);
    void scanDirs(const string &data);
    void pushRequest(unique_ptr<Request> request);

    void checkFile();
    void print();

private:
    void fillFileList(const fs::path &fPath);
    void compareFiles(fileInfoIterator &it, fileInfoIterator &it1);
    uint32_t getBlock(fileInfoIterator &it, fs::ifstream &ifs, const size_t block);
    uint32_t hashBulk(fs::ifstream &fstream);
    bool good(const fs::path &file);

    co::vector<string> m_included;
    co::vector<unique_ptr<Request>> m_requests;
    co::vector<FileInfo> filesAtCurrentPath;
    unique_ptr<iStrategy> hashStrategy;
    int m_level = 0;
    size_t m_blockSize = 0;
    random_generator gen;
    char *m_buf = nullptr;

};


#endif // PROCESSOR_H
