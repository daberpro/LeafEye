#pragma once
#include "AddUserDialog.g.h"

namespace winrt::LeafEye::implementation
{
    struct AddUserDialog : AddUserDialogT<AddUserDialog>
    {
        AddUserDialog();

        hstring Username();
        hstring Password();
        bool IsAdmin();
        void UsernameTextBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void PasswordBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void ContentDialog_PrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void ShowMessage(winrt::hstring const& message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity);
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct AddUserDialog : AddUserDialogT<AddUserDialog, implementation::AddUserDialog>
    {
    };
}