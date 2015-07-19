#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "class.hpp"
#include "disassembler.hpp"
#include "flags.hpp"
#include "instructions.hpp"
#include "objects.hpp"
#include "types.hpp"

void print_flags(jjde::Flags const& flags, std::string indent="\t") {
    if (flags.is_public) std::clog << indent << "public" << std::endl;
    if (flags.is_private) std::clog << indent << "private" << std::endl;
    if (flags.is_protected) std::clog << indent << "protected" << std::endl;
    if (flags.is_static) std::clog << indent << "static" << std::endl;
    if (flags.is_final) std::clog << indent << "final" << std::endl;
    if (flags.is_class && flags.is_special_super) std::clog << indent << "special super" << std::endl;
    if (!flags.is_class && flags.is_synchronized) std::clog << indent << "synchronized" << std::endl;
    if (flags.is_volatile) std::clog << indent << "volatile" << std::endl;
    if (flags.is_transient) std::clog << indent << "transient" << std::endl;
    if (flags.is_native) std::clog << indent << "native" << std::endl;
    if (flags.is_interface) std::clog << indent << "interface" << std::endl;
    if (flags.is_abstract) std::clog << indent << "abstract" << std::endl;
    if (flags.is_strictfp) std::clog << indent << "strictfp" << std::endl;
}

std::string cencode(std::string const& data) {
    std::stringstream output;
    output << std::hex;
    for (char c : data) {
        if (32 <= c && c <= 126) {
            output << c;
        } else {
            int value = c;
            if (value < 0) value += 256;
            if (value <= 15) {
                output << "\\x0" << value;
            } else {
                output << "\\x" << value;
            }
        }
    }
    return output.str();
}

std::string cencode(std::vector<unsigned char> const& data) {
    std::stringstream output;
    output << std::hex;
    for (unsigned char uc : data) {
        if (32 <= uc && uc <= 126) {
            output << (char) uc;
        } else if (uc <= 15) {
            output << "\\x0" << (int) uc;
        } else {
            output << "\\x" << (int) uc;
        }
    }
    return output.str();
}

std::string hexencode(std::string const& data) {
    std::stringstream output;
    output << std::hex;
    for (char c : data) {
        int value = c;
        if (value < 0) value += 256;
        if (value <= 15) {
            output << "\\x0" << value;
        } else {
            output << "\\x" << value;
        }
    }
    return output.str();
}

std::string hexencode(std::vector<unsigned char> const& data) {
    std::stringstream output;
    output << std::hex;
    for (unsigned char uc : data) {
        if (uc <= 15) {
            output << "\\x0" << (int) uc;
        } else {
            output << "\\x" << (int) uc;
        }
    }
    return output.str();
}

#define STRINGIFY(X) # X
#define PREFIXTYPE(M) jjde::Constant::Type:: M
#define PTYPEN(S, X) case PREFIXTYPE(X): S << STRINGIFY(X); return S.str();
#define PTYPEC(S, X, C) case PREFIXTYPE(X): S << STRINGIFY(X) << ":" << C; return S.str()
std::string get_constant_description(jjde::Constant const& c) {
    std::stringstream stream;
    switch (c.type) {
    PTYPEC(stream, CLASS_REFERENCE, " reference = " << c.value.reference);
    PTYPEC(stream, DOUBLE, " value = " << c.value.double_);
    PTYPEN(stream, EMPTY);
    PTYPEC(stream, FIELD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, FLOAT, " value = " << c.value.float_);
    PTYPEC(stream, INTEGER, " value = " << c.value.integer);
    PTYPEC(stream, INTERFACE_METHOD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, INVOKE_DYNAMIC, " value = " << c.value.invoke_dynamic);
    PTYPEC(stream, LONG, " value = " << c.value.long_);
    PTYPEC(stream, METHOD_HANDLE, " type_index = " << c.value.method_handle.first << ", index = " << c.value.method_handle.second);
    PTYPEC(stream, METHOD_REFERENCE, " class_reference = " << c.value.pair_reference.first << ", name_type_descriptor = " << c.value.pair_reference.second);
    PTYPEC(stream, METHOD_TYPE, " reference = " << c.value.reference);
    PTYPEC(stream, NAME_TYPE_DESCRIPTOR, " name_index = " << c.value.pair_reference.first << ", type_index = " << c.value.pair_reference.second);
    PTYPEC(stream, STRING, " value = '" << cencode(c.value.string) << "'");
    PTYPEC(stream, STRING_REFERENCE, " reference = " << c.value.reference);
    default: return "Error";
    }
}

int main(int argc, char *argv[]) {

    assert(argc > 1);

    jjde::Class class_ = jjde::read_class(argv[1]);

    std::cout << argv[1] << std::endl
              << "├ Class name:   " << class_.name << std::endl
              << "├ Parent class: " << class_.parent << std::endl
              << "├ Java version: " << class_.version.major << "." << class_.version.minor << std::endl
              << "├ Constant pool" << std::endl;

    for (uint16_t index = 1; index < class_.constants.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << get_constant_description(class_.constants[index]) << std::endl;
    }

    std::cout << "├ Flags" << std::endl;
    print_flags(class_.flags, "│ ├ ");

    std::cout << "├ Interfaces" << std::endl;
    for (uint16_t index = 0; index < class_.interfaces.size(); ++index) {
        std::cout << (index == class_.interfaces.size() - 1 ? "│ └[" : "│ ├[") << index << "]\t" << class_.interfaces[index] << std::endl;
    }

    std::cout << "├ Fields" << std::endl;
    for (uint16_t index = 0; index < class_.fields.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << class_.constants[class_.fields[index].name_index].value.string << std::endl;
        std::cout << "│ │   \t├ Type: " << jjde::decode_type(class_.constants[class_.fields[index].descriptor_index].value.string).to_string() << std::endl;
        std::cout << "│ │   \t├ Flags" << std::endl;
        print_flags(class_.fields[index].flags, "│ │   \t│ ├ ");
        std::cout << "│ │   \t├ Attributes" << std::endl;
        for (uint16_t j = 0; j < class_.fields[index].attributes.size(); ++j) {
            std::cout << "│ │   \t│ ├ " << class_.constants[class_.fields[index].attributes[j].name_index].value.string << std::endl;
            std::cout << "│ │   \t│ │ └ " << hexencode(class_.fields[index].attributes[j].data) << std::endl;
        }
    }

    std::cout << "├ Methods" << std::endl;
    for (uint16_t index = 0; index < class_.methods.size(); ++index) {
        std::cout << "│ ├[" << index << "]\t" << class_.constants[class_.methods[index].name_index].value.string << std::endl;
        std::cout << "│ │   \t├ Type: " << jjde::decode_type(class_.constants[class_.methods[index].descriptor_index].value.string).to_string() << std::endl;
        std::cout << "│ │   \t├ Flags" << std::endl;
        print_flags(class_.methods[index].flags, "│ │   \t│ ├ ");
        std::cout << "│ │   \t├ Attributes" << std::endl;
        for (uint16_t j = 0; j < class_.methods[index].attributes.size(); ++j) {
            std::cout << "│ │   \t│ ├ " << class_.constants[class_.methods[index].attributes[j].name_index].value.string << std::endl;
            std::cout << "│ │   \t│ │ └ " << hexencode(class_.methods[index].attributes[j].data) << std::endl;
        }
    }

    std::cout << "├ Attributes" << std::endl;
    for (uint16_t index = 0; index < class_.attributes.size(); ++index) {
        std::cout << "│ ├ " << class_.constants[class_.attributes[index].name_index].value.string << std::endl;
        std::cout << "│ │ └ " << hexencode(class_.attributes[index].data) << std::endl;
    }

    // Write java code.

    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------------------------------" << std::endl;
    /* Attributes to check:
     *     Code                Method code (+ more information)
     *     ConstantValue       Constant values for primitve 'final' fields
     *     Exceptions          Exceptions thrown by a method
     *     InnerClasses        Inner classes
     *     LineNumberTable     Line numbers (debugging information)
     *     LocalVariableTable  Local variable names (debugging information)
     *     SourceFile          Source code file name (.java)
     *     Synthetic           Field or method is compiler-generated
     */
    // Write class name and parent
    std::cout << class_.flags.to_string() << " class " << class_.name << " extends " << class_.parent;
    // Write interfaces
    if (class_.interfaces.size() > 0) {
        std::cout << " implements " << class_.interfaces[0];
        for (std::size_t index = 1; index < class_.interfaces.size(); ++index) {
            std::cout << ", " << class_.interfaces[index];
        }
    }
    std::cout << " {" << std::endl;

    // Fields
    for (jjde::Object const& field : class_.fields) {
        // Flags
        std::string flags = field.flags.to_string();
        if (flags.size() > 0) flags += " ";
        // Type
        std::string type = jjde::decode_type(class_.constants[field.descriptor_index].value.string).to_string();
        auto it = std::find_if(field.attributes.begin(), field.attributes.end(), [&class_](jjde::Attribute const& attr){ return (class_.constants[attr.name_index].value.string == "Signature"); });
        if (it != field.attributes.end()) {
            // Get signature instead of type (fixes generics type erasure)
            type = jjde::decode_type(class_.constants[jjde::parse<uint16_t>(jjde::convert<2>(it->data))].value.string).to_string();
        }
        // Name
        std::string name = class_.constants[field.name_index].value.string;
        // Output (without value)
        std::cout << "    " << flags << type << " " << name;
        // Check for default value of primitive types in the ConstantValue attribute
        it = std::find_if(field.attributes.begin(), field.attributes.end(), [&class_](jjde::Attribute const& attr){ return (class_.constants[attr.name_index].value.string == "ConstantValue"); });
        if (it != field.attributes.end()) {
            std::cout << " = " << class_.constants[jjde::parse<u_int16_t>(jjde::convert<2>(it->data))].to_value_string(class_.constants);
        }
        std::cout << ";" << std::endl;
    }

    // Methods
    for (jjde::Object const& method : class_.methods) {
        // Flags
        std::string flags = method.flags.to_string();
        if (flags.size() > 0) flags += " ";
        // Name
        std::string name = class_.constants[method.name_index].value.string;
        std::string::size_type dollar = name.find("$"); // Function overloads are numbered using $0, $1, etc.
        if (dollar != std::string::npos) {
            name = name.substr(0, dollar);
        }
        // Type
        jjde::Type jjde_type = jjde::decode_type(class_.constants[method.descriptor_index].value.string);
        auto it = std::find_if(method.attributes.begin(), method.attributes.end(), [&class_](jjde::Attribute const& attr){ return (class_.constants[attr.name_index].value.string == "Signature"); });
        if (it != method.attributes.end()) {
            // Get signature instead of type (fixes generics type erasure)
            jjde_type = jjde::decode_type(class_.constants[jjde::parse<uint16_t>(jjde::convert<2>(it->data))].value.string);
        }
        //  - Get argument names
        std::vector<std::string> argument_names;
        for (std::size_t index = 0; index < jjde_type.argument_types.size(); ++index) {
            argument_names.push_back("arg" + std::to_string(index));
        }
        //  - Get proper type
        std::string signature = jjde_type.to_string(name, argument_names);
        // Output (without value)
        std::cout << "    " << flags << signature << " {" << std::endl;
        // Output code
        it = std::find_if(method.attributes.begin(), method.attributes.end(), [&class_](jjde::Attribute const& attr){ return (class_.constants[attr.name_index].value.string == "Code"); });
        if (it != method.attributes.end()) {
            jjde::Code code = jjde::disassemble(it->data);
            std::cout << "        max. stack size: " << code.max_stack_size << std::endl;
            std::cout << "        local variables: " << code.local_variable_count << std::endl;
            std::cout << "        instructions:" << std::endl;
            for (jjde::Instruction const& instruction : code.instructions) {
                std::cout << "          " << jjde::Instruction::name[instruction.operation] << " " << hexencode(instruction.arguments) << std::endl;
            }
            std::cout << "        exception handlers:" << std::endl;
            for (jjde::ExceptionHandler const& handler : code.exception_handlers) {
                std::cout << "          {" << handler.start << ", " << handler.end << ", " << handler.handler << ", " << handler.exception << "}" << std::endl;
            }
            std::cout << "        attributes:" << std::endl;
            for (jjde::Attribute const& attribute : code.attributes) {
                std::cout << "          " << class_.constants[attribute.name_index].value.string << std::endl;
                std::cout << "            " << hexencode(attribute.data) << std::endl;
            }
        } else {
            std::cout << "        <no code>" << std::endl;
        }
        // Output closing brace
        std::cout << "    }" << std::endl;
    }

    std::cout << "}" << std::endl;
    std::cout << "---------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;

    return 0;
}

