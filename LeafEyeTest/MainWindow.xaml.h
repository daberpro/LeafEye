#pragma once
#include "MainWindow.g.h"
#include <winrt/LeafEyeCore.h>

namespace winrt::LeafEyeTest::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        // Database Instance
        winrt::LeafEyeCore::Database m_db{ nullptr };

        // Current Logged-In User
        winrt::LeafEyeCore::UserModel m_currentUser{ nullptr };

        // ─── Setup & Login Handlers ──────────────────────────────────────────
        winrt::fire_and_forget InitDb_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget SeedDb_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget LoginButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        // ─── Navigation Handlers ─────────────────────────────────────────────
        void MainNav_SelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
        void Logout_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& args);
    };
}

namespace winrt::LeafEyeTest::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}