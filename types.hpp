#ifndef JJDE_TYPES_HPP
#define JJDE_TYPES_HPP

#include <algorithm>
#include <memory>
#include <vector>

namespace jjde {

/* Types */

struct Type {
    enum Usage {
        FUNCTION,
        STANDARD
    };

    // Bytecode type
    std::string internal_type;

    // Standard or function type?
    Usage usage;

    // For normal and function types
    std::vector<Type> generics;

    // For leaf types (always normal types)
    std::string java_type;
    std::size_t array_dimensions;

    // For function types
    std::shared_ptr<Type> return_type; // structs cannot contain instances of themselves.
    std::vector<Type> argument_types;

    Type(std::string const& internal_type_, Usage usage_, std::vector<Type> const& generics_, std::string const& java_type_, std::size_t array_dimensions_)
        : internal_type(internal_type_)
        , usage(usage_)
        , generics(generics_)
        , java_type(java_type_)
        , array_dimensions(array_dimensions_) {}

    Type(std::string const& internal_type_, Usage usage_, std::vector<Type> const& generics_, Type const& return_type_, std::vector<Type> argument_types_)
        : internal_type(internal_type_)
        , usage(usage_)
        , generics(generics_)
        , return_type(std::make_shared<Type>(return_type_))
        , argument_types(argument_types_) {}

    Type() = default;

    std::string to_string(std::string name="", std::vector<std::string> argument_names=std::vector<std::string>()) const{
        // Text before and after the generics
        std::string pre_generics, post_generics;
        if (usage == Usage::FUNCTION) {
            // Only the function return type comes before the generics
            pre_generics = return_type->to_string();
            // After the generics, the next token is either '(' (without name) or ' name(' (with name)
            post_generics = (name == "" ? "" : " " + name) + "(";
            // Add arguments
            for (std::size_t argument = 0; argument < argument_types.size(); ++argument) {
                // Add comma if necessary
                if (argument != 0) post_generics += ", ";
                // Add the argument type
                post_generics += argument_types[argument].to_string();
                // If a name is given for this argument, use it
                if (argument_names.size() > argument && argument_names[argument] != "") {
                    post_generics += " " + argument_names[argument];
                }
            }
            // Close parenthesis
            post_generics += ")";
        } else {
            // The part before the generics is identical to `java_type`
            pre_generics = java_type;
            // Only array specifiers are placed after the generics
            post_generics = "";
            for (std::size_t dimension = 0; dimension < array_dimensions; ++dimension) {
                post_generics += "[]";
            }
            if (name != "") {
                post_generics += " " + name;
            }
        }
        // Add generics
        std::string in_generics = "";
        if (generics.size() > 0) {
            // Add <
            in_generics += "<";
            // Add generic types
            for (std::size_t generic = 0; generic < generics.size(); ++generic) {
                // Comma, if necessary
                if (generic != 0) in_generics += ", ";
                // Add type
                in_generics += generics[generic].to_string();
            }
            // Add >
            in_generics += ">";
        }
        // Concatenate the parts
        return pre_generics + in_generics + post_generics;
    }
};

/* Decoding bytecode type names */

/* Example:
 *
 * The function
 *     String[] process(java.util.ArrayList<Integer> param);
 * would generally be expected to have the following signature:
 *     java.lang.String[](java.util.ArrayList<java.lang.Integer>)
 * In bytecode, however, the type signature is
 *     (Ljava/util/ArrayList<Ljava/lang/Integer;>;)[Ljava/lang/String;
 * (the missing '[' is intentional)
 * With generic type erasure, the actual type (stored in the type descriptor rather than the method attributes) is
 *     (Ljava/util/ArrayList;)[Ljava/lang/String;
 */

inline Type decode_type(std::string const& internal_type);

enum type_state {
    FULL_TYPE,
    CLASS_TYPE,
    GENERIC_TYPE
};

std::vector<Type> decode_types(std::string const& internal_type) {
    std::string::size_type open = internal_type.find('(');
    std::string::size_type close = internal_type.find(')');

    if (open == std::string::npos && close == std::string::npos) {
        // Standard type
        std::vector<Type> types;

        // Temporaries for string walking
        std::size_t array_count = 0;
        std::size_t generic_count = 0;
        std::string current_type;
        std::string current_generics = "";
        type_state state = FULL_TYPE;

        // Iterate over the string
        for (std::string::size_type index = 0; index < internal_type.size(); ++index) {
            char c = internal_type[index];
            if (c == '<') {
                current_generics += c;
                ++generic_count;
            } else if (c == '>') {
                current_generics += c;
                --generic_count;
            } else if (state == FULL_TYPE) {
                switch (c) {
                case '[':
                    ++array_count;
                    break;
                case 'B':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "byte", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'C':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "char", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'D':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "double", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'F':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "float", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'I':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "int", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'J':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "long", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'S':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "short", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'V':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "void", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'Z':
                    types.push_back(Type{current_type, Type::Usage::STANDARD, std::vector<Type>(), "boolean", array_count});
                    array_count = 0;
                    current_type = "";
                    break;
                case 'T':
                    state = GENERIC_TYPE;
                    break;
                case 'L':
                    state = CLASS_TYPE;
                    break;
                default:
                    throw std::logic_error("Invalid type specifier");
                }
            } else {
                if (generic_count > 0) {
                    current_generics += c;
                } else if (c == ';') {
                    // Parse generics
                    std::vector<Type> generics;
                    if (current_generics.size() > 0) {
                        generics = decode_types(current_generics.substr(1, current_generics.size() - 2));
                    }

                    // Add type
                    std::string proper_type = current_type;
                    std::replace(proper_type.begin(), proper_type.end(), '/', '.');
                    std::replace(proper_type.begin(), proper_type.end(), '$', '.');

                    std::string base;
                    if (state == CLASS_TYPE) base = "L" + current_type + ";";
                    else if (state == GENERIC_TYPE) base = "T" + current_type + ";";
                    types.push_back(Type{base, Type::Usage::STANDARD, generics, proper_type, array_count});

                    // Reset
                    current_type = "";
                    current_generics = "";
                    generic_count = 0;
                    array_count = 0;
                    state = FULL_TYPE;
                } else {
                    current_type += c;
                }
            }
        }

        return types;
    } else if (open == std::string::npos || close == std::string::npos) {
        throw std::logic_error("Malformed type: " + internal_type);
    } else {
        // Function type
        std::string generics = internal_type.substr(0, open);
        std::string arguments = internal_type.substr(open + 1, close - open - 1);
        std::string return_type = internal_type.substr(close + 1);

        // Recurse
        return std::vector<Type>{Type{internal_type, Type::Usage::FUNCTION, decode_types(generics), decode_type(return_type), decode_types(arguments)}};
    }
}

inline Type decode_type(std::string const& internal_type) {
    return decode_types(internal_type).at(0); // Use at to avoid segfaults.
}

std::string decode_class_name(std::string class_name) {
    std::replace(class_name.begin(), class_name.end(), '/', '.');
    return class_name;
}

}

#endif // JJDE_TYPES_HPP
