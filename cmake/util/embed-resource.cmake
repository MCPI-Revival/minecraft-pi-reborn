# Read Hex Data
file(READ "${EMBED_IN}" data HEX)

# Convert Hex Data For C Compatibility
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," data "${data}")

# Get C Name
get_filename_component(name "${EMBED_IN}" NAME)
string(MAKE_C_IDENTIFIER "${name}" name)

# Write Data
file(WRITE "${EMBED_OUT}" "#include <stddef.h>\nconst unsigned char ${name}[] = {${data}};\nconst size_t ${name}_len = sizeof (${name});\n")
