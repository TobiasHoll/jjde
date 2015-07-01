#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "class.hpp"
#include "flags.hpp"
#include "objects.hpp"
#include "types.hpp"

void print_flags(jjde::Flags const& flags, std::string indent="\t") {
    if (flags.is_public) std::clog << indent << "public" << std::endl;
    if (flags.is_private) std::clog << indent << "private" << std::endl;
    if (flags.is_protected) std::clog << indent << "protected" << std::endl;
    if (flags.is_static) std::clog << indent << "static" << std::endl;
    if (flags.is_final) std::clog << indent << "final" << std::endl;
    if (flags.is_class && flags.is_special_super) std::clog << indent << "special super" << std::endl;
    if (!flags.is_class && flags.is_synchronized) std::clog << indent << "synchronized" << std::endl;
    if (flags.is_volatile) std::clog << indent << "volatile" << std::endl;
    if (flags.is_transient) std::clog << indent << "transient" << std::endl;
    if (flags.is_native) std::clog << indent << "native" << std::endl;
    if (flags.is_interface) std::clog << indent << "interface" << std::endl;
    if (flags.is_abstract) std::clog << indent << "abstract" << std::endl;
    if (flags.is_strictfp) std::clog << indent << "strictfp" << std::endl;
}

std::string cencode(std::string const& data) {
    std::stringstream output;
    output << std::hex;
    for (char c : data) {
        if (32 <= c && c <= 126) {
            output << c;
        } else {
            int value = c;
            if (value < 0) value += 256;
            if (value <= 15) {
                output << "\\x0" << value;
            } else {
                output << "\\x" << value;
            }
        }
    }
    return output.str();
}

std::string cencode(std::vector<unsigned char> const& data) {
    std::stringstream output;
    output << std::hex;
    for (unsigned char uc : data) {
        if (32 <= uc && uc <= 126) {
            output << (char) uc;
        } else if (uc <= 15) {
            output << "\\x0" << (int) uc;
        } else {
            output << "\\x" << (int) uc;
        }
    }
    return output.str();
}

#define STRINGIFY(X) # X
#define PREFIXTYPE(M) jjde::Constant::Type:: M
#define PTYPEN(S, X) case PREFIXTYPE(X): S << STRINGIFY(X); return S.str();
#define PTYPEC(S, X, C) case PREFIXTYPE(X): S << STRINGIFY(X) << ":" << C; return S.str()
std::string get_constant_description(jjde::Constant const& c) {
    std::stringstream stream;
    switch (c.type) {
    PTYPEC(stream, CLASS_REFERENCE, " reference = " << c.value.reference);
    PTYPEC(stream, DOUBLE, " value = " << c.value.double_);
    PTYPEN(stream, EMPTY);
    PTYPEC(stream, FIELD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, FLOAT, " value = " << c.value.float_);
    PTYPEC(stream, INTEGER, " value = " << c.value.integer);
    PTYPEC(stream, INTERFACE_METHOD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, INVOKE_DYNAMIC, " value = " << c.value.invoke_dynamic);
    PTYPEC(stream, LONG, " value = " << c.value.long_);
    PTYPEC(stream, METHOD_HANDLE, " type_index = " << c.value.method_handle.first << ", index = " << c.value.method_handle.second);
    PTYPEC(stream, METHOD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, METHOD_TYPE, " reference = " << c.value.reference);
    PTYPEC(stream, NAME_TYPE_DESCRIPTOR, " name_index = " << c.value.pair_reference.first << ", type_index = " << c.value.pair_reference.second);
    PTYPEC(stream, STRING, " value = '" << cencode(c.value.string) << "'");
    PTYPEC(stream, STRING_REFERENCE, " reference = " << c.value.reference);
    default: return "Error";
    }
}

int main(int argc, char *argv[]) {

    assert(argc > 1);

    jjde::Class class_ = jjde::read_class(argv[1]);

    std::cout << argv[1] << std::endl
              << "├ Class name:   " << class_.name << std::endl
              << "├ Parent class: " << class_.parent << std::endl
              << "├ Java version: " << class_.version.major << "." << class_.version.minor << std::endl
              << "├ Constant pool" << std::endl;

    for (uint16_t index = 1; index < class_.constants.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << get_constant_description(class_.constants[index]) << std::endl;
    }

    std::cout << "├ Flags" << std::endl;
    print_flags(class_.flags, "│ ├ ");

    std::cout << "├ Interfaces" << std::endl;
    for (uint16_t index = 0; index < class_.interfaces.size(); ++index) {
        std::cout << (index == class_.interfaces.size() - 1 ? "│ └[" : "│ ├[") << index << "]\t" << class_.interfaces[index] << std::endl;
    }

    std::cout << "├ Fields" << std::endl;
    for (uint16_t index = 0; index < class_.fields.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << class_.constants[class_.fields[index].name_index].value.string << std::endl;
        std::cout << "│ │   \t├ Flags" << std::endl;
        print_flags(class_.fields[index].flags, "│ │   \t│ ├ ");
        std::cout << "│ │   \t├ Attributes" << std::endl;
        for (uint16_t j = 0; j < class_.fields[index].attributes.size(); ++j) {
            std::cout << "│ │   \t│ ├ " << class_.constants[class_.fields[index].attributes[j].name_index].value.string << std::endl;
            std::cout << "│ │   \t│ │ └ " << cencode(class_.fields[index].attributes[j].data) << std::endl;
        }
    }

    std::cout << "├ Methods" << std::endl;
    for (uint16_t index = 0; index < class_.methods.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << class_.constants[class_.methods[index].name_index].value.string << std::endl;
        std::cout << "│ │   \t├ Flags" << std::endl;
        print_flags(class_.methods[index].flags, "│ │   \t│ ├ ");
        std::cout << "│ │   \t├ Attributes" << std::endl;
        for (uint16_t j = 0; j < class_.methods[index].attributes.size(); ++j) {
            std::cout << "│ │   \t│ ├ " << class_.constants[class_.methods[index].attributes[j].name_index].value.string << std::endl;
            std::cout << "│ │   \t│ │ └ " << cencode(class_.methods[index].attributes[j].data) << std::endl;
        }
    }

    std::cout << "├ Attributes" << std::endl;
    for (uint16_t index = 0; index < class_.attributes.size(); ++index) {
        std::cout << "│ ├ " << class_.constants[class_.attributes[index].name_index].value.string << std::endl;
        std::cout << "│ │ └ " << cencode(class_.attributes[index].data) << std::endl;
    }

    // Basic shell

    std::size_t cmd = 1;
    std::clog << "Enter a constant pool index to decode the type string stored there. Enter '0' to quit." << std::endl << ">>> ";
    std::clog.flush();
    while (true) {
        std::cin >> cmd;
        if (cmd == 0) break;
        if (cmd >= class_.constants.size() || class_.constants[cmd].type != jjde::Constant::Type::STRING) {
            if (cmd >= class_.constants.size()) std::clog << "Invalid argument (constant " << cmd << " does not exist)" << std::endl;
            else std::clog << "Invalid argument (constant " << cmd << " is not a string)" << std::endl;
            std::clog << ">>> ";
            std::clog.flush();
            continue;
        }
        try {
            std::clog << jjde::decode_type(class_.constants[cmd].value.string).to_string() << std::endl;
        } catch (...) {
            std::clog << "Invalid argument (constant " << cmd << " is not a type string)" << std::endl;
        }

        std::clog << ">>> ";
        std::clog.flush();
    }

    return 0;
}

