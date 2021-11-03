#pragma once
#include <cinttypes>
#include <string>
#include <vector>

namespace Reader {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum class ByteOrder {
    BigEndian,
    LittleEndian
};

class FilestreamReader {
    FilestreamReader() = delete;

    const ByteOrder m_default_order;
    size_t m_buffer_capacity;
    u8* m_buffer;
    FILE* m_file_handle;

    u8 m_current_byte;
    size_t m_loaded_bytes_count = 0;

    bool m_eof = false;
    bool m_error = false;

    size_t m_byte_cursor = 0;
    size_t m_bit_cursor = 0;

    inline bool set_eof(bool eof)
    {
        m_eof = eof;
        return eof;
    }

    inline bool set_error(bool error)
    {
        m_error = error;
        return error;
    }

    void reload_buffer();

    bool reload_byte_if_necessary();

public:
    FilestreamReader(const std::string& file_name, ByteOrder order = ByteOrder::BigEndian, const size_t internal_buffer_capacity = 4096);
    FilestreamReader(const std::string& file_name, const size_t internal_buffer_capacity);
    ~FilestreamReader();

    // Returns and clears the error flag.
    inline bool handle_error()
    {
        bool old = m_error;
        set_error(false);
        return old;
    }

    inline bool end_of_file() { return m_eof; }
    inline bool end_of_buffer() { return m_byte_cursor >= m_loaded_bytes_count; }
    inline bool end_of_byte() { return m_bit_cursor > 7; }
    inline bool end_of_stream() { return end_of_byte() && end_of_buffer() && end_of_file(); }

    // Reads `n` bits. Advances the bit cursor.
    u64 read_bits(u8, const ByteOrder order = ByteOrder::BigEndian);

    // Reads 8 bits. Advances bit (if not aligned) and byte cursor.
    u8 read_byte(const ByteOrder order) { return (u8)read_bits(8, order); }
    u8 read_byte() { return read_byte(m_default_order); }

    // Reads 16 bits and arranges them as per the supplied endianness.
    // Advances bit (if not aligned) and byte cursor.
    u16 read_word(const ByteOrder order) { return (u16)read_bits(16, order); }
    u16 read_word() { return read_word(m_default_order); }

    // Reads 32 bits and arranges them as per the supplied endianness.
    // Advances bit (if not aligned) and byte cursor.
    u32 read_dword(const ByteOrder order) { return (u32)read_bits(32, order); }
    u32 read_dword() { return read_dword(m_default_order); }

    // Reads 64 bits and arranges them as per the supplied endianness.
    // Advances bit (if not aligned) and byte cursor.
    u64 read_qword(const ByteOrder order) { return (u64)read_bits(64, order); }
    u64 read_qword() { return read_qword(m_default_order); }
};

}
