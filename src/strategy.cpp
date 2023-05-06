#include "strategy.h"

uint32_t DefaultHash::hashBulk(char *buf, int)
{
    boost::hash<co::string> string_hash;
    return string_hash(buf);
}

uint32_t CRC32::hashBulk(char *buf, int block)
{
    boost::crc_32_type result;
    result.process_bytes(buf, block);
    return result.checksum();
}
