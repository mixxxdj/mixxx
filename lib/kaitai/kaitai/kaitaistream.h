#ifndef KAITAI_STREAM_H
#define KAITAI_STREAM_H

// Kaitai Struct runtime API version: x.y.z = 'xxxyyyzzz' decimal
#define KAITAI_STRUCT_VERSION 11000L

// check for C++11 support - https://stackoverflow.com/a/40512515
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
#define KAITAI_STREAM_H_CPP11_SUPPORT
#endif

#include <stdint.h> // int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t

#include <ios> // std::streamsize, forward declaration of std::istream  // IWYU pragma: keep
#include <cstddef> // std::size_t
#include <climits> // LLONG_MAX, ULLONG_MAX
#include <sstream> // std::istringstream  // IWYU pragma: keep
#include <string> // std::string

namespace kaitai {

/**
 * Kaitai Stream class (kaitai::kstream) is an implementation of
 * <a href="https://doc.kaitai.io/stream_api.html">Kaitai Struct stream API</a>
 * for C++/STL. It's implemented as a wrapper over generic STL std::istream.
 *
 * It provides a wide variety of simple methods to read (parse) binary
 * representations of primitive types, such as integer and floating
 * point numbers, byte arrays and strings, and also provides stream
 * positioning / navigation methods with unified cross-language and
 * cross-toolkit semantics.
 *
 * Typically, end users won't access Kaitai Stream class manually, but would
 * describe a binary structure format using .ksy language and then would use
 * Kaitai Struct compiler to generate source code in desired target language.
 * That code, in turn, would use this class and API to do the actual parsing
 * job.
 */
class kstream {
public:
    /**
     * Constructs new Kaitai Stream object, wrapping a given std::istream.
     * \param io istream object to use for this Kaitai Stream
     */
    kstream(std::istream* io);

    /**
     * Constructs new Kaitai Stream object, wrapping a given in-memory data
     * buffer.
     * \param data data buffer to use for this Kaitai Stream
     */
    kstream(const std::string& data);

    void close();

    /** @name Stream positioning */
    //@{
    /**
     * Check if stream pointer is at the end of stream. Note that the semantics
     * are different from traditional STL semantics: one does *not* need to do a
     * read (which will fail) after the actual end of the stream to trigger EOF
     * flag, which can be accessed after that read. It is sufficient to just be
     * at the end of the stream for this method to return true.
     * \return "true" if we are located at the end of the stream.
     */
    bool is_eof() const;

    /**
     * Set stream pointer to designated position.
     * \param pos new position (offset in bytes from the beginning of the stream)
     */
    void seek(uint64_t pos);

    /**
     * Get current position of a stream pointer.
     * \return pointer position, number of bytes from the beginning of the stream
     */
    uint64_t pos();

    /**
     * Get total size of the stream in bytes.
     * \return size of the stream in bytes
     */
    uint64_t size();
    //@}

    /** @name Integer numbers */
    //@{

    // ------------------------------------------------------------------------
    // Signed
    // ------------------------------------------------------------------------

    int8_t read_s1();

    // ........................................................................
    // Big-endian
    // ........................................................................

    int16_t read_s2be();
    int32_t read_s4be();
    int64_t read_s8be();

    // ........................................................................
    // Little-endian
    // ........................................................................

    int16_t read_s2le();
    int32_t read_s4le();
    int64_t read_s8le();

    // ------------------------------------------------------------------------
    // Unsigned
    // ------------------------------------------------------------------------

    uint8_t read_u1();

    // ........................................................................
    // Big-endian
    // ........................................................................

    uint16_t read_u2be();
    uint32_t read_u4be();
    uint64_t read_u8be();

    // ........................................................................
    // Little-endian
    // ........................................................................

    uint16_t read_u2le();
    uint32_t read_u4le();
    uint64_t read_u8le();

    //@}

    /** @name Floating point numbers */
    //@{

    // ........................................................................
    // Big-endian
    // ........................................................................

    float read_f4be();
    double read_f8be();

    // ........................................................................
    // Little-endian
    // ........................................................................

    float read_f4le();
    double read_f8le();

    //@}

    /** @name Unaligned bit values */
    //@{

    void align_to_byte();
    uint64_t read_bits_int_be(int n);
    uint64_t read_bits_int(int n);
    uint64_t read_bits_int_le(int n);

    //@}

    /** @name Byte arrays */
    //@{

    std::string read_bytes(std::streamsize len);
    std::string read_bytes_full();
    std::string read_bytes_term(char term, bool include, bool consume, bool eos_error);
    std::string read_bytes_term_multi(std::string term, bool include, bool consume, bool eos_error);
    std::string ensure_fixed_contents(std::string expected);

    static std::string bytes_strip_right(std::string src, char pad_byte);
    static std::string bytes_terminate(std::string src, char term, bool include);
    static std::string bytes_terminate_multi(std::string src, std::string term, bool include);
    static std::string bytes_to_str(const std::string src, const char *src_enc);

    //@}

    /** @name Byte array processing */
    //@{

    /**
     * Performs XOR processing on the given data, XORing each byte of the input with a
     * single-byte key.
     * @param data data to process
     * @param key byte value to XOR with
     * @return processed data
     */
    static std::string process_xor_one(std::string data, uint8_t key);

    /**
     * Performs XOR processing on the given data, XORing all bytes of the input with a
     * multi-byte key, repeating the key as many times as necessary (if the input data is
     * longer than the key).
     * @param data data to process
     * @param key array of bytes to XOR with
     * @return processed data
     */
    static std::string process_xor_many(std::string data, std::string key);

    /**
     * Performs a circular left rotation shift for a given buffer by a given amount of bits,
     * using groups of 1 bytes each time. Right circular rotation should be performed
     * using this procedure with corrected amount.
     * @param data source data to process
     * @param amount number of bits to shift by
     * @return copy of source array with requested shift applied
     */
    static std::string process_rotate_left(std::string data, int amount);

#ifdef KS_ZLIB
    /**
     * Performs an unpacking ("inflation") of zlib-compressed data with usual zlib headers.
     * @param data data to unpack
     * @return unpacked data
     * @throws IOException
     */
    static std::string process_zlib(std::string data);
#endif

    //@}

    /**
     * Performs modulo operation between two integers: dividend `a`
     * and divisor `b`. Divisor `b` is expected to be positive. The
     * result is always 0 <= x <= b - 1.
     */
    static int mod(int a, int b);

    // NB: the following 6 overloads of `to_string` are exactly the ones that
    // [`std::to_string`](https://en.cppreference.com/w/cpp/string/basic_string/to_string) has.
    // Testing has shown that they are all necessary: if you remove any of them, you will get
    // something like `error: call to 'to_string' is ambiguous` when trying to call `to_string`
    // with the integer type for which you removed the overload.

    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(int)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(int val) {
        return to_string_signed(val);
    }

    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(long)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(long val) {
        return to_string_signed(val);
    }

// The `long long` type is technically available only since C++11. It is usually
// supported even by ancient compilers that have very limited C++11 support, but we can
// still check if the `LLONG_MAX` macro is defined (it seems very unlikely that it is not,
// though).
#ifdef LLONG_MAX
    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(long long)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(long long val) {
        return to_string_signed(val);
    }
#endif

    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(unsigned)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(unsigned val) {
        return to_string_unsigned(val);
    }

    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(unsigned long)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(unsigned long val) {
        return to_string_unsigned(val);
    }

// The `unsigned long long` type is technically available only since C++11. It is usually
// supported even by ancient compilers that have very limited C++11 support, but we can
// still check if the `LLONG_MAX` macro is defined (it seems very unlikely that it is not,
// though).
#ifdef ULLONG_MAX
    /**
     * Converts given integer `val` to a decimal string representation.
     * Should be used in place of `std::to_string(unsigned long long)` (which is available only
     * since C++11) in older C++ implementations.
     */
    static std::string to_string(unsigned long long val) {
        return to_string_unsigned(val);
    }
#endif

    /**
     * Converts string `str` to an integer value. Throws an exception if the
     * string is not a valid integer.
     *
     * This one is supposed to mirror `std::stoll()` (which is available only
     * since C++11) in older C++ implementations.
     *
     * Major difference between standard `std::stoll()` and `string_to_int()`
     * is that this one does not perform any partial conversions and always
     * throws `std::invalid_argument` if the string is not a valid integer.
     *
     * @param str String to convert
     * @param base Base of the integer (default: 10)
     * @throws std::invalid_argument if the string is not a valid integer
     * @throws std::out_of_range if the integer is out of range
     * @return Integer value of the string
     */
    static int64_t string_to_int(const std::string& str, int base = 10);

    /**
     * Reverses given string `val`, so that the first character becomes the
     * last and the last one becomes the first. This should be used to avoid
     * the need of local variables at the caller.
     */
    static std::string reverse(std::string val);

    /**
     * Finds the minimal byte in a byte array, treating bytes as
     * unsigned values.
     * @param val byte array to scan
     * @return minimal byte in byte array as integer
     */
    static uint8_t byte_array_min(const std::string val);

    /**
     * Finds the maximal byte in a byte array, treating bytes as
     * unsigned values.
     * @param val byte array to scan
     * @return maximal byte in byte array as integer
     */
    static uint8_t byte_array_max(const std::string val);

private:
    std::istream* m_io;
    std::istringstream m_io_str;
    int m_bits_left;
    uint64_t m_bits;

    void init();
    void exceptions_enable() const;

    static void unsigned_to_decimal(uint64_t number, char *buf, std::size_t &buf_contents_start);
    static std::string to_string_signed(int64_t val);
    static std::string to_string_unsigned(uint64_t val);

#ifdef KS_STR_ENCODING_WIN32API
    enum {
        KAITAI_CP_UNSUPPORTED = -1,
        KAITAI_CP_UTF16LE = -2,
        KAITAI_CP_UTF16BE = -3,
    };

    /**
     * Converts string name of the encoding into a Windows codepage number. We extend standard Windows codepage list
     * with a few special meanings (see KAITAI_CP_* enum), reserving negative values of integer for that.
     * @param src_enc string name of the encoding; this should match canonical name of the encoding as per discussion
     *     in https://github.com/kaitai-io/kaitai_struct/issues/116
     * @return Windows codepage number or member of KAITAI_CP_* enum.
     * @ref https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
     */
    static int encoding_to_win_codepage(const char *src_enc);

    /**
     * Converts bytes packed in std::string into a UTF-8 string, based on given source encoding indicated by `codepage`.
     * @param src bytes to be converted
     * @param codepage Windows codepage number or member of KAITAI_CP_* enum.
     * @return UTF-8 string
     */
    static std::string bytes_to_str(const std::string src, int codepage);
#endif

    static const int ZLIB_BUF_SIZE = 128 * 1024;
};

}

#endif
