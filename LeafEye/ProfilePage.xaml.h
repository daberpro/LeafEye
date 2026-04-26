#pragma once
#include "ProfilePage.g.h"
#include "MainWindow.xaml.h"
#include "LoginPage.xaml.h"

namespace winrt::LeafEye::implementation
{
    struct ProfilePage : ProfilePageT<ProfilePage>
    {
    private:
        bool m_is_changed{ false };
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        winrt::Windows::Storage::StorageFile m_profile_photo{ nullptr };

    public:
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
        void RaisePropertyChanged(hstring const& propertyName);

        ProfilePage();
        void EvaluateSaveState();
        winrt::LeafEyeCore::ProfileModel Profile();
        winrt::LeafEyeCore::UserModel User();
        bool IsChanged();
        void IsChanged(bool is_changed);
        static hstring GetUserNameAndFullNameString(hstring const& username, hstring const& fullname);
        void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget EditPhoto_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget Save_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FullNameInput_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void UsernameInput_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void OldPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NewPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ConfirmPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct ProfilePage : ProfilePageT<ProfilePage, implementation::ProfilePage>
    {
    };
}
