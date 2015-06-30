#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#include "bytes.hpp"
#include "flags.hpp"

namespace jjde {

/* Attributes */

struct Attribute {
    uint16_t name_index; // for the constant pool
    std::vector<unsigned char> data;
};

Attribute read_attribute(std::ifstream & stream) {
    uint16_t attribute_name_index = parse<uint16_t>(extract<2>(stream));
    uint32_t attribute_length = parse<uint32_t>(extract<4>(stream));
    std::vector<unsigned char> attribute_data = extract(stream, attribute_length);

    return Attribute{attribute_name_index, attribute_data};
}

/* Fields and methods */

struct Object {
    Flags flags;
    uint16_t name_index; // for the constant pool
    uint16_t descriptor_index; // for the constant pool
    std::vector<Attribute> attributes;
};

Object read_object(std::ifstream & stream) {
    Flags flags(extract<2>(stream), false);
    uint16_t name_index = parse<uint16_t>(extract<2>(stream));
    uint16_t descriptor_index = parse<uint16_t>(extract<2>(stream));
    uint16_t attribute_count = parse<uint16_t>(extract<2>(stream));

    std::vector<Attribute> attributes;
    for (uint16_t attribute_id = 0; attribute_id < attribute_count; ++attribute_id) {
        attributes.push_back(read_attribute(stream));
    }

    return Object{flags, name_index, descriptor_index, attributes};
}

}

#endif // OBJECTS_HPP
