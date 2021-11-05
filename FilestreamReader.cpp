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

bool FilestreamReader::ensure_valid_initialization()
{
    if (m_file_handle == nullptr) {
        dbg_error("Couldn't open file for reading!\n");
        return false;
    }
    reload_buffer();
    m_current_byte = m_buffer[m_byte_cursor++];
    return true;
}

FilestreamReader::FilestreamReader(const std::string& file_name, const ByteOrder order, const size_t internal_buffer_capacity)
    : m_default_order(order)
    , m_buffer_capacity(internal_buffer_capacity)
    , m_buffer(new u8[m_buffer_capacity])
    , m_file_handle(fopen(file_name.c_str(), "r"))
{
    if (!ensure_valid_initialization())
        set_error(true);
}

FilestreamReader::FilestreamReader(const std::string& file_name, const size_t internal_buffer_capacity)
    : m_default_order(ByteOrder::BigEndian)
    , m_buffer_capacity(internal_buffer_capacity)
    , m_buffer(new u8[m_buffer_capacity])
    , m_file_handle(fopen(file_name.c_str(), "r"))
{
    if (!ensure_valid_initialization())
        set_error(true);
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
        if (!reload_byte_if_necessary()) {
            set_error(true);
            return accumulator;
        }
        u8 read_count = min(amount, 8 - m_bit_cursor);
        u8 shift_width = 8 - (m_bit_cursor + read_count);
        u64 extracted_bits = (m_current_byte & (bitmask[read_count] << shift_width)) >> shift_width;

        if (order == ByteOrder::LittleEndian) {
            accumulator |= (extracted_bits << accumulation_count);
            accumulation_count += read_count;
        } else
            accumulator = (accumulator << read_count) | extracted_bits;

        dbg_monitor(16, read_count, m_bit_cursor, extracted_bits, accumulator);

        amount -= read_count;
        m_bit_cursor += read_count;
    }

    return accumulator;
}

size_t FilestreamReader::remaining_bits_in_buffer() const
{
    size_t remaining_full_bytes_in_buffer = m_loaded_bytes_count - m_byte_cursor;
    size_t remaining_bits_in_current_byte = 8 - m_bit_cursor;
    return (remaining_full_bytes_in_buffer * 8) + remaining_bits_in_current_byte;
}

void FilestreamReader::wrap_state(u8 amount)
{
    m_state.file_cursor = ftell(m_file_handle);
    m_state.byte_cursor = m_byte_cursor;
    m_state.bit_cursor = m_bit_cursor;
    m_state.current_byte = m_current_byte;
    m_state.loaded_bytes_count = m_loaded_bytes_count;
    m_state.eof = m_eof;

    size_t remaining = remaining_bits_in_buffer();
    m_state.will_reload_buffer = (amount > remaining);
}

void FilestreamReader::unwrap_state()
{
    size_t seek_position = m_state.file_cursor - m_state.loaded_bytes_count;
    bool failed = fseek(m_file_handle, seek_position, SEEK_SET) == -1;
    if (failed) {
        set_error(true);
        return;
    }
    reload_buffer();
    m_byte_cursor = m_state.byte_cursor;
    m_bit_cursor = m_state.bit_cursor;
    m_current_byte = m_state.current_byte;
    m_loaded_bytes_count = m_state.loaded_bytes_count;
    m_eof = m_state.eof;
}

u64 FilestreamReader::peak_bits(u8 amount, const ByteOrder order)
{
    wrap_state(amount);
    u64 bits = read_bits(amount, order);
    unwrap_state();
    return bits;
}

}
