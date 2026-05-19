#include "pch.h"
#include "UpdateUserDialog.xaml.h"
#if __has_include("UpdateUserDialog.g.cpp")
#include "UpdateUserDialog.g.cpp"
#endif

namespace winrt::LeafEye::implementation
{
    winrt::hstring TrimTextForUpdate(winrt::hstring const& input)
    {
        std::wstring_view sv = input;
        const std::wstring_view whitespace = L" \t\n\r\f\v";
        size_t start = sv.find_first_not_of(whitespace);
        if (start == std::wstring_view::npos)
        {
            return L"";
        }
        size_t end = sv.find_last_not_of(whitespace);
        return winrt::hstring(sv.substr(start, end - start + 1));
    }

    UpdateUserDialog::UpdateUserDialog()
    {
        InitializeComponent();
    }

    void UpdateUserDialog::SetUserData(winrt::hstring const& username, bool isAdmin, const int32_t& role)
    {
        UsernameTextBox().Text(username);
        IsAdminCheckBox().IsChecked(isAdmin);
        RoleComboBox().SelectedIndex(role);
    }

    hstring UpdateUserDialog::Username()
    {
        return UsernameTextBox().Text();
    }

    hstring UpdateUserDialog::Password()
    {
        return PasswordBox().Password();
    }

    bool UpdateUserDialog::IsAdmin()
    {
        auto isChecked = IsAdminCheckBox().IsChecked();
        return isChecked ? isChecked.GetBoolean() : false;
    }

    int32_t UpdateUserDialog::Role()
    {
        auto selectedItem = RoleComboBox().SelectedItem();
        if (selectedItem) {
            auto comboBoxItem = selectedItem.as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
            if (comboBoxItem) {
                auto tagValue = comboBoxItem.Tag().as<hstring>();
                if (!tagValue.empty()) {
                    try {
                        return std::stoi(winrt::to_string(tagValue));
                    }
                    catch (const std::exception&) {
                        return -1;
                    }
                }
                else {
                    return -1;
                }
            }
        }
        return -1;
	}

    void UpdateUserDialog::UsernameTextBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == winrt::Windows::System::VirtualKey::Enter) {
            PasswordBox().Focus(winrt::Microsoft::UI::Xaml::FocusState::Keyboard);
            e.Handled(true);
        }
    }

    void UpdateUserDialog::PasswordBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
    }

    void UpdateUserDialog::ContentDialog_PrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args)
    {
        winrt::hstring username = TrimTextForUpdate(UsernameTextBox().Text());

        // Hanya validasi username. Password dibiarkan bisa kosong jika tidak diubah
        if (username.empty())
        {
            args.Cancel(true);
            ShowMessage(L"Username tidak boleh kosong!", winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            return;
        }

        StatusInfoBar().IsOpen(false);
    }

    void UpdateUserDialog::ShowMessage(winrt::hstring const& message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        StatusInfoBar().Severity(severity);
        StatusInfoBar().Message(message);
        StatusInfoBar().IsOpen(true);
    }
}
void winrt::LeafEye::implementation::UpdateUserDialog::RoleComboBox_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{

}
