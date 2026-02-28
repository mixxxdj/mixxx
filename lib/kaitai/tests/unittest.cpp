#ifdef GTEST_NANO
#include "tests/gtest-nano.h"
#else
// These IWYU pragmas are needed for versions of GoogleTest older than 1.12,
// see https://github.com/kaitai-io/kaitai_struct_cpp_stl_runtime/pull/72#issuecomment-2093287161
#include "gtest/gtest.h" // IWYU pragma: keep
// IWYU pragma: no_include <gtest/gtest-message.h>
// IWYU pragma: no_include <gtest/gtest-test-part.h>
// IWYU pragma: no_include "gtest/gtest_pred_impl.h"
#endif

#include "kaitai/kaitaistream.h"
#include "kaitai/exceptions.h"

#include <stdint.h> // int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t

#include <limits> // std::numeric_limits
#include <sstream> // std::istringstream
#include <stdexcept> // std::out_of_range, std::invalid_argument
#include <string> // std::string

#define SETUP_STREAM(...)                                                                  \
    const uint8_t input_bytes[] = { __VA_ARGS__ };                                         \
    std::string input_str(reinterpret_cast<const char*>(input_bytes), sizeof input_bytes); \
    std::istringstream is(input_str);                                                      \
    kaitai::kstream ks(&is);

TEST(KaitaiStreamTest, read_s1)
{
    SETUP_STREAM(42, 0xff, 0x80);
    EXPECT_EQ(ks.read_s1(), 42);
    EXPECT_EQ(ks.read_s1(), -1);
    EXPECT_EQ(ks.read_s1(), -128);
}

TEST(KaitaiStreamTest, read_u1)
{
    SETUP_STREAM(42, 0xff, 0x80);
    EXPECT_EQ(ks.read_u1(), 42);
    EXPECT_EQ(ks.read_u1(), 255);
    EXPECT_EQ(ks.read_u1(), 128);
}

TEST(KaitaiStreamTest, read_f4le)
{
    SETUP_STREAM(208, 15, 73, 64);
    EXPECT_FLOAT_EQ(ks.read_f4le(), 3.14159f);
}

TEST(KaitaiStreamTest, read_f4be)
{
    SETUP_STREAM(64, 73, 15, 208);
    EXPECT_FLOAT_EQ(ks.read_f4be(), 3.14159f);
}

TEST(KaitaiStreamTest, read_f8le)
{
    SETUP_STREAM(110, 134, 27, 240, 249, 33, 9, 64);
    EXPECT_DOUBLE_EQ(ks.read_f8le(), 3.14159);
}

TEST(KaitaiStreamTest, read_f8be)
{
    SETUP_STREAM(64, 9, 33, 249, 240, 27, 134, 110);
    EXPECT_DOUBLE_EQ(ks.read_f8be(), 3.14159);
}

TEST(KaitaiStreamTest, to_string)
{
    EXPECT_EQ(kaitai::kstream::to_string(123), "123");
    EXPECT_EQ(kaitai::kstream::to_string(-123), "-123");
}

// Since `kstream::to_string` must have several overloads (just like
// [`std::to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string)) to
// cover all [standard integer
// types](https://en.cppreference.com/w/cpp/language/types#Properties) while avoiding
// templates, it's a good idea to test whether it actually works with each standard
// integer type. If even just one of the 6 required overloads is missing or not working,
// these tests should be able to detect it.
//
// We test the standard integer types (keywords), not [fixed width integer
// types](https://en.cppreference.com/w/cpp/header/cstdint) (like `int32_t`), because then
// we could potentially have a blind spot: `int32_t` tends to be almost universally
// equivalent to `int`, but `int64_t` is either `long` (typically on 64-bit Linux) or
// `long long` (typically on 64-bit Windows) but not both. So I believe that using
// standard integer types gives us better coverage.

TEST(KaitaiStreamTest, to_string_unsigned_char)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned char>::min()), "0");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned char>::max()), "255");
}

TEST(KaitaiStreamTest, to_string_signed_char)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<signed char>::min()), "-128");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<signed char>::max()), "127");
}

TEST(KaitaiStreamTest, to_string_unsigned_short)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned short>::min()), "0");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned short>::max()), "65535");
}

