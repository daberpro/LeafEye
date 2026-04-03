#pragma once

#include "HistoryPage.g.h"


namespace winrt::LeafEye::implementation
{
    struct HistoryPage : HistoryPageT<HistoryPage>
    {
        HistoryPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct HistoryPage : HistoryPageT<HistoryPage, implementation::HistoryPage>
    {
    };
}
