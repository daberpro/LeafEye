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
        m_profile = winrt::LeafEyeCore::ProfileModel(0,L"No Profile",L"Assets/default_avatar.png",3);
    }


    void UsersPage::OnNavigatedTo(Navigation::NavigationEventArgs const& e)
    {
        
        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
        RegisterPaginationCallback(
            UserFilterType::None,
            [this,database](int64_t offset, int64_t limit) -> winrt::fire_and_forget {

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto result = co_await database.GetAllUsers(offset, limit);
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
            [this,database]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                auto result = co_await database.GetUserCount();
                if (!result.IsError() && result.IsValueExists()) {
					co_return static_cast<uint64_t>(result.ResultValue().as<size_t>());
                }
                co_return 0;
            }
        );

        RegisterPaginationCallback(
            UserFilterType::Username,
            [this,database](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
                auto username = SearchBar().Text();
                winrt::LeafEyeCore::Result result;
                
                if (username.empty()) {
                    SetPaginationFilter(UserFilterType::None);
                    co_return;
                }

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                result = co_await database.GetUserContainsUsername(username, offset, limit);

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
				co_return static_cast<uint64_t>(m_users_list.Size());
            }
        );

        RegisterPaginationCallback(
            UserFilterType::AccessLevel,
            [this,database](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
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
                result = co_await database.GetUserByAccessLevel(is_admin, offset, limit);

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
            [this,database]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                co_return static_cast<uint64_t>(m_users_list.Size());
            }
        );

        SetPaginationFilter(UserFilterType::None);
    }

    void UsersPage::SearchBar_TextChanged(winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox const& sender, winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBoxTextChangedEventArgs const& args)
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
        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
        LeafEye::AddUserDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());

        dialog.PrimaryButtonClick([this, dialog, database](winrt::Microsoft::UI::Xaml::Controls::ContentDialog sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs args) -> winrt::fire_and_forget {
            auto deferral = args.GetDeferral();
            if (args.Cancel()) { deferral.Complete(); co_return; }

            // 1. Ambil data dari UI selagi masih di UI Thread
            hstring username = dialog.Username();
            hstring password = dialog.Password();
            bool isAdmin = dialog.IsAdmin();
            int32_t role = dialog.Role();

            // 2. Simpan konteks UI thread & pindah ke background thread
            winrt::apartment_context ui_thread;
            co_await winrt::resume_background();

            // --- SEKARANG DI BACKGROUND THREAD ---
            auto user = winrt::LeafEyeCore::UserModel(username, password, isAdmin);
            auto profile = winrt::LeafEyeCore::ProfileModel(0, L"User", L"ms-appx:///Assets/PlaceholderImage.png", role);

            auto result = co_await database.AddUser(user);
            auto result_create_profile = co_await database.AddUserProfile(profile);

            bool is_success = false;
            hstring error_message = L"";

            if (!result.IsError()) {
                if (!result_create_profile.IsError()) {
                    auto link_profile_to_user = co_await database.LinkUserProfile(result.ResultValue().as<obx_id>(), result_create_profile.ResultValue().as<obx_id>());
                    if (!link_profile_to_user.IsError()) {
                        user.Id(result.ResultValue().as<obx_id>());
                        is_success = true;
                    }
                    else {
                        error_message = link_profile_to_user.Message();
                    }
                }
                else {
                    error_message = result_create_profile.Message();
                }
            }
            else {
                error_message = result.Message();
            }

            // 3. Kembali ke UI thread untuk memanipulasi koleksi & dialog
            co_await ui_thread;

            // --- KEMBALI DI UI THREAD ---
            if (is_success) {
                m_users_list.Append(user);
            }
            else {
                args.Cancel(true); // Tahan dialog agar tidak tertutup jika ada error
                dialog.ShowMessage(error_message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            }

            deferral.Complete();
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
        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
        auto btn = sender.as<winrt::Microsoft::UI::Xaml::Controls::Button>();
        auto user_data = btn.Tag().as<winrt::LeafEyeCore::UserModel>();

        // 1. Ambil Data Profile di Background Thread
        winrt::apartment_context ui_thread_initial;
        co_await winrt::resume_background();

        auto profile_result = co_await database.GetUserProfileByLink(user_data.Id());

        co_await ui_thread_initial; // Kembali ke UI

        if (profile_result.IsError()) {
            winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
            errorDialog.Title(box_value(L"Error"));
            errorDialog.Content(box_value(profile_result.Message()));
            errorDialog.CloseButtonText(L"Ok");
            errorDialog.XamlRoot(this->XamlRoot());
            co_await errorDialog.ShowAsync();
            co_return;
        }

        auto profile = profile_result.ResultValue().as<winrt::LeafEyeCore::ProfileModel>();

        uint64_t id = user_data.Id();
        winrt::hstring username = user_data.Username();
        bool isAdmin = user_data.IsAdmin();
        int32_t role = profile.Role(); // Bisa digunakan jika updateDialog juga mengatur role

        winrt::LeafEye::UpdateUserDialog updateDialog;
        updateDialog.XamlRoot(this->XamlRoot());
        updateDialog.SetUserData(username, isAdmin, role);
        auto update_result = co_await updateDialog.ShowAsync();
        if (update_result == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {

            // Ambil data UI SEBELUM pindah ke background
            winrt::hstring new_username = updateDialog.Username();
            bool new_isAdmin = updateDialog.IsAdmin();
            winrt::hstring password = updateDialog.Password();
			int32_t role = updateDialog.Role();

            auto new_user_data = winrt::LeafEyeCore::UserModel(new_username, password, new_isAdmin);
            new_user_data.Id(id);

            profile.Role(role);

            // 2. Simpan konteks UI & pindah ke Background untuk KEDUA proses update
            winrt::apartment_context ui_thread_update;
            co_await winrt::resume_background();

            bool is_all_success = false;
            winrt::hstring final_message = L"";

            // Eksekusi Update User pertama
            auto result = co_await database.UpdateUser(new_user_data);

            if (!result.IsError()) {
                // HANYA eksekusi Update Profile jika Update User berhasil
                auto update_profile_result = co_await database.UpdateUserProfile(profile);

                if (!update_profile_result.IsError()) {
                    is_all_success = true;
                    final_message = result.Message(); // Atau pesan kustom: L"User dan Profile berhasil diperbarui."
                }
                else {
                    final_message = update_profile_result.Message();
                }
            }
            else {
                final_message = result.Message();
            }

            // 3. Kembali ke UI Thread HANYA SATU KALI setelah semua operasi DB selesai
            co_await ui_thread_update;

			m_profile.Role(role); // Update role di profile yang di-bind ke dialog update (jika dialog menampilkan role, ini akan otomatis memperbarui tampilannya)

            // Eksekusi pembaruan UI dan tampilkan pop-up berdasarkan hasil akhir
            if (is_all_success) {
                user_data.Username(new_username);
                user_data.IsAdmin(new_isAdmin);

                winrt::Microsoft::UI::Xaml::Controls::ContentDialog successDialog{};
                successDialog.Title(box_value(L"Info"));
                successDialog.Content(box_value(final_message));
                successDialog.CloseButtonText(L"Ok");
                successDialog.XamlRoot(this->XamlRoot());
                co_await successDialog.ShowAsync();
            }
            else {
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
                errorDialog.Title(box_value(L"Error"));
                errorDialog.Content(box_value(final_message));
                errorDialog.CloseButtonText(L"Ok");
                errorDialog.XamlRoot(this->XamlRoot());
                co_await errorDialog.ShowAsync();
            }
        }
    }

    winrt::fire_and_forget UsersPage::DeleteUserBtn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
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

            auto result = co_await database.DeleteUser(user_data.Id());

            co_await ui_thread;

            if (!result.IsError()) {

                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto result_delete_profile = co_await database.DeleteUserProfile(user_data.Id());
                co_await ui_thread;

                if (!result_delete_profile.IsError()) {
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
                    errorDialog.Content(box_value(result_delete_profile.Message()));
                    errorDialog.CloseButtonText(L"Ok");
                    errorDialog.XamlRoot(this->XamlRoot());
                    co_await errorDialog.ShowAsync();
                }
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
    
    winrt::fire_and_forget UsersPage::UserListView_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e)
    {
        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
		auto data = e.ClickedItem().as<winrt::LeafEyeCore::UserModel>();
		winrt::apartment_context ui_thread;
        auto id = data.Id();
		co_await winrt::resume_background();
        auto profile = co_await database.GetUserProfileByLink(id);
		co_await ui_thread;

        if (!profile.IsError() && profile.IsValueExists()) {
		    auto profile_data = profile.ResultValue().as<winrt::LeafEyeCore::ProfileModel>();
			m_profile.Id(profile_data.Id());
			m_profile.Fullname(profile_data.Fullname());
			m_profile.AvatarPath(profile_data.AvatarPath());
            m_profile.Role(profile_data.Role());
        }
        else {
            winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
            errorDialog.Title(box_value(L"Error"));
            errorDialog.Content(box_value(profile.Message()));
            errorDialog.CloseButtonText(L"Ok");
            errorDialog.XamlRoot(this->XamlRoot());
            co_await errorDialog.ShowAsync();
        }
        co_return;
    }

    winrt::LeafEyeCore::ProfileModel UsersPage::Profile() {
        return m_profile;
    };

    void UsersPage::UserRefreshContainer_RefreshRequested(winrt::Microsoft::UI::Xaml::Controls::RefreshContainer const& sender, winrt::Microsoft::UI::Xaml::Controls::RefreshRequestedEventArgs const& args)
    {
        PaginationSetUp();
    }

}

