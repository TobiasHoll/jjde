#ifndef JJDE_SIMULATION_HPP
#define JJDE_SIMULATION_HPP

#include "bytes.hpp"
#include "class.hpp"
#include "constants.hpp"
#include "disassembler.hpp"
#include "instructions.hpp"

#include <deque>

namespace jjde {

struct Simulation {
    std::deque<std::string> stack;

    Class const& class_;
    bool static_;

    Simulation(Class const& the_class, Bytecode const& bytecode, bool is_static)
        : stack(bytecode.max_stack_size)
        , class_(the_class)
        , static_(is_static) {}

    std::string str;

    void load_constant(uint16_t index) {
        std::string tmp;

        if (index >= class_.constants.size()) {
            std::cerr << "Constant pool entry " << index << " requested, but only " << (class_.constants.size() - 1) << " entries are available." << std::endl;
            throw std::logic_error("Invalid constant pool index.");
        }

        switch (class_.constants[index].type) {
        case Constant::EMPTY:
            std::cerr << "Code attempts loading an empty constant value." << std::endl;
            throw std::logic_error("Invalid constant pool data type.");
        case Constant::STRING:
        case Constant::STRING_REFERENCE:
        case Constant::INTEGER:
            // No additional markers
            stack.push_back(class_.constants[index].to_string(class_.constants));
            break;
        case Constant::FLOAT:
            // "f" marker
            stack.push_back(class_.constants[index].to_string(class_.constants) + "f");
            break;
        case Constant::LONG:
            // LONG and DOUBLE use two stack spaces
            tmp = class_.constants[index].to_string(class_.constants) + "L";
            stack.push_back(tmp);
            stack.push_back(tmp);
            break;
        case Constant::DOUBLE:
            // LONG and DOUBLE use two stack spaces
            tmp = class_.constants[index].to_string(class_.constants);
            stack.push_back(tmp);
            stack.push_back(tmp);
            break;
        case Constant::CLASS_REFERENCE:
            stack.push_back(std::string("Class<") + class_.constants[index].to_string(class_.constants) + ">");
            break;
        // Constant::STRING_REFERENCE handled with Constant::STRING
        case Constant::METHOD_HANDLE:
            stack.push_back("java.lang.invoke.MethodHandle");
            break;
        case Constant::METHOD_TYPE:
            stack.push_back("java.lang.invoke.MethodType");
            break;
        case Constant::FIELD_REFERENCE:
        case Constant::METHOD_REFERENCE:
        case Constant::INTERFACE_METHOD_REFERENCE:
        case Constant::NAME_TYPE_DESCRIPTOR:
        case Constant::INVOKE_DYNAMIC:
        default:
            std::cerr << "Code attempts loading a constant value of invalid type." << std::endl;
            throw std::logic_error("Invalid constant pool data type.");
            break;
        }
    }

