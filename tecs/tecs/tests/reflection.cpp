// Google Test
#include <gtest/gtest.h>

// TECS
#include "tecs/tecs.h"

TEST(tecs, reflection_field)
{
    struct SampleStruct
    {
        int int_field = 0;
        float float_field = 0.0f;
        std::string string_field = "none";
    };

    // Create instance of SampleStruct
    SampleStruct sample_instance;

    // Create reflection fields
    tecs::ReflectionFieldType<int> int_field("int_field", offsetof(SampleStruct, int_field));
    tecs::ReflectionFieldType<float> float_field("float_field", offsetof(SampleStruct, float_field));
    tecs::ReflectionFieldType<std::string> string_field("string_field", offsetof(SampleStruct, string_field));

    // Set field values using reflection
    const int kTestIntValue = 10;
    int_field.SetValue(reinterpret_cast<std::byte*>(&sample_instance), kTestIntValue);

    const float kTestFloatValue = 3.14f;
    float_field.SetValue(reinterpret_cast<std::byte*>(&sample_instance), kTestFloatValue);

    const std::string kTestStringValue = "test_string";
    string_field.SetValue(reinterpret_cast<std::byte*>(&sample_instance), kTestStringValue);

    // Get field values using reflection and verify
    int retrieved_int 
        = std::any_cast<int>(int_field.GetValue(reinterpret_cast<std::byte*>(&sample_instance)));
    EXPECT_EQ(retrieved_int, kTestIntValue);

    float retrieved_float 
        = std::any_cast<float>(float_field.GetValue(reinterpret_cast<std::byte*>(&sample_instance)));
    EXPECT_FLOAT_EQ(retrieved_float, kTestFloatValue);

    std::string retrieved_string 
        = std::any_cast<std::string>(string_field.GetValue(reinterpret_cast<std::byte*>(&sample_instance)));
    EXPECT_EQ(retrieved_string, kTestStringValue);
}

TEST(tecs, reflection)
{
    struct SampleStruct
    {
        int int_field = 0;
        float float_field = 0.0f;
        std::string string_field = "none";
    };

    // Create instance of SampleStruct
    SampleStruct sample_instance;

    // Create reflection
    tecs::Reflection reflection(
        reinterpret_cast<std::byte*>(&sample_instance), []
        {
            std::vector<std::unique_ptr<tecs::ReflectionField>> fields;
            fields.push_back(std::make_unique<tecs::ReflectionFieldType<int>>("int_field", offsetof(SampleStruct, int_field)));
            fields.push_back(std::make_unique<tecs::ReflectionFieldType<float>>("float_field", offsetof(SampleStruct, float_field)));
            fields.push_back(std::make_unique<tecs::ReflectionFieldType<std::string>>("string_field", offsetof(SampleStruct, string_field)));
            return fields;
        }());

    // Set field values using reflection
    const int kTestIntValue = 20;
    reflection.SetFieldValue("int_field", kTestIntValue);

    const float kTestFloatValue = 6.28f;
    reflection.SetFieldValue("float_field", kTestFloatValue);

    const std::string kTestStringValue = "reflection_test";
    reflection.SetFieldValue("string_field", kTestStringValue);

    // Get field values using reflection and verify
    int retrieved_int = std::any_cast<int>(reflection.GetFieldValue("int_field"));
    EXPECT_EQ(retrieved_int, kTestIntValue);

    float retrieved_float = std::any_cast<float>(reflection.GetFieldValue("float_field"));
    EXPECT_FLOAT_EQ(retrieved_float, kTestFloatValue);

    std::string retrieved_string = std::any_cast<std::string>(reflection.GetFieldValue("string_field"));
    EXPECT_EQ(retrieved_string, kTestStringValue);

    // Verify field type names
    EXPECT_EQ(reflection.GetFieldTypeName("int_field"), "int");
    EXPECT_EQ(reflection.GetFieldTypeName("float_field"), "float");
    EXPECT_EQ(reflection.GetFieldTypeName("string_field"), "std::string");

    // Verify field names list
    std::vector<std::string> field_names = reflection.GetFieldNames();
    EXPECT_EQ(field_names.size(), 3);
    EXPECT_NE(std::find(field_names.begin(), field_names.end(), "int_field"), field_names.end());
    EXPECT_NE(std::find(field_names.begin(), field_names.end(), "float_field"), field_names.end());
    EXPECT_NE(std::find(field_names.begin(), field_names.end(), "string_field"), field_names.end());
}