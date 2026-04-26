#pragma once
#include <winrt/LeafEyeCore.h>
#include <winrt/Windows.Security.Credentials.h>
#include "Utils/AppSession.h"
#include "LoginPage.g.h"
#include "MainWindow.xaml.h"

namespace winrt::LeafEye::implementation
{
    struct LoginPage : LoginPageT<LoginPage>
    {
        LoginPage();
        winrt::fire_and_forget OnNavigatedTo(
            winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        winrt::fire_and_forget Button_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        // Jadikan public dan static agar bisa diakses global
        static constexpr std::wstring_view VaultResource = L"LeafEye";

        static void SaveCredentials(winrt::hstring const& username, winrt::hstring const& password);
        static void ClearCredentials();
        static winrt::Windows::Security::Credentials::PasswordCredential LoadCredentials();
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct LoginPage : LoginPageT<LoginPage, implementation::LoginPage>
    {
    };
}