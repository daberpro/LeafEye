#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Windows.Storage.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>

// Forward declarations for the pages we will create next
// #include "UsersPage.xaml.h"
// #include "ProfilesPage.xaml.h"
// #include "HistoryPage.xaml.h"
// #include "FileHistoryPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::LeafEyeCore;

namespace winrt::LeafEyeTest::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    winrt::fire_and_forget MainWindow::InitDb_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto folder = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
        winrt::hstring dbPath = folder.Path() + L"\\LeafEyeTestDB";

        m_db = winrt::LeafEyeCore::Database(dbPath, 1024 * 1024);

        auto result = co_await m_db.InitializeAsync();

        if (result.IsError()) {
            LoginErrorText().Text(L"DB Error: " + result.Message());
        }
        else {
            LoginErrorText().Text(L"Database initialized successfully. Ready to login.");
        }
    }

    winrt::fire_and_forget MainWindow::SeedDb_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!m_db) {
            LoginErrorText().Text(L"Please initialize the database first.");
            co_return;
        }

        try {
            // Seed base admin for login testing
            winrt::LeafEyeCore::UserModel u1;
            u1.Username(L"admin");
            u1.Password(L"admin123");
            u1.IsAdmin(true);

            co_await m_db.AddUser(u1);

            LoginErrorText().Text(L"Mock data seeded! You can login with admin / admin123");
        }
        catch (...) {
            LoginErrorText().Text(L"Failed to seed data.");
        }
    }

    winrt::fire_and_forget MainWindow::LoginButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (!m_db) {
            LoginErrorText().Text(L"Please initialize the database first.");
            co_return;
        }

        hstring username = LoginUsername().Text();
        hstring password = LoginPassword().Password();

        LoginButton().IsEnabled(false);
        LoginErrorText().Text(L"Logging in...");

        auto result = co_await m_db.ValidateUserCredentials(username, password);

        if (result.IsValueExists() && !result.IsError()) {
            // Save current session
            m_currentUser = result.ResultValue().as<winrt::LeafEyeCore::UserModel>();

            // Hide login, show main app
            LoginPanel().Visibility(Visibility::Collapsed);
            MainNav().Visibility(Visibility::Visible);
            LoginErrorText().Text(L"");

            // Trigger navigation to the first page (Users)
            MainNav().SelectedItem(MainNav().MenuItems().GetAt(0));
        }
        else {
            LoginErrorText().Text(L"Invalid credentials or user not found.");
        }

        LoginButton().IsEnabled(true);
    }

    void MainWindow::MainNav_SelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const&, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args)
    {
        if (args.IsSettingsSelected()) return;

        auto item = args.SelectedItemContainer();
        if (!item) return;

        hstring tag = unbox_value<hstring>(item.Tag());

        // TODO: We will uncomment these Navigate calls as we create each Page in the next steps.
        
        if (tag == L"Users") {
            ContentFrame().Navigate(xaml_typename<LeafEyeTest::UsersPage>(), winrt::box_value(m_db));
        } else if (tag == L"Profiles") {
            ContentFrame().Navigate(xaml_typename<LeafEyeTest::ProfilesPage>(), winrt::box_value(m_db));
        }/* else if (tag == L"History") {
            ContentFrame().Navigate(xaml_typename<LeafEyeTest::HistoryPage>(), winrt::box_value(m_db));
        } else if (tag == L"FileHistory") {
            ContentFrame().Navigate(xaml_typename<LeafEyeTest::FileHistoryPage>(), winrt::box_value(m_db));
        }
        */
    }

    void MainWindow::Logout_Tapped(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const&)
    {
        // Clear session
        m_currentUser = nullptr;
        LoginUsername().Text(L"");
        LoginPassword().Password(L"");

        // Reset UI
        ContentFrame().Content(nullptr);
        MainNav().Visibility(Visibility::Collapsed);
        LoginPanel().Visibility(Visibility::Visible);
    }
}