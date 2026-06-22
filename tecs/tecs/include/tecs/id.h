#pragma once

// STL
#include <cstdint>
#include <memory>

namespace tecs
{

/**
 * @brief
 * ID is a class that represents a unique identifier.
 * It has a 64-bit unsigned integer (bits) that encodes both the ID and generation.
 */
class ID
{
public:
    /**
     * @brief Construct a new ID object as invalid
     * If no parameters are provided, the id is considered invalid
     */
    ID();

    /**
     * @brief : Construct a new ID object with the given ID and generation
     * @param id : The unique identifier for the id
     * @param generation : The generation number for the id
     */
    ID(uint32_t id, uint32_t gen);

    /**
     * @brief No special cleanup is needed for the ID object
     */
    ~ID() = default;

    /**
     * @brief Get the ID of the id
     * @return uint32_t The number representing the ID
     */
    uint32_t GetID() const;

    /**
     * @brief
     * Get the generation of the id
     */
    uint32_t GetGen() const;

    /**
     * @brief
     * Check if the id is valid
     * 
     * @return true 
     * If the id is valid
     * 
     * @return false 
     * If the id is invalid
     */
    bool IsValid() const;

    /**
     * @brief
     * Get the encoded bits representing the id
     * 
     * @return uint64_t 
     * The encoded bits of the id
     */
    uint64_t GetBits() const;

    /**
     * @brief
     * Equality operator for ID
     * 
     * @param other 
     * The other id to compare with
     * 
     * @return true 
     * If both entities are equal
     * 
     * @return false 
     * If both entities are not equal
     */
    bool operator==(const ID& other) const;

    /**
     * @brief
     * Inequality operator for ID
     * 
     * @param other 
     * The other id to compare with
     * 
     * @return true 
     * If both entities are not equal
     * 
     * @return false 
     * If both entities are equal
     */
    bool operator!=(const ID& other) const;

    /**
     * @brief
     * Less-than operator for ID
     * 
     * @param other 
     * The other id to compare with
     * 
     * @return true 
     * If this id is less than the other id
     * 
     * @return false 
     * If this id is not less than the other id
     */
    bool operator<(const ID& other) const;

private:
    // id, generation, validity bit
    uint64_t bits_;

    // Bit shifts for encoding and decoding the id information
    static constexpr unsigned VALID_SHIFT = 0;
    static constexpr unsigned ID_SHIFT = 1;
    static constexpr unsigned GEN_SHIFT   = 32;

    // Masks for extracting fields
    static constexpr uint64_t VALID_MASK = uint64_t{1} << VALID_SHIFT;
    static constexpr uint64_t ID_MASK = (uint64_t{1} << 31) - 1; // 31bit
    static constexpr uint64_t GEN_MASK = (uint64_t{1} << 32) - 1; // 32bit

    /**
     * @brief
     * Create the encoded bits for the id
     * 
     * @param id 
     * The unique identifier for the id
     * 
     * @param gen 
     * The generation number for the id
     * 
     * @param valid 
     * The validity of the id
     * 
     * @return uint64_t 
     * The encoded bits representing the id
     */
    uint64_t MakeBits(uint32_t id, uint32_t gen, bool valid);
};

} // namespace tecs

namespace std
{
    /**
     * @brief
     * Specialization of std::hash for tecs::ID
     * Allows using ID as a key in unordered containers
     */
    template <>
    struct hash<tecs::ID>
    {
        size_t operator()(const tecs::ID& id) const noexcept
        {
            return static_cast<size_t>(id.GetBits());
        }
    };
} // namespace std