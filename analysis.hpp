#ifndef JJDE_ANALYSIS_HPP
#define JJDE_ANALYSIS_HPP

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "disassembler.hpp"
#include "instructions.hpp"

namespace jjde {

struct CodeFlowItem {
    std::vector<Instruction> instructions;
    std::vector<std::size_t> parents;
    std::vector<std::size_t> children;
    bool deleted = false;

    CodeFlowItem() : deleted(true) {}
    CodeFlowItem(Instruction && single) {
        instructions.emplace_back(std::move(single));
    }
};

struct CodeFlow {
    std::vector<std::shared_ptr<CodeFlowItem>> items;
    std::map<std::size_t, std::size_t> address_map;
    std::vector<ExceptionHandler> exception_handlers;

    CodeFlow(jjde::Bytecode && bytecode) : exception_handlers(std::move(bytecode.exception_handlers)) {
        for (std::size_t index = 0; index < bytecode.instructions.size(); ++index) {
            address_map.emplace(bytecode.instructions[index].location, index);
            items.emplace_back(std::move(std::make_shared<CodeFlowItem>(std::move(bytecode.instructions[index]))));
        }

        // Link items together
        for (std::size_t index = 0; index < items.size(); ++index) {
            // Avoid copying instructions. Always just take the first element, since at this point, all items only contain one instruction
            Instruction* ptr = items[index]->instructions.data();

            bool allow_jump_to_next;
            std::vector<std::size_t> additional_targets;

            switch (ptr->operation) {
            case Instruction::IFEQ:
            case Instruction::IFNE:
            case Instruction::IFLT:
            case Instruction::IFGE:
            case Instruction::IFGT:
            case Instruction::IFLE:
            case Instruction::IF_ICMPEQ:
            case Instruction::IF_ICMPNE:
            case Instruction::IF_ICMPLT:
            case Instruction::IF_ICMPGE:
            case Instruction::IF_ICMPGT:
            case Instruction::IF_ICMPLE:
            case Instruction::IF_ACMPEQ:
            case Instruction::IF_ACMPNE:
            case Instruction::IFNULL:
            case Instruction::IFNONNULL:
                // Allow jumping to next, but also add jump target
                allow_jump_to_next = true;
                additional_targets.push_back(ptr->location + parse<int16_t>(convert<2>(ptr->arguments)));
                break;
            case Instruction::GOTO:
            case Instruction::JSR:
                // Forbid jumping to next, add jump target
                allow_jump_to_next = false;
                additional_targets.push_back(ptr->location + parse<int16_t>(convert<2>(ptr->arguments)));
                break;
            case Instruction::RET:
                // Where on earth do we jump here?
                throw std::runtime_error("Cannot analyze code flow with RET instructions");
            case Instruction::TABLESWITCH:
                throw std::logic_error("TODO");
                break;
            case Instruction::LOOKUPSWITCH:
                throw std::logic_error("TODO");
                break;
            case Instruction::IRETURN:
            case Instruction::LRETURN:
            case Instruction::FRETURN:
            case Instruction::DRETURN:
            case Instruction::ARETURN:
            case Instruction::RETURN:
            case Instruction::ATHROW:
                // No jumps from here
                allow_jump_to_next = false;
                additional_targets = {};
                break;
            case Instruction::GOTO_W:
            case Instruction::JSR_W:
                // Forbid jumping to next, add jump target
                allow_jump_to_next = false;
                additional_targets.push_back(ptr->location + parse<int32_t>(convert<4>(ptr->arguments)));
                break;
            default:
                allow_jump_to_next = true;
                additional_targets = {};
            }

            if (allow_jump_to_next) {
                items[index]->children.push_back(index + 1);
                items[index + 1]->parents.push_back(index);
            }

            for (std::size_t address : additional_targets) {
                std::size_t target_index = address_map[address];
                items[index]->children.push_back(target_index);
                items[target_index]->parents.push_back(index);
            }
        }

        // Tighten chains
        for (std::size_t index = 0; index < items.size();) {
            if (items[index]->deleted) {
                // Skip deleted blocks
                ++index;
                continue;
            }
            if (items[index]->parents.size() != 1) {
                // Skip blocks with multiple paths to them
                ++index;
                continue;
            }
            std::size_t parent = items[index]->parents[0];
            if (items[parent]->children.size() != 1) {
                // Skip blocks whose parent has more than one leaving path
                ++index;
                continue;
            }
            // Attach the current block to the parent (leave the deleted item in the list for now, to keep indices intact)
            items[parent]->children = items[index]->children;
            items[parent]->instructions.insert(items[parent]->instructions.end(), items[index]->instructions.begin(), items[index]->instructions.end());
            // Update the children's parent entries
            for (std::size_t child : items[parent]->children) {
                std::replace(items[child]->parents.begin(), items[child]->parents.end(), index, parent);
            }
            // Clear the current block
            items[index] = std::make_shared<CodeFlowItem>();
            // Continue to the next element, unless the parent is before the current item
            index = std::min(parent, index + 1);
        }

        // Delete removed elements? Or is the required complexity (for each (deleted) item check all children/parents of each item) too much?
    }
};

void _visualize_code_flow(jjde::Bytecode copied) {
    CodeFlow cf(std::move(copied));
    for (std::size_t index = 0; index < cf.items.size(); ++index) {
        if (cf.items[index]->deleted) continue;
        std::cout << index;
        std::cout << "\t\t\t\t\t<-- ";
        for (std::size_t parent : cf.items[index]->parents) {
            std::cout << parent << " ";
        }
        for (Instruction inst : cf.items[index]->instructions) {
            std::cout << std::endl << "\t "  << std::uppercase << Instruction::name[inst.operation] << " " << hexencode(inst.arguments);
        }
        std::cout << "\t\t--> ";
        for (std::size_t child : cf.items[index]->children) {
            std::cout << child << " ";
        }
        std::cout << std::endl;
    }
}

}

#endif // JJDE_ANALYSIS_HPP
