#ifndef STRATEGY_H
#define STRATEGY_H
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/string.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/crc.hpp>

namespace fs = boost::filesystem;
namespace co = boost::container;
using boost::movelib::unique_ptr;

namespace impl {
template<class Iterator>
struct is_random_access_iter
{
    using category = typename std::iterator_traits<Iterator>::iterator_category;
    static const bool value = std::is_same_v<category, std::random_access_iterator_tag>;
};

template<class Container>
struct is_random_access_container
{
    static const bool value = is_random_access_iter<typename Container::iterator>::value;
};
} // namespace impl

template<typename T>
void emplace(T &container, const co::string &s,
         typename std::enable_if_t<impl::is_random_access_container<T>::value> * = {})
{
    container.emplace_back(s);
}

template<typename T>
void emplace(T &container, const co::string &s,
         typename std::enable_if_t<!impl::is_random_access_container<T>::value> * = {})
{
    container.emplace(s);
}

template<typename T>
void split(const co::string &data, T &container)
{
    using boost::char_separator;

    char_separator<char> sep(" ");
    using tokenizer = boost::tokenizer<char_separator<char>, co::string::const_iterator, co::string>;

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
    iStrategy() = default;
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