TEST(KaitaiStreamTest, to_string_short)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<short>::min()), "-32768");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<short>::max()), "32767");
}

TEST(KaitaiStreamTest, to_string_unsigned)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned>::min()), "0");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned>::max()), "4294967295");
}

TEST(KaitaiStreamTest, to_string_int)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<int>::min()), "-2147483648");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<int>::max()), "2147483647");
}

#ifdef _MSC_VER
#pragma warning(push)
// Disable `warning C4127: conditional expression is constant`
// (see https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4127?view=msvc-170)
#pragma warning(disable: 4127)
#endif

TEST(KaitaiStreamTest, to_string_unsigned_long)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned long>::min()), "0");
    if (sizeof(unsigned long) == 4) {
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned long>::max()), "4294967295");
    } else {
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned long>::max()), "18446744073709551615");
    }
}

TEST(KaitaiStreamTest, to_string_long)
{
    if (sizeof(long) == 4) {
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long>::min()), "-2147483648");
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long>::max()), "2147483647");
    } else {
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long>::min()), "-9223372036854775808");
        EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long>::max()), "9223372036854775807");
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

TEST(KaitaiStreamTest, to_string_unsigned_long_long)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned long long>::min()), "0");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<unsigned long long>::max()), "18446744073709551615");
}

TEST(KaitaiStreamTest, to_string_long_long)
{
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long long>::min()), "-9223372036854775808");
    EXPECT_EQ(kaitai::kstream::to_string(std::numeric_limits<long long>::max()), "9223372036854775807");
}

TEST(KaitaiStreamTest, string_to_int)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("123"), 123);
    EXPECT_EQ(kaitai::kstream::string_to_int("-123"), -123);
}

TEST(KaitaiStreamTest, string_to_int_uint8)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0"), std::numeric_limits<uint8_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("255"), std::numeric_limits<uint8_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_int8)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("-128"), std::numeric_limits<int8_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("127"), std::numeric_limits<int8_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_uint16)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0"), std::numeric_limits<uint16_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("65535"), std::numeric_limits<uint16_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_int16)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("-32768"), std::numeric_limits<int16_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("32767"), std::numeric_limits<int16_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_uint32)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0"), std::numeric_limits<uint32_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("4294967295"), std::numeric_limits<uint32_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_int32)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("-2147483648"), std::numeric_limits<int32_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("2147483647"), std::numeric_limits<int32_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_int64)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("-9223372036854775808"), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(kaitai::kstream::string_to_int("9223372036854775807"), std::numeric_limits<int64_t>::max());
}

TEST(KaitaiStreamTest, string_to_int_base13)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0", 13), 0);
    EXPECT_EQ(kaitai::kstream::string_to_int("123", 13), 198);
    EXPECT_EQ(kaitai::kstream::string_to_int("-123", 13), -198);
    EXPECT_EQ(kaitai::kstream::string_to_int("4a7b9c", 13), 1788149);
}

TEST(KaitaiStreamTest, string_to_int_base8)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0", 8), 0);
    EXPECT_EQ(kaitai::kstream::string_to_int("123", 8), 83);
    EXPECT_EQ(kaitai::kstream::string_to_int("-123", 8), -83);
    EXPECT_EQ(kaitai::kstream::string_to_int("777", 8), 511);
}

TEST(KaitaiStreamTest, string_to_int_base16)
{
    EXPECT_EQ(kaitai::kstream::string_to_int("0", 16), 0);
    EXPECT_EQ(kaitai::kstream::string_to_int("123", 16), 291);
    EXPECT_EQ(kaitai::kstream::string_to_int("-123", 16), -291);
    EXPECT_EQ(kaitai::kstream::string_to_int("4a7b9f", 16), 4881311);
}

TEST(KaitaiStreamTest, string_to_int_out_of_range)
{
    // 99999999999999999999999 = 0x152d_02c7_e14a_f67f_ffff =>
    // will need at least 77 bits to store, so should fail on 64-bit int
    try {
        kaitai::kstream::string_to_int("99999999999999999999999");
        FAIL() << "Expected out_of_range exception";
    } catch (std::out_of_range& e) {
        EXPECT_EQ(e.what(), std::string("string_to_int"));
    }
}

