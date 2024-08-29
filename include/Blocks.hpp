#pragma once

#include "StandardBlock.hpp"
#include <memory>
#include <string>
#include <variant>

namespace allocity {

class Blocks {
private:
    std::unique_ptr<StandardBlock> m_StandardBlock;

public:
    Blocks() : m_StandardBlock(nullptr) {}

    const StandardBlock* GetStandardBlock() const {

        return m_StandardBlock.get();
        
    }

    void SetStandardBlock(std::unique_ptr<StandardBlock> block) {

        m_StandardBlock = std::move(block);

    }

    bool HasStandardBlock() const { 

        return m_StandardBlock != nullptr; 

    }

    std::variant<int, double, std::string> GetBlockDefine() const {

        return m_StandardBlock ? m_StandardBlock->BlockDefine : std::variant<int, double, std::string>{}; 

    }

    void SetBlockDefine(std::variant<int, double, std::string> blockDefine) {

        if (m_StandardBlock) m_StandardBlock->BlockDefine = std::move(blockDefine);

    }

    std::string GetBlockName() const { 

        return m_StandardBlock ? m_StandardBlock->BlockName : ""; 

    }

    void SetBlockName(std::string blockName) {

        if (m_StandardBlock) m_StandardBlock->BlockName = std::move(blockName);

    }

    std::size_t GetBlockQuantity() const { 

        return m_StandardBlock ? m_StandardBlock->BlockQuantity : 0; 

    }

    void SetBlockQuantity(std::size_t blockQuantity) {

        if (m_StandardBlock) {

            m_StandardBlock->BlockQuantity = blockQuantity;

        }
    }

    std::size_t GetBlockSize() const {

        return m_StandardBlock ? m_StandardBlock->BlockSize : 0; 

    }

    void SetBlockSize(std::size_t blockSize) {

        if (m_StandardBlock) {

            m_StandardBlock->BlockSize = blockSize;

        }
    }

    bool GetIsFree() const { 

        return m_StandardBlock ? m_StandardBlock->IsFree : false; 

    }

    void SetIsFree(bool isFree) {

        if (m_StandardBlock) {

            m_StandardBlock->IsFree = isFree;

        }
    }

    StandardBlock* GetNext() const {

        return m_StandardBlock ? m_StandardBlock->Next.get() : nullptr;

    }

    void SetNext(std::unique_ptr<StandardBlock> next) {

        if (m_StandardBlock) {

            m_StandardBlock->Next = std::move(next);

        }
    }

    StandardBlock* GetOperations() const {

        return m_StandardBlock ? m_StandardBlock->Operations.get() : nullptr;

    }

    void SetOperations(std::unique_ptr<StandardBlock> operations) {

        if (m_StandardBlock) {

            m_StandardBlock->Operations = std::move(operations);

        }
    }
};

} 
