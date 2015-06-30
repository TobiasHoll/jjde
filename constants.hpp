#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "bytes.hpp"

namespace jjde {

/* Constants */

struct Constant {
    enum Type {
        EMPTY = 0,
        STRING = 1,
        INTEGER = 3,
        FLOAT = 4,
        LONG = 5,
        DOUBLE = 6,
        CLASS_REFERENCE = 7,
        STRING_REFERENCE = 8,
        FIELD_REFERENCE = 9,
        METHOD_REFERENCE = 10,
        INTERFACE_METHOD_REFERENCE = 11,
        NAME_TYPE_DESCRIPTOR = 12,
        METHOD_HANDLE = 15,
        METHOD_TYPE = 16,
        INVOKE_DYNAMIC = 18
    };

    struct Value { // Must be a struct, because vectors cannot be stored in unions
        std::string string;
        int32_t integer = 0;
        float float_ = 0.0f;
        int64_t long_ = 0L;
        double double_ = 0.0;
        uint16_t reference = 0; // CLASS_REFERENCE, STRING_REFERENCE, METHOD_TYPE
        std::pair<uint16_t, uint16_t> pair_reference; // FIELD_REFERENCE, METHOD_REFERENCE, INTERFACE_METHOD_REFERENCE, NAME_TYPE_DESCRIPTOR
        std::pair<uint8_t, uint16_t> method_handle;
        uint32_t invoke_dynamic = 0;
    };

    Constant() : type(Type::EMPTY) {}
    Constant(Type t, Value v) : type(t), value(v) {}

    Type type;
    Value value;
};

std::string convert_java_string(std::vector<unsigned char> const& string) {
    std::stringstream stream;
    for (unsigned char c : string) {
        stream << (char) c;
    }
    return stream.str();
}

Constant read_constant(std::ifstream & stream, Constant::Type type) {
    uint16_t string_length, ref1, ref2;
    Constant::Value value;
    switch (type) {
    case Constant::Type::STRING:
        // u2 string length + mUTF-8 string
        string_length = parse<uint16_t>(extract<2>(stream));
        value.string = convert_java_string(extract(stream, string_length));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- String (1) : length=" << value.string.size() << ", value='" << value.string << "'" << std::endl;
        break;
    case Constant::Type::INTEGER:
        // s4 integer
        value.integer = parse<int32_t>(extract<4>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Integer (3) : value=" << value.integer << std::endl;
        break;
    case Constant::Type::FLOAT:
        // s4 float
        value.float_ = parse<float>(extract<4>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Float (4) : value=" << value.float_ << std::endl;
        break;
    case Constant::Type::LONG:
        // s8 long
        value.long_ = parse<int64_t>(extract<8>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Long (5) : value=" << value.long_ << std::endl;
        break;
    case Constant::Type::DOUBLE:
        // s8 double
        value.double_ = parse<double>(extract<8>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Double (6) : value=" << value.double_ << std::endl;
        break;
    case Constant::Type::CLASS_REFERENCE:
        // u2 class reference (index to entry of string type containing the class name)
        value.reference = parse<uint16_t>(extract<2>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Class Reference (7) : index=" << value.reference << std::endl;
        break;
    case Constant::Type::STRING_REFERENCE:
        // u2 string reference (index to entry of string type)
        value.reference = parse<uint16_t>(extract<2>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- String Reference (8) : index=" << value.reference << std::endl;
        break;
    case Constant::Type::FIELD_REFERENCE:
        // 2u2 field reference (index to class reference and index to name/type descriptor)
        ref1 = parse<uint16_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.pair_reference = std::pair<uint16_t, uint16_t>(ref1, ref2);
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Field Reference (9) : class ref. index=" << value.pair_reference.first << ", n/t desc. index=" << value.pair_reference.second << std::endl;
        break;
    case Constant::Type::METHOD_REFERENCE:
        // 2u2 method reference (index to class reference and index to name/type descriptor)
        ref1 = parse<uint16_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.pair_reference = std::pair<uint16_t, uint16_t>(ref1, ref2);
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Method Reference (10) : class ref. index=" << value.pair_reference.first << ", n/t desc. index=" << value.pair_reference.second << std::endl;
        break;
    case Constant::Type::INTERFACE_METHOD_REFERENCE:
        // 2u2 interface method reference (index to class reference and index to name/type descriptor)
        ref1 = parse<uint16_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.pair_reference = std::pair<uint16_t, uint16_t>(ref1, ref2);
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Interface Method Reference (11) : class ref. index=" << value.pair_reference.first << ", n/t desc. index=" << value.pair_reference.second << std::endl;
        break;
    case Constant::Type::NAME_TYPE_DESCRIPTOR:
        // 2u2 name/type descriptor (index to the name and index to the entry containing the type descriptor)
        ref1 = parse<uint16_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.pair_reference = std::pair<uint16_t, uint16_t>(ref1, ref2);
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Name / Type Descriptor (12) : name index=" << value.pair_reference.first << ", type index=" << value.pair_reference.second << std::endl;
        break;
    case Constant::Type::METHOD_HANDLE:
        // u1u2 method handle (type descriptor and pool index)
        ref1 = parse<uint8_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.method_handle = std::pair<uint8_t, uint16_t>(ref1, ref2);
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Method Handle (15) : type desc. index=" << value.pair_reference.first << ", index=" << value.pair_reference.second << std::endl;
        break;
    case Constant::Type::METHOD_TYPE:
        // u2 method type (pool index)
        value.reference = parse<uint16_t>(extract<2>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- Method Type (16) : index=" << value.reference << std::endl;
        break;
    case Constant::Type::INVOKE_DYNAMIC:
        // u4 InvokeDynamic
        value.invoke_dynamic = parse<uint32_t>(extract<4>(stream));
        std::clog << "[D] constant pool [" << stream.tellg() << "] <- InvokeDynamic (18) : value=" << value.invoke_dynamic << std::endl;
        break;
    default:
        throw std::logic_error("Unknown constant type");
    }
    return Constant(type, value);
}

}

#endif // CONSTANTS_HPP
