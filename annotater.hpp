#ifndef ANNOTATER_HPP
#define ANNOTATER_HPP

#include "bytes.hpp"
#include "class.hpp"
#include "disassembler.hpp"
#include "types.hpp"

#include <deque>

namespace jjde {

namespace annotater {

struct JavaReference {
    enum {
        ARRAY,
        CLASS,
        INTERFACE,
        NULL_
    } mode;
};

struct JavaObject {
    enum {
        REFERENCE,
        BYTE,
        CHAR,
        DOUBLE,
        FLOAT,
        INTEGER,
        LONG,
        SHORT,
        BOOLEAN
    } type;

    union Value {
        unsigned char byte;
        char char_;
        double double_;
        float float_;
        std::int32_t integer;
        std::int64_t long_;
        std::int16_t short_;
        bool boolean;
        JavaReference reference;
    } value;
};

}

struct Code {
    // TODO : this is temporary
    Class class_;
    Bytecode bytecode;
    std::string to_string() const {
        std::stringstream stream;
        stream << "        max. stack size: " << bytecode.max_stack_size << std::endl;
        stream << "        local variables: " << bytecode.local_variable_count << std::endl;
        stream << "        exception handlers:" << std::endl;
        for (jjde::ExceptionHandler const& handler : bytecode.exception_handlers) {
            stream << "          {" << handler.start << ", " << handler.end << ", " << handler.handler << ", " << handler.exception << "}" << std::endl;
        }
        stream << "        attributes:" << std::endl;
        for (jjde::Attribute const& attribute : bytecode.attributes) {
            stream << "          " << class_.constants[attribute.name_index].value.string << std::endl;
            stream << "            " << hexencode(attribute.data) << std::endl;
        }
        return stream.str();
    }
};

Code annotate(Class const& class_, Bytecode const& bytecode) {
    std::deque<annotater::JavaObject> stack_simulator(bytecode.max_stack_size);
    std::vector<annotater::JavaObject> local_variable_simulator(bytecode.local_variable_count);

    std::cerr << std::hex << std::uppercase << std::setfill('0');
    for (Instruction instruction : bytecode.instructions) {
        std::cerr << "        " << std::setw(4) << instruction.location << "\t" << Instruction::name[instruction.operation] << " " << hexencode(instruction.arguments);
        annotater::JavaObject::Value value;
        switch (instruction.operation) {
        /*case Instruction::NOP:
            break;
        case Instruction::ACONST_NULL:
            value.reference = annotater::JavaReference { annotater::JavaReference::NULL_ };
            stack_simulator.push_back(annotater::JavaObject { annotater::JavaObject::REFERENCE, value });
            break;*/
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
            std::cerr << " (" << std::setw(4) << (instruction.location + parse<int16_t>(convert<2>(instruction.arguments))) << ")";
            break;
        // Show constant table information for instructions where it is required
        case Instruction::LDC:
            // One-byte index, constant value
            std::cerr << " (" << class_.constants[parse<uint8_t>(convert<1>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::LDC_W:
        case Instruction::LDC2_W:
            // Two-byte index, constant value
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::GETFIELD:
        case Instruction::GETSTATIC:
        case Instruction::PUTFIELD:
        case Instruction::PUTSTATIC:
            // Two-byte index, field reference
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::ANEWARRAY:
        case Instruction::CHECKCAST:
        case Instruction::INSTANCEOF:
        case Instruction::NEW:
            // Two-byte index, class reference
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKESPECIAL:
        case Instruction::INVOKESTATIC:
        case Instruction::INVOKEVIRTUAL:
            // Two-byte index, method reference
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::MULTIANEWARRAY:
            // Index is two out of three argument bytes, class reference
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKEDYMANIC:
            // Index is two out of four argument bytes, method reference
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        case Instruction::INVOKEINTERFACE:
            // Index is two out of four argument bytes, method reference (third is another argument, therefore separate branches)
            std::cerr << " (" << class_.constants[parse<uint16_t>(convert<2>(instruction.arguments))].to_string(class_.constants) << ")";
            break;
        default:
            break;
        }
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
    return Code { class_, bytecode };
}

}

#endif // ANNOTATER_HPP
