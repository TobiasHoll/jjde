#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>

#include "analysis.hpp"
#include "annotater.hpp"
#include "class.hpp"
#include "disassembler.hpp"
#include "flags.hpp"
#include "instructions.hpp"
#include "objects.hpp"
#include "types.hpp"


int main(int argc, char *argv[]) {

    if (argc <= 1) {
        std::cerr << "Usage:" << std::endl
                  << "    " << argv[0] << " <file.class>" << std::endl;
        return 1;
    }

    jjde::Class class_ = jjde::read_class(argv[1]);

    // Write java code.

    std::cout << std::endl;
    std::cout << argv[1] << std::endl;
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
    std::cout << class_.flags.to_string() << " class " << class_.name;
    if (class_.parent != "java.lang.Object") {
        std::cout << " extends " << class_.parent;
    }

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
            std::cout << " = " << class_.constants[jjde::parse<uint16_t>(jjde::convert<2>(it->data))].to_string(class_.constants);
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
        std::cout << "    " << flags << signature;

        // Output code
        it = std::find_if(method.attributes.begin(), method.attributes.end(), [&class_](jjde::Attribute const& attr){ return (class_.constants[attr.name_index].value.string == "Code"); });
        if (it != method.attributes.end()) {
            std::cout << " {" << std::endl;
            jjde::Bytecode bytecode = jjde::disassemble(it->data);
            jjde::Code code = jjde::annotate(class_, bytecode, method.flags.is_static);
            std::cout << code.to_string();
            std::cout << "    }" << std::endl;

            std::cout << "    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl;
            jjde::_visualize_code_flow(std::move(bytecode));
            std::cout << "    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;
        } else {
            std::cout << " {}" << std::endl;
        }

        std::cout << std::endl;
    }

    std::cout << "}" << std::endl;
    std::cout << "---------------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;

    return 0;
}