TEST(KaitaiStreamTest, string_to_int_garbage)
{
    try {
        kaitai::kstream::string_to_int("123.^&@#!@");
        FAIL() << "Expected invalid_argument exception";
    } catch (std::invalid_argument& e) {
        EXPECT_EQ(e.what(), std::string("string_to_int"));
    }
}

// Tests a successful zlib decompression.
TEST(KaitaiStreamTest, process_zlib_ok)
{
    /*
    Python code to generate (used Python 3.10.12 and
    `zlib.ZLIB_RUNTIME_VERSION == zlib.ZLIB_VERSION == '1.2.11'`):

    ```python
    import zlib
    data = zlib.compress(b"Hi")
    print(", ".join([f"0x{b:02x}" for b in data]))
    ```
    */
    SETUP_STREAM(0x78, 0x9c, 0xf3, 0xc8, 0x04, 0x00, 0x00, 0xfb, 0x00, 0xb2)
    EXPECT_EQ(kaitai::kstream::process_zlib(ks.read_bytes_full()), "Hi");
}

// It's probably not a good idea to run this test in CI because it has to allocate 4 GiB of memory.
// That's why it is disabled (see
// https://google.github.io/googletest/advanced.html#temporarily-disabling-tests). You can still run
// it locally using `.build/run-unittest --valgrind -- --gtest_also_run_disabled_tests` or
// `.build\run-unittest.ps1 --gtest_also_run_disabled_tests`.
TEST(KaitaiStreamTest, DISABLED_process_zlib_input_too_long)
{
    try {
        kaitai::kstream::process_zlib(
            std::string(
                static_cast<std::string::size_type>(std::numeric_limits<unsigned>::max()) + 1,
                '\x00'
            )
        );
        FAIL() << "Expected runtime_error exception";
    } catch (const std::length_error& e) {
        EXPECT_EQ(e.what(), std::string(
            "process_zlib: input is 4294967296 bytes long, which exceeds"
            " the maximum supported length of 4294967295 bytes"
        ));
    }
}

// Tests a failed zlib decompression due to the `inflate()` function returning `Z_BUF_ERROR`.
TEST(KaitaiStreamTest, process_zlib_z_buf_error)
{
    // The same bytes as in the previous `process_zlib_ok` test, but truncated (without the last byte).
    /*
    Python code to generate (used Python 3.10.12 and
    `zlib.ZLIB_RUNTIME_VERSION == zlib.ZLIB_VERSION == '1.2.11'`):

    ```python
    import zlib
    data = zlib.compress(b"Hi")
    truncated_data = data[:-1]
    print(", ".join([f"0x{b:02x}" for b in truncated_data]))
    ```
    */
    SETUP_STREAM(0x78, 0x9c, 0xf3, 0xc8, 0x04, 0x00, 0x00, 0xfb, 0x00)
    try {
        kaitai::kstream::process_zlib(ks.read_bytes_full());
        FAIL() << "Expected runtime_error exception";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(e.what(), std::string("process_zlib: inflate() failed: incomplete or truncated input data"));
    }
}

// Tests a failed zlib decompression due to the `inflate()` function returning `Z_DATA_ERROR`.
TEST(KaitaiStreamTest, process_zlib_z_data_error)
{
    /*
    Python code to generate (used Python 3.10.12 and
    `zlib.ZLIB_RUNTIME_VERSION == zlib.ZLIB_VERSION == '1.2.11'`):

    ```python
    import zlib
    data = bytearray(zlib.compress(b"Hi"))
    # Just change the value of `FCHECK` (see https://www.rfc-editor.org/rfc/rfc1950.html#page-5),
    # which will invalidate the zlib header
    data[1] ^= 0x01
    print(", ".join([f"0x{b:02x}" for b in data]))
    ```
    */
    SETUP_STREAM(0x78, 0x9d, 0xf3, 0xc8, 0x04, 0x00, 0x00, 0xfb, 0x00, 0xb2)
    try {
        kaitai::kstream::process_zlib(ks.read_bytes_full());
        FAIL() << "Expected runtime_error exception";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(e.what(), std::string("process_zlib: inflate() failed: incorrect header check"));
    }
}

