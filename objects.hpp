#ifndef JJDE_OBJECTS_HPP
#define JJDE_OBJECTS_HPP

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
    uint16_t name_index = parse<uint16_t>(extract<2>(stream));
    uint32_t length = parse<uint32_t>(extract<4>(stream));
    std::vector<unsigned char> data = extract(stream, length);

    return Attribute{name_index, data};
}

std::vector<Attribute> read_attribute_block(std::ifstream & stream) {
    std::vector<Attribute> attributes;

    uint16_t count = parse<uint16_t>(extract<2>(stream));
    for (uint16_t id = 0; id < count; ++id) {
        attributes.push_back(read_attribute(stream));
    }

    return attributes;
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

    std::vector<Attribute> attributes = read_attribute_block(stream);

    return Object{flags, name_index, descriptor_index, attributes};
}

std::vector<Object> read_object_block(std::ifstream & stream) {
    std::vector<Object> objects;

    uint16_t count = parse<uint16_t>(extract<2>(stream));
    for (uint16_t id = 0; id < count; ++id) {
        objects.push_back(read_object(stream));
    }

    return objects;
}

}

#endif // JJDE_OBJECTS_HPP
