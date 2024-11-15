# Read Hex Data
file(READ "${EMBED_IN}" data HEX)

# Convert Hex Data For C Compatibility
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," data "${data}")

# Get C Name
cmake_path(GET EMBED_OUT STEM name)

# Write Data
file(WRITE "${EMBED_OUT}"
    "#include <stddef.h>\n"
    "const unsigned char ${name}[] = {${data}};\n"
    "const size_t ${name}_len = sizeof (${name});\n"
)
