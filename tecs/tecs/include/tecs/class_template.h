#pragma once

// STL
#include <cassert>
#include <memory>

namespace tecs
{

// Copy Prohibited Mix-in
struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

template<typename T>
class Singleton : public NonCopyable
{
public:
    Singleton()
    {
        // Cast to accommodate offset changes during multiple inheritance
        constexpr size_t align_size = alignof(T);
        const std::intptr_t offset = (std::intptr_t)(T*)align_size - (std::intptr_t)(Singleton<T>*)(T*)align_size;
        singleton_ = (T*)((std::intptr_t)this + offset);
    }

    virtual ~Singleton()
    {
        singleton_ = nullptr;
    }

    static T& GetInstance()
    {
        assert(singleton_ != nullptr && "Singleton instance is not created yet.");
        return *singleton_;
    }

    static bool IsInstantiated()
    {
        return singleton_ != nullptr;
    }

private:
    static inline T* singleton_ = nullptr;
};

} // namespace tecs