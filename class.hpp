#ifndef JJDE_CLASS_HPP
#define JJDE_CLASS_HPP

#include <cstdint>
#include <fstream>
#include <vector>

#include "bytes.hpp"
#include "constants.hpp"
#include "flags.hpp"
#include "objects.hpp"

namespace jjde {

struct Class {
    std::string name;
    std::string parent;

    struct {
        uint16_t major;
        uint16_t minor;
    } version;
    std::vector<jjde::Constant> constants;
    jjde::Flags flags;
    std::vector<std::string> interfaces;
    std::vector<jjde::Object> fields;
    std::vector<jjde::Object> methods;
    std::vector<jjde::Attribute> attributes;
};

Class read_class(std::ifstream & stream) {
    // Extract and verify magic number (0xCAFEBABE)

    uint32_t magic = jjde::parse<uint32_t>(jjde::extract<4>(stream));
    if (magic != 0xCAFEBABE) {
        throw std::logic_error("Invalid bytecode (wrong magic number)");
    }

    // Extract Java version information

    uint16_t minor = jjde::parse<uint16_t>(jjde::extract<2>(stream));
    uint16_t major = jjde::parse<uint16_t>(jjde::extract<2>(stream)) - 44; // Java 1.0 is major version 45

    // Extract constants

    std::vector<jjde::Constant> constants = jjde::read_constant_block(stream);

    // Extract class flags

    jjde::Flags class_flags = jjde::read_class_flags(stream);

    // Extract name information about this class

    uint16_t class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(stream));
    if (constants[class_ref_index].type != jjde::Constant::Type::CLASS_REFERENCE) {
        throw std::logic_error("Invalid bytecode (does not contain class name)");
    }
    uint16_t string_index = constants[class_ref_index].value.reference;
    if (constants[string_index].type != jjde::Constant::Type::STRING) {
        throw std::logic_error("Invalid bytecode (does not contain class name)");
    }
    std::string class_name = constants[string_index].value.string;
    std::replace(class_name.begin(), class_name.end(), '/', '.');


    // Extract information about the parent class

    class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(stream));
    if (constants[class_ref_index].type != jjde::Constant::Type::CLASS_REFERENCE) {
        throw std::logic_error("Invalid bytecode (does not contain parent class name)");
    }
    string_index = constants[class_ref_index].value.reference;
    if (constants[string_index].type != jjde::Constant::Type::STRING) {
        throw std::logic_error("Invalid bytecode (does not contain parent class name)");
    }
    std::string parent_class_name = constants[string_index].value.string;
    std::replace(parent_class_name.begin(), parent_class_name.end(), '/', '.');

    // Extract interfaces

    uint16_t interface_count = jjde::parse<uint16_t>(jjde::extract<2>(stream));

    // Extract interfaces

    std::vector<std::string> interfaces;

    for (uint16_t interface_id = 0; interface_id < interface_count; ++interface_id) {
        class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(stream));
        if (constants[class_ref_index].type != jjde::Constant::Type::CLASS_REFERENCE) {
            throw std::logic_error("Invalid bytecode (interface name not set)");
        }
        string_index = constants[class_ref_index].value.reference;
        if (constants[string_index].type != jjde::Constant::Type::STRING) {
            throw std::logic_error("Invalid bytecode (interface name not set)");
        }
        std::string interface_name = constants[string_index].value.string;
        std::replace(interface_name.begin(), interface_name.end(), '/', '.');
        interfaces.push_back(interface_name);
    }

    // Extract fields

    std::vector<jjde::Object> fields = jjde::read_object_block(stream);

    // Extract methods

    std::vector<jjde::Object> methods = jjde::read_object_block(stream);

    // Extract attributes

    std::vector<jjde::Attribute> attributes = jjde::read_attribute_block(stream);

    // Make class object

    return Class{class_name, parent_class_name, {major, minor}, constants, class_flags, interfaces, fields, methods, attributes};
}

Class read_class(std::string const& filename) {
    std::ifstream stream(filename, std::ios::binary);
    return read_class(stream);
}

}

#endif // JJDE_CLASS_HPP
