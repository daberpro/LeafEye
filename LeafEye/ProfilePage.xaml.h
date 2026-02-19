#pragma once

#include "ProfilePage.g.h"

namespace winrt::LeafEye::implementation
{
    struct ProfilePage : ProfilePageT<ProfilePage>
    {
        ProfilePage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct ProfilePage : ProfilePageT<ProfilePage, implementation::ProfilePage>
    {
    };
}
