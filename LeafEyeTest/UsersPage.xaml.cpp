#include "pch.h"
#include "UsersPage.xaml.h"
#if __has_include("UsersPage.g.cpp")
#include "UsersPage.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Dispatching.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace winrt::LeafEyeCore;

namespace winrt::LeafEyeTest::implementation
{
    UsersPage::UsersPage()
    {
        InitializeComponent();
        // Initialize our observable vector once
        m_usersList = winrt::single_threaded_observable_vector<winrt::LeafEyeCore::UserModel>();
    }

    winrt::hstring UsersPage::FormatId(uint64_t id)
    {
        return winrt::to_hstring(id);
    }

    winrt::hstring UsersPage::FormatAdmin(bool isAdmin)
    {
        return isAdmin ? L"Admin" : L"User";
    }

    void UsersPage::OnNavigatedTo(NavigationEventArgs const& e)
    {
        if (e.Parameter())
        {
            m_db = unbox_value<winrt::LeafEyeCore::Database>(e.Parameter());
            UsersListView().ItemsSource(m_usersList);
            auto _ = LoadDataAsync();
        }
    }

    winrt::Windows::Foundation::IAsyncAction UsersPage::LoadDataAsync()
    {
        if (!m_db) co_return;

        auto dispatcher = this->DispatcherQueue();

        try
        {
            auto result = co_await m_db.GetAllUsers(m_offset, m_limit);

            // Notice: Normal lambda returning void. Safe!
            dispatcher.TryEnqueue([this, result]()
                {
                    try
                    {
                        if (result.IsValueExists() && !result.IsError())
                        {
                            auto users = result.ResultValue().as<winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel>>();

                            m_usersList.Clear();
                            for (auto const& user : users)
                            {
                                m_usersList.Append(user);
                            }

                            PrevPageButton().IsEnabled(m_currentPage > 1);
                            NextPageButton().IsEnabled(users.Size() == m_limit);
                        }
                        PageInfoText().Text(L"Page " + winrt::to_hstring(m_currentPage));
                    }
                    catch (...) {}
                });
        }
        catch (...) {}
    }

    winrt::fire_and_forget UsersPage::RefreshButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try {
            m_currentPage = 1;
            m_offset = 0;
            co_await LoadDataAsync();
        }
        catch (...) {}
    }

    winrt::fire_and_forget UsersPage::PrevPageButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try {
            if (m_currentPage > 1) {
                m_currentPage--;
                m_offset = (m_currentPage - 1) * m_limit;
                co_await LoadDataAsync();
            }
        }
        catch (...) {}
    }

    winrt::fire_and_forget UsersPage::NextPageButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        try {
            m_currentPage++;
            m_offset = (m_currentPage - 1) * m_limit;
            co_await LoadDataAsync();
        }
        catch (...) {}
    }

    winrt::fire_and_forget UsersPage::AddUserButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Add New User"));
        dialog.PrimaryButtonText(L"Save");
        dialog.CloseButtonText(L"Cancel");

        StackPanel panel;
        panel.Spacing(10);

        TextBox userBox;
        userBox.Header(winrt::box_value(L"Username"));

        PasswordBox passBox;
        passBox.Header(winrt::box_value(L"Password"));

        CheckBox adminCheck;
        adminCheck.Content(winrt::box_value(L"Is Administrator"));

        panel.Children().Append(userBox);
        panel.Children().Append(passBox);
        panel.Children().Append(adminCheck);
        dialog.Content(panel);

        auto dialogResult = co_await dialog.ShowAsync();
        if (dialogResult == ContentDialogResult::Primary)
        {
            auto dispatcher = this->DispatcherQueue();
            winrt::LeafEyeCore::UserModel newUser;
            newUser.Username(userBox.Text());
            newUser.Password(passBox.Password());
            newUser.IsAdmin(adminCheck.IsChecked().GetBoolean());

            try {
                co_await m_db.AddUser(newUser);
                auto fetchResult = co_await m_db.GetUserByUsername(newUser.Username());

                dispatcher.TryEnqueue([this, fetchResult]() {
                    if (fetchResult.IsValueExists()) {
                        auto insertedUser = fetchResult.ResultValue().as<winrt::LeafEyeCore::UserModel>();
                        m_usersList.Append(insertedUser);
                    }
                    });
            }
            catch (...) {}
        }
    }

    // --- COMPLETELY REWRITTEN ---
    winrt::fire_and_forget UsersPage::EditUser_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto button = sender.as<Button>();
        uint64_t id = unbox_value<uint64_t>(button.Tag());

        // 1. Fetch user locally instead of querying DB. We stay safely on the UI Thread!
        winrt::LeafEyeCore::UserModel userToEdit{ nullptr };
        uint32_t targetIndex = 0;
        for (uint32_t i = 0; i < m_usersList.Size(); i++) {
            if (m_usersList.GetAt(i).Id() == id) {
                userToEdit = m_usersList.GetAt(i);
                targetIndex = i;
                break;
            }
        }

        if (!userToEdit) co_return;

        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Edit User"));
        dialog.PrimaryButtonText(L"Update");
        dialog.CloseButtonText(L"Cancel");

        StackPanel panel;
        panel.Spacing(10);

        TextBox userBox;
        userBox.Header(winrt::box_value(L"Username (Cannot be changed directly)"));
        userBox.Text(userToEdit.Username());
        userBox.IsEnabled(false);

        PasswordBox passBox;
        passBox.Header(winrt::box_value(L"New Password"));

        CheckBox adminCheck;
        adminCheck.Content(winrt::box_value(L"Is Administrator"));
        adminCheck.IsChecked(userToEdit.IsAdmin());

        panel.Children().Append(userBox);
        panel.Children().Append(passBox);
        panel.Children().Append(adminCheck);
        dialog.Content(panel);

        auto dialogResult = co_await dialog.ShowAsync();
        if (dialogResult == ContentDialogResult::Primary)
        {
            auto dispatcher = this->DispatcherQueue();
            try {
                // Update properties
                userToEdit.Password(passBox.Password());
                userToEdit.IsAdmin(adminCheck.IsChecked().GetBoolean());

                // 2. Call DB. (Execution jumps to background thread safely here)
                co_await m_db.UpdateUser(userToEdit);

                // 3. Normal void lambda to safely return to UI thread
                dispatcher.TryEnqueue([this, targetIndex, userToEdit]() {
                    m_usersList.RemoveAt(targetIndex);
                    m_usersList.InsertAt(targetIndex, userToEdit);
                    });
            }
            catch (...) {}
        }
    }

    winrt::fire_and_forget UsersPage::DeleteUser_Click(IInspectable const& sender, RoutedEventArgs const&)
    {
        auto button = sender.as<Button>();
        uint64_t id = unbox_value<uint64_t>(button.Tag());

        ContentDialog dialog;
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.Title(winrt::box_value(L"Confirm Delete"));
        dialog.Content(winrt::box_value(L"Are you sure you want to delete this user? This action cannot be undone."));
        dialog.PrimaryButtonText(L"Delete");
        dialog.CloseButtonText(L"Cancel");

        auto dialogResult = co_await dialog.ShowAsync();
        if (dialogResult == ContentDialogResult::Primary)
        {
            auto dispatcher = this->DispatcherQueue();
            try {
                co_await m_db.DeleteUser(id);

                dispatcher.TryEnqueue([this, id]() {
                    for (uint32_t i = 0; i < m_usersList.Size(); i++) {
                        if (m_usersList.GetAt(i).Id() == id) {
                            m_usersList.RemoveAt(i);
                            break;
                        }
                    }
                    });
            }
            catch (...) {}
        }
    }
}