    void process(Instruction const& instruction) {
        uint16_t index;
        std::string expr;
        std::size_t previous;
        std::string tmp;

        switch (instruction.operation) {
        case Instruction::NOP: break;
        case Instruction::ACONST_NULL:
            stack.push_back("null");
            break;
        case Instruction::ICONST_M1:
            stack.push_back("-1");
            break;
        case Instruction::ICONST_0:
            stack.push_back("0");
            break;
        case Instruction::ICONST_1:
            stack.push_back("1");
            break;
        case Instruction::ICONST_2:
            stack.push_back("2");
            break;
        case Instruction::ICONST_3:
            stack.push_back("3");
            break;
        case Instruction::ICONST_4:
            stack.push_back("4");
            break;
        case Instruction::ICONST_5:
            stack.push_back("5");
            break;
        case Instruction::LCONST_0:
            stack.push_back("0L");
            break;
        case Instruction::LCONST_1:
            stack.push_back("1L");
            break;
        case Instruction::FCONST_0:
            stack.push_back("0.0f");
            break;
        case Instruction::FCONST_1:
            stack.push_back("1.0f");
            break;
        case Instruction::FCONST_2:
            stack.push_back("2.0f");
            break;
        case Instruction::DCONST_0:
            stack.push_back("0.0");
            break;
        case Instruction::DCONST_1:
            stack.push_back("1.0");
            break;
        case Instruction::BIPUSH:
            stack.push_back(std::to_string(parse<int8_t>(convert<1>(instruction.arguments))));
            break;
        case Instruction::SIPUSH:
            stack.push_back(std::to_string(parse<int16_t>(convert<2>(instruction.arguments))));
            break;
        case Instruction::LDC:
            index = parse<uint8_t>(convert<1>(instruction.arguments));
            load_constant(index);
            break;
        case Instruction::LDC_W:
        case Instruction::LDC2_W:
            index = parse<uint16_t>(convert<2>(instruction.arguments));
            load_constant(index);
            break;
        case Instruction::ILOAD:
        case Instruction::LLOAD:
        case Instruction::FLOAD:
        case Instruction::DLOAD:
        case Instruction::ALOAD:
            index = parse<uint8_t>(convert<1>(instruction.arguments));
            stack.push_back("var" + std::to_string(index));
            break;
        case Instruction::ILOAD_0:
        case Instruction::LLOAD_0:
        case Instruction::FLOAD_0:
        case Instruction::DLOAD_0:
        case Instruction::ALOAD_0:
            stack.push_back("var0");
            break;
        case Instruction::ILOAD_1:
        case Instruction::LLOAD_1:
        case Instruction::FLOAD_1:
        case Instruction::DLOAD_1:
        case Instruction::ALOAD_1:
            stack.push_back("var1");
            break;
        case Instruction::ILOAD_2:
        case Instruction::LLOAD_2:
        case Instruction::FLOAD_2:
        case Instruction::DLOAD_2:
        case Instruction::ALOAD_2:
            stack.push_back("var2");
            break;
        case Instruction::ILOAD_3:
        case Instruction::LLOAD_3:
        case Instruction::FLOAD_3:
        case Instruction::DLOAD_3:
        case Instruction::ALOAD_3:
            stack.push_back("var3");
            break;
        case Instruction::IALOAD:
        case Instruction::LALOAD:
        case Instruction::FALOAD:
        case Instruction::DALOAD:
        case Instruction::AALOAD:
        case Instruction::BALOAD:
        case Instruction::CALOAD:
        case Instruction::SALOAD:
            expr = stack[stack.size() - 2] + "[" + stack[stack.size() - 1] + "]";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::ISTORE:
        case Instruction::LSTORE:
        case Instruction::FSTORE:
        case Instruction::DSTORE:
        case Instruction::ASTORE:
            index = parse<uint8_t>(convert<1>(instruction.arguments));
            std::cout << "var" << index << " = " << stack[stack.size() - 1] << std::endl;
            stack.pop_back();
            break;
        case Instruction::ISTORE_0:
        case Instruction::LSTORE_0:
        case Instruction::FSTORE_0:
        case Instruction::DSTORE_0:
        case Instruction::ASTORE_0:
            std::cout << "var0 = " << stack[stack.size() - 1] << std::endl;
            stack.pop_back();
            break;
        case Instruction::ISTORE_1:
        case Instruction::LSTORE_1:
        case Instruction::FSTORE_1:
        case Instruction::DSTORE_1:
        case Instruction::ASTORE_1:
            std::cout << "var1 = " << stack[stack.size() - 1] << std::endl;
            stack.pop_back();
            break;
        case Instruction::ISTORE_2:
        case Instruction::LSTORE_2:
        case Instruction::FSTORE_2:
        case Instruction::DSTORE_2:
        case Instruction::ASTORE_2:
            std::cout << "var2 = " << stack[stack.size() - 1] << std::endl;
            stack.pop_back();
            break;
        case Instruction::ISTORE_3:
        case Instruction::LSTORE_3:
        case Instruction::FSTORE_3:
        case Instruction::DSTORE_3:
        case Instruction::ASTORE_3:
            std::cout << "var3 = " << stack[stack.size() - 1] << std::endl;
            stack.pop_back();
            break;
        //TODO: Add array store instructions here
        case Instruction::POP:
            stack.pop_back();
            break;
        case Instruction::POP2:
            stack.resize(stack.size() - 2);
            break;
        case Instruction::DUP:
            stack.push_back(stack[stack.size() - 1]);
            break;
        case Instruction::DUP_X1:
            stack.insert(stack.begin() + (stack.size() - 2), stack[stack.size() - 1]);
            break;
        case Instruction::DUP_X2:
            stack.insert(stack.begin() + (stack.size() - 3), stack[stack.size() - 1]);
            break;
        case Instruction::DUP2:
            // Insert top two values at the top of the stack
            stack.push_back(stack[stack.size() - 2]);
            stack.push_back(stack[stack.size() - 2]);
            break;
        case Instruction::DUP2_X1:
            // Insert top two values below the third element
            previous = stack.size();
            stack.insert(stack.begin() + (previous - 3), stack[stack.size() - 2]);
            stack.insert(stack.begin() + (previous - 3), stack[stack.size() - 1]);
            break;
            break;
        case Instruction::DUP2_X2:
            // Insert top two values below the fourth element
            previous = stack.size();
            stack.insert(stack.begin() + (previous - 4), stack[stack.size() - 2]);
            stack.insert(stack.begin() + (previous - 4), stack[stack.size() - 1]);
            break;
        case Instruction::SWAP:
            tmp = stack[stack.size() - 1];
            stack[stack.size() - 1] = stack[stack.size() - 2];
            stack[stack.size() - 2] = tmp;
            break;
        case Instruction::IADD:
        case Instruction::LADD:
        case Instruction::FADD:
        case Instruction::DADD:
            expr = "(" + stack[stack.size() - 2] + " + " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::ISUB:
        case Instruction::LSUB:
        case Instruction::FSUB:
        case Instruction::DSUB:
            expr = "(" + stack[stack.size() - 2] + " - " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IMUL:
        case Instruction::LMUL:
        case Instruction::FMUL:
        case Instruction::DMUL:
            expr = "(" + stack[stack.size() - 2] + " * " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IDIV:
        case Instruction::LDIV:
        case Instruction::FDIV:
        case Instruction::DDIV:
            expr = "(" + stack[stack.size() - 2] + " / " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IREM:
        case Instruction::LREM:
        case Instruction::FREM:
        case Instruction::DREM:
            expr = "(" + stack[stack.size() - 2] + " % " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::INEG:
        case Instruction::LNEG:
        case Instruction::FNEG:
        case Instruction::DNEG:
            stack[stack.size() - 1] = "(-" + stack[stack.size() - 1] + ")";
            break;
        case Instruction::ISHL:
        case Instruction::LSHL:
            expr = "(" + stack[stack.size() - 2] + " << " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::ISHR:
        case Instruction::LSHR:
            expr = "(" + stack[stack.size() - 2] + " >> " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IUSHR:
        case Instruction::LUSHR:
            expr = "(" + stack[stack.size() - 2] + " >>> " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IAND:
        case Instruction::LAND:
            expr = "(" + stack[stack.size() - 2] + " & " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IOR:
        case Instruction::LOR:
            expr = "(" + stack[stack.size() - 2] + " | " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        case Instruction::IXOR:
        case Instruction::LXOR:
            expr = "(" + stack[stack.size() - 2] + " ^ " + stack[stack.size() - 1] + ")";
            stack.resize(stack.size() - 2);
            stack.push_back(expr);
            break;
        //TODO: Insert IINC here (should output something, basically varX += Y)
        //TODO: Insert conversion instructions here
        //TODO: Insert comparison instructions here
        //TODO: Insert other instructions here
        default:
            std::cout << "Simulation not yet implemented for opcode " << Instruction::name[instruction.operation] << std::endl;
            break;
        }
    }
};

}

#endif // JJDE_SIMULATION_HPP
