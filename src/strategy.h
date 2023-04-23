#ifndef STRATEGY_H
#define STRATEGY_H
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/set.hpp>
#include <boost/container/string.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/functional/hash.hpp>
#include <boost/crc.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace co = boost::container;
using boost::char_separator;
using boost::tokenizer;
using boost::container::basic_string;
using boost::container::string;
using boost::movelib::unique_ptr;

namespace impl {
template<class Iterator>
struct is_random_access_iter
{
    using category = typename std::iterator_traits<Iterator>::iterator_category;
    static const bool value = std::is_same<category, std::random_access_iterator_tag>::value;
};

template<class Container>
struct is_random_access_container
{
    static const bool value = is_random_access_iter<typename Container::iterator>::value;
};
} // namespace impl

template<typename T>
void emplace(T &container, const string &s,
         typename std::enable_if<impl::is_random_access_container<T>::value>::type * = {})
{
    container.emplace_back(boost::move(s));
}

template<typename T>
void emplace(T &container, const string &s,
         typename std::enable_if<!impl::is_random_access_container<T>::value>::type * = {})
{
    container.emplace(boost::move(s));
}

template<typename T>
void split(const string &data, T &container)
{
    char_separator<char> sep(" ");
    using tokenizer = tokenizer<char_separator<char>, string::const_iterator, string>;

    tokenizer tok(data, sep);
    for (tokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg) {
        emplace(container, *beg);
    }
}

class iStrategy
{
public:
    virtual ~iStrategy() {};
    virtual uint32_t hashBulk(char *buf, int block) = 0;

protected:
    iStrategy();
};

class DefaultHash : public iStrategy
{
public:
    DefaultHash() {};
    ~DefaultHash() {};
    uint32_t hashBulk(char *buf, int) override;
};

class CRC32 : public iStrategy
{
public:
    CRC32() {};
    ~CRC32() {};
    uint32_t hashBulk(char *buf, int block) override;
};

#endif // STRATEGY_H
