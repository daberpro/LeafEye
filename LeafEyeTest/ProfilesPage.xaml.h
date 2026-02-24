#pragma once

#include "ProfilesPage.g.h"
#include <winrt/LeafEyeCore.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::LeafEyeTest::implementation
{
    struct ProfilesPage : ProfilesPageT<ProfilesPage>
    {
        ProfilesPage();

        // ─── Lifecycle & Navigation ──────────────────────────────────────────
        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction LoadDataAsync();

        // ─── UI Event Handlers ───────────────────────────────────────────────
        winrt::fire_and_forget RefreshButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget AddProfileButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        // ─── Row Actions ─────────────────────────────────────────────────────
        winrt::fire_and_forget EditProfile_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget DeleteProfile_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        // ─── Pagination Handlers ─────────────────────────────────────────────
        winrt::fire_and_forget PrevPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        winrt::LeafEyeCore::Database m_db{ nullptr };

        // Pagination state
        int32_t m_offset{ 0 };
        int32_t m_limit{ 10 }; // Items per page
        int32_t m_currentPage{ 1 };

        void UpdatePaginationUI();
    };
}

namespace winrt::LeafEyeTest::factory_implementation
{
    struct ProfilesPage : ProfilesPageT<ProfilesPage, implementation::ProfilesPage>
    {
    };
}