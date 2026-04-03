#include "pch.h"
#include "AddUserDialog.xaml.h"
#if __has_include("AddUserDialog.g.cpp")
#include "AddUserDialog.g.cpp"
#endif

namespace winrt::LeafEye::implementation
{
    winrt::hstring TrimText(winrt::hstring const& input)
    {
        std::wstring_view sv = input;
        const std::wstring_view whitespace = L" \t\n\r\f\v"; // Karakter spasi, tab, enter, dll.
        size_t start = sv.find_first_not_of(whitespace);
        if (start == std::wstring_view::npos)
        {
            return L"";
        }
        size_t end = sv.find_last_not_of(whitespace);
        return winrt::hstring(sv.substr(start, end - start + 1));
    }

    AddUserDialog::AddUserDialog()
    {
        InitializeComponent();
    }

    hstring AddUserDialog::Username()
    {
        return UsernameTextBox().Text();
    }

    hstring AddUserDialog::Password()
    {
        return PasswordBox().Password();
    }

    bool AddUserDialog::IsAdmin()
    {
        // GetBoolean() digunakan untuk mengonversi IReference<bool> ke tipe bool standar
        auto isChecked = IsAdminCheckBox().IsChecked();
        return isChecked ? isChecked.GetBoolean() : false;
    }

    void AddUserDialog::UsernameTextBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {
        if (e.Key() == winrt::Windows::System::VirtualKey::Enter) {
            PasswordBox().Focus(winrt::Microsoft::UI::Xaml::FocusState::Keyboard);
            e.Handled(true);
        }
    }

    void AddUserDialog::PasswordBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e)
    {

    }

    void AddUserDialog::ContentDialog_PrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args)
    {
        winrt::hstring username = TrimText(UsernameTextBox().Text());
        winrt::hstring password = PasswordBox().Password();

        if (username.empty() || password.empty())
        {
            args.Cancel(true);

            ShowMessage(L"Username dan Password tidak boleh kosong!", winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity::Error);
            return;
        }

        StatusInfoBar().IsOpen(false);
    }

    void AddUserDialog::ShowMessage(winrt::hstring const& message, winrt::Microsoft::UI::Xaml::Controls::InfoBarSeverity severity)
    {
        StatusInfoBar().Severity(severity);
        StatusInfoBar().Message(message);
        StatusInfoBar().IsOpen(true);
    }
}
