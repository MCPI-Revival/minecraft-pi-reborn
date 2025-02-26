#include <string>
#include <vector>

// Base64-URL Encode/Decode (https://stackoverflow.com/a/57314480)
static constexpr char base64_url_alphabet[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
};
std::string misc_base64_encode(const std::string &data) {
    std::string out;
    int val = 0;
    int valb = -6;
    const size_t len = data.length();
    for (unsigned int i = 0; i < len; i++) {
        const unsigned char c = data[i];
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_url_alphabet[(val >> valb) & 0x3f]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_url_alphabet[((val << 8) >> (valb + 8)) & 0x3f]);
    }
    return out;
}
std::string misc_base64_decode(const std::string &input) {
    std::string out;
    std::vector T(256, -1);
    for (unsigned int i = 0; i < 64; i++) {
        T[base64_url_alphabet[i]] = i;
    }

    int val = 0;
    int valb = -8;
    for (unsigned int i = 0; i < input.length(); i++) {
        const unsigned char c = input[i];
        if (T[c] == -1) {
            break;
        }
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xff));
            valb -= 8;
        }
    }
    return out;
}