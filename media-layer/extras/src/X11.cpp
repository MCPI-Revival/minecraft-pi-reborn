#include <elf.h>
#include <vector>

#include <X11/Xlib.h>

#include <libreborn/libreborn.h>
#include <libreborn/media-layer/core.h>

#ifndef MEDIA_LAYER_PROXY_SERVER
// Store Adresses That Are Part Of MCPI
struct text_section_data {
    void *section;
    Elf32_Word size;
};
static std::vector<text_section_data> &get_text_sections() {
    static std::vector<text_section_data> sections;
    return sections;
}
static void text_section_callback(void *section, Elf32_Word size, __attribute__((unused)) void *data) {
    text_section_data section_data;
    section_data.section = section;
    section_data.size = size;
    get_text_sections().push_back(section_data);
}
// Check If The Current Return Address Is Part Of MCPI Or An External Library
static inline int _is_returning_to_external_library(void *return_addr) {
    // Load Text Sections If Not Loaded
    if (get_text_sections().size() < 1) {
        iterate_text_sections(text_section_callback, NULL);
    }
    // Iterate Text Sections
    for (std::vector<text_section_data>::size_type i = 0; i < get_text_sections().size(); i++) {
        text_section_data section_data = get_text_sections()[i];
        // Check Text Section
        if (return_addr >= section_data.section && return_addr < (((unsigned char *) section_data.section) + section_data.size)) {
            // Part Of MCPI
            return 0;
        }
    }
    // Part Of An External Library
    return 1;
}
#define is_returning_to_external_library() _is_returning_to_external_library(__builtin_extract_return_addr(__builtin_return_address(0)))
#else
// When Using The Media Layer Proxy, None Of These Functions Are Ever Called By An External Library
#define is_returning_to_external_library() 0
#endif

// Don't Directly Use XLib Unless Returning To An External library
HOOK(XTranslateCoordinates, int, (void *display, XID src_w, XID dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return, XID *child_return)) {
    if (is_returning_to_external_library()) {
        // Use Real Function
        ensure_XTranslateCoordinates();
        return (*real_XTranslateCoordinates)(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, child_return);
    } else {
        // Use MCPI Replacemnt Function
        *dest_x_return = src_x;
        *dest_y_return = src_y;
        return 1;
    }
}
HOOK(XGetWindowAttributes, int, (void *display, XID w, XWindowAttributes *window_attributes_return)) {
    if (is_returning_to_external_library()) {
        // Use Real Function
        ensure_XGetWindowAttributes();
        return (*real_XGetWindowAttributes)(display, w, window_attributes_return);
    } else {
        // Use MCPI Replacemnt Function
        XWindowAttributes attributes;
        attributes.x = 0;
        attributes.y = 0;
        media_get_framebuffer_size(&attributes.width, &attributes.height);
        *window_attributes_return = attributes;
        return 1;
    }
}