// Tests a failed zlib decompression due to the `inflate()` function returning `Z_NEED_DICT`.
TEST(KaitaiStreamTest, process_zlib_z_need_dict)
{
    /*
    Python code to generate (used Python 3.10.12 and
    `zlib.ZLIB_RUNTIME_VERSION == zlib.ZLIB_VERSION == '1.2.11'`):

    ```python
    import zlib
    co = zlib.compressobj(zdict=b"foobar")
    data = (co.compress(b"foobar") + co.flush())
    print(", ".join([f"0x{b:02x}" for b in data]))
    ```
    */
    SETUP_STREAM(0x78, 0xbb, 0x08, 0xab, 0x02, 0x7a, 0x4b, 0x03, 0x93, 0x00, 0x08, 0xab, 0x02, 0x7a)
    try {
        kaitai::kstream::process_zlib(ks.read_bytes_full());
        FAIL() << "Expected runtime_error exception";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(e.what(), std::string("process_zlib: inflate() failed: preset dictionary needed"));
    }
}

TEST(KaitaiStreamTest, bytes_to_str_ascii)
{
    std::string res = kaitai::kstream::bytes_to_str("Hello, world!", "ASCII");
    EXPECT_EQ(res, "Hello, world!");
}

TEST(KaitaiStreamTest, bytes_to_str_empty_ascii)
{
    std::string res = kaitai::kstream::bytes_to_str("", "ASCII");
    EXPECT_EQ(res, "");
}

TEST(KaitaiStreamTest, bytes_to_str_empty_utf16le)
{
    std::string res = kaitai::kstream::bytes_to_str("", "UTF-16LE");
    EXPECT_EQ(res, "");
}

TEST(KaitaiStreamTest, bytes_to_str_empty_utf16be)
{
    std::string res = kaitai::kstream::bytes_to_str("", "UTF-16BE");
    EXPECT_EQ(res, "");
}

#ifndef KS_STR_ENCODING_NONE
TEST(KaitaiStreamTest, bytes_to_str_iso_8859_1)
{
    std::string res = kaitai::kstream::bytes_to_str("\xC4\xD6\xDC\xE4\xF6\xFC\x80\x9F\xDF\xA6", "ISO-8859-1");
    EXPECT_EQ(res,
        "\xC3\x84"  // U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS
        "\xC3\x96"  // U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS
        "\xC3\x9C"  // U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS
        "\xC3\xA4"  // U+00E4 LATIN SMALL LETTER A WITH DIAERESIS
        "\xC3\xB6"  // U+00F6 LATIN SMALL LETTER O WITH DIAERESIS
        "\xC3\xBC"  // U+00FC LATIN SMALL LETTER U WITH DIAERESIS
        "\xC2\x80"  // U+0080 <Padding Character> (PAD)
        "\xC2\x9F"  // U+009F <Application Program Command> (APC)
        "\xC3\x9F"  // U+00DF LATIN SMALL LETTER SHARP S
        "\xC2\xA6"  // U+00A6 BROKEN BAR
    );
}

TEST(KaitaiStreamTest, bytes_to_str_iso_8859_15)
{
    std::string res = kaitai::kstream::bytes_to_str("\xC4\xD6\xDC\xE4\xF6\xFC\x80\x9F\xDF\xA6", "ISO-8859-15");
    EXPECT_EQ(res,
        "\xC3\x84"  // U+00C4 LATIN CAPITAL LETTER A WITH DIAERESIS
        "\xC3\x96"  // U+00D6 LATIN CAPITAL LETTER O WITH DIAERESIS
        "\xC3\x9C"  // U+00DC LATIN CAPITAL LETTER U WITH DIAERESIS
        "\xC3\xA4"  // U+00E4 LATIN SMALL LETTER A WITH DIAERESIS
        "\xC3\xB6"  // U+00F6 LATIN SMALL LETTER O WITH DIAERESIS
        "\xC3\xBC"  // U+00FC LATIN SMALL LETTER U WITH DIAERESIS
        "\xC2\x80"  // U+0080 <Padding Character> (PAD)
        "\xC2\x9F"  // U+009F <Application Program Command> (APC)
        "\xC3\x9F"  // U+00DF LATIN SMALL LETTER SHARP S
        "\xC5\xA0"  // U+0160 LATIN CAPITAL LETTER S WITH CARON
    );
}

TEST(KaitaiStreamTest, bytes_to_str_windows1252)
{
    std::string res = kaitai::kstream::bytes_to_str("\x80\x9F\xDF\xA6", "windows-1252");
    EXPECT_EQ(res,
        "\xE2\x82\xAC"  // U+20AC EURO SIGN
        "\xC5\xB8"      // U+0178 LATIN CAPITAL LETTER Y WITH DIAERESIS
        "\xC3\x9F"      // U+00DF LATIN SMALL LETTER SHARP S
        "\xC2\xA6"      // U+00A6 BROKEN BAR
    );
}

TEST(KaitaiStreamTest, bytes_to_str_gb2312)
{
    std::string res = kaitai::kstream::bytes_to_str("\xC4\xE3\xBA\xC3\xCA\xC0\xBD\xE7", "GB2312");
    EXPECT_EQ(res,
        "\xE4\xBD\xA0"  // U+4F60 CJK UNIFIED IDEOGRAPH-4F60
        "\xE5\xA5\xBD"  // U+597D CJK UNIFIED IDEOGRAPH-597D
        "\xE4\xB8\x96"  // U+4E16 CJK UNIFIED IDEOGRAPH-4E16
        "\xE7\x95\x8C"  // U+754C CJK UNIFIED IDEOGRAPH-754C
    );
}

TEST(KaitaiStreamTest, bytes_to_str_ibm437)
{
    std::string res = kaitai::kstream::bytes_to_str("\xCC\xB2\x40", "IBM437");
    EXPECT_EQ(res,
        "\xE2\x95\xA0"  // U+2560 BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
        "\xE2\x96\x93"  // U+2593 DARK SHADE
        "\x40"          // U+0040 COMMERCIAL AT
    );
}

TEST(KaitaiStreamTest, bytes_to_str_utf16le)
{
    // NB: UTF-16 bytes representation will have binary zeroes in the middle, so we need
    // to convert it to std::string with explicit length
    std::string res = kaitai::kstream::bytes_to_str(std::string("\x41\x00\x42\x00\x91\x25\x70\x24", 8), "UTF-16LE");
    EXPECT_EQ(res,
        "AB"
        "\xE2\x96\x91"  // U+2591 LIGHT SHADE
        "\xE2\x91\xB0"  // U+2470 CIRCLED NUMBER SEVENTEEN
    );
}

TEST(KaitaiStreamTest, bytes_to_str_utf16be)
{
    // NB: UTF-16 bytes representation will have binary zeroes in the middle, so we need
    // to convert it to std::string with explicit length
    std::string res = kaitai::kstream::bytes_to_str(std::string("\x00\x41\x00\x42\x25\x91\x24\x70", 8), "UTF-16BE");
    EXPECT_EQ(res,
        "AB"
        "\xE2\x96\x91"  // U+2591 LIGHT SHADE
        "\xE2\x91\xB0"  // U+2470 CIRCLED NUMBER SEVENTEEN
    );
}

TEST(KaitaiStreamTest, bytes_to_str_big_dest)
{
    // Prepare a string in IBM437 that is reasonably big, fill it with U+2248 ALMOST EQUAL TO
    // character, which is just 1 byte 0xFB in IBM437.
    const int len = 10000000;
    std::string src(len, '\xF7');

    std::string res = kaitai::kstream::bytes_to_str(src, "IBM437");

    // In UTF-8, result is expected to be 3x more: every U+2248 is rendered as 3 bytes "\xE2\x89\x88"
    EXPECT_EQ(res.length(), len * 3);
}

TEST(KaitaiStreamTest, bytes_to_str_unknown_encoding)
{
    try {
        std::string res = kaitai::kstream::bytes_to_str("abc", "invalid");
        FAIL() << "Expected unknown_encoding exception";
    } catch (const kaitai::unknown_encoding& e) {
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: unknown encoding: `invalid`"));
    }
}

// If the bytes_to_str implementation treats the empty string as a special case, it should
// make sure to throw an unknown_encoding exception even if the input string is empty (in
// other words, the check for an empty string and possible early successful return should
// only be done after checking the encoding).
TEST(KaitaiStreamTest, bytes_to_str_unknown_encoding_empty)
{
    try {
        std::string res = kaitai::kstream::bytes_to_str("", "invalid");
        FAIL() << "Expected unknown_encoding exception";
    } catch (const kaitai::unknown_encoding& e) {
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: unknown encoding: `invalid`"));
    }
}

// Different libraries have different ideas of what they consider "illegal sequence", so
// we end up testing several cases which seem to be flagged by everybody equally.

TEST(KaitaiStreamTest, bytes_to_str_invalid_seq_euc_jp_too_short)
{
    // In EUC-JP, 0xb0 introduces a sequence of 2 bytes in so-called "code set 1", but 0xb0
    // by itself is invalid.

    try {
        std::string res = kaitai::kstream::bytes_to_str("\xb0", "EUC-JP");
        FAIL() << "Expected illegal_seq_in_encoding exception";
    } catch (const kaitai::illegal_seq_in_encoding& e) {
#ifdef KS_STR_ENCODING_ICONV
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: EINVAL"));
#elif defined(KS_STR_ENCODING_WIN32API)
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: MultiByteToWideChar"));
#else
#error Unknown KS_STR_ENCODING
#endif
    }
}

TEST(KaitaiStreamTest, bytes_to_str_invalid_seq_gb2312_too_short)
{
    try {
        std::string res = kaitai::kstream::bytes_to_str("\xb0", "GB2312");
        FAIL() << "Expected illegal_seq_in_encoding exception";
    } catch (const kaitai::illegal_seq_in_encoding& e) {
#ifdef KS_STR_ENCODING_ICONV
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: EINVAL"));
#elif defined(KS_STR_ENCODING_WIN32API)
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: MultiByteToWideChar"));
#else
#error Unknown KS_STR_ENCODING
#endif
    }
}

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__APPLE__)
TEST(KaitaiStreamTest, DISABLED_bytes_to_str_invalid_seq_gb2312_two_bytes)
#else
TEST(KaitaiStreamTest, bytes_to_str_invalid_seq_gb2312_two_bytes)
#endif
{
    // 0xB0 0x30 is illegal sequence in GB2312: 0xB0 must be followed by [0xA1..0xFE]. However,
    // some iconv engines, namely CITRUS integrated with modern FreeBSD (10+) and NetBSD, are
    // not considering this an error and thus not returning EILSEQ. Iconv preinstalled in the
    // GitHub Actions `macos-14` runner image does not consider this an error either.
    //
    try {
        std::string res = kaitai::kstream::bytes_to_str("\xb0\x30", "GB2312");
        FAIL() << "Expected illegal_seq_in_encoding exception";
    } catch (const kaitai::illegal_seq_in_encoding& e) {
#ifdef KS_STR_ENCODING_ICONV
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: EILSEQ"));
#elif defined(KS_STR_ENCODING_WIN32API)
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: MultiByteToWideChar"));
#else
#error Unknown KS_STR_ENCODING
#endif
    }
}

TEST(KaitaiStreamTest, bytes_to_str_invalid_seq_utf16le_odd_bytes)
{
    // UTF-16 requires even number of bytes, so 3 bytes is incomplete UTF-16 sequence.
    try {
        std::string res = kaitai::kstream::bytes_to_str("abc", "UTF-16LE");
        FAIL() << "Expected illegal_seq_in_encoding exception";
    } catch (const kaitai::illegal_seq_in_encoding& e) {
#ifdef KS_STR_ENCODING_ICONV
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: EINVAL"));
#elif defined(KS_STR_ENCODING_WIN32API)
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: incomplete"));
#else
#error Unknown KS_STR_ENCODING
#endif
    }
}

TEST(KaitaiStreamTest, bytes_to_str_invalid_seq_utf16le_incomplete_high_surrogate)
{
    // UTF-16 disallows having high surrogate (any value in the range of 0xd800..0xdbff) not
    // followed by low surrogate (any value in the range of 0xdc00..0xdfff).
    try {
        std::string res = kaitai::kstream::bytes_to_str("\xd8\xd8", "UTF-16LE");
        FAIL() << "Expected illegal_seq_in_encoding exception";
    } catch (const kaitai::illegal_seq_in_encoding& e) {
#ifdef KS_STR_ENCODING_ICONV
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: EINVAL"));
#elif defined(KS_STR_ENCODING_WIN32API)
        EXPECT_EQ(e.what(), std::string("bytes_to_str error: illegal sequence: WideCharToMultiByte"));
#else
#error Unknown KS_STR_ENCODING
#endif
    }
}
#endif

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
