project(LIEF)

# Silence Warnings
add_compile_options(-w -Wno-psabi)

## LIEF

# Options
force_set(LIEF_C_API FALSE BOOL)
force_set(LIEF_EXAMPLES FALSE BOOL)
force_set(LIEF_PYTHON_API FALSE BOOL)
force_set(LIEF_TESTS FALSE BOOL)
force_set(LIEF_USE_CCACHE FALSE BOOL)
force_set(LIEF_LOGGING FALSE BOOL)
force_set(LIEF_LOGGING_DEBUG FALSE BOOL)
force_set(LIEF_ENABLE_JSON FALSE BOOL)
force_set(LIEF_ELF TRUE BOOL)
force_set(LIEF_PE FALSE BOOL)
force_set(LIEF_MACHO FALSE BOOL)
force_set(LIEF_DEX FALSE BOOL)
force_set(LIEF_ART FALSE BOOL)
force_set(LIEF_OAT FALSE BOOL)
force_set(LIEF_VDEX FALSE BOOL)

# Download
set(MESSAGE_QUIET TRUE)
add_subdirectory(src EXCLUDE_FROM_ALL SYSTEM)
unset(MESSAGE_QUIET)

# Install
setup_library(LIB_LIEF TRUE TRUE)

# License
install(FILES src/LICENSE DESTINATION "${MCPI_LEGAL_DIR}/LIEF")

# Fix Flags
function(fix_flags property)
    get_target_property(flags LIB_LIEF "${property}")
    list(REMOVE_ITEM flags "_GLIBCXX_USE_CXX11_ABI=1")
    set_target_properties(LIB_LIEF PROPERTIES "${property}" "${flags}")
endfunction()
fix_flags(COMPILE_DEFINITIONS)
fix_flags(INTERFACE_COMPILE_DEFINITIONS)