#include "pch.h"
#include "ProfilesPage.xaml.h"
#if __has_include("ProfilesPage.g.cpp")
#include "ProfilesPage.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <string>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace winrt::LeafEyeCore;

namespace winrt::LeafEyeTest::implementation
{
    ProfilesPage::ProfilesPage()
    {
        InitializeComponent();
    }

    void ProfilesPage::OnNavigatedTo(NavigationEventArgs const& e)
    {
        if (e.Parameter())
        {
            // Retrieve the database instance passed from MainWindow
            m_db = unbox_value<winrt::LeafEyeCore::Database>(e.Parameter());
            LoadDataAsync();
        }
    }

    winrt::Windows::Foundation::IAsyncAction ProfilesPage::LoadDataAsync()
    {
        if (!m_db) co_return;

        // Note: Because Database.h lacks a GetAllProfiles method, we are paginating through
        // Users and fetching their linked profiles. 
        auto usersResult = co_await m_db.GetAllUsers(m_offset, m_limit);
        auto profilesList = winrt::single_threaded_vector<winrt::LeafEyeCore::ProfileModel>();

        if (usersResult.IsValueExists())
        {
            auto users = usersResult.ResultValue().as<winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel>>();

            for (auto const& user : users)
            {
                auto profileResult = co_await m_db.GetUserProfileByLink(user.Id());
                if (profileResult.IsValueExists())
                {
                    profilesList.Append(profileResult.ResultValue().as<winrt::LeafEyeCore::ProfileModel>());
                }
            }

            // Bind data to the ListView
            ProfilesListView().ItemsSource(profilesList);

            // Update pagination button states
            PrevPageButton().IsEnabled(m_currentPage > 1);
            NextPageButton().IsEnabled(users.Size() == m_limit);
        }

        UpdatePaginationUI();
    }

    void ProfilesPage::UpdatePaginationUI()
    {
        PageInfoText().Text(L"Page " + winrt::to_hstring(m_currentPage));
    }

    winrt::fire_and_forget ProfilesPage::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        m_currentPage = 1;
        m_offset = 0;
        co_await LoadDataAsync();
    }

    winrt::fire_and_forget ProfilesPage::PrevPageButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_currentPage > 1)
        {
            m_currentPage--;
            m_offset = (m_currentPage - 1) * m_limit;
            co_await LoadDataAsync();
        }
    }

    winrt::fire_and_forget ProfilesPage::NextPageButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        m_currentPage++;
        m_offset = (m_currentPage - 1) * m_limit;
        co_await LoadDataAsync();
    }

    winrt::fire_and_forget ProfilesPage::AddProfileButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Add New Profile"));
        dialog.PrimaryButtonText(L"Save");
        dialog.CloseButtonText(L"Cancel");

        StackPanel panel;
        panel.Spacing(10);

        TextBox nameBox;
        nameBox.Header(winrt::box_value(L"Full Name"));

        TextBox avatarBox;
        avatarBox.Header(winrt::box_value(L"Avatar Path"));
        avatarBox.PlaceholderText(L"/assets/default.png");

        TextBox roleBox;
        roleBox.Header(winrt::box_value(L"Role (Integer)"));
        roleBox.Text(L"1");

        panel.Children().Append(nameBox);
        panel.Children().Append(avatarBox);
        panel.Children().Append(roleBox);
        dialog.Content(panel);

        auto result = co_await dialog.ShowAsync();
        if (result == ContentDialogResult::Primary)
        {
            try {
                winrt::LeafEyeCore::ProfileModel newProfile;
                newProfile.Fullname(nameBox.Text());
                newProfile.AvatarPath(avatarBox.Text());
                newProfile.Role(std::stoi(winrt::to_string(roleBox.Text())));

                co_await m_db.AddUserProfile(newProfile);
                co_await LoadDataAsync(); // Refresh table
            }
            catch (...) {
                // Ignore parsing errors for Role in this simple test UI
            }
        }
    }

    winrt::fire_and_forget ProfilesPage::EditProfile_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto button = sender.as<Button>();
        uint64_t id = unbox_value<uint64_t>(button.Tag());

        // We can't fetch a profile strictly by ID with the current API unless it's linked to a user, 
        // but for the sake of the UI update, we will use the ListView's DataContext or just pass the whole model.
        // A better long-term fix is adding GetProfileById to your Database.cpp. 
        // For now, let's just refresh the data if we had an edit. (Skipping full pre-fill without GetProfileById).

        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Edit Profile"));
        dialog.Content(winrt::box_value(L"To edit the profile details correctly, please ensure GetProfileById is added to the Database API. \n\nProceeding will execute a dummy update."));
        dialog.PrimaryButtonText(L"Understood");
        dialog.CloseButtonText(L"Cancel");

        auto dialogResult = co_await dialog.ShowAsync();
        if (dialogResult == ContentDialogResult::Primary)
        {
            // Placeholder for real update logic
            co_await LoadDataAsync();
        }
    }

    winrt::fire_and_forget ProfilesPage::DeleteProfile_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto button = sender.as<Button>();
        uint64_t id = unbox_value<uint64_t>(button.Tag());

        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Confirm Delete"));
        dialog.Content(winrt::box_value(L"Are you sure you want to delete this profile?"));
        dialog.PrimaryButtonText(L"Delete");
        dialog.CloseButtonText(L"Cancel");

        auto dialogResult = co_await dialog.ShowAsync();
        if (dialogResult == ContentDialogResult::Primary)
        {
            co_await m_db.DeleteUserProfile(id);
            co_await LoadDataAsync(); // Refresh table
        }
    }
}