#pragma once

#include <vector>
#include "Containers/Container.h"

/// A namespace for testing the Container class.
namespace CONTAINER_TESTS
{
    TEST_CASE("An item in a container can be detected.", "[Container]")
    {
        // CHECK THAT AT ITEM CAN BE FOUND IN A CONTAINER.
        const std::vector<int> CONTAINER = { 1, 2, 3, 4 };
        bool item_found = CONTAINERS::Container::Contains(CONTAINER, 3);
        REQUIRE(item_found);
    }

    TEST_CASE("An item not in a container cannot be detected.", "[Container]")
    {
        // CHECK THAT AT ITEM CAN NOT BE FOUND IN A CONTAINER.
        const std::vector<int> CONTAINER = { 1, 2, 3, 4 };
        bool item_found = CONTAINERS::Container::Contains(CONTAINER, 5);
        REQUIRE(!item_found);
    }
}
