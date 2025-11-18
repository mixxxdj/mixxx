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
#else // !__APPLE__ or !_MSC_VER or !__QNX__
#include <endian.h>
#include <byteswap.h>
#endif

#include <iostream>
#include <vector>
#include <stdexcept>

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
    std::iostream::pos_type cur_pos = m_io->tellg();
    m_io->seekg(0, std::ios::end);
    std::iostream::pos_type len = m_io->tellg();
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
    return reinterpret_cast<float &>(t);
}

double kaitai::kstream::read_f8be() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __LITTLE_ENDIAN
    t = bswap_64(t);
#endif
    return reinterpret_cast<double &>(t);
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
    return reinterpret_cast<float &>(t);
}

double kaitai::kstream::read_f8le() {
    uint64_t t;
    m_io->read(reinterpret_cast<char *>(&t), 8);
#if __BYTE_ORDER == __BIG_ENDIAN
    t = bswap_64(t);
#endif
    return reinterpret_cast<double &>(t);
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
    // http://en.cppreference.com/w/cpp/io/streamsize
    if (len < 0) {
        throw std::runtime_error("read_bytes: requested a negative amount");
    }

    if (len > 0) {
        m_io->read(&result[0], len);
    }

    return std::string(result.begin(), result.end());
}

std::string kaitai::kstream::read_bytes_full() {
    std::iostream::pos_type p1 = m_io->tellg();
    m_io->seekg(0, std::ios::end);
    std::iostream::pos_type p2 = m_io->tellg();
    size_t len = p2 - p1;

    // Note: this requires a std::string to be backed with a
    // contiguous buffer. Officially, it's a only requirement since
    // C++11 (C++98 and C++03 didn't have this requirement), but all
    // major implementations had contiguous buffers anyway.
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

// ========================================================================
// Byte array processing
// ========================================================================

std::string kaitai::kstream::process_xor_one(std::string data, uint8_t key) {
    size_t len = data.length();
    std::string result(len, ' ');

    for (size_t i = 0; i < len; i++)
        result[i] = data[i] ^ key;

    return result;
}

std::string kaitai::kstream::process_xor_many(std::string data, std::string key) {
    size_t len = data.length();
    size_t kl = key.length();
    std::string result(len, ' ');

    size_t ki = 0;
    for (size_t i = 0; i < len; i++) {
        result[i] = data[i] ^ key[ki];
        ki++;
        if (ki >= kl)
            ki = 0;
    }

    return result;
}

std::string kaitai::kstream::process_rotate_left(std::string data, int amount) {
    size_t len = data.length();
    std::string result(len, ' ');

    for (size_t i = 0; i < len; i++) {
        uint8_t bits = data[i];
        result[i] = (bits << amount) | (bits >> (8 - amount));
    }

    return result;
}

#ifdef KS_ZLIB
#include <zlib.h>

std::string kaitai::kstream::process_zlib(std::string data) {
    int ret;

    unsigned char *src_ptr = reinterpret_cast<unsigned char *>(&data[0]);
    std::stringstream dst_strm;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK)
        throw std::runtime_error("process_zlib: inflateInit error");

    strm.next_in = src_ptr;
    strm.avail_in = data.length();

    unsigned char outbuffer[ZLIB_BUF_SIZE];
    std::string outstring;

    // get the decompressed bytes blockwise using repeated calls to inflate
    do {
        strm.next_out = reinterpret_cast<Bytef *>(outbuffer);
        strm.avail_out = sizeof(outbuffer);

        ret = inflate(&strm, 0);

        if (outstring.size() < strm.total_out)
            outstring.append(reinterpret_cast<char *>(outbuffer), strm.total_out - outstring.size());
    } while (ret == Z_OK);

    if (ret != Z_STREAM_END) { // an error occurred that was not EOF
        std::ostringstream exc_msg;
        exc_msg << "process_zlib: error #" << ret << "): " << strm.msg;
        throw std::runtime_error(exc_msg.str());
    }

    if (inflateEnd(&strm) != Z_OK)
        throw std::runtime_error("process_zlib: inflateEnd error");

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

#include <algorithm>
void kaitai::kstream::unsigned_to_decimal(uint64_t number, char *buffer) {
    // Implementation from https://ideone.com/nrQfA8 by Alf P. Steinbach
    // (see https://www.zverovich.net/2013/09/07/integer-to-string-conversion-in-cplusplus.html#comment-1033931478)
    if (number == 0) {
        *buffer++ = '0';
    } else {
        char *p_first = buffer;
        while (number != 0) {
            *buffer++ = static_cast<char>('0' + number % 10);
            number /= 10;
        }
        std::reverse(p_first, buffer);
    }
    *buffer = '\0';
}

int64_t kaitai::kstream::string_to_int(const std::string& str, int base) {
    char *str_end;

    errno = 0;
    int64_t res = strtoll(str.c_str(), &str_end, base);

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
#include <cerrno>
#include <stdexcept>

std::string kaitai::kstream::bytes_to_str(const std::string src, const char *src_enc) {
    iconv_t cd = iconv_open(KS_STR_DEFAULT_ENCODING, src_enc);

    if (cd == (iconv_t)-1) {
        if (errno == EINVAL) {
            throw unknown_encoding(src_enc);
        } else {
            throw bytes_to_str_error("error opening iconv");
        }
    }

    size_t src_len = src.length();
    size_t src_left = src_len;

    // Start with a buffer length of double the source length.
    size_t dst_len = src_len * 2;
    std::string dst(dst_len, ' ');
    size_t dst_left = dst_len;

    // NB: this should be const char *, but for some reason iconv() requires non-const in its 2nd argument,
    // so we force it with a cast.
    char *src_ptr = const_cast<char*>(src.data());
    char *dst_ptr = &dst[0];

    while (true) {
        size_t res = iconv(cd, &src_ptr, &src_left, &dst_ptr, &dst_left);

        if (res == (size_t)-1) {
            if (errno == E2BIG) {
                // dst buffer is not enough to accommodate whole string
                // enlarge the buffer and try again
                size_t dst_used = dst_len - dst_left;
                dst_left += dst_len;
                dst_len += dst_len;
                dst.resize(dst_len);

                // dst.resize might have allocated destination buffer in another area
                // of memory, thus our previous pointer "dst" will be invalid; re-point
                // it using "dst_used".
                dst_ptr = &dst[dst_used];
            } else if (errno == EILSEQ) {
                throw illegal_seq_in_encoding("EILSEQ");
            } else if (errno == EINVAL) {
                throw illegal_seq_in_encoding("EINVAL");
            } else {
                throw bytes_to_str_error(to_string(errno));
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
#include <limits>

// Unbreak std::numeric_limits<T>::max, as otherwise MSVC substitutes "useful" max() macro.
#undef max

int kaitai::kstream::encoding_to_win_codepage(const char *src_enc) {
    std::string enc(src_enc);
    if (enc == "UTF-8") {
        return CP_UTF8;
    } else if (enc == "UTF-16LE") {
        return KAITAI_CP_UTF16LE;
    } else if (enc == "UTF-16BE") {
        return KAITAI_CP_UTF16BE;
    } else if (enc == "IBM437") {
        return 437;
    } else if (enc == "IBM850") {
        return 850;
    } else if (enc == "SHIFT_JIS") {
        return 932;
    } else if (enc == "GB2312") {
        return 936;
    } else if (enc == "ASCII") {
        return 20127;
    } else if (enc == "EUC-JP") {
        return 20932;
    } else if (enc == "ISO-8859-1") {
        return 28591;
    } else if (enc == "ISO-8859-2") {
        return 28592;
    } else if (enc == "ISO-8859-3") {
        return 28593;
    } else if (enc == "ISO-8859-4") {
        return 28594;
    } else if (enc == "ISO-8859-5") {
        return 28595;
    } else if (enc == "ISO-8859-6") {
        return 28596;
    } else if (enc == "ISO-8859-7") {
        return 28597;
    } else if (enc == "ISO-8859-8") {
        return 28598;
    } else if (enc == "ISO-8859-9") {
        return 28599;
    } else if (enc == "ISO-8859-10") {
        return 28600;
    } else if (enc == "ISO-8859-11") {
        return 28601;
    } else if (enc == "ISO-8859-13") {
        return 28603;
    } else if (enc == "ISO-8859-14") {
        return 28604;
    } else if (enc == "ISO-8859-15") {
        return 28605;
    } else if (enc == "ISO-8859-16") {
        return 28606;
    }

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

    // Step 2: convert bytes to UTF-16 ("wide char") string
    std::wstring utf16;
    int32_t utf16_len;
    int32_t src_len;
    if (src.length() > std::numeric_limits<int32_t>::max()) {
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