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
    uint16_t max_stack_size = parse<uint16_t>(convert<2>(code));
    uint16_t local_variable_count = parse<uint16_t>(convert<2>(code, 2));
    // Code
    uint16_t code_bytes = parse<uint16_t>(convert<2>(code, 4));
    std::vector<Instruction> instructions;
    for (std::size_t index = 0; index < code_bytes; ++index) {
        // Read an instruction
        unsigned char opcode = code[index + 6];
        Instruction::Operation op = (Instruction::Operation) opcode;
        std::size_t argument_count = Instruction::argument_count[opcode];
        std::vector<unsigned char> arguments(argument_count);
        std::copy(code.begin() + index + 7, code.begin() + index + 6 + argument_count)
    }
    // Exception handlers
    uint16_t exception_table_length = parse<uint16_t>(convert<2>(code, code_bytes + 4));
    std::vector<ExceptionHandler> exception_handlers;
    for (std::size_t index = 0; index < exception_table_length; ++index) {
        uint16_t start = parse<uint16_t>(convert<2>(code, code_bytes + 6 + (8 * index)));
        uint16_t end = parse<uint16_t>(convert<2>(code, code_bytes + 6 + (8 * index) + 2));
        uint16_t handler = parse<uint16_t>(convert<2>(code, code_bytes + 6 + (8 * index) + 4));
        uint16_t exception = parse<uint16_t>(convert<2>(code, code_bytes + 6 + (8 * index) + 6));
        exeption_handlers.push_back(ExceptionHandler{start, end, handler, exception});
    }
    std::size_t next = code_bytes + 6 + (8 * exception_table_length);
    // Attributes
    uint16_t attribute_count = parse<uint16_t>(convert<2>(code, next));
    std::vector<Attribute> attributes;
    for (std::size_t index = 0; index < attribute_count; ++index) {
        uint16_t name_index = parse<uint16_t>(extract<2>(stream, next + 2));
        uint32_t length = parse<uint32_t>(extract<4>(stream, next + 4));
        std::vector<unsigned char> data;
        std::copy(stream.begin() + next + 8, stream.begin() + next + 8 + length, std::back_inserter(data));
        next += 8 + length;
        attributes.push_back(Attribute{name_index, data});
    }

}

}

#endif // DISASSEMBLER_HPP
