#pragma once
#include "ComponentTraits.hpp"
#include <unordered_map>
#include <functional>

namespace Engine::ECS {

// Component metadata for runtime operations
struct ComponentMetadata {
    size_t size{ 0 };
    size_t alignment{ 1 };
    std::function<void(void*)> defaultConstruct{ nullptr };        // Default constructor
    std::function<void(void*, const void*)> copyConstruct{ nullptr }; // Copy constructor
    std::function<void(void*)> destruct{ nullptr };                // Destructor
};

// Global component type registry
// Stores metadata for each registered component type
class ComponentRegistry {
public:
    static ComponentRegistry& Instance()
    {
        static ComponentRegistry instance;
        return instance;
    }

    // Register a component type
    template<Component T>
    void Register()
    {
        const auto typeID = GetComponentTypeID<T>();
        if (_metadata.contains(typeID))
            return; // Already registered

        ComponentMetadata meta;
        meta.size = sizeof(T);
        meta.alignment = alignof(T);

        // Default constructor wrapper
        meta.defaultConstruct = [](void* ptr) {
            new (ptr) T();
        };

        // Copy constructor wrapper
        meta.copyConstruct = [](void* dst, const void* src) {
            new (dst) T(*static_cast<const T*>(src));
        };

        // Destructor wrapper
        meta.destruct = [](void* ptr) {
            static_cast<T*>(ptr)->~T();
        };

        _metadata[typeID] = meta;
    }

    // Get metadata for a component type
    [[nodiscard]] const ComponentMetadata* GetMetadata(ComponentTypeID typeID) const noexcept
    {
        auto it = _metadata.find(typeID);
        return it != _metadata.end() ? &it->second : nullptr;
    }

    // Get metadata for a component type T
    template<Component T>
    [[nodiscard]] const ComponentMetadata* GetMetadata() const noexcept
    {
        return GetMetadata(GetComponentTypeID<T>());
    }

private:
    ComponentRegistry() = default;
    std::unordered_map<ComponentTypeID, ComponentMetadata> _metadata;
};

// Helper to auto-register component types
template<Component T>
struct AutoRegisterComponent {
    AutoRegisterComponent()
    {
        ComponentRegistry::Instance().Register<T>();
    }
};

// Macro to register a component type (use in cpp files or headers)
#define REGISTER_COMPONENT(T) \
    static Engine::ECS::AutoRegisterComponent<T> _autoReg_##T;

} // namespace Engine::ECS

