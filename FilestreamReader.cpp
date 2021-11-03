#include "FilestreamReader.h"

#define DEBUG 0

#if DEBUG
#    include <iostream>
#    define dbg_error(arg) std::cerr << arg

#    include <bitset>
using namespace Reader;

template<size_t ac_width>
void monitor(u8 read_count, u8 bit_cursor, u64 extracted_bits, u64 accumulator)
{
    printf("Read count: %u\n", read_count);
    printf("Bit cursor: %u\n", bit_cursor);
    printf("Extracted: 0x%lx (%s)\n", extracted_bits, std::bitset<8>(extracted_bits).to_string().c_str());
    printf("Accumulated: %s\n\n", std::bitset<ac_width>(accumulator).to_string().c_str());
}

#    define dbg_monitor(ac_width, rc, bc, eb, ac) monitor<ac_width>(rc, bc, eb, ac)
#else
#    define dbg_error(arg)
#    define dbg_monitor(ac_width, rc, bc, eb, ac)
#endif

namespace Reader {
constexpr u8 bitmask[9] = { 0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

inline u8 min(u8 a, u8 b)
{
    return (a < b) ? a : b;
}

FilestreamReader::FilestreamReader(const std::string& file_name, const ByteOrder order, const size_t internal_buffer_capacity)
    : m_default_order(order)
    , m_buffer_capacity(internal_buffer_capacity)
    , m_buffer(new u8[m_buffer_capacity])
    , m_file_handle(fopen(file_name.c_str(), "r"))
{
    reload_buffer();
    m_current_byte = m_buffer[m_byte_cursor++];
}

FilestreamReader::FilestreamReader(const std::string& file_name, const size_t internal_buffer_capacity)
    : m_default_order(ByteOrder::BigEndian)
    , m_buffer_capacity(internal_buffer_capacity)
    , m_buffer(new u8[m_buffer_capacity])
    , m_file_handle(fopen(file_name.c_str(), "r"))
{
    reload_buffer();
    m_current_byte = m_buffer[m_byte_cursor++];
}

FilestreamReader::~FilestreamReader()
{
    if (m_file_handle != nullptr) {
        fclose(m_file_handle);
        m_file_handle = nullptr;
        delete[] m_buffer;
    }
}

void FilestreamReader::reload_buffer()
{
    // Fixme: We are ignoring any possibility of errors.
    m_loaded_bytes_count = fread(m_buffer, 1, m_buffer_capacity, m_file_handle);
    set_eof(m_loaded_bytes_count < m_buffer_capacity);
    if (m_loaded_bytes_count > 0) {
        m_bit_cursor = 0;
        m_byte_cursor = 0;
    }
}

bool FilestreamReader::reload_byte_if_necessary()
{
    if (end_of_byte()) {
        if (end_of_buffer()) {
            reload_buffer();
            if (end_of_stream()) {
                set_error(true);
                dbg_error("Stream was exhausted!\n");
                return false;
            }
        }

        m_current_byte = m_buffer[m_byte_cursor++];
        m_bit_cursor = 0;
    }
    return true;
}

u64 FilestreamReader::read_bits(u8 amount, const ByteOrder order)
{
    if (amount > 64) {
        set_error(true);
        dbg_error("Cannot read more than 64 bits at once!\n");
        return 0;
    }

    u64 accumulator = 0;
    u8 accumulation_count = 0;

    while (amount > 0) {
        if (!reload_byte_if_necessary())
            return accumulator;
        u8 read_count = min(amount, 8 - m_bit_cursor);
        u8 shift_width = 8 - (m_bit_cursor + read_count);
        u64 extracted_bits = (m_current_byte & (bitmask[read_count] << shift_width)) >> shift_width;

        if (order == ByteOrder::LittleEndian) {
            accumulator |= (extracted_bits << accumulation_count);
            accumulation_count += read_count;
        } else
            accumulator = (accumulator << read_count) | extracted_bits;

        dbg_monitor(32, read_count, m_bit_cursor, extracted_bits, accumulator);

        amount -= read_count;
        m_bit_cursor += read_count;
    }

    return accumulator;
}

}
