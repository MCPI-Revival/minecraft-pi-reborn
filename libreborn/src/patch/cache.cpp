#include <unordered_map>
#include <optional>
#include <vector>

#include <libreborn/patch.h>
#include "patch-internal.h"

// Tracker
class Cache {
    std::unordered_map<void *, std::unordered_set<void *>> address_to_callsites;
protected:
    virtual std::optional<void *> get_target(void *addr) = 0;
public:
    virtual ~Cache() = default;
    void remove_callsite(void *addr) {
        const std::optional<void *> target = get_target(addr);
        if (target.has_value() && address_to_callsites.contains(target.value())) {
            address_to_callsites.at(target.value()).erase(addr);
        }
    }
    void add_callsite(void *addr) {
        const std::optional<void *> target = get_target(addr);
        if (target.has_value()) {
            address_to_callsites[target.value()].insert(addr);
        }
    }
    std::unordered_set<void *> get_callsites(void *addr) const {
        if (address_to_callsites.contains(addr)) {
            return address_to_callsites.at(addr);
        } else {
            return std::unordered_set<void *>();
        }
    }
};

// .text Cache
#define def(name, val) static void *name = (void *) (val)
def(TEXT_START, 0xde60);
def(TEXT_END, 0x1020c0);
class TextCache final : public Cache {
    std::optional<void *> get_target(void *addr) override {
        const unsigned char opcode = get_opcode(addr);
        if (is_branch_instruction(opcode)) {
            return extract_from_bl_instruction((unsigned char *) addr);
        } else {
            return std::nullopt;
        }
    }
} text_cache;

// .rodata And .data.rel.ro Cache (Used For VTables)
def(RODATA_START, 0x1020c8);
def(RODATA_END, 0x11665c);
def(DATA_REL_RO_START, 0x1352b8);
def(DATA_REL_RO_END, 0x135638);
#undef def
struct VTableCache final : Cache {
    std::optional<void *> get_target(void *addr) override {
        return (void *) *(uint32_t *) addr;
    }
} vtable_cache;

// Get Cache For Address
static Cache *get_cache(const void *addr) {
    if (addr >= TEXT_START && addr < TEXT_END) {
        return &text_cache;
    } else if (
        (addr >= RODATA_START && addr < RODATA_END) ||
        (addr >= DATA_REL_RO_START && addr < DATA_REL_RO_END)
    ) {
        return &vtable_cache;
    } else {
        return nullptr;
    }
}

// API
void add_callsite(void *addr) {
    Cache *cache = get_cache(addr);
    if (cache) {
        cache->add_callsite(addr);
    }
}
void remove_callsite(void *addr) {
    Cache *cache = get_cache(addr);
    if (cache) {
        cache->remove_callsite(addr);
    }
}
std::unordered_set<void *> get_normal_callsites(void *addr) {
    return text_cache.get_callsites(addr);
}
std::unordered_set<void *> get_virtual_callsites(void *addr) {
    return vtable_cache.get_callsites(addr);
}

// Init
void init_cache() {
    DEBUG("Caching Callsites...");
    const std::vector<std::pair<void *, void *>> ranges = {
        {TEXT_START, TEXT_END},
        {RODATA_START, RODATA_END},
        {DATA_REL_RO_START, DATA_REL_RO_END}
    };
    for (const std::pair<void *, void *> &range : ranges) {
        unsigned char *start = (unsigned char *) range.first;
        const unsigned char *end = (unsigned char *) range.second;
        for (unsigned char *addr = start; addr < end; addr += sizeof(void *)) {
            add_callsite(addr);
        }
    }
}