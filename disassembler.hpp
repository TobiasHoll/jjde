#ifndef DISASSEMBLER_HPP
#define DISASSEMBLER_HPP

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

struct Code {
    uint16_t max_stack_size;
    uint16_t local_variable_count;
    std::vector<Instruction> instructions;
    std::vector<ExceptionHandler> exception_handlers;
    std::vector<Attribute> attributes;
};

Code disassemble(std::vector<unsigned char> code) {
    auto iterator = code.begin();
    uint16_t max_stack_size = parse<uint16_t>(convert<2>(iterator));
    uint16_t local_variable_count = parse<uint16_t>(convert<2>(iterator));
    // Code
    uint16_t code_bytes = parse<uint16_t>(convert<4>(iterator));
    std::vector<Instruction> instructions;
    for (std::size_t index = 0; index < code_bytes; ++index) {
        // Read an instruction
        unsigned char opcode = *iterator++;
        Instruction::Operation op = (Instruction::Operation) opcode;
        std::size_t argument_count = Instruction::argument_count[opcode];
        std::vector<unsigned char> arguments;
        for (std::size_t argument_byte = 0; argument_byte < argument_count; ++argument_byte) {
            arguments.push_back(*iterator++);
            ++index;
        }
        instructions.push_back(Instruction{op, arguments});
    }
    // Exception handlers
    uint16_t exception_table_length = parse<uint16_t>(convert<2>(iterator));
    std::vector<ExceptionHandler> exception_handlers;
    for (std::size_t index = 0; index < exception_table_length; ++index) {
        uint16_t start = parse<uint16_t>(convert<2>(iterator));
        uint16_t end = parse<uint16_t>(convert<2>(iterator));
        uint16_t handler = parse<uint16_t>(convert<2>(iterator));
        uint16_t exception = parse<uint16_t>(convert<2>(iterator));
        exception_handlers.push_back(ExceptionHandler{start, end, handler, exception});
    }
    // Attributes
    uint16_t attribute_count = parse<uint16_t>(convert<2>(iterator));
    std::vector<Attribute> attributes;
    for (std::size_t index = 0; index < attribute_count; ++index) {
        uint16_t name_index = parse<uint16_t>(convert<2>(iterator));
        uint32_t length = parse<uint32_t>(convert<4>(iterator));
        std::vector<unsigned char> data;
        for (std::size_t attribute_byte = 0; attribute_byte < length; ++attribute_byte) {
            data.push_back(*iterator++);
        }
        attributes.push_back(Attribute{name_index, data});
    }

    return Code { max_stack_size, local_variable_count, instructions, exception_handlers, attributes };
}

}

#endif // DISASSEMBLER_HPP
