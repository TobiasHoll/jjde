#ifndef JJDE_ANNOTATER_HPP
#define JJDE_ANNOTATER_HPP

#include "bytes.hpp"
#include "class.hpp"
#include "disassembler.hpp"
#include "simulation.hpp"
#include "types.hpp"

namespace jjde {

struct Code {
    Class class_;
    Bytecode bytecode;
    std::string to_string() const {
        std::stringstream stream;
        stream << std::uppercase;
        stream << "        max. stack size: " << bytecode.max_stack_size << std::endl;
        stream << "        local variables: " << bytecode.local_variable_count << std::endl;
        stream << "        exception handlers:" << std::endl;
        for (jjde::ExceptionHandler const& handler : bytecode.exception_handlers) {
			if (handler.exception == 0) {
				// Any exception, finally blocks
				stream << "          (Any exception / finally)" << std::endl;
			} else {
				// Specified exception (class descriptor in constant pool)
				stream << "          " << class_.constants[handler.exception].to_string(class_.constants) << std::endl;
			}
			stream << "            " << std::hex << std::setfill('0')
			                         << std::setw(4) << handler.start
				                     << " to "
				                     << std::setw(4) << (handler.end - 1) // handler.end is exclusive, while handler.start is inclusive
                                     << " handled at "
                                     << std::setw(4) << handler.handler
                                     << std::endl;
        }
        stream << "        attributes:" << std::endl;
        for (jjde::Attribute const& attribute : bytecode.attributes) {
            stream << "          " << class_.constants[attribute.name_index].value.string << std::endl;
            stream << "            " << hexencode(attribute.data) << std::endl;
        }
        return stream.str();
    }
};

Code annotate(Class const& class_, Bytecode const& bytecode, bool static_) {
    Simulation simulation(class_, bytecode, static_);

    std::cout << std::hex << std::setfill('0');
    for (Instruction instruction : bytecode.instructions) {
        std::cout << "        " << std::setw(4) << std::uppercase << instruction.location << "\t" << Instruction::name[instruction.operation] << " " << hexencode(instruction.arguments);
        switch (instruction.operation) {
        // Show absolute jump information for IF... and GOTO... instructions
        case Instruction::GOTO:
        case Instruction::GOTO_W:
        case Instruction::IFEQ:
        case Instruction::IFGE:
        case Instruction::IFGT:
        case Instruction::IFLE:
        case Instruction::IFLT:
        case Instruction::IFNE:
        case Instruction::IFNONNULL:
        case Instruction::IFNULL:
        case Instruction::IF_ACMPEQ:
        case Instruction::IF_ACMPNE:
        case Instruction::IF_ICMPEQ:
        case Instruction::IF_ICMPGE:
        case Instruction::IF_ICMPGT:
        case Instruction::IF_ICMPLE:
        case Instruction::IF_ICMPLT:
        case Instruction::IF_ICMPNE:
            std::cout << " (" << std::setw(4) << (instruction.location + parse<int16_t>(convert<2>(instruction.arguments))) << ")";
            break;
        // Show constant table information for instructions where it is required
        case Instruction::LDC:
            // One-byte index, constant value
            std::cout << " (" << class_.constants[parse<uint8_t>(convert<1>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::LDC_W:
        case Instruction::LDC2_W:
            // Two-byte index, constant value
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::GETFIELD:
        case Instruction::GETSTATIC:
        case Instruction::PUTFIELD:
        case Instruction::PUTSTATIC:
            // Two-byte index, field reference
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::ANEWARRAY:
        case Instruction::CHECKCAST:
        case Instruction::INSTANCEOF:
        case Instruction::NEW:
            // Two-byte index, class reference
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKESPECIAL:
        case Instruction::INVOKESTATIC:
        case Instruction::INVOKEVIRTUAL:
            // Two-byte index, method reference
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::MULTIANEWARRAY:
            // Index is two out of three argument bytes, class reference
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKEDYMANIC:
            // Index is two out of four argument bytes, method reference
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKEINTERFACE:
            // Index is two out of four argument bytes, method reference (third is another argument, therefore separate branches)
            std::cout << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        default:
            break;
        }
        std::cout << std::endl;
        //simulation.process(instruction);
    }
    return Code { class_, bytecode };
}

}

#endif // JJDE_ANNOTATER_HPP
