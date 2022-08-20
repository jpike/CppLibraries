#pragma once

namespace GRAPHICS::DIRECT_X
{
    /// A GPU resource allocated with Direct X.
    /// This class helps automate releasing of resources.
    template <typename ResourceType>
    class DirectXGpuResource
    {
    public:
        // CONSTRUCTION/DESTRUCTION.
        ~DirectXGpuResource();

        // OPERATORS.
        DirectXGpuResource& operator=(ResourceType* resource);
        ResourceType* operator*() const;
        ResourceType* operator->();

        // RELEASE METHODS.
        void Release();

        // PUBLIC MEMBER VARIABLES FOR EASY ACCESS.
        /// The resource being wrapped.
        ResourceType* Resource = nullptr;
    };

    /// Destructor to release the resource if it still exists.
    template <typename ResourceType>
    DirectXGpuResource<ResourceType>::~DirectXGpuResource()
    {
        Release();
    }

    /// Assignment operator to allow easy assignment of raw resources to this pointer.
    /// Note that if this wrapper is already holding a resource, no releasing is one.
    /// @param[in]  resource - The resource to wrap in this class.
    /// @return This wrapped resource, after assignment.
    template <typename ResourceType>
    DirectXGpuResource<ResourceType>& DirectXGpuResource<ResourceType>::operator=(ResourceType* resource)
    {
        Resource = resource;

        return *this;
    }

    /// Dereference operator to access the underlying resource.
    /// @return The underlying wrapped resource; a pointer is returned to meet common DirectX interfaces.
    template <typename ResourceType>
    ResourceType* DirectXGpuResource<ResourceType>::operator*() const
    {
        return Resource;
    }

    /// Indirection operator to access the underlying resource.
    /// @return The underlying wrapped resource; a pointer is returned to meet common DirectX interfaces.
    template <typename ResourceType>
    ResourceType* DirectXGpuResource<ResourceType>::operator->()
    {
        return Resource;
    }

    /// Releases the resource if it still exists.
    /// Allows for earlier releasing.
    template <typename ResourceType>
    void DirectXGpuResource<ResourceType>::Release()
    {
        if (Resource)
        {
            Resource->Release();
            Resource = nullptr;
        }
    }
}

