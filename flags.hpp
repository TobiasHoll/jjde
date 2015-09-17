#ifndef JJDE_FLAGS_HPP
#define JJDE_FLAGS_HPP

#include <array>
#include <fstream>

#include "bytes.hpp"

namespace jjde {

/* Flags */

struct Flags {
    bool is_class;

    bool is_public;
    bool is_private;
    bool is_protected;
    bool is_static;
    bool is_final;
    bool is_special_super; // for classes, always set except for really old code
    bool is_synchronized; // for variables
    bool is_volatile;
    bool is_transient;
    bool is_native;
    bool is_interface;
    bool is_abstract;
    bool is_strictfp;

    Flags(std::array<unsigned char, 2> values, bool is_class_)
        : is_class(is_class_)
        , is_public(values[1] & 0x01)
        , is_private(values[1] & 0x02)
        , is_protected(values[1] & 0x04)
        , is_static(values[1] & 0x08)
        , is_final(values[1] & 0x10)
        // set both is_special_super and is_synchronized
        , is_special_super(values[1] & 0x20)
        , is_synchronized(values[1] & 0x20)
        , is_volatile(values[1] & 0x40)
        , is_transient(values[1] & 0x80)
        , is_native(values[0] & 0x01)
        , is_interface(values[0] & 0x02)
        , is_abstract(values[0] & 0x04)
        , is_strictfp(values[0] & 0x08)
        // synthetic, ...
    {}

    std::string to_string() const {
        std::string output;
        if (is_public) output += "public ";
        if (is_private) output += "private ";
        if (is_protected) output += "protected ";
        if (is_static) output += "static ";
        if (is_final) output += "final ";
        if (!is_class && is_synchronized) output += "synchronized "; // No output for special super.
        if (is_volatile) output += "volatile ";
        if (is_transient) output += "transient ";
        if (is_native) output += "native ";
        if (is_interface) output += "interface ";
        if (is_abstract) output += "abstract ";
        if (is_strictfp) output += "strictfp ";
        return output.substr(0, output.size() - 1);
    }
};

Flags read_class_flags(std::ifstream & stream) {
    return Flags(extract<2>(stream), true);
}

}

#endif // JJDE_FLAGS_HPP
