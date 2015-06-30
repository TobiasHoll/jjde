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

int main(int argc, char *argv[]) {

    assert(argc > 1);
    std::ifstream in(argv[1]);

    // Extract and verify magic number (0xCAFEBABE)

    uint32_t magic = jjde::parse<uint32_t>(jjde::extract<4>(in));
    assert(magic == 0xCAFEBABE);

    // Extract Java version information

    uint16_t minor = jjde::parse<uint16_t>(jjde::extract<2>(in));
    uint16_t major = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] Java version: " << (major - 44) << "." << minor << std::endl;

    // Extract constants

    uint16_t constant_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " <<  constant_count - 1 << " constants" << std::endl;

    uint16_t skip = 0;

    std::vector<jjde::Constant> constants = {jjde::Constant()}; // Initialize with an empty constant, since Java starts counting at 1.

    for (uint16_t constant_id = 1; constant_id < constant_count; ++constant_id) {

        std::clog << constant_id << ":\t";

        if (skip > 0) {
            constants.emplace_back(jjde::Constant());
            skip -= 1;
            std::clog << "(skip)" << std::endl;
            continue;
        }

        jjde::Constant::Type constant_type = (jjde::Constant::Type) jjde::parse<uint8_t>(jjde::extract<1>(in));
        if (constant_type == jjde::Constant::Type::LONG || constant_type == jjde::Constant::Type::DOUBLE) skip = 1;

        constants.emplace_back(jjde::read_constant(in, constant_type));
    }

    // Extract class flags

    jjde::Flags class_flags = jjde::read_class_flags(in);
    std::clog << "[D] Class flags:" << std::endl;
    print_flags(class_flags);

    // Extract name information about this class

    uint16_t class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
    assert(constants[class_ref_index].type == jjde::Constant::Type::CLASS_REFERENCE);
    uint16_t string_index = constants[class_ref_index].value.reference;
    assert(constants[string_index].type == jjde::Constant::Type::STRING);
    std::string class_name = constants[string_index].value.string;
    std::replace(class_name.begin(), class_name.end(), '/', '.');
    std::clog << "[D] Class name:" << std::endl << "\t" << class_name << std::endl;

    // Extract information about the parent class

    class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
    assert(constants[class_ref_index].type == jjde::Constant::Type::CLASS_REFERENCE);
    string_index = constants[class_ref_index].value.reference;
    assert(constants[string_index].type == jjde::Constant::Type::STRING);
    std::string parent_class_name = constants[string_index].value.string;
    std::replace(parent_class_name.begin(), parent_class_name.end(), '/', '.');
    std::clog << "[D] Parent class name:" << std::endl << "\t" << parent_class_name << std::endl;

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

    // Extract field count

    uint16_t field_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " << field_count << " fields" << std::endl;

    // Extract fields

    std::vector<jjde::Object> fields;
    for (uint16_t field_id = 0; field_id < field_count; ++field_id) {
        fields.push_back(jjde::read_object(in));
        std::string field_name = constants[fields.back().name_index].value.string;
        std::clog << field_id << ":\t" << field_name << " [desc idx: " << fields.back().descriptor_index << "]" << std::endl;
        std::clog << "\tFlags:" << std::endl;
        print_flags(fields.back().flags, "\t\t");
        if (fields.back().attributes.size() > 0) {
            std::clog << "\tAttributes:" << std::endl;
            for (jjde::Attribute const& attr : fields.back().attributes) {
                std::clog << "\t\t" << constants[attr.name_index].value.string << "\t:\t" << cencode(attr.data) << std::endl;
            }
        }
    }

    // Extract method count

    uint16_t method_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " << method_count << " methods" << std::endl;

    // Extract methods

    std::vector<jjde::Object> methods;
    for (uint16_t method_id = 0; method_id < method_count; ++method_id) {
        methods.push_back(jjde::read_object(in));
        std::string method_name = constants[methods.back().name_index].value.string;
        std::clog << method_id << ":\t" << method_name << " [desc idx: " << methods.back().descriptor_index << "]" << std::endl;
        std::clog << "\tFlags:" << std::endl;
        print_flags(methods.back().flags, "\t\t");
        if (methods.back().attributes.size() > 0) {
            std::clog << "\tAttributes:" << std::endl;
            for (jjde::Attribute const& attr : methods.back().attributes) {
                std::clog << "\t\t" << constants[attr.name_index].value.string << "\t:\t" << cencode(attr.data) << std::endl;
            }
        }
    }

    // Extract attribute count

    uint16_t attribute_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " << attribute_count << " attributes" << std::endl;

    // Extract attributes

    std::vector<jjde::Attribute> attributes;
    for (uint16_t attribute_id = 0; attribute_id < attribute_count; ++attribute_id) {
        attributes.push_back(jjde::read_attribute(in));
        std::clog << attribute_id << ":\t" << constants[attributes.back().name_index].value.string << "\t:\t" << cencode(attributes.back().data) << std::endl;
    }

    std::clog << "Processed " << in.tellg() << " bytes" << std::endl;

    return 0;
}

