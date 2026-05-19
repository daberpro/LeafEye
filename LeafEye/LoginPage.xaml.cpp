#include "pch.h"
#include "LoginPage.xaml.h"
#if __has_include("LoginPage.g.cpp")
#include "LoginPage.g.cpp"
#endif
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Windows::Security::Credentials;

namespace winrt::LeafEye::implementation
{
    LoginPage::LoginPage()
    {
        InitializeComponent();
    }

    // ── Credential Helpers ──────────────────────────────────────────

    void LoginPage::SaveCredentials(hstring const& username, hstring const& password)
    {
        try
        {
            auto vault = winrt::Windows::Security::Credentials::PasswordVault{};

            try {
                auto all = vault.RetrieveAll();
                for (auto const& c : all)
                {
                    if (c.Resource() == VaultResource.data())
                        vault.Remove(c);
                }
            }
            catch (...) {}

            vault.Add(winrt::Windows::Security::Credentials::PasswordCredential{ VaultResource.data(), username, password });
        }
        catch (...) {}
    }

    void LoginPage::ClearCredentials()
    {
        try
        {
            auto vault = winrt::Windows::Security::Credentials::PasswordVault{};
            auto list = vault.FindAllByResource(VaultResource.data());
            for (auto const& c : list)
                vault.Remove(c);
        }
        catch (...) {}
    }

    winrt::Windows::Security::Credentials::PasswordCredential LoginPage::LoadCredentials()
    {
        try
        {
            auto vault = winrt::Windows::Security::Credentials::PasswordVault{};
            auto all = vault.RetrieveAll();

            for (auto const& c : all)
            {
                if (c.Resource() == VaultResource.data())
                {
                    c.RetrievePassword();
                    return c;
                }
            }
        }
        catch (...) {}

        return nullptr;
    }


    // ── OnNavigatedTo: cek auto-login ──────────────────────────────

    winrt::fire_and_forget LoginPage::OnNavigatedTo(
        winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        auto lifetime = get_strong();
        winrt::apartment_context ui_thread;

        auto credential = LoadCredentials();
        if (!credential) co_return; 
        
        co_await winrt::resume_background();
        auto result = co_await winrt::LeafEye::Utils::AppSession::GetDatabase().ValidateUserCredentials(
            credential.UserName(),
            credential.Password()
        );
        auto profile = co_await winrt::LeafEye::Utils::AppSession::GetDatabase().GetUserProfileByLink(
            result.IsValueExists() ? result.ResultValue().as<winrt::LeafEyeCore::UserModel>().Id() : 0
		);
        co_await ui_thread;

        if (result.IsValueExists() && profile.IsValueExists())
        {
            winrt::LeafEye::Utils::AppSession::SetUser(result.ResultValue().as<winrt::LeafEyeCore::UserModel>());
            winrt::LeafEye::Utils::AppSession::GetUser().Password(L"");
            winrt::LeafEye::Utils::AppSession::SetProfile(profile.ResultValue().as<winrt::LeafEyeCore::ProfileModel>());
            if (auto mainWindow = App::Window().try_as<winrt::LeafEye::implementation::MainWindow>()) {
				mainWindow->Profile(winrt::LeafEye::Utils::AppSession::GetProfile());
				mainWindow->User(winrt::LeafEye::Utils::AppSession::GetUser());
                mainWindow->DismissOverlay();
            }
        }
        else
        {

            // Kredensial kadaluarsa/tidak valid, hapus
            ClearCredentials();
        }
    }

    // ── Button_Click: login manual ─────────────────────────────────

    winrt::fire_and_forget LoginPage::Button_Click(
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        auto lifetime = get_strong();
        winrt::apartment_context ui_thread;

        ErrorBar().IsOpen(false);

        hstring username = UsernameInput().Text();
        hstring password = PasswordInput().Password();

        if (username.empty() || password.empty())
        {
            ErrorBar().Message(L"Username dan password tidak boleh kosong.");
            if (!ErrorBar().IsOpen()) {
                ErrorBar().IsOpen(true);
            }
            co_return;
        }

        co_await winrt::resume_background();
        auto result = co_await winrt::LeafEye::Utils::AppSession::GetDatabase().ValidateUserCredentials(username, password);
        auto profile = co_await winrt::LeafEye::Utils::AppSession::GetDatabase().GetUserProfileByLink(
            result.IsValueExists() ? result.ResultValue().as<winrt::LeafEyeCore::UserModel>().Id() : 0
        );
        co_await ui_thread;

        if (result.IsError())
        {
            ErrorBar().Message(result.Message());
            if (!ErrorBar().IsOpen()) {
                ErrorBar().IsOpen(true);
            }
        }
        else if (result.IsValueExists() && profile.IsValueExists())
        {
            winrt::LeafEye::Utils::AppSession::SetUser(result.ResultValue().as<winrt::LeafEyeCore::UserModel>());
            winrt::LeafEye::Utils::AppSession::GetUser().Password(L"");
            winrt::LeafEye::Utils::AppSession::SetProfile(profile.ResultValue().as<winrt::LeafEyeCore::ProfileModel>());
            SaveCredentials(username, password); // simpan ke vault
            if (auto mainWindow = App::Window().try_as<winrt::LeafEye::implementation::MainWindow>()) {
                mainWindow->Profile(winrt::LeafEye::Utils::AppSession::GetProfile());
                mainWindow->User(winrt::LeafEye::Utils::AppSession::GetUser());
                mainWindow->DismissOverlay();
            }
        }
        else
        {
            ErrorBar().Message(L"Username atau password salah.");
            if (!ErrorBar().IsOpen()) {
                ErrorBar().IsOpen(true);
            }
        }
    }
}