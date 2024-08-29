#pragma once

#include <string>
#include <variant>
#include <memory>

namespace allocity {

struct StandardBlock {

    std::variant<int, double, std::string> BlockDefine;
    std::string BlockName;

    std::size_t BlockQuantity;
    std::size_t BlockSize;

    bool IsFree;
    
    std::unique_ptr<StandardBlock> Next;
    std::unique_ptr<StandardBlock> Operations;

    StandardBlock(std::variant<int, double, std::string> blockDefine, 
                  std::string blockName, 
                  std::size_t blockQuantity, 
                  std::size_t blockSize, 
                  bool isFree, 
                  std::unique_ptr<StandardBlock> next, 
                  std::unique_ptr<StandardBlock> operations)
        : BlockDefine(std::move(blockDefine)), 
          BlockName(std::move(blockName)), 
          BlockQuantity(blockQuantity), 
          BlockSize(blockSize), 
          IsFree(isFree), 
          Next(std::move(next)), 
          Operations(std::move(operations)) {}

    StandardBlock(const StandardBlock&) = delete;
    StandardBlock(StandardBlock&&) = default;
    StandardBlock& operator=(const StandardBlock&) = delete;
    StandardBlock& operator=(StandardBlock&&) = default;

};

} 
