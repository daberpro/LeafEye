#pragma once

#include "UsersPage.g.h"
#include <winrt/LeafEyeCore.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::LeafEyeTest::implementation
{
    struct UsersPage : UsersPageT<UsersPage>
    {
        UsersPage();

        // ─── Format Helpers untuk XAML x:Bind ────────────────────────────────
        static winrt::hstring FormatId(uint64_t id);
        static winrt::hstring FormatAdmin(bool isAdmin);

        // ─── Lifecycle & Navigation ──────────────────────────────────────────
        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        winrt::Windows::Foundation::IAsyncAction LoadDataAsync();

        // ─── UI Event Handlers ───────────────────────────────────────────────
        winrt::fire_and_forget RefreshButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget AddUserButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        // ─── Row Actions ─────────────────────────────────────────────────────
        winrt::fire_and_forget EditUser_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget DeleteUser_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        // ─── Pagination Handlers ─────────────────────────────────────────────
        winrt::fire_and_forget PrevPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::fire_and_forget NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        winrt::LeafEyeCore::Database m_db{ nullptr };

        // This acts as our local UI cache!
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::LeafEyeCore::UserModel> m_usersList{ nullptr };

        // Pagination state
        int32_t m_offset{ 0 };
        int32_t m_limit{ 10 };
        int32_t m_currentPage{ 1 };

        void UpdatePaginationUI();
    };
}

namespace winrt::LeafEyeTest::factory_implementation
{
    struct UsersPage : UsersPageT<UsersPage, implementation::UsersPage>
    {
    };
}