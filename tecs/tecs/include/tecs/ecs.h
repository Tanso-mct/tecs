#pragma once

// STL
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

// TECS
#include "tecs/reflection.h"
#include "tecs/guid.h"

namespace tecs
{

/**
 * @brief
 * Class representing an entity in the ECS (Entity-Component-System) architecture
 * Entities are identified by a unique ID and a generation number
 */
class Entity
{
public:
    /**
     * @brief
     * Construct a new Entity object
     * If no parameters are provided, the entity is considered invalid
     */
    Entity();

    /**
     * @brief
     * Construct a new Entity object with the given ID and generation
     * 
     * @param id 
     * The unique identifier for the entity
     * 
     * @param generation 
     * The generation number for the entity
     */
    Entity(uint32_t id, uint32_t gen);

    /**
     * @brief
     * Destroy the Entity object. No special cleanup is required.
     */
    ~Entity() = default;

    /**
     * @brief
     * Get the ID of the entity
     */
    uint32_t GetID() const;

    /**
     * @brief
     * Get the generation of the entity
     */
    uint32_t GetGen() const;

    /**
     * @brief
     * Check if the entity is valid
     * 
     * @return true 
     * If the entity is valid
     * 
     * @return false 
     * If the entity is invalid
     */
    bool IsValid() const;

private:
    // id, generation, validity bit
    uint64_t bits_;

    // Bit shifts for encoding and decoding the entity information
    static constexpr unsigned VALID_SHIFT = 0;
    static constexpr unsigned ID_SHIFT = 1;
    static constexpr unsigned GEN_SHIFT   = 32;

    /**
     * @brief
     * Create the encoded bits for the entity
     * 
     * @param id 
     * The unique identifier for the entity
     * 
     * @param gen 
     * The generation number for the entity
     * 
     * @param valid 
     * The validity of the entity
     * 
     * @return uint64_t 
     * The encoded bits representing the entity
     */
    uint64_t MakeBits(size_t id, size_t gen, bool valid);
};

/**
 * @brief
 * Base class for all components in the ECS architecture
 * Components can be attached to entities to define their data and behavior
 */
class Component
{
public:
    /**
     * @brief
     * Configuration class for importing and exporting component data
     * You can derive from this class to implement custom configuration data
     */
    class Config
    {
    public:
        /**
         * @brief
         * Construct a new Config object with the given Reflection object
         * 
         * @param reflection
         * Unique pointer to the Reflection object for runtime type information
         */
        Config(std::unique_ptr<Reflection> reflection);

        /**
         * @brief
         * Destroy the Config object. No special cleanup is required.
         */
        virtual ~Config() = default;

        /**
         * @brief
        * Get the Reflection object associated with this configuration
        * 
        * @return const Reflection&
        * Reference to the Reflection object
         */
        const Reflection& GetReflection() const;

    private:
        // Reflection object for runtime type information
        std::unique_ptr<Reflection> reflection_ = nullptr;
    };

    /**
     * @brief
     * Construct a new Component object with the given name and GUID
     * 
     * @param name
     * The name of the component
     * 
     * @param guid
     * The GUID of the component
     */
    Component(std::string_view name, const GUID& guid);

    /**
     * @brief
     * Destroy the Component object. No special cleanup is required.
     */
    virtual ~Component() = default;

    /**
     * @brief
     * Import component data from the given configuration
     * 
     * @param config
     * Unique pointer to the configuration object containing the data to import
     * 
     * @return true 
     * If the import was successful
     * 
     * @return false 
     * If the import failed
     */
    virtual bool Import(std::unique_ptr<Config> config) = 0;

    /**
     * @brief
     * Apply configuration data to the component
     * 
     * @param config
     * Unique pointer to the configuration object containing the data to apply
     * 
     * @return true 
     * If the apply was successful
     * 
     * @return false 
     * If the apply failed
     */
    virtual bool Apply(std::unique_ptr<Config> config) = 0;

    /**
     * @brief
     * Export component data to a configuration object
     * 
     * @return std::unique_ptr<Config>
     * Unique pointer to the configuration object containing the exported data
     */
    virtual std::unique_ptr<Config> Export() const = 0;

    /**
     * @brief
     * Get the name of the component
     * 
     * @return std::string_view 
     * The name of the component
     */
    std::string_view GetName() const { return name_; }

    /**
     * @brief
     * Get the GUID of the component
     * 
     * @return const GUID& 
     * Reference to the GUID of the component
     */
    const GUID& GetGUID() const { return guid_; }

protected:
    // Next unique component ID
    static uint32_t next_id_;

private:
    // Component name
    const std::string name_;

    // Component GUID
    const GUID guid_;
};

/**
 * @brief
 * Templated base class for all components in the ECS architecture
 */
template <typename T>
class ComponentBase : public Component
{
public:
    /**
     * @brief
     * Construct a new ComponentBase object with the given name and GUID
     * 
     * @param name 
     * The name of the component
     * 
     * @param guid 
     * The GUID of the component
     */
    ComponentBase(std::string_view name, const GUID& guid) :
        Component(name, guid)
    {
    }

    /**
     * @brief
     * Destroy the ComponentBase object. No special cleanup is required.
     */
    virtual ~ComponentBase() = default;

    /**
     * @brief
     * Get the unique component ID for the derived component type T
     * 
     * @return uint32_t 
     * The unique component type ID
     */
    static uint32_t ID()
    {
        static uint32_t id = next_id_++;
        return id;
    }
};

class World
{

};

/**
 * @brief
 * Base class for all systems in the ECS architecture
 * Systems operate on entities and their components to implement logic
 */
class System
{
public:
    /**
     * @brief
     * Destroy the System object. No special cleanup is required.
     */
    virtual ~System() = default;

    /**
     * @brief
     * Update the system with the given world and delta time
     * 
     * @param world
     * Reference to the world object
     * 
     * @param delta_time
     * Time elapsed since the last update
     * 
     * @return true
     * If the system update will continue
     * 
     * @return false
     * If the system update should stop
     */
    virtual bool Update(World& world, float delta_time) = 0;

protected:
    // Next unique system ID
    static uint32_t next_id_;
};

/**
 * @brief
 * Templated base class for all systems in the ECS architecture
 */
class SystemBase : public System
{
public:
    /**
     * @brief
     * Construct a new SystemBase object. No special initialization is required.
     */
    SystemBase() = default;

    /**
     * @brief
     * Destroy the SystemBase object. No special cleanup is required.
     */
    virtual ~SystemBase() = default;

    /**
     * @brief
     * Get the unique system ID for the derived system type T
     * 
     * @return uint32_t 
     * The unique system type ID
     */
    template <typename T>
    static uint32_t ID()
    {
        static uint32_t id = next_id_++;
        return id;
    }
};

} // namespace tecs