#pragma once

#include "App.xaml.g.h"

namespace winrt::LeafEye::implementation
{
    struct App : AppT<App>
    {
        App();

        static winrt::Microsoft::UI::Xaml::Window Window() { return window; }
        static void ClearWindow() { window = nullptr; }
        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        static winrt::Microsoft::UI::Xaml::Window window;
    };
}
