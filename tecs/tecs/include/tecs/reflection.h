#pragma once

// STL
#include <string>
#include <string_view>
#include <any>
#include <unordered_map>

// TECS
#include "tecs/class_template.h"

namespace tecs
{

class ReflectionField
{
public:
    /**
     * @brief
     * Construct a new Reflection Field object with the given name and offset
     */
    ReflectionField(std::string_view name, size_t offset);

    /**
     * @brief
     * Destroy the Reflection Field object. No special cleanup is required.
     */
    virtual ~ReflectionField() = default;

    /**
     * @brief
     * Get the name of the field
     * 
     * @return std::string_view
     * The name of the field
     */
    std::string_view GetName() const { return name_; }

    /**
     * @brief
     * Get the offset of the field within the component
     * 
     * @return size_t
     * The offset of the field
     */
    size_t GetOffset() const { return offset_; }

    /**
     * @brief
     * Get the type name of the field
     * 
     * @return std::string_view
     * The type name of the field
     */
    virtual std::string_view GetTypeName() const = 0;

    /**
     * @brief
     * Get the value of the field from the given instance
     * 
     * @param instance
     * Pointer to the component instance
     * 
     * @return std::any
     * The value of the field as std::any
     */
    virtual std::any GetValue(std::byte* instance) const = 0;

    /**
     * @brief
     * Set the value of the field for the given instance
     * 
     * @param instance
     * Pointer to the component instance
     * 
     * @param value
     * The value to set as std::any
     */
    virtual void SetValue(std::byte* instance, const std::any& value) const = 0;

protected:
    // Field name
    std::string name_;

    // Field offset
    size_t offset_;
};

template <typename T>
class ReflectionFieldType : public ReflectionField
{
};

// --- Specialization for types ---

template <>
class ReflectionFieldType<int> : public ReflectionField
{
public:
    ReflectionFieldType(std::string_view name, size_t offset) : ReflectionField(name, offset) {}
    std::string_view GetTypeName() const override { return "int"; }
    std::any GetValue(std::byte* instance) const override { return *reinterpret_cast<int*>(instance + offset_); }
    void SetValue(std::byte* instance, const std::any& value) const override { *reinterpret_cast<int*>(instance + offset_) = std::any_cast<int>(value); }
};


template <>
class ReflectionFieldType<float> : public ReflectionField
{
public:
    ReflectionFieldType(std::string_view name, size_t offset) : ReflectionField(name, offset) {}
    std::string_view GetTypeName() const override { return "float"; }
    std::any GetValue(std::byte* instance) const override { return *reinterpret_cast<float*>(instance + offset_); }
    void SetValue(std::byte* instance, const std::any& value) const override { *reinterpret_cast<float*>(instance + offset_) = std::any_cast<float>(value); }
};

template <>
class ReflectionFieldType<std::string> : public ReflectionField
{
public:
    ReflectionFieldType(std::string_view name, size_t offset) : ReflectionField(name, offset) {}
    std::string_view GetTypeName() const override { return "std::string"; }
    std::any GetValue(std::byte* instance) const override { return *reinterpret_cast<std::string*>(instance + offset_); }
    void SetValue(std::byte* instance, const std::any& value) const override { *reinterpret_cast<std::string*>(instance + offset_) = std::any_cast<std::string>(value); }
};


class Reflection
{
public:
    Reflection(
        std::byte* instance,
        std::vector<std::unique_ptr<ReflectionField>> fields);

    /**
     * @brief
     * Get the value of the specified field by name
     * 
     * @param field_name
     * The name of the field
     * 
     * @return std::any
     * The value of the field as std::any
     */
    std::any GetFieldValue(std::string_view field_name) const;

    /**
     * @brief
     * Set the value of the specified field by name
     * 
     * @param field_name
     * The name of the field
     * 
     * @param value
     * The value to set as std::any
     */
    void SetFieldValue(std::string_view field_name, const std::any& value);

    /**
     * @brief
     * Get the type name of the specified field by name
     * 
     * @param field_name
     * The name of the field
     * 
     * @return std::string_view
     * The type name of the field
     */
    std::string_view GetFieldTypeName(std::string_view field_name) const;

    /**
     * @brief
     * Get a list of all field names in the reflection
     * 
     * @return std::vector<std::string>
     * Vector of field names
     */
    std::vector<std::string> GetFieldNames() const;

private:
    // Pointer to the instance
    std::byte* instance_ = nullptr;

    // Map of field name to ReflectionField
    std::unordered_map<std::string, std::unique_ptr<ReflectionField>> fields_;
};

} // namespace tecs