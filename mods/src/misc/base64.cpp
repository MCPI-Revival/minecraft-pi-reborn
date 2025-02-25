#include <cstdint>
#include <string>

// Base64 Encoder (https://gist.github.com/tomykaira/f0fd86b6c73063283afe550bc5d77594)
std::string misc_base64_encode(const std::string &data) {
    static constexpr char encoding_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };

    const size_t in_len = data.size();
    const size_t out_len = 4 * ((in_len + 2) / 3);
    std::string ret(out_len, '\0');
    size_t i;
    char *p = const_cast<char *>(ret.c_str());

    for (i = 0; i < in_len - 2; i += 3) {
        *p++ = encoding_table[(data[i] >> 2) & 0x3f];
        *p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xf0) >> 4)];
        *p++ = encoding_table[((data[i + 1] & 0xf) << 2) | ((int) (data[i + 2] & 0xc0) >> 6)];
        *p++ = encoding_table[data[i + 2] & 0x3f];
    }
    if (i < in_len) {
        *p++ = encoding_table[(data[i] >> 2) & 0x3f];
        if (i == (in_len - 1)) {
            *p++ = encoding_table[((data[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else {
            *p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xf0) >> 4)];
            *p++ = encoding_table[((data[i + 1] & 0xf) << 2)];
        }
        *p++ = '=';
    }

    return ret;
}

// Base64 Decoder
std::string misc_base64_decode(const std::string &input) {
    static constexpr unsigned char decoding_table[] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57,
        58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0, 1, 2, 3, 4, 5, 6,
        7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
        25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64
    };

    const size_t in_len = input.size();
    if (in_len % 4 != 0) {
        return "";
    }

    std::string out = "";
    size_t out_len = in_len / 4 * 3;
    if (in_len >= 1 && input[in_len - 1] == '=') out_len--;
    if (in_len >= 2 && input[in_len - 2] == '=') out_len--;
    out.resize(out_len);

    for (size_t i = 0, j = 0; i < in_len;) {
        const uint32_t a = input[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(input[i++])];
        const uint32_t b = input[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(input[i++])];
        const uint32_t c = input[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(input[i++])];
        const uint32_t d = input[i] == '=' ? 0 & i++ : decoding_table[static_cast<int>(input[i++])];

        const uint32_t triple = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if (j < out_len) {
            out[j++] = (triple >> 2 * 8) & 0xFF;
        }
        if (j < out_len) {
            out[j++] = (triple >> 1 * 8) & 0xFF;
        }
        if (j < out_len) {
            out[j++] = (triple >> 0 * 8) & 0xFF;
        }
    }

    return out;
}