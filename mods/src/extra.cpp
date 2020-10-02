#include <string>
#include <fstream>
#include <streambuf>

#include <unistd.h>

extern "C" {
    std::string readAssetFile(__attribute__((unused)) unsigned char *obj, const std::string& path) {
        std::string full_path("./data/");
        full_path.append(path.c_str());
        std::ifstream stream(full_path);
        std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return str;
    }
}
