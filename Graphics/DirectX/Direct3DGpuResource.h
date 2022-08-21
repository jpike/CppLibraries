#pragma once

namespace GRAPHICS::DIRECT_X
{
    /// A GPU resource allocated with Direct 3D.
    /// This class helps automate releasing of resources.
    template <typename ResourceType>
    class Direct3DGpuResource
    {
    public:
        // CONSTRUCTION/DESTRUCTION.
        ~Direct3DGpuResource();

        // OPERATORS.
        Direct3DGpuResource& operator=(ResourceType* resource);
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
    Direct3DGpuResource<ResourceType>::~Direct3DGpuResource()
    {
        Release();
    }

    /// Assignment operator to allow easy assignment of raw resources to this pointer.
    /// Note that if this wrapper is already holding a resource, no releasing is one.
    /// @param[in]  resource - The resource to wrap in this class.
    /// @return This wrapped resource, after assignment.
    template <typename ResourceType>
    Direct3DGpuResource<ResourceType>& Direct3DGpuResource<ResourceType>::operator=(ResourceType* resource)
    {
        Resource = resource;

        return *this;
    }

    /// Dereference operator to access the underlying resource.
    /// @return The underlying wrapped resource; a pointer is returned to meet common DirectX interfaces.
    template <typename ResourceType>
    ResourceType* Direct3DGpuResource<ResourceType>::operator*() const
    {
        return Resource;
    }

    /// Indirection operator to access the underlying resource.
    /// @return The underlying wrapped resource; a pointer is returned to meet common DirectX interfaces.
    template <typename ResourceType>
    ResourceType* Direct3DGpuResource<ResourceType>::operator->()
    {
        return Resource;
    }

    /// Releases the resource if it still exists.
    /// Allows for earlier releasing.
    template <typename ResourceType>
    void Direct3DGpuResource<ResourceType>::Release()
    {
        if (Resource)
        {
            Resource->Release();
            Resource = nullptr;
        }
    }
}

