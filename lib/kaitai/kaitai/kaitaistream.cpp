#include <kaitai/kaitaistream.h>
#include <kaitai/exceptions.h>

#if defined(__APPLE__)
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>
#define bswap_16(x) OSSwapInt16(x)
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#define __BYTE_ORDER    BYTE_ORDER
#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#elif defined(_MSC_VER) // !__APPLE__
#include <stdlib.h>
#define __LITTLE_ENDIAN     1234
#define __BIG_ENDIAN        4321
#define __BYTE_ORDER        __LITTLE_ENDIAN
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)
#elif defined(__QNX__) // __QNX__
#include <sys/param.h>
#include <gulliver.h>
#define bswap_16(x) ENDIAN_RET16(x)
#define bswap_32(x) ENDIAN_RET32(x)
#define bswap_64(x) ENDIAN_RET64(x)
#define __BYTE_ORDER    BYTE_ORDER
#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#else
// At this point it's either Linux or BSD. Both have "sys/param.h", so it's safe to include
#include <sys/param.h> // `BSD` macro  // IWYU pragma: keep
#if defined(BSD)
// Supposed to work on FreeBSD: https://man.freebsd.org/cgi/man.cgi?query=bswap16&manpath=FreeBSD+14.0-RELEASE
// Supposed to work on NetBSD: https://man.netbsd.org/NetBSD-10.0/bswap16.3
#include <sys/endian.h>
#include <sys/types.h>
#define bswap_16(x) bswap16(x)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#define __BYTE_ORDER    BYTE_ORDER
#define __BIG_ENDIAN    BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#else // !__APPLE__ or !_MSC_VER or !__QNX__ or !BSD
#include <endian.h>
#include <byteswap.h>
#endif
#endif

#include <stdint.h> // int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t

#include <algorithm> // std::reverse
#include <cerrno> // errno, EINVAL, E2BIG, EILSEQ, ERANGE
#include <cstdlib> // std::size_t, std::strtoll
#include <cstring> // std::memcpy
#include <ios> // std::streamsize
#include <istream> // std::istream  // IWYU pragma: keep
#include <limits> // std::numeric_limits
#include <sstream> // std::stringstream, std::ostringstream  // IWYU pragma: keep
#include <stdexcept> // std::runtime_error, std::invalid_argument, std::out_of_range
#include <string> // std::string, std::getline
#include <vector> // std::vector

#ifdef KAITAI_STREAM_H_CPP11_SUPPORT
#include <type_traits> // std::enable_if, std::is_trivial

// Taken from https://en.cppreference.com/w/cpp/numeric/bit_cast#Possible_implementation
// (for compatibility with early C++11 compilers like `x86-64 gcc 4.9.4`, `x86-64 clang 3.6` or
// `x86-64 icc 13.0.1`, `std::is_trivially_copyable` was replaced with `std::is_trivial` and the
// `std::is_trivially_default_constructible` assertion was omitted)
template<class To, class From>
typename std::enable_if<
        sizeof(To) == sizeof(From) &&
        std::is_trivial<From>::value &&
        std::is_trivial<To>::value,
        To
>::type
// constexpr support needs compiler magic
static bit_cast(const From &src) noexcept
{
    // // NOTE: because of `To dst;`, we need the `To` type to be trivially default constructible,
    // // which is not true for all trivial types:
    // // https://quuxplusone.github.io/blog/2024/04/02/trivial-but-not-default-constructible/
    // //
    // // However, we don't check this requirement (and just assume it's met), because
    // // `std::is_trivially_default_constructible` is not supported by some (very) old compilers
    // // with incomplete C++11 support (`x86-64 gcc 4.9.4`, `x86-64 clang 3.6` or
    // // `x86-64 icc 13.0.1` at https://godbolt.org/).
    // static_assert(std::is_trivially_default_constructible<To>::value,
    //               "This implementation additionally requires "
    //               "destination type to be trivially default constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
#else
// The following implementation of `StaticAssert` was inspired by https://stackoverflow.com/a/6765840

// empty default template
template <bool b>
struct StaticAssert;

// template specialized on true
template <>
struct StaticAssert<true> {};

template<class To, class From>
To
static bit_cast(const From &src)
{
    StaticAssert<sizeof(To) == sizeof(From)>();

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}
#endif

kaitai::kstream::kstream(std::istream *io) {
    m_io = io;
    init();
}

kaitai::kstream::kstream(const std::string &data) : m_io_str(data) {
    m_io = &m_io_str;
    init();
}

void kaitai::kstream::init() {
    exceptions_enable();
    align_to_byte();
}

void kaitai::kstream::close() {
    //  m_io->close();
}

void kaitai::kstream::exceptions_enable() const {
    m_io->exceptions(
        std::istream::eofbit |
        std::istream::failbit |
        std::istream::badbit
    );
}

// ========================================================================
// Stream positioning
// ========================================================================

bool kaitai::kstream::is_eof() const {
    if (m_bits_left > 0) {
        return false;
    }
    char t;
    m_io->exceptions(std::istream::badbit);
    m_io->get(t);
    if (m_io->eof()) {
        m_io->clear();
        exceptions_enable();
        return true;
    } else {
        m_io->unget();
        exceptions_enable();
        return false;
    }
}

void kaitai::kstream::seek(uint64_t pos) {
    m_io->seekg(pos);
}

uint64_t kaitai::kstream::pos() {
    return m_io->tellg();
}

uint64_t kaitai::kstream::size() {
    std::istream::pos_type cur_pos = m_io->tellg();
    m_io->seekg(0, std::istream::end);
    std::istream::pos_type len = m_io->tellg();
    m_io->seekg(cur_pos);
    return len;
}

// ========================================================================
// Integer numbers
// ========================================================================

// ------------------------------------------------------------------------
// Signed
// ------------------------------------------------------------------------

int8_t kaitai::kstream::read_s1() {
    char t;
    m_io->get(t);
    return t;
}

// ........................................................................
// Big-endian
// ........................................................................

int16_t kaitai::kstream::read_s2be() {
    int16_t t;
    m_io->read(reinterpret_cast<char *>(&t), 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_16(t);
#endif
    return t;
}

int32_t kaitai::kstream::read_s4be() {
    int32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_32(t);
#endif
    return t;
}

int64_t kaitai::kstream::read_s8be() {
    int64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_64(t);
#endif
    return t;
}

// ........................................................................
// Little-endian
// ........................................................................

int16_t kaitai::kstream::read_s2le() {
    int16_t t;
    m_io->read(reinterpret_cast<char *>(&t), 2);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_16(t);
#endif
    return t;
}

int32_t kaitai::kstream::read_s4le() {
    int32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_32(t);
#endif
    return t;
}

int64_t kaitai::kstream::read_s8le() {
    int64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_64(t);
#endif
    return t;
}

// ------------------------------------------------------------------------
// Unsigned
// ------------------------------------------------------------------------

uint8_t kaitai::kstream::read_u1() {
    char t;
    m_io->get(t);
    return t;
}

// ........................................................................
// Big-endian
// ........................................................................

uint16_t kaitai::kstream::read_u2be() {
    uint16_t t;
    m_io->read(reinterpret_cast<char *>(&t), 2);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_16(t);
#endif
    return t;
}

uint32_t kaitai::kstream::read_u4be() {
    uint32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_32(t);
#endif
    return t;
}

uint64_t kaitai::kstream::read_u8be() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_64(t);
#endif
    return t;
}

// ........................................................................
// Little-endian
// ........................................................................

uint16_t kaitai::kstream::read_u2le() {
    uint16_t t;
    m_io->read(reinterpret_cast<char *>(&t), 2);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_16(t);
#endif
    return t;
}

uint32_t kaitai::kstream::read_u4le() {
    uint32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_32(t);
#endif
    return t;
}

uint64_t kaitai::kstream::read_u8le() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_64(t);
#endif
    return t;
}

// ========================================================================
// Floating point numbers
// ========================================================================

// ........................................................................
// Big-endian
// ........................................................................

float kaitai::kstream::read_f4be() {
    uint32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_32(t);
#endif
    return bit_cast<float>(t);
}

double kaitai::kstream::read_f8be() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_64(t);
#endif
    return bit_cast<double>(t);
}

// ........................................................................
// Little-endian
// ........................................................................

float kaitai::kstream::read_f4le() {
    uint32_t t;
    m_io->read(reinterpret_cast<char *>(&t), 4);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_32(t);
#endif
    return bit_cast<float>(t);
}

double kaitai::kstream::read_f8le() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_64(t);
#endif
    return bit_cast<double>(t);
}

// ========================================================================
// Unaligned bit values
// ========================================================================

void kaitai::kstream::align_to_byte() {
    m_bits_left = 0;
    m_bits = 0;
}

uint64_t kaitai::kstream::read_bits_int_be(int n) {
    uint64_t res = 0;

    int bits_needed = n - m_bits_left;
    m_bits_left = -bits_needed & 7; // `-bits_needed mod 8`

    if (bits_needed > 0) {
        // 1 bit  => 1 byte
        // 8 bits => 1 byte
        // 9 bits => 2 bytes
        int bytes_needed = ((bits_needed - 1) / 8) + 1; // `ceil(bits_needed / 8)`
        if (bytes_needed > 8)
            throw std::runtime_error("read_bits_int_be: more than 8 bytes requested");
        uint8_t buf[8];
        m_io->read(reinterpret_cast<char *>(buf), bytes_needed);
        for (int i = 0; i < bytes_needed; i++) {
            res = res << 8 | buf[i];
        }

        uint64_t new_bits = res;
        res = res >> m_bits_left | (bits_needed < 64 ? m_bits << bits_needed : 0); // avoid undefined behavior of `x << 64`
        m_bits = new_bits; // will be masked at the end of the function
    } else {
        res = m_bits >> -bits_needed; // shift unneeded bits out
    }

    uint64_t mask = (static_cast<uint64_t>(1) << m_bits_left) - 1; // `m_bits_left` is in range 0..7, so `(1 << 64)` does not have to be considered
    m_bits &= mask;

    return res;
}

