#include "strategy.h"

iStrategy::iStrategy()
{

}

uint32_t DefaultHash::hashBulk(char *buf, int)
{
    boost::hash<string> string_hash;
    return string_hash(buf);
}

uint32_t CRC32::hashBulk(char *buf, int block)
{
    boost::crc_32_type result;
    result.process_bytes(buf, block);
    return result.checksum();
}
