#pragma once
#include "UsersPage.g.h"
#include "AddUserDialog.xaml.h"
#include "UpdateUserDialog.xaml.h"
#include "Base/PaginationBaseExperiment.h" // Include base class pagination kita
//#include "Base/PaginationBase.h" // Include base class pagination kita

namespace winrt::LeafEye::implementation
{
    enum class UserFilterType
    {
        None,
        Username,
        AccessLevel
    };

    struct UsersPage : UsersPageT<UsersPage>, public winrt::LeafEye::Utils::PaginationBase<UserFilterType>
    {
    private:
        winrt::LeafEyeCore::Database m_db{ nullptr };

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::LeafEyeCore::UserModel> m_users_list;

        winrt::hstring m_searchQuery{ L"" };
        bool m_accessLevelFilter{ false };

        void RaisedPropertyChanged(const winrt::hstring&);

        // ================================= Pagination =========================================
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        bool m_prevPageEnabled{ false };
        bool m_nextPageEnabled{ false };
        // ================================= Pagination =========================================

    public:

        // ================================= Pagination =========================================
        void PrevPageEnabled(bool value) override;
        void NextPageEnabled(bool value) override;
        void UpdatePaginationInfo() override;
        hstring GetPaginationInfo();
        void TotalPage(int64_t value);
        int64_t TotalPage();
        int64_t CurrentPage();
        void CurrentPage(int64_t value);
        bool PrevPageEnabled();
        bool NextPageEnabled();
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
        // ================================= Pagination =========================================

        static winrt::hstring ConvertToUserStatus(bool is_admin);

        UsersPage();

        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

        // Event Handlers UI yang sudah ada
        void SearchBar_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        winrt::fire_and_forget AddUserButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget UpdateUserBtn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget DeleteUserBtn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void AccessLevel_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void PageSize_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);

        // Event Handlers Pagination (Baru - untuk disambungkan ke XAML nanti)
        void PrevPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct UsersPage : UsersPageT<UsersPage, implementation::UsersPage>
    {
    };
}