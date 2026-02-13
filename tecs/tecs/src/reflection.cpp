#include "pch.h"
#include "tecs/reflection.h"

namespace tecs
{

ReflectionField::ReflectionField(std::string_view name, size_t offset) :
    name_(name), offset_(offset)
{
}

Reflection::Reflection(
    std::byte* instance,
    std::vector<std::unique_ptr<ReflectionField>> fields) : 
    instance_(instance)
{
    // Ensure instance pointer is not null
    assert(instance_ != nullptr && "Instance pointer cannot be null");

    // Iterate over the fields and populate the map
    for (auto& field : fields)
    {
        assert(field != nullptr && "ReflectionField pointer cannot be null");

        // Add field to the map
        fields_[field->GetName().data()] = std::move(field);
    }
}

std::any Reflection::GetFieldValue(std::string_view field_name) const
{
    // Find the field in the map
    auto it = fields_.find(field_name.data());
    assert(it != fields_.end() && "Field name not found in reflection");

    // Get the field and retrieve its value
    const ReflectionField* field = it->second.get();
    return field->GetValue(instance_);
}

void Reflection::SetFieldValue(std::string_view field_name, const std::any& value)
{
    // Find the field in the map
    auto it = fields_.find(field_name.data());
    assert(it != fields_.end() && "Field name not found in reflection");

    // Get the field and set its value
    ReflectionField* field = it->second.get();
    field->SetValue(instance_, value);
}

std::string_view Reflection::GetFieldTypeName(std::string_view field_name) const
{
    // Find the field in the map
    auto it = fields_.find(field_name.data());
    assert(it != fields_.end() && "Field name not found in reflection");

    // Get the field and retrieve its type name
    const ReflectionField* field = it->second.get();
    return field->GetTypeName();
}

std::vector<std::string> Reflection::GetFieldNames() const
{
    std::vector<std::string> field_names;

    // Iterate over the fields and collect their names
    for (const auto& pair : fields_)
        field_names.push_back(pair.first);

    return field_names;
}

} // namespace tecs