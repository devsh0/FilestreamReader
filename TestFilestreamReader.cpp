#include "FilestreamReader.h"
#include <iostream>

namespace Reader {

class TestFilestreamReader {
    static constexpr const char* s_path_8b_dat = "../test-files/8b.dat";
    static constexpr const char* s_path_9b_dat = "../test-files/9b.dat";

    std::string m_current;

    void register_new(const std::string& name)
    {
        m_current.resize(name.size());
        for (unsigned i = 0; i < name.size(); i++) {
            char ch = name[i];
            ch = ch == '_' ? ch : (char)(ch - 32);
            m_current[i] = ch;
        }
    }

    void report_passed()
    {
        std::cout << "\033[33m" << m_current;
        std::cout << "\033[32;1m - PASSED";
        std::cout << "\033[0m";
        std::cout << "\n";
    }

    void report_failed()
    {
        std::cout << "\033[33m" << m_current;
        std::cout << "\033[31;1m - FAILED";
        std::cout << "\033[0m";
        std::cout << "\n";
    }

public:
#define expect(condition) \
    if (!(condition)) {   \
        report_failed();  \
        return;           \
    }

    void test_reading_aligned_bytes()
    {
        static constexpr u8 bytes[] = { 0xff, 0x10, 0xab, 0x30, 0x63, 0x58, 0xd7, 0x45 };
        register_new("reading_aligned_bytes");
        FilestreamReader reader(s_path_8b_dat);
        for (auto byte : bytes)
            expect(byte == reader.read_byte());
        report_passed();
    }

    void test_reading_aligned_big_endian_words()
    {
        static constexpr u16 words[] = { 0xff10, 0xab30, 0x6358, 0xd745 };
        register_new("reading_aligned_big_endian_words");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        for (auto word : words)
            expect(word == reader.read_word());
        report_passed();
    }

    void test_reading_aligned_little_endian_words()
    {
        register_new("reading_aligned_little_endian_words");
        static constexpr u16 words[] = { 0x10ff, 0x30ab, 0x5863, 0x45d7 };
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        for (auto word : words)
            expect(word == reader.read_word());
        report_passed();
    }

    void test_reading_aligned_big_endian_dwords()
    {
        static constexpr u32 dwords[] = { 0xff10ab30, 0x6358d745 };
        register_new("reading_aligned_big_endian_dwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        for (auto dword : dwords)
            expect(dword == reader.read_dword());
        report_passed();
    }

    void test_reading_aligned_little_endian_dwords()
    {
        static constexpr u32 dwords[] = { 0x30ab10ff, 0x45d75863 };
        register_new("reading_aligned_little_endian_dwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        for (auto dword : dwords)
            expect(dword == reader.read_dword());
        report_passed();
    }

    void test_reading_aligned_big_endian_qwords()
    {
        static constexpr u64 qwords[] = { 0xff10ab306358d745 };
        register_new("reading_aligned_big_endian_qwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        for (auto qword : qwords)
            expect(qword == reader.read_qword());
        report_passed();
    }

    void test_reading_aligned_little_endian_qwords()
    {
        static constexpr u64 qwords[] = { 0x45d7586330ab10ff };
        register_new("reading_aligned_little_endian_qwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        for (auto qword : qwords)
            expect(qword == reader.read_qword());
        report_passed();
    }

    // Fixme: Misalignment poses an interesting question. What should we do
    //  when reading `k` bits such that the next `k` bits end up touching multiple
    //  adjacent bytes? Should we care about endianness at this point and shift
    //  appropriately at the edge of the physical byte while filling an octet?
    //  I am not sure what's the appropriate thing to do in this case. Currently,
    //  we proceed with endian-aware shifts.
    void test_reading_unaligned_big_endian_bytes()
    {
        register_new("reading_unaligned_big_endian_bytes");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        reader.read_bits(3); // Disturb alignment.

        expect(reader.read_byte() == 0b11111000);

        // Read upto 7th bit of the second byte.
        reader.read_bits(4);
        expect(reader.read_byte() == 0b01010101);
        report_passed();
    }

    void test_reading_unaligned_little_endian_bytes()
    {
        register_new("reading_unaligned_little_endian_bytes");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        reader.read_bits(3); // Disturb alignment.

        expect(reader.read_byte() == 0b00011111);

        // Read upto 7th bit of the second byte.
        reader.read_bits(4);
        expect(reader.read_byte() == 0b10101010);
        report_passed();
    }

    void test_reading_unaligned_big_endian_words()
    {
        register_new("reading_unaligned_big_endian_words");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        reader.read_bits(7);
        expect(reader.read_word() == 0b1000100001010101)
        report_passed();
    }

    void test_reading_unaligned_little_endian_words()
    {
        register_new("reading_unaligned_little_endian_words");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        reader.read_bits(7);
        expect(reader.read_word() == 0b1010101000100001)
        report_passed();
    }

    void test_reading_unaligned_big_endian_dwords()
    {
        register_new("reading_unaligned_big_endian_dwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::BigEndian);
        reader.read_bits(7);
        u32 constant = 0b10001000010101011001100000110001;
        expect(reader.read_dword() == constant);
        report_passed();
    }

    void test_reading_unaligned_little_endian_dwords()
    {
        register_new("reading_unaligned_little_endian_dwords");
        FilestreamReader reader(s_path_8b_dat, ByteOrder::LittleEndian);
        reader.read_bits(7);
        u32 constant = 0b01100010011000010101011000100001;
        expect(reader.read_dword() == constant);
        report_passed();
    }

    void test_reading_unaligned_big_endian_qwords()
    {
        register_new("reading_unaligned_big_endian_qwords");
        FilestreamReader reader(s_path_9b_dat, ByteOrder::BigEndian);
        reader.read_bits(7);
        u64 constant = 0b1000100001010101100110000011000110101100011010111010001010111011;
        expect(reader.read_qword() == constant);
        report_passed();
    }

    void test_reading_unaligned_little_endian_qwords()
    {
        register_new("reading_unaligned_little_endian_qwords");
        FilestreamReader reader(s_path_9b_dat, ByteOrder::LittleEndian);
        reader.read_bits(7);
        u64 constant = 0b0111011010001011101011101011000011000110011000010101011000100001;
        expect(reader.read_qword() == constant);
        report_passed();
    }

    void test_error_flag_is_set_when_reading_past_the_file()
    {
        register_new("error_flag_is_set_when_reading_past_the_file");
        FilestreamReader reader(s_path_8b_dat);
        reader.read_qword();
        expect(reader.handle_error() == false);

        // At this point we are reading past the file.
        reader.read_byte();
        expect(reader.handle_error() == true);
        report_passed();
    }

    void test_end_of_byte_flag_is_set_when_byte_is_fully_consumed()
    {
        register_new("end_of_byte_flag_is_set_when_byte_is_fully_consumed");
        FilestreamReader reader(s_path_8b_dat);
        expect(reader.end_of_byte() == false);
        reader.read_bits(4);
        expect(reader.end_of_byte() == false);
        reader.read_bits(4);
        expect(reader.end_of_byte() == true);
        report_passed();
    }

    void test_end_of_buffer_flag_is_set_when_buffer_is_exhausted()
    {
        register_new("end_of_buffer_flag_is_set_when_buffer_is_exhausted");
        FilestreamReader reader(s_path_8b_dat, 8);

        expect(reader.end_of_buffer() == false);
        reader.read_qword();
        expect(reader.end_of_buffer() == true);
        report_passed();
    }

    void test_end_of_file_flag_is_set_when_file_is_fully_read()
    {
        register_new("end_of_file_flag_is_set_when_file_is_fully_read");
        FilestreamReader reader(s_path_8b_dat, 8);

        expect(reader.end_of_file() == false);
        reader.read_qword();
        expect(reader.end_of_file() == false);

        // At this point we are reading past the file.
        reader.read_byte();
        expect(reader.end_of_file() == true);
        report_passed();
    }

    void test_aligned_reads()
    {
        test_reading_aligned_bytes();

        test_reading_aligned_big_endian_words();
        test_reading_aligned_little_endian_words();

        test_reading_aligned_big_endian_dwords();
        test_reading_aligned_little_endian_dwords();

        test_reading_aligned_big_endian_qwords();
        test_reading_aligned_little_endian_qwords();
    }

    void test_error_flag_is_set_if_file_does_not_exist()
    {
        register_new("error_flag_is_set_if_file_does_not_exist");
        FilestreamReader reader("non-existent.file");
        expect(reader.handle_error() == true);
        expect(!reader);
        report_passed();
    }

    void test_remaining_bits_in_buffer()
    {
#define bytes 8
        register_new("remaining_bits_in_buffer");
        FilestreamReader reader(s_path_8b_dat);
        expect(reader.remaining_bits_in_buffer() == 8 * bytes);
        reader.read_bits(7 * bytes);
        expect(reader.remaining_bits_in_buffer() == 1 * bytes);
        reader.read_byte();
        expect(reader.remaining_bits_in_buffer() == 0 * bytes);
        report_passed();
#undef bytes
    }

    void test_peaking_beyond_the_edge_of_buffer()
    {
        register_new("peaking_beyond_the_edge_of_buffer");
        FilestreamReader reader(s_path_8b_dat, 1);
        reader.read_bits(7);
        expect(reader.peak_word() == 0b1000100001010101);
        expect(reader.read_word() == 0b1000100001010101);
        report_passed();
    }

    void test_peaking_beyond_the_end_of_file()
    {
        register_new("peaking_beyond_the_end_of_file");
        FilestreamReader reader(s_path_8b_dat, 2);
        reader.read_word();
        reader.peak_bits(64);
        expect(reader.handle_error() == true);
        u8 bytes[] = {0xab, 0x30, 0x63, 0x58, 0xd7, 0x45};
        for (u8 byte : bytes)
            expect(reader.read_byte() == byte);
        expect(reader.handle_error() == false);
        report_passed();
    }

    void test_unaligned_reads()
    {
        test_reading_unaligned_big_endian_bytes();
        test_reading_unaligned_little_endian_bytes();

        test_reading_unaligned_big_endian_words();
        test_reading_unaligned_little_endian_words();

        test_reading_unaligned_big_endian_dwords();
        test_reading_unaligned_little_endian_dwords();

        test_reading_unaligned_big_endian_qwords();
        test_reading_unaligned_little_endian_qwords();
    }

    void test_peaking()
    {
        test_peaking_beyond_the_edge_of_buffer();
        test_peaking_beyond_the_end_of_file();
    }

    void run_all()
    {
        test_aligned_reads();
        test_unaligned_reads();
        test_peaking();
        test_error_flag_is_set_when_reading_past_the_file();
        test_end_of_buffer_flag_is_set_when_buffer_is_exhausted();
        test_end_of_byte_flag_is_set_when_byte_is_fully_consumed();
        test_end_of_file_flag_is_set_when_file_is_fully_read();
        test_error_flag_is_set_if_file_does_not_exist();
        test_remaining_bits_in_buffer();
    }
};
}

int main()
{
    Reader::TestFilestreamReader test;
    test.run_all();
    return 0;
}
