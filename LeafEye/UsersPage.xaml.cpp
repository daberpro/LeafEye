#include "pch.h"
#include "UsersPage.xaml.h"
#if __has_include("UsersPage.g.cpp")
#include "UsersPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::LeafEye::implementation
{
    hstring UsersPage::ConvertToUserStatus(bool is_admin) {
        return is_admin ? L"Admin" : L"User";
    }

    UsersPage::UsersPage()
    {
        InitializeComponent();
        m_users_list = winrt::single_threaded_observable_vector<winrt::LeafEyeCore::UserModel>();
        UserListView().ItemsSource(m_users_list);
    }


    void UsersPage::OnNavigatedTo(Navigation::NavigationEventArgs const& e)
    {
        if (e.Parameter())
        {
            m_db = e.Parameter().as<winrt::LeafEyeCore::Database>();
        }
        
        
        RegisterPaginationCallback(
            UserFilterType::None,
            [this](int64_t offset, int64_t limit) -> winrt::fire_and_forget {

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto result = co_await m_db.GetAllUsers(offset, limit);
                co_await ui_thread;

                if (!result.IsError() && result.IsValueExists()) {
                    auto users = result.ResultValue().as<
                        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel>
                    >();

                    m_users_list.Clear();
                    for (auto user : users) {
                        m_users_list.Append(user);
                    }

                }

            },
            [this]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                auto result = co_await m_db.GetUserCount();
                if (!result.IsError() && result.IsValueExists()) {
                    co_return result.ResultValue().as<uint64_t>();
                }
                co_return 0;
            }
        );

        RegisterPaginationCallback(
            UserFilterType::Username,
            [this](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
                auto username = SearchBar().Text();
                winrt::LeafEyeCore::Result result;
                
                if (username.empty()) {
                    SetPaginationFilter(UserFilterType::None);
                    co_return;
                }

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                result = co_await m_db.GetUserContainsUsername(username, offset, limit);

                co_await ui_thread;

                if (!result.IsError() && result.IsValueExists()) {
                    auto users = result.ResultValue().as<
                        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel>
                    >();

                    m_users_list.Clear();
                    for (auto user : users) {
                        m_users_list.Append(user);
                    }

                }

            },
            [this]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                co_return m_users_list.Size();
            }
        );

        RegisterPaginationCallback(
            UserFilterType::AccessLevel,
            [this](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
                auto item = AccessLevel().SelectedItem().as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
                auto access_level = item.Content().as<winrt::hstring>();
                winrt::LeafEyeCore::Result result;

                if (access_level == L"Semua Level") {
                    SetPaginationFilter(UserFilterType::None);
                    co_return;
                }

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto is_admin = access_level == L"Admin";
                result = co_await m_db.GetUserByAccessLevel(is_admin, offset, limit);

                co_await ui_thread;

                if (!result.IsError() && result.IsValueExists()) {
                    auto users = result.ResultValue().as<
                        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel>
                    >();

                    m_users_list.Clear();
                    for (auto user : users) {
                        m_users_list.Append(user);
                    }

                }

            },
            [this]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                co_return m_users_list.Size();
            }
        );

        SetPaginationFilter(UserFilterType::None);
    }

    void UsersPage::SearchBar_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
    {
        SetPaginationFilter(UserFilterType::Username);
    }

    void UsersPage::AccessLevel_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
    {
        SetPaginationFilter(UserFilterType::AccessLevel);
    }

    void UsersPage::PageSize_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
    {
        auto item = PageSize().SelectedItem().as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
        auto content = winrt::unbox_value<winrt::hstring>(item.Content());
        int size = std::stoi(winrt::to_string(content));
        m_pagination_limit = size;
        PaginationSetUp(); // melakukan setup ulang agar mengambil data kembali dengan limit yang berbeda
    }

    void UsersPage::PrevPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        PrevPage();
    }

    void UsersPage::NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        NextPage();
    }

    winrt::fire_and_forget UsersPage::AddUserButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LeafEye::AddUserDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());

        dialog.PrimaryButtonClick([this, dialog](winrt::Microsoft::UI::Xaml::Controls::ContentDialog sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs args) -> winrt::fire_and_forget {
            auto defferal = args.GetDeferral();
            if (args.Cancel()) { defferal.Complete(); co_return; }

            hstring username = dialog.Username();
            hstring password = dialog.Password();
            bool isAdmin = dialog.IsAdmin();

            auto user = winrt::LeafEyeCore::UserModel(username, password, isAdmin);
            auto result = co_await m_db.AddUser(user);

            if (!result.IsError()) {
                m_users_list.Append(user);
            }
            else {
                args.Cancel(true);
                dialog.ShowMessage(result.Message(), winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }
            defferal.Complete();
            });

        auto dialog_result = co_await dialog.ShowAsync();

        if (dialog_result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {
            winrt::Microsoft::UI::Xaml::Controls::ContentDialog successDialog{};
            successDialog.Title(box_value(L"Info"));
            successDialog.Content(box_value(L"User berhasil ditambahkan."));
            successDialog.CloseButtonText(L"Ok");
            successDialog.XamlRoot(this->XamlRoot());
            co_await successDialog.ShowAsync();
        }
    }

    winrt::fire_and_forget UsersPage::UpdateUserBtn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto btn = sender.as<winrt::Microsoft::UI::Xaml::Controls::Button>();
        auto user_data = btn.Tag().as<winrt::LeafEyeCore::UserModel>();

        uint64_t id = user_data.Id();
        winrt::hstring username = user_data.Username();
        bool isAdmin = user_data.IsAdmin();

        winrt::LeafEye::UpdateUserDialog updateDialog;
        updateDialog.XamlRoot(this->XamlRoot());
        updateDialog.SetUserData(username, isAdmin);

        auto update_result = co_await updateDialog.ShowAsync();
        if (update_result == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {

            winrt::apartment_context ui_thread;
            co_await winrt::resume_background();

            auto new_user_data = winrt::LeafEyeCore::UserModel(
                updateDialog.Username(), L"", updateDialog.IsAdmin()
            );
            new_user_data.Id(id);

            auto result = co_await m_db.UpdateUser(new_user_data);

            co_await ui_thread;

            if (!result.IsError()) {
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog successDialog{};
                successDialog.Title(box_value(L"Info"));
                successDialog.Content(box_value(result.Message()));
                successDialog.CloseButtonText(L"Ok");
                successDialog.XamlRoot(this->XamlRoot());
                co_await successDialog.ShowAsync();

                user_data.Username(updateDialog.Username());
                user_data.IsAdmin(updateDialog.IsAdmin());
            }
            else {
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
                errorDialog.Title(box_value(L"Error"));
                errorDialog.Content(box_value(result.Message()));
                errorDialog.CloseButtonText(L"Ok");
                errorDialog.XamlRoot(this->XamlRoot());
                co_await errorDialog.ShowAsync();
            }
        }
    }

    winrt::fire_and_forget UsersPage::DeleteUserBtn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto btn = sender.as<winrt::Microsoft::UI::Xaml::Controls::Button>();
        auto user_data = btn.Tag().as<winrt::LeafEyeCore::UserModel>();

        winrt::Microsoft::UI::Xaml::Controls::ContentDialog deleteDialog{};
        deleteDialog.Title(box_value(L"Info"));
        deleteDialog.Content(box_value(std::format(L"Apakah anda yakin akan menghapus user \"{}\"", user_data.Username())));
        deleteDialog.CloseButtonText(L"Batalkan");
        deleteDialog.PrimaryButtonText(L"Hapus");
        deleteDialog.XamlRoot(this->XamlRoot());
        auto is_accept = co_await deleteDialog.ShowAsync();

        if (is_accept == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {
            winrt::apartment_context ui_thread;
            co_await winrt::resume_background();

            auto result = co_await m_db.DeleteUser(user_data.Id());

            co_await ui_thread;

            if (!result.IsError()) {
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog successDialog{};
                successDialog.Title(box_value(L"Info"));
                successDialog.Content(box_value(result.Message()));
                successDialog.CloseButtonText(L"Ok");
                successDialog.XamlRoot(this->XamlRoot());
                co_await successDialog.ShowAsync();

                uint32_t index_of_data;
                m_users_list.IndexOf(user_data, index_of_data);
                m_users_list.RemoveAt(index_of_data);
            }
            else {
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
                errorDialog.Title(box_value(L"Error"));
                errorDialog.Content(box_value(result.Message()));
                errorDialog.CloseButtonText(L"Ok");
                errorDialog.XamlRoot(this->XamlRoot());
                co_await errorDialog.ShowAsync();
            }
        }
    }

    // ================================= Pagination =========================================

    void UsersPage::UpdatePaginationInfo() {
        RaisedPropertyChanged(L"GetPaginationInfo");
    }

    winrt::hstring UsersPage::GetPaginationInfo() {
        
        return winrt::to_hstring(
            std::format(
            "Halaman ke {} dari {}",
            m_pagination_current_page + 1,
            m_pagination_total_page
        ));
    
    };

    void UsersPage::TotalPage(int64_t value)
    {
        if (m_pagination_total_page != value) {
            m_pagination_total_page = value;
            RaisedPropertyChanged(L"TotalPage");
            RaisedPropertyChanged(L"GetPaginationInfo");

        }
    }

    int64_t UsersPage::TotalPage()
    {
        return m_pagination_total_page;
    }

    int64_t UsersPage::CurrentPage()
    {
        return m_pagination_current_page;
    }

    void UsersPage::CurrentPage(int64_t value)
    {
        if (m_pagination_current_page != value) {
            m_pagination_current_page = value;
            RaisedPropertyChanged(L"CurrentPage");
        }
    }

    void UsersPage::PrevPageEnabled(bool value)
    {
        if (m_prevPageEnabled != value) {
            m_prevPageEnabled = value;
            RaisedPropertyChanged(L"PrevPageEnabled");
        }
    }

    bool UsersPage::PrevPageEnabled()
    {
        return m_prevPageEnabled;
    }

    void UsersPage::NextPageEnabled(bool value)
    {
        if (m_nextPageEnabled != value) {
            m_nextPageEnabled = value;
            RaisedPropertyChanged(L"NextPageEnabled");
            RaisedPropertyChanged(L"GetPaginationInfo");
        }
    }

    bool UsersPage::NextPageEnabled()
    {
        return m_nextPageEnabled;
    }

    winrt::event_token UsersPage::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void UsersPage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void UsersPage::RaisedPropertyChanged(const winrt::hstring& property_name) {
        if (m_propertyChanged)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
        }
    }

    // ================================= Pagination =========================================

}