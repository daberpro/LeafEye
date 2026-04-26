#pragma once
#include "UserRoleHelper.g.h"

namespace winrt::LeafEyeCore::implementation
{
    struct UserRoleHelper : UserRoleHelperT<UserRoleHelper>
    {
        UserRoleHelper() = default;

        static hstring ConvertUserRoleToString(winrt::LeafEyeCore::UserRole const& role);
        static hstring ConvertUserIntRoleToString(int32_t role);
    };
}

namespace winrt::LeafEyeCore::factory_implementation
{
    struct UserRoleHelper : UserRoleHelperT<UserRoleHelper, implementation::UserRoleHelper>
    {
    };
}