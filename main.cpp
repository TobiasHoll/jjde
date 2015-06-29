#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#include "constants.hpp"
#include "flags.hpp"

void print_flags(jjde::Flags const& flags) {
    if (flags.is_public) std::clog << "\tpublic" << std::endl;
    if (flags.is_private) std::clog << "\tprivate" << std::endl;
    if (flags.is_protected) std::clog << "\tprotected" << std::endl;
    if (flags.is_static) std::clog << "\tstatic" << std::endl;
    if (flags.is_final) std::clog << "\tfinal" << std::endl;
    if (flags.is_class && flags.is_special_super) std::clog << "\tspecial super" << std::endl;
    if (!flags.is_class && flags.is_synchronized) std::clog << "\tsynchronized" << std::endl;
    if (flags.is_volatile) std::clog << "\tvolatile" << std::endl;
    if (flags.is_transient) std::clog << "\ttransient" << std::endl;
    if (flags.is_native) std::clog << "\tnative" << std::endl;
    if (flags.is_interface) std::clog << "\tinterface" << std::endl;
    if (flags.is_abstract) std::clog << "\tabstract" << std::endl;
    if (flags.is_strictfp) std::clog << "\tstrictfp" << std::endl;
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
    std::string class_name = jjde::convert_java_string(constants[string_index].value.string);
    std::replace(class_name.begin(), class_name.end(), '/', '.');
    std::clog << "[D] Class name:" << std::endl << "\t" << class_name << std::endl;

    // Extract information about the parent class

    class_ref_index = jjde::parse<uint16_t>(jjde::extract<2>(in));
    assert(constants[class_ref_index].type == jjde::Constant::Type::CLASS_REFERENCE);
    string_index = constants[class_ref_index].value.reference;
    assert(constants[string_index].type == jjde::Constant::Type::STRING);
    std::string parent_class_name = jjde::convert_java_string(constants[string_index].value.string);
    std::replace(parent_class_name.begin(), parent_class_name.end(), '/', '.');
    std::clog << "[D] Parent class name:" << std::endl << "\t" << parent_class_name << std::endl;

    // Extract interface count

    uint16_t interface_count = jjde::parse<uint16_t>(jjde::extract<2>(in));
    std::clog << "[D] " << interface_count << " interfaces" << std::endl;

    return 0;
}

