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
#include "types.hpp"

namespace jjde {

std::string encode(std::string const& data) {
    std::stringstream output;
    output << "\"" << std::oct;
    for (char c : data) {
        if (c == '"') output << "\\\"";
        else if (c == '\\') output << "\\\\";
        else if (32 <= c && c <= 126) output << c;
        else if (c == '\t') output << "\\t";
        else if (c == '\b') output << "\\b";
        else if (c == '\n') output << "\\n";
        else if (c == '\r') output << "\\r";
        else if (c == '\f') output << "\\f";
        else {
            int value = c;
            if (value < 0) value += 256;
            if (value < 8) output << "\\00" << value;
            if (value < 64) output << "\\0" << value;
            else output << "\\" << value;
        }
    }
    output << "\"";
    return output.str();
}

std::string hexencode(std::string const& data) {
    std::stringstream output;
    output << std::hex;
    for (char c : data) {
        int value = c;
        if (value < 0) value += 256;
        output << (value <= 15 ? "0" : "") << value << " ";
    }
    return output.str();
}

std::string hexencode(std::vector<unsigned char> const& data) {
    std::stringstream output;
    output << std::hex;
    for (unsigned char uc : data) {
        output << (uc <= 15 ? "0" : "") << (int) uc << " ";
    }
    return output.str();
}


std::string convert_java_string(std::vector<unsigned char> const& string) {
    std::stringstream stream;
    // TODO: Special java string encoding
    for (unsigned char c : string) {
        stream << (char) c;
    }
    return stream.str();
}

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

    struct Value { // Must be a struct, because strings cannot be stored in unions
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

    std::string to_string(std::vector<Constant> const& pool) const {
        switch (type) {
        case EMPTY:                      return "<! empty !>";
        case STRING:                     return encode(value.string);
        case INTEGER:                    return std::to_string(value.integer);
        case FLOAT:                      return std::to_string(value.float_);
        case LONG:                       return std::to_string(value.long_);
        case DOUBLE:                     return std::to_string(value.double_);
        case CLASS_REFERENCE:            return pool[value.reference].to_string(pool);
        case STRING_REFERENCE:           return pool[value.reference].to_string(pool);
        case FIELD_REFERENCE:            return "field \"" + pool[value.pair_reference.second].to_string(pool) + "\" of class " + pool[value.pair_reference.first].to_string(pool);
        case METHOD_REFERENCE:           return "method \"" + pool[value.pair_reference.second].to_string(pool) + "\" of class " + pool[value.pair_reference.first].to_string(pool);
        case INTERFACE_METHOD_REFERENCE: return "interface method \"" + pool[value.pair_reference.second].to_string(pool) + "\" of class " + pool[value.pair_reference.first].to_string(pool);
        case NAME_TYPE_DESCRIPTOR:       return decode_type(pool[value.pair_reference.second].value.string).to_string(pool[value.pair_reference.first].value.string);
        case METHOD_HANDLE:              return "<! method handle !>";
        case METHOD_TYPE:                return "<! method type !>";
        case INVOKE_DYNAMIC:             return "<! INVOKE_DYNAMIC !>";
        default:                         return "<! invalid type !>";
        }
    }
};

std::pair<Constant, bool> read_constant(std::ifstream & stream) {
    Constant::Type type = (Constant::Type) parse<uint8_t>(extract<1>(stream));
    bool skip = (type == Constant::Type::LONG || type == Constant::Type::DOUBLE);
    uint16_t string_length, ref1, ref2;
    Constant::Value value;
    switch (type) {
    case Constant::Type::STRING:
        // u2 string length + mUTF-8 string
        string_length = parse<uint16_t>(extract<2>(stream));
        value.string = convert_java_string(extract(stream, string_length));
        break;
    case Constant::Type::INTEGER:
        // s4 integer
        value.integer = parse<int32_t>(extract<4>(stream));
        break;
    case Constant::Type::FLOAT:
        // s4 float
        value.float_ = parse<float>(extract<4>(stream));
        break;
    case Constant::Type::LONG:
        // s8 long
        value.long_ = parse<int64_t>(extract<8>(stream));
        break;
    case Constant::Type::DOUBLE:
        // s8 double
        value.double_ = parse<double>(extract<8>(stream));
        break;
    case Constant::Type::CLASS_REFERENCE:
    case Constant::Type::STRING_REFERENCE:
    case Constant::Type::METHOD_TYPE:
        // u2 class reference (index to entry of string type containing the class name)
        // u2 string reference (index to entry of string type)
        // u2 method type (pool index)
        value.reference = parse<uint16_t>(extract<2>(stream));
        break;
    case Constant::Type::FIELD_REFERENCE:
    case Constant::Type::METHOD_REFERENCE:
    case Constant::Type::INTERFACE_METHOD_REFERENCE:
    case Constant::Type::NAME_TYPE_DESCRIPTOR:
        // 2u2 field reference (index to class reference and index to name/type descriptor)
        // 2u2 method reference (index to class reference and index to name/type descriptor)
        // 2u2 interface method reference (index to class reference and index to name/type descriptor)
        // 2u2 name/type descriptor (index to the name and index to the entry containing the type descriptor)
        ref1 = parse<uint16_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.pair_reference = std::pair<uint16_t, uint16_t>(ref1, ref2);
        break;
    case Constant::Type::METHOD_HANDLE:
        // u1u2 method handle (type descriptor and pool index)
        ref1 = parse<uint8_t>(extract<2>(stream));
        ref2 = parse<uint16_t>(extract<2>(stream));
        value.method_handle = std::pair<uint8_t, uint16_t>(ref1, ref2);
        break;
    case Constant::Type::INVOKE_DYNAMIC:
        // u4 InvokeDynamic
        value.invoke_dynamic = parse<uint32_t>(extract<4>(stream));
        break;
    default:
        throw std::logic_error("Unknown constant type " + std::to_string((unsigned int) type));
    }
    return std::pair<Constant, bool>(Constant(type, value), skip);
}

std::vector<Constant> read_constant_block(std::ifstream & stream) {
    uint16_t count = parse<uint16_t>(extract<2>(stream));
    bool skip = false;

    std::vector<Constant> constants = {Constant()}; // Initialize with an empty constant, since Java starts counting at 1.

    for (uint16_t id = 1; id < count; ++id) {
        if (skip) {
            constants.push_back(Constant());
            skip = false;
            continue;
        }

        std::pair<Constant, bool> result = read_constant(stream);

        constants.push_back(result.first);
        skip = result.second;
    }

    return constants;
}

}

#endif // CONSTANTS_HPP