// Deprecated, use read_bits_int_be() instead.
uint64_t kaitai::kstream::read_bits_int(int n) {
    return read_bits_int_be(n);
}

uint64_t kaitai::kstream::read_bits_int_le(int n) {
    uint64_t res = 0;
    int bits_needed = n - m_bits_left;

    if (bits_needed > 0) {
        // 1 bit  => 1 byte
        // 8 bits => 1 byte
        // 9 bits => 2 bytes
        int bytes_needed = ((bits_needed - 1) / 8) + 1; // `ceil(bits_needed / 8)`
        if (bytes_needed > 8)
            throw std::runtime_error("read_bits_int_le: more than 8 bytes requested");
        uint8_t buf[8];
        m_io->read(reinterpret_cast<char *>(buf), bytes_needed);
        for (int i = 0; i < bytes_needed; i++) {
            res |= static_cast<uint64_t>(buf[i]) << (i * 8);
        }

        // NB: for bit shift operators in C++, "if the value of the right operand is
        // negative or is greater or equal to the number of bits in the promoted left
        // operand, the behavior is undefined." (see
        // https://en.cppreference.com/w/cpp/language/operator_arithmetic#Bitwise_shift_operators)
        // So we define our desired behavior here.
        uint64_t new_bits = bits_needed < 64 ? res >> bits_needed : 0;
        res = res << m_bits_left | m_bits;
        m_bits = new_bits;
    } else {
        res = m_bits;
        m_bits >>= n;
    }

    m_bits_left = -bits_needed & 7; // `-bits_needed mod 8`

    if (n < 64) {
        uint64_t mask = (static_cast<uint64_t>(1) << n) - 1;
        res &= mask;
    }
    // if `n == 64`, do nothing
    return res;
}

// ========================================================================
// Byte arrays
// ========================================================================

std::string kaitai::kstream::read_bytes(std::streamsize len) {
    std::vector<char> result(len);

    // NOTE: streamsize type is signed, negative values are only *supposed* to not be used.
    // https://en.cppreference.com/w/cpp/io/streamsize
    if (len < 0) {
        throw std::runtime_error("read_bytes: requested a negative amount");
    }

    if (len > 0) {
        m_io->read(&result[0], len);
    }

    return std::string(result.begin(), result.end());
}

std::string kaitai::kstream::read_bytes_full() {
    std::istream::pos_type p1 = m_io->tellg();
    m_io->seekg(0, std::istream::end);
    std::istream::pos_type p2 = m_io->tellg();
    std::size_t len = p2 - p1;

    // NOTE: this requires `std::string` to be backed by a contiguous buffer. Officially,
    // it's only a requirement since C++11 (C++98 and C++03 didn't have this requirement),
    // but all major implementations had contiguous buffers anyway.
    std::string result(len, ' ');
    m_io->seekg(p1);
    m_io->read(&result[0], len);

    return result;
}

std::string kaitai::kstream::read_bytes_term(char term, bool include, bool consume, bool eos_error) {
    std::string result;
    std::getline(*m_io, result, term);
    if (m_io->eof()) {
        // encountered EOF
        if (eos_error) {
            throw std::runtime_error("read_bytes_term: encountered EOF");
        }
    } else {
        // encountered terminator
        if (include)
            result.push_back(term);
        if (!consume)
            m_io->unget();
    }
    return result;
}

std::string kaitai::kstream::read_bytes_term_multi(std::string term, bool include, bool consume, bool eos_error) {
    std::size_t term_len = term.length();
    if (term_len > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::runtime_error("read_bytes_term_multi: terminator too long");
    }
    std::streamsize unit_size = static_cast<std::streamsize>(term_len);

    std::string result;
    std::string c(term_len, ' ');
    m_io->exceptions(std::istream::badbit);
    while (true) {
        // NOTE: this requires `std::string` to be backed by a contiguous buffer. Officially,
        // it's only a requirement since C++11 (C++98 and C++03 didn't have this requirement),
        // but all major implementations had contiguous buffers anyway.
        m_io->read(&c[0], unit_size);
        if (m_io->eof()) {
            m_io->clear();
            exceptions_enable();
            if (eos_error) {
                throw std::runtime_error("read_bytes_term_multi: encountered EOF");
            }
            result.append(c, 0, static_cast<std::size_t>(m_io->gcount()));
            return result;
        }

        if (c == term) {
            exceptions_enable();
            if (include)
                result += c;
            if (!consume)
                m_io->seekg(-unit_size, std::istream::cur);

            return result;
        }

        result += c;
    }
}

std::string kaitai::kstream::ensure_fixed_contents(std::string expected) {
    std::string actual = read_bytes(expected.length());

    if (actual != expected) {
        // NOTE: I think printing it outright is not best idea, it could contain non-ASCII characters
        // like backspace and beeps and whatnot. It would be better to print hexlified version, and
        // also to redirect it to stderr.
        throw std::runtime_error("ensure_fixed_contents: actual data does not match expected data");
    }

    return actual;
}

std::string kaitai::kstream::bytes_strip_right(std::string src, char pad_byte) {
    std::size_t new_len = src.length();

    while (new_len > 0 && src[new_len - 1] == pad_byte)
        new_len--;

    return src.substr(0, new_len);
}

std::string kaitai::kstream::bytes_terminate(std::string src, char term, bool include) {
    std::size_t new_len = 0;
    std::size_t max_len = src.length();

    while (new_len < max_len && src[new_len] != term)
        new_len++;

    if (include && new_len < max_len)
        new_len++;

    return src.substr(0, new_len);
}

std::string kaitai::kstream::bytes_terminate_multi(std::string src, std::string term, bool include) {
    std::size_t unit_size = term.length();
    if (unit_size == 0) {
        return std::string();
    }
    std::size_t len = src.length();
    std::size_t i_term = 0;
    for (std::size_t i_src = 0; i_src < len;) {
        if (src[i_src] != term[i_term]) {
            i_src += unit_size - i_term;
            i_term = 0;
            continue;
        }
        i_src++;
        i_term++;
        if (i_term == unit_size) {
            return src.substr(0, i_src - (include ? 0 : unit_size));
        }
    }
    return src;
}

// ========================================================================
// Byte array processing
// ========================================================================

std::string kaitai::kstream::process_xor_one(std::string data, uint8_t key) {
    std::size_t len = data.length();
    std::string result(len, ' ');

    for (std::size_t i = 0; i < len; i++)
        result[i] = data[i] ^ key;

    return result;
}

std::string kaitai::kstream::process_xor_many(std::string data, std::string key) {
    std::size_t len = data.length();
    std::size_t kl = key.length();
    std::string result(len, ' ');

    std::size_t ki = 0;
    for (std::size_t i = 0; i < len; i++) {
        result[i] = data[i] ^ key[ki];
        ki++;
        if (ki >= kl)
            ki = 0;
    }

    return result;
}

std::string kaitai::kstream::process_rotate_left(std::string data, int amount) {
    std::size_t len = data.length();
    std::string result(len, ' ');

    for (std::size_t i = 0; i < len; i++) {
        uint8_t bits = data[i];
        result[i] = (bits << amount) | (bits >> (8 - amount));
    }

    return result;
}

#ifdef KS_ZLIB
#include <zlib.h>

// This instructs include-what-you-use not to suggest `#include <zconf.h>` just because it contains
// the definition of `Bytef`. It seems `<zconf.h>` is not a header for public use or at least it's
// not considered necessary to include it on top of `<zlib.h>`, because official usage examples that
// use `Bytef` only include `<zlib.h>`, see
// https://github.com/madler/zlib/blob/0f51fb4933fc9ce18199cb2554dacea8033e7fd3/test/example.c#L71
//
// IWYU pragma: no_include <zconf.h>

std::string kaitai::kstream::process_zlib(std::string data) {
    unsigned char *src_ptr = reinterpret_cast<unsigned char *>(&data[0]);

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    // See https://www.zlib.net/manual.html#:~:text=ZEXTERN%20int%20ZEXPORT-,inflateInit,-(z_streamp%20strm)%3B
    int ret = inflateInit(&strm);
    if (ret != Z_OK) {
        std::string msg;
        switch (ret) {
            case Z_MEM_ERROR:
                msg = "out of memory";
                break;
            case Z_VERSION_ERROR:
                msg = "zlib version mismatch";
                break;
            case Z_STREAM_ERROR:
                msg = "inconsistent stream state";
                break;
            default:
                msg = "unknown error (return value: " + to_string(ret) + ")";
                break;
        }
        throw std::runtime_error("process_zlib: inflateInit() failed: " + msg);
    }

    strm.next_in = src_ptr;
    if (data.length() > std::numeric_limits<uInt>::max()) {
        inflateEnd(&strm); // avoid a memory leak
        throw std::length_error(
            "process_zlib: input is " + to_string(data.length()) + " bytes long, which exceeds"
                " the maximum supported length of " + to_string(std::numeric_limits<uInt>::max()) + " bytes"
        );
    }
    strm.avail_in = static_cast<uInt>(data.length());

    unsigned char outbuffer[ZLIB_BUF_SIZE];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        strm.next_out = reinterpret_cast<Bytef *>(outbuffer);
        strm.avail_out = sizeof(outbuffer);

        // See https://www.zlib.net/manual.html#:~:text=ZEXTERN%20int%20ZEXPORT-,inflate,-(z_streamp%20strm%2C%20int%20flush)%3B
        ret = inflate(&strm, Z_NO_FLUSH);

        if (outstring.size() < strm.total_out) {
            outstring.append(reinterpret_cast<char *>(outbuffer), strm.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    if (ret != Z_STREAM_END) { // an error occurred that was not EOF
        std::string msg;
        switch (ret) {
            // The note at https://www.zlib.net/zlib_how.html applies here as well:
            // > For this routine, we have no idea what the dictionary is, so the
            // > `Z_NEED_DICT` indication is converted to a `Z_DATA_ERROR`.
            case Z_NEED_DICT:
                msg = "preset dictionary needed";
                break;
            case Z_DATA_ERROR:
                if (strm.msg) {
                    msg = strm.msg;
                } else {
                    // After looking at the [zlib source
                    // code](https://github.com/madler/zlib/blob/5a82f71ed1dfc0bec044d9702463dbdf84ea3b71/inflate.c#L590),
                    // it seems that this never happens (if `inflate()` returns
                    // `Z_DATA_ERROR`, it always sets a meaningful message to
                    // `strm->msg`), but let's handle it anyway.
                    msg = "invalid input data";
                }
                break;
            case Z_STREAM_ERROR:
                msg = "inconsistent stream state";
                break;
            case Z_MEM_ERROR:
                msg = "out of memory";
                break;
            case Z_BUF_ERROR:
                msg = "incomplete or truncated input data";
                break;
            default:
                msg = "unknown error (return value: " + to_string(ret) + ")";
                break;
        }
        inflateEnd(&strm); // avoid a memory leak
        throw std::runtime_error("process_zlib: inflate() failed: " + msg);
    }

    // See https://www.zlib.net/manual.html#:~:text=ZEXTERN%20int%20ZEXPORT-,inflateEnd,-(z_streamp%20strm)%3B
    ret = inflateEnd(&strm);
    if (ret != Z_OK) {
        std::string msg;
        if (ret == Z_STREAM_ERROR) {
            msg = "inconsistent stream state";
        } else {
            msg = "unknown error (return value: " + to_string(ret) + ")";
        }
        throw std::runtime_error("process_zlib: inflateEnd() failed: " + msg);
    }

    return outstring;
}
#endif

// ========================================================================
// Misc utility methods
// ========================================================================

int kaitai::kstream::mod(int a, int b) {
    if (b <= 0)
        throw std::invalid_argument("mod: divisor b <= 0");
    int r = a % b;
    if (r < 0)
        r += b;
    return r;
}

void kaitai::kstream::unsigned_to_decimal(uint64_t number, char *buf, std::size_t &buf_contents_start) {
    // Implementation inspired by https://ideone.com/nrQfA8 by Alf P. Steinbach
    // (see https://vitaut.net/posts/2013/integer-to-string-conversion-in-cplusplus/)
    do {
        buf[--buf_contents_start] = static_cast<char>('0' + (number % 10));
        number /= 10;
    } while (number != 0);
}

std::string kaitai::kstream::to_string_signed(int64_t val) {
    // `digits10 + 2` because of minus sign + leading digit (NB: null terminator is not used)
    char buf[std::numeric_limits<int64_t>::digits10 + 2];
    std::size_t buf_contents_start = sizeof(buf);
    if (val < 0) {
        // NB: `val` is negative and we need to get its absolute value (i.e. minus `val`). However, since
        // `int64_t` uses two's complement representation, its range is `[-2**63, 2**63 - 1] =
        // [-0x8000_0000_0000_0000, 0x7fff_ffff_ffff_ffff]` (both ends inclusive) and thus the naive
        // `-val` operation will overflow for `val = std::numeric_limits<int64_t>::min() =
        // -0x8000_0000_0000_0000` (because the result of `-val` is mathematically
        // `-(-0x8000_0000_0000_0000) = 0x8000_0000_0000_0000`, but the `int64_t` type can represent at
        // most `0x7fff_ffff_ffff_ffff`). And signed integer overflow is undefined behavior in C++.
        //
        // To avoid undefined behavior for `val = -0x8000_0000_0000_0000 = -2**63`, we do the following
        // steps for all negative `val`s:
        //
        // 1. Convert the signed (and negative) `val` to an unsigned `uint64_t` type. This is a
        //    well-defined operation in C++: the resulting `uint64_t` value will be `val mod 2**64` (`mod`
        //    is modulo). The maximum `val` we can have here is `-1` (because `val < 0`), a theoretical
        //    minimum we are able to support would be `-2**64 + 1 = -0xffff_ffff_ffff_ffff` (even though
        //    in practice the widest standard type is `int64_t` with the minimum of `-2**63`):
        //
        //    * `static_cast<uint64_t>(-1) = -1 mod 2**64 = 2**64 + (-1) = 0xffff_ffff_ffff_ffff = 2**64 - 1`
        //    * `static_cast<uint64_t>(-2**64 + 1) = (-2**64 + 1) mod 2**64 = 2**64 + (-2**64 + 1) = 1`
        //
        // 2. Subtract `static_cast<uint64_t>(val)` from `2**64 - 1 = 0xffff_ffff_ffff_ffff`. Since
        //    `static_cast<uint64_t>(val)` is in range `[1, 2**64 - 1]` (see step 1), the result of this
        //    subtraction will be mathematically in range `[0, (2**64 - 1) - 1] = [0, 2**64 - 2]`. So the
        //    mathematical result cannot be negative, hence this unsigned integer subtraction can never
        //    wrap around (which wouldn't be a good thing to rely upon because it confuses programmers and
        //    code analysis tools).
        //
        // 3. Since we did mathematically `(2**64 - 1) - (2**64 + val) = -val - 1` so far (and we wanted
        //    to do `-val`), we add `1` to correct that. From step 2 we know that the result of `-val - 1`
        //    is in range `[0, 2**64 - 2]`, so adding `1` will not wrap (at most we could get `2**64 - 1 =
        //    0xffff_ffff_ffff_ffff`, which is still in the valid range of `uint64_t`).
        unsigned_to_decimal((std::numeric_limits<uint64_t>::max() - static_cast<uint64_t>(val)) + 1, buf, buf_contents_start);

        buf[--buf_contents_start] = '-';
    } else {
        unsigned_to_decimal(static_cast<uint64_t>(val), buf, buf_contents_start);
    }
    return std::string(&buf[buf_contents_start], sizeof(buf) - buf_contents_start);
}

std::string kaitai::kstream::to_string_unsigned(uint64_t val) {
    // `digits10 + 1` because of leading digit (NB: null terminator is not used)
    char buf[std::numeric_limits<uint64_t>::digits10 + 1];
    std::size_t buf_contents_start = sizeof(buf);
    unsigned_to_decimal(val, buf, buf_contents_start);
    return std::string(&buf[buf_contents_start], sizeof(buf) - buf_contents_start);
}

int64_t kaitai::kstream::string_to_int(const std::string& str, int base) {
    char *str_end;

    errno = 0;
    int64_t res = std::strtoll(str.c_str(), &str_end, base);

    // Check for successful conversion and throw an exception if the entire string was not converted
    if (str_end != str.c_str() + str.size()) {
        throw std::invalid_argument("string_to_int");
    }

    if (errno == ERANGE) {
        throw std::out_of_range("string_to_int");
    }

    return res;
}

std::string kaitai::kstream::reverse(std::string val) {
    std::reverse(val.begin(), val.end());

    return val;
}

uint8_t kaitai::kstream::byte_array_min(const std::string val) {
    uint8_t min = 0xff; // UINT8_MAX
    std::string::const_iterator end = val.end();
    for (std::string::const_iterator it = val.begin(); it != end; ++it) {
        uint8_t cur = static_cast<uint8_t>(*it);
        if (cur < min) {
            min = cur;
        }
    }
    return min;
}

uint8_t kaitai::kstream::byte_array_max(const std::string val) {
    uint8_t max = 0; // UINT8_MIN
    std::string::const_iterator end = val.end();
    for (std::string::const_iterator it = val.begin(); it != end; ++it) {
        uint8_t cur = static_cast<uint8_t>(*it);
        if (cur > max) {
            max = cur;
        }
    }
    return max;
}

// ========================================================================
// Other internal methods
// ========================================================================

#ifndef KS_STR_DEFAULT_ENCODING
#define KS_STR_DEFAULT_ENCODING "UTF-8"
#endif

#ifdef KS_STR_ENCODING_ICONV
#include <iconv.h>

std::string kaitai::kstream::bytes_to_str(const std::string src, const char *src_enc) {
    iconv_t cd = iconv_open(KS_STR_DEFAULT_ENCODING, src_enc);

    if (cd == (iconv_t)-1) {
        if (errno == EINVAL) {
            throw unknown_encoding(src_enc);
        } else {
            throw bytes_to_str_error("error opening iconv");
        }
    }

    std::size_t src_len = src.length();
    std::size_t src_left = src_len;

    // Start with a buffer length of double the source length.
    std::size_t dst_len = src_len * 2;
    std::string dst(dst_len, ' ');
    std::size_t dst_left = dst_len;

    // NB: this should be const char *, but for some reason iconv() requires non-const in its 2nd argument,
    // so we force it with a cast.
    char *src_ptr = const_cast<char*>(src.data());
    char *dst_ptr = &dst[0];

    while (true) {
        std::size_t res = iconv(cd, &src_ptr, &src_left, &dst_ptr, &dst_left);

        if (res == (std::size_t)-1) {
            const int saved_errno = errno;
            if (saved_errno == E2BIG) {
                // dst buffer is not enough to accommodate whole string
                // enlarge the buffer and try again
                std::size_t dst_used = dst_len - dst_left;
                dst_left += dst_len;
                dst_len += dst_len;
                dst.resize(dst_len);

                // dst.resize might have allocated destination buffer in another area
                // of memory, thus our previous pointer "dst" will be invalid; re-point
                // it using "dst_used".
                dst_ptr = &dst[dst_used];
            } else {
                // Try to close the conversion descriptor, ignore any errors: if `iconv_close`
                // fails, there is nothing we can do, since we mainly want to deliver the error from
                // `iconv` as an exception. We only call `iconv_close` here to prevent memory leaks,
                // it is not a critical operation.
                iconv_close(cd);
                if (saved_errno == EILSEQ) {
                    throw illegal_seq_in_encoding("EILSEQ");
                }
                if (saved_errno == EINVAL) {
                    throw illegal_seq_in_encoding("EINVAL");
                }
                throw bytes_to_str_error(to_string(saved_errno));
            }
        } else {
            // conversion successful
            dst.resize(dst_len - dst_left);
            break;
        }
    }

    if (iconv_close(cd) != 0) {
        throw bytes_to_str_error("iconv close error");
    }

    return dst;
}
#elif defined(KS_STR_ENCODING_NONE)
std::string kaitai::kstream::bytes_to_str(const std::string src, const char *src_enc) {
    return src;
}
#elif defined(KS_STR_ENCODING_WIN32API)
#include <windows.h>

// Unbreak std::numeric_limits<T>::max, as otherwise MSVC substitutes "useful" max() macro.
#undef max

int kaitai::kstream::encoding_to_win_codepage(const char *src_enc) {
    std::string enc(src_enc);

    // See https://learn.microsoft.com/en-us/windows/win32/intl/code-page-identifiers
    //
    // This method should handle at least all canonical encoding names listed in
    // <https://github.com/kaitai-io/kaitai_struct_compiler/blob/5832a81a48e10c3c207748486e09bd58b9aa4000/shared/src/main/scala/io/kaitai/struct/EncodingList.scala>,
    // preferably in the same order so that both sets of encodings can be easily compared.
    if (enc == "ASCII")
        return 20127;
    if (enc == "UTF-8")
        return CP_UTF8;
    if (enc == "UTF-16BE")
        return KAITAI_CP_UTF16BE;
    if (enc == "UTF-16LE")
        return KAITAI_CP_UTF16LE;
    if (enc == "UTF-32BE") {
        // It has a code page number 12001 assigned to it, but it's "available only to
        // managed applications", so we can't use it.
        return KAITAI_CP_UNSUPPORTED;
    }
    if (enc == "UTF-32LE") {
        // It has a code page number 12000 assigned to it, but it's "available only to
        // managed applications", so we can't use it.
        return KAITAI_CP_UNSUPPORTED;
    }
    if (enc == "ISO-8859-1")
        return 28591;
    if (enc == "ISO-8859-2")
        return 28592;
    if (enc == "ISO-8859-3")
        return 28593;
    if (enc == "ISO-8859-4")
        return 28594;
    if (enc == "ISO-8859-5")
        return 28595;
    if (enc == "ISO-8859-6")
        return 28596;
    if (enc == "ISO-8859-7")
        return 28597;
    if (enc == "ISO-8859-8")
        return 28598;
    if (enc == "ISO-8859-9")
        return 28599;
    if (enc == "ISO-8859-10") {
        // According to <https://docs.rs/encoding_rs/latest/encoding_rs/static.ISO_8859_10.html>:
        // > The Windows code page number for this encoding is 28600, but kernel32.dll
        // > does not support this encoding.
        return KAITAI_CP_UNSUPPORTED;
    }
    if (enc == "ISO-8859-11") {
        // The Windows code page 874 (`windows-874`) is the best match we can use here,
        // although it's actually an extension of ISO-8859-11, see
        // https://en.wikipedia.org/wiki/ISO/IEC_8859-11#Code_page_874_(Microsoft)_/_1162
        return 874;
    }
    if (enc == "ISO-8859-13")
        return 28603;
    if (enc == "ISO-8859-14") {
        // According to <https://docs.rs/encoding_rs/latest/encoding_rs/static.ISO_8859_14.html>:
        // > The Windows code page number for this encoding is 28604, but kernel32.dll
        // > does not support this encoding.
        return KAITAI_CP_UNSUPPORTED;
    }
    if (enc == "ISO-8859-15")
        return 28605;
    if (enc == "ISO-8859-16") {
        // According to <https://docs.rs/encoding_rs/latest/encoding_rs/static.ISO_8859_16.html>:
        // > The Windows code page number for this encoding is 28606, but kernel32.dll
        // > does not support this encoding.
        return KAITAI_CP_UNSUPPORTED;
    }
    if (enc == "windows-1250")
        return 1250;
    if (enc == "windows-1251")
        return 1251;
    if (enc == "windows-1252")
        return 1252;
    if (enc == "windows-1253")
        return 1253;
    if (enc == "windows-1254")
        return 1254;
    if (enc == "windows-1255")
        return 1255;
    if (enc == "windows-1256")
        return 1256;
    if (enc == "windows-1257")
        return 1257;
    if (enc == "windows-1258")
        return 1258;
    if (enc == "IBM437")
        return 437;
    if (enc == "IBM850")
        return 850;
    if (enc == "IBM866")
        return 866;
    if (enc == "Shift_JIS")
        return 932;
    if (enc == "GB2312")
        return 936;
    if (enc == "Big5")
        return 950;
    if (enc == "EUC-JP")
        return 20932;
    if (enc == "EUC-KR")
        return 51949;

    return KAITAI_CP_UNSUPPORTED;
}

std::string kaitai::kstream::bytes_to_str(const std::string src, const char *src_enc) {
    // Step 1: convert encoding name to codepage number
    int codepage = encoding_to_win_codepage(src_enc);
    if (codepage == KAITAI_CP_UNSUPPORTED) {
        throw unknown_encoding(src_enc);
    }
    return bytes_to_str(src, codepage);
}

std::string kaitai::kstream::bytes_to_str(const std::string src, int codepage) {
    // Shortcut: if we're already in UTF-8, no need to convert anything
    if (codepage == CP_UTF8) {
        return src;
    }
    // If `src` is empty, no conversion is needed either (in fact, the Win32 functions we use, i.e.
    // MultiByteToWideChar and WideCharToMultiByte, fail with ERROR_INVALID_PARAMETER when they
    // encounter an empty string, so we avoid this by returning early)
    if (src.empty()) {
        return src;
    }

    // Step 2: convert bytes to UTF-16 ("wide char") string
    std::wstring utf16;
    int32_t utf16_len;
    int32_t src_len;
    if (src.length() > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
        throw bytes_to_str_error("buffers longer than int32_t are unsupported");
    } else {
        src_len = static_cast<int32_t>(src.length());
    }

    switch (codepage) {
    case KAITAI_CP_UTF16LE:
        // If our source is already UTF-16LE, just copy it

        if (src_len % 2 != 0) {
            throw illegal_seq_in_encoding("incomplete");
        }

        utf16_len = src_len / 2;
        utf16 = std::wstring((wchar_t*)src.c_str(), utf16_len);
        break;
    case KAITAI_CP_UTF16BE:
        // If our source is in UTF-16BE, convert it to UTF-16LE by swapping bytes

        if (src_len % 2 != 0) {
            throw illegal_seq_in_encoding("incomplete");
        }

        utf16_len = src_len / 2;

        utf16 = std::wstring(utf16_len, L'\0');
        for (int32_t i = 0; i < utf16_len; i++) {
            utf16[i] = (static_cast<uint8_t>(src[i * 2]) << 8) | static_cast<uint8_t>(src[i * 2 + 1]);
        }
        break;
    default:
        // Calculate the length of the UTF-16 string
        utf16_len = MultiByteToWideChar(codepage, 0, src.c_str(), src_len, 0, 0);
        if (utf16_len == 0) {
            throw bytes_to_str_error("MultiByteToWideChar length calculation error");
        }

        // Convert to UTF-16 string
        utf16 = std::wstring(utf16_len, L'\0');
        if (MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, src.c_str(), src_len, &utf16[0], utf16_len) == 0) {
            auto err = GetLastError();
            if (err == ERROR_NO_UNICODE_TRANSLATION) {
                throw illegal_seq_in_encoding("MultiByteToWideChar");
            } else {
                throw bytes_to_str_error("MultiByteToWideChar conversion error");
            }
        }
    }

    // Step 3: convert UTF-16 string to UTF-8 string

    // Calculate the length of the UTF-8 string
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, &utf16[0], utf16_len, 0, 0, 0, 0);
    if (utf8_len == 0) {
        throw bytes_to_str_error("WideCharToMultiByte length calculation error");
    }

    // Convert to UTF-8 string
    std::string utf8(utf8_len, '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &utf16[0], utf16_len, &utf8[0], utf8_len, 0, 0) == 0) {
        auto err = GetLastError();
        if (err == ERROR_NO_UNICODE_TRANSLATION) {
            throw illegal_seq_in_encoding("WideCharToMultiByte");
        } else {
            throw bytes_to_str_error("WideCharToMultiByte conversion error");
        }
    }

    return utf8;
}

#else
#error Need to decide how to handle strings: please define one of: KS_STR_ENCODING_ICONV, KS_STR_ENCODING_WIN32API, KS_STR_ENCODING_NONE
#endif
