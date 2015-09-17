#ifndef JJDE_DISASSEMBLER_HPP
#define JJDE_DISASSEMBLER_HPP

#include "bytes.hpp"
#include "instructions.hpp"
#include "objects.hpp"

namespace jjde {

struct ExceptionHandler {
    uint16_t start;
    uint16_t end;
    uint16_t handler;
    uint16_t exception;
};

struct Bytecode {
    uint16_t max_stack_size;
    uint16_t local_variable_count;
    std::vector<Instruction> instructions;
    std::vector<ExceptionHandler> exception_handlers;
    std::vector<Attribute> attributes;
};

Bytecode disassemble(std::vector<unsigned char> const& code) {
    auto iterator = code.begin();
    uint16_t max_stack_size = parse<uint16_t>(convert<2>(iterator));
    uint16_t local_variable_count = parse<uint16_t>(convert<2>(iterator));
    // Code
    uint32_t code_bytes = parse<uint16_t>(convert<4>(iterator));
    std::vector<Instruction> instructions;
    for (uint32_t index = 0; index < code_bytes; ++index) {
        // Read an instruction
        uint32_t location = index;
        unsigned char opcode = *iterator++;
        Instruction::Operation op = (Instruction::Operation) opcode;
        // Load arguments
        std::vector<unsigned char> arguments;
        std::size_t argument_count = Instruction::argument_count[opcode];
        if (argument_count == JJDE_VARIABLE_ARGUMENT_COUNT) {
            // Handle special cases with variable argument counts
            // Predeclare variables for switch (required for MSVC)
            Instruction::Operation actual;
            int32_t low, high;
            std::size_t padding, offset_count, additional_arguments;
            // Inspect the instruction
            switch (op) {
            case Instruction::TABLESWITCH: {
                // TABLESWITCH [padding] [arguments]
                // Skip padding - do not add it to the argument list. If necessary, the amount of padding can be deduced from the instruction locations
                padding = 3 - (index % 4); // 0, 1, 2 or 3 bytes so that the next byte's location is a multiple of four.
                iterator += padding;
                index += padding;
                // Padding is followed by the default offset, a four-byte value which can be added directly to the arguments
                arguments.insert(arguments.end(), iterator, iterator + 4);
                iterator += 4;
                index += 4;
                // Next are the boundaries of the lookup table, which are extracted to obtain the number of offsets to be read
                std::array<unsigned char, 4> table_low_bytes = convert<4>(iterator);
                std::array<unsigned char, 4> table_high_bytes = convert<4>(iterator);
                low = parse<int32_t>(table_low_bytes);
                high = parse<int32_t>(table_high_bytes);
                offset_count = high - low + 1;
                // Insert data
                arguments.insert(arguments.end(), table_low_bytes.begin(), table_low_bytes.end());
                arguments.insert(arguments.end(), table_high_bytes.begin(), table_high_bytes.end());
                index += 8;
                // Insert table_offset_count more four-byte offsets
                arguments.insert(arguments.end(), iterator, iterator + offset_count * 4);
                iterator += offset_count * 4;
                index += offset_count * 4;
                break;
            }
            case Instruction::LOOKUPSWITCH: {
                // LOOKUPSWITCH [padding] [arguments]
                // Skip padding - do not add it to the argument list. If necessary, the amount of padding can be deduced from the instruction locations
                padding = (4 - (index % 4)) % 4; // 0, 1, 2 or 3 bytes so that the next byte's location is a multiple of four.
                iterator += padding;
                index += padding;
                // Padding is followed by the default offset, a four-byte value which can be added directly to the arguments
                arguments.insert(arguments.end(), iterator, iterator + 4);
                iterator += 4;
                index += 4;
                // Next is the number of value-offset pairs (also four bytes)
                std::array<unsigned char, 4> lookup_offset_count_bytes = convert<4>(iterator);
                offset_count = parse<int32_t>(lookup_offset_count_bytes);
                // Insert that value
                arguments.insert(arguments.end(), lookup_offset_count_bytes.begin(), lookup_offset_count_bytes.end());
                index += 4;
                // Insert lookup_offset_count eight-byte value-offset pairs
                arguments.insert(arguments.end(), iterator, iterator + offset_count * 8);
                iterator += offset_count * 8;
                index += offset_count * 8;
                break;
            }
            case Instruction::WIDE: {
                // WIDE opcode [arguments]
                arguments.push_back(*iterator++);
                actual = (Instruction::Operation) arguments[0];
                // WIDE IINC has four additional arguments, other WIDE instructions only have two.
                additional_arguments = (actual == Instruction::IINC) ? 4 : 2;
                for (std::size_t argument_byte = 0; argument_byte < additional_arguments; ++argument_byte) {
                    arguments.push_back(*iterator++);
                }
                index += arguments.size();
                break;
            }
            default:
                std::cerr << "Reported variable argument count for invalid instruction (" << (int) opcode << ")" << std::endl;
                throw std::logic_error("Invalid instruction for variable argument count");
            }
        } else {
            for (std::size_t argument_byte = 0; argument_byte < argument_count; ++argument_byte) {
                arguments.push_back(*iterator++);
                ++index;
            }
        }
        instructions.push_back(Instruction{op, arguments, location});
    }
    // Exception handlers
    uint16_t exception_table_length = parse<uint16_t>(convert<2>(iterator));
    std::vector<ExceptionHandler> exception_handlers;
    for (uint16_t index = 0; index < exception_table_length; ++index) {
        uint16_t start = parse<uint16_t>(convert<2>(iterator));
        uint16_t end = parse<uint16_t>(convert<2>(iterator));
        uint16_t handler = parse<uint16_t>(convert<2>(iterator));
        uint16_t exception = parse<uint16_t>(convert<2>(iterator));
        exception_handlers.push_back(ExceptionHandler{start, end, handler, exception});
    }
    // Attributes
    uint16_t attribute_count = parse<uint16_t>(convert<2>(iterator));
    std::vector<Attribute> attributes;
    for (uint16_t index = 0; index < attribute_count; ++index) {
        uint16_t name_index = parse<uint16_t>(convert<2>(iterator));
        uint32_t length = parse<uint32_t>(convert<4>(iterator));
        std::vector<unsigned char> data;
        for (uint32_t attribute_byte = 0; attribute_byte < length; ++attribute_byte) {
            data.push_back(*iterator++);
        }
        attributes.push_back(Attribute{name_index, data});
    }

    return Bytecode { max_stack_size, local_variable_count, instructions, exception_handlers, attributes };
}

}

#endif // JJDE_DISASSEMBLER_HPP
