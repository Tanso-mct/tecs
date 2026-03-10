#include "pch.h"
#include "tecs/id.h"

namespace tecs
{
    
ID::ID()
{
    // Initialize as invalid id
    bits_ = MakeBits(0, 0, false);
}

ID::ID(uint32_t id, uint32_t gen)
{
    // Initialize as valid id with given id and generation
    bits_ = MakeBits(id, gen, true);
}

uint32_t ID::GetID() const
{
    return static_cast<std::uint32_t>((bits_ >> ID_SHIFT) & ID_MASK);
}

uint32_t ID::GetGen() const
{
    return static_cast<std::uint32_t>((bits_ >> GEN_SHIFT) & GEN_MASK);
}

bool ID::IsValid() const
{
    return (bits_ & VALID_MASK) != 0;
}

uint64_t ID::GetBits() const
{
    return bits_;
}

bool ID::operator==(const ID& other) const
{
    return bits_ == other.bits_;
}

bool ID::operator!=(const ID& other) const
{
    return bits_ != other.bits_;
}

bool ID::operator<(const ID& other) const
{
    // Get Gens for comparison
    uint32_t this_gen = GetGen();
    uint32_t other_gen = other.GetGen();

    // Compare generations first
    if (this_gen != other_gen)
        return this_gen < other_gen;

    // Get IDs for comparison
    uint32_t this_id = GetID();
    uint32_t other_id = other.GetID();

    // Compare IDs if generations are equal
    return this_id < other_id;
}

uint64_t ID::MakeBits(uint32_t id, uint32_t gen, bool valid)
{
    uint64_t bits = 0;

    // Encode validity as 1 bit
    uint64_t v = valid ? uint64_t{1} : uint64_t{0};

    // Pack the fields into bits
    bits |= (v & uint64_t{1}) << VALID_SHIFT;
    bits |= (uint64_t(id) & ID_MASK) << ID_SHIFT;
    bits |= (uint64_t(gen) & GEN_MASK) << GEN_SHIFT;

    // Return the encoded bits
    return bits;
}

} // namespace tecs