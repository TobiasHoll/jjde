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

#include "bytes.hpp"
#include "constants.hpp"
#include "flags.hpp"
#include "objects.hpp"
#include "types.hpp"

/* This is ugly as hell and should be rewritten. */

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
#define PTYPEC(S, X, C) case PREFIXTYPE(X): S << STRINGIFY(X) << ":" << std::endl << C; return S.str()
std::string get_constant_description(jjde::Constant const& c) {
    std::stringstream stream;
    switch (c.type) {
    PTYPEC(stream, CLASS_REFERENCE, "\t\treference = " << c.value.reference);
    PTYPEC(stream, DOUBLE, "\t\tvalue = " << c.value.double_);
    PTYPEN(stream, EMPTY);
    PTYPEC(stream, FIELD_REFERENCE, "\t\tclass_reference = " << c.value.pair_reference.first << std::endl << "\t\tname_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, FLOAT, "\t\tvalue = " << c.value.float_);
    PTYPEC(stream, INTEGER, "\t\tvalue = " << c.value.integer);
    PTYPEC(stream, INTERFACE_METHOD_REFERENCE, "\t\tclass_reference = " << c.value.pair_reference.first << std::endl << "\t\tname_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, INVOKE_DYNAMIC, "\t\tvalue = " << c.value.invoke_dynamic);
    PTYPEC(stream, LONG, "\t\tvalue = " << c.value.long_);
    PTYPEC(stream, METHOD_HANDLE, "\t\ttype_index = " << c.value.method_handle.first << std::endl << "\t\tindex = " << c.value.method_handle.second);
    PTYPEC(stream, METHOD_REFERENCE, "\t\tclass_reference = " << c.value.pair_reference.first << std::endl << "\t\tname_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, METHOD_TYPE, "\t\treference = " << c.value.reference);
    PTYPEC(stream, NAME_TYPE_DESCRIPTOR, "\t\tname_index = " << c.value.pair_reference.first << std::endl << "\t\ttype_index = " << c.value.pair_reference.second);
    PTYPEC(stream, STRING, "\t\tvalue = '" << cencode(c.value.string) << "'");
    PTYPEC(stream, STRING_REFERENCE, "\t\treference = " << c.value.reference);
    default: return "Error";
    }
}

int main(int argc, char *argv[]) {

    assert(argc > 1);
    std::ifstream in(argv[1]);

    // Extract and verify magic number (0xCAFEBABE)

    uint32_t magic = jjde::parse<uint32_t>(jjde::extract<4>(in));
    if (magic != 0xCAFEBABE) {
        throw std::logic_error("Not a valid Java bytecode (.class) file");
    }

    // Extract Java version information

    uint16_t minor = jjde::parse<uint16_t>(jjde::extract<2>(in));
    uint16_t major = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "Java version: " << (major - 44) << "." << minor << std::endl;

    // Extract constants

    std::vector<jjde::Constant> constants = jjde::read_constant_block(in);
    for (uint16_t index = 1; index < constants.size(); ++index) {
        std::clog << "[" << index << "]\t" << get_constant_description(constants[index]) << std::endl;
    }

    // Extract class flags

    jjde::Flags class_flags = jjde::read_class_flags(in);
    std::clog << "Class flags:" << std::endl;
    print_flags(class_flags);

    // Extract name information about this class

    uint16_t class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
    assert(constants[class_ref_index].type == jjde::Constant::Type::CLASS_REFERENCE);
    uint16_t string_index = constants[class_ref_index].value.reference;
    assert(constants[string_index].type == jjde::Constant::Type::STRING);
    std::string class_name = constants[string_index].value.string;
    std::replace(class_name.begin(), class_name.end(), '/', '.');
    std::clog << "Class name:" << std::endl << "\t" << class_name << std::endl;

    // Extract information about the parent class

    class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
    assert(constants[class_ref_index].type == jjde::Constant::Type::CLASS_REFERENCE);
    string_index = constants[class_ref_index].value.reference;
    assert(constants[string_index].type == jjde::Constant::Type::STRING);
    std::string parent_class_name = constants[string_index].value.string;
    std::replace(parent_class_name.begin(), parent_class_name.end(), '/', '.');
    std::clog << "Parent class name:" << std::endl << "\t" << parent_class_name << std::endl;

    // Extract interface count

    uint16_t interface_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " << interface_count << " interfaces" << std::endl;

    // Extract interfaces

    std::vector<std::string> interfaces;

    for (uint16_t interface_id = 1; interface_id < interface_count; ++interface_id) {
        string_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
        assert(constants[string_index].type == jjde::Constant::Type::STRING);
        std::string interface_name = constants[string_index].value.string;
        std::replace(interface_name.begin(), interface_name.end(), '/', '.');
        std::clog << interface_id << ":\t" << interface_name << std::endl;
    }

    // Extract fields

    std::vector<jjde::Object> fields = jjde::read_object_block(in);

    std::clog << fields.size() << " fields" << std::endl;
    for (jjde::Object const& field : fields) {
        std::string field_name = constants[field.name_index].value.string;
        std::clog << "\t" << field_name << " [desc idx: " << field.descriptor_index << "]" << std::endl;
        std::clog << "\tFlags:" << std::endl;
        print_flags(field.flags, "\t\t");
        if (field.attributes.size() > 0) {
            std::clog << "\tAttributes:" << std::endl;
            for (jjde::Attribute const& attr : field.attributes) {
                std::clog << "\t\t" << constants[attr.name_index].value.string << "\t\t\t" << cencode(attr.data) << std::endl;
            }
        }
    }

    // Extract methods

    std::vector<jjde::Object> methods = jjde::read_object_block(in);

    std::clog << methods.size() << " methods" << std::endl;
    for (jjde::Object const& method : methods) {
        std::string method_name = constants[method.name_index].value.string;
        std::clog << "\t" << method_name << " [desc idx: " << method.descriptor_index << "]" << std::endl;
        std::clog << "\tFlags:" << std::endl;
        print_flags(method.flags, "\t\t");
        if (method.attributes.size() > 0) {
            std::clog << "\tAttributes:" << std::endl;
            for (jjde::Attribute const& attr : method.attributes) {
                std::clog << "\t\t" << constants[attr.name_index].value.string << "\t\t\t" << cencode(attr.data) << std::endl;
            }
        }
    }

    // Extract attributes

    std::vector<jjde::Attribute> attributes = jjde::read_attribute_block(in);
    for (jjde::Attribute const& attr : attributes) {
        std::clog << "\t" << constants[attr.name_index].value.string << "\t\t\t" << cencode(attr.data) << std::endl;
    }

    std::clog << "Processed " << in.tellg() << " bytes" << std::endl;

    // Basic shell

    std::size_t cmd = 1;
    std::clog << "Enter a constant pool index to decode the type string stored there. Enter '0' to quit." << std::endl << ">>> ";
    std::clog.flush();
    while (true) {
        std::cin >> cmd;
        if (cmd == 0) break;
        if (cmd > constants.size() || constants[cmd].type != jjde::Constant::Type::STRING) {
            if (cmd > constants.size()) std::clog << "Invalid argument (constant " << cmd << " does not exist)" << std::endl;
            else std::clog << "Invalid argument (constant " << cmd << " is not a string)" << std::endl;
            std::clog << ">>> ";
            std::clog.flush();
            continue;
        }
        try {
            std::clog << jjde::decode_type(constants[cmd].value.string).to_string() << std::endl;
        } catch (...) {
            std::clog << "Invalid argument (constant " << cmd << " is not a type string)" << std::endl;
        }

        std::clog << ">>> ";
        std::clog.flush();
    }

    return 0;
}

