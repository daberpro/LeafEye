#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <string_view>
#include "HomePage.xaml.h"
#include "HistoryPage.xaml.h"
#include "ProfilePage.xaml.h"
#include "SettingsPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;

namespace winrt::LeafEye::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        ExtendsContentIntoTitleBar(true);
        SetTitleBar(AppTitleBar());


        OutputDebugString(
            winrt::to_hstring(
                std::format(
                    "\n========\nUSING OBJECT BOX VERSION : {}\n=========\n", 
                    obx_version_string()
                )
            ).c_str()
        );

        
        auto resources = Application::Current().Resources();

        // Setup Flyout Profil (SRS-UI-02) [cite: 52, 86]
        auto profileFlyout = MenuFlyout{};
        profileFlyout.Placement(Primitives::FlyoutPlacementMode::Bottom);

        auto profileMenuItem = MenuFlyoutItem{};
        auto logoutMenuItem = MenuFlyoutItem{};

        profileMenuItem.Text(L"Profile");
        profileMenuItem.Icon(SymbolIcon{ Symbol::Contact });

        auto LogoutBrushForeground = resources.Lookup(box_value(L"LogoutBrushForeground")).as<Media::Brush>();
        auto LogoutBrushBackground = resources.Lookup(box_value(L"LogoutBrushBackground")).as<Media::Brush>();

        logoutMenuItem.Text(L"Logout");
        auto logoutIcon = FontIcon{};
        logoutIcon.Glyph(L"\uF3B1");

        logoutMenuItem.Foreground(LogoutBrushForeground);
        logoutMenuItem.Background(LogoutBrushBackground);
        logoutIcon.Foreground(LogoutBrushForeground);
        logoutMenuItem.Icon(logoutIcon);

        profileFlyout.Items().Append(profileMenuItem);
        profileFlyout.Items().Append(logoutMenuItem);
        Primitives::FlyoutBase::SetAttachedFlyout(ProfilePic(), profileFlyout);
    }

    void MainWindow::RaisePropertyChanged(hstring const& propertyName)
    {
        m_propertyChanged(*this, Data::PropertyChangedEventArgs{ propertyName });
    }

    hstring MainWindow::MyStatusText() { return m_myStatusText; }

    void MainWindow::MyStatusText(hstring const& value)
    {
        if (m_myStatusText != value) {
            m_myStatusText = value;
            RaisePropertyChanged(L"MyStatusText");
        }
    }

    event_token MainWindow::PropertyChanged(Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void MainWindow::PropertyChanged(event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void MainWindow::AppTitleBar_PaneToggleRequested(TitleBar const&, IInspectable const&)
    {
        AppNavigationView().IsPaneOpen(!AppNavigationView().IsPaneOpen());
    }

    void MainWindow::ProfilePic_Tapped(IInspectable const&, Input::TappedRoutedEventArgs const&)
    {
        if (auto attachFlyOut = Primitives::FlyoutBase::GetAttachedFlyout(ProfilePic())) {
            attachFlyOut.ShowAt(ProfilePic());
        }
    }

    void MainWindow::NavigationView_Loaded(IInspectable const&, RoutedEventArgs const&)
    {
        AppNavigationView().Header(box_value(L"Home"));
        contentFrame().Navigate(xaml_typename<LeafEye::HomePage>());
    }

    void MainWindow::NavigationView_ItemInvoked(NavigationView const& sender, NavigationViewItemInvokedEventArgs const& args)
    {
        if (args.IsSettingsInvoked()) {
            sender.Header(box_value(L"Settings"));
            contentFrame().Navigate(xaml_typename<LeafEye::SettingsPage>());
        }
        else {
            auto item = args.InvokedItemContainer().as<NavigationViewItem>();
            if (item != nullptr) {
                sender.Header(item.Content());
                hstring tag = unbox_value<hstring>(item.Tag());

                if (tag == L"HomePage") {
                    contentFrame().Navigate(xaml_typename<LeafEye::HomePage>());
                }
                else if (tag == L"HistoryPage") {
                    contentFrame().Navigate(xaml_typename<LeafEye::HistoryPage>());
                }
                else if (tag == L"ProfilePage") {
                    contentFrame().Navigate(xaml_typename<LeafEye::ProfilePage>());
                }
            }
        }
    }

    void MainWindow::AppTitleBar_BackRequested(TitleBar const&, IInspectable const&)
    {
        if (contentFrame().CanGoBack()) {
            contentFrame().GoBack();
        }
    }

    void MainWindow::contentFrame_Navigated(IInspectable const&, NavigationEventArgs const& e)
    {
        // Update status tombol Back di TitleBar
        AppTitleBar().IsBackButtonEnabled(contentFrame().CanGoBack());

        // Sinkronisasi Seleksi Menu NavigationView agar indikator visual berpindah saat Back
        if (e.SourcePageType().Name == xaml_typename<LeafEye::SettingsPage>().Name) {
            AppNavigationView().SelectedItem(AppNavigationView().SettingsItem());
            AppNavigationView().Header(box_value(L"Settings"));
        }
        else {
            std::wstring_view currentFullName{ e.SourcePageType().Name };

            auto findAndSetItem = [&](auto const& collection) -> bool {
                for (auto const& element : collection) {
                    if (auto item = element.try_as<NavigationViewItem>()) {
                        hstring tag = unbox_value_or<hstring>(item.Tag(), L"");
                        // Menggunakan std::wstring_view::find untuk mencocokkan Tag dengan nama Page [cite: 81]
                        if (!tag.empty() && currentFullName.find(tag) != std::wstring_view::npos) {
                            AppNavigationView().SelectedItem(item);
                            AppNavigationView().Header(item.Content());
                            return true;
                        }
                    }
                }
                return false;
                };

            // Cari di menu utama dan menu footer (Profil) [cite: 82, 86]
            if (!findAndSetItem(AppNavigationView().MenuItems())) {
                findAndSetItem(AppNavigationView().FooterMenuItems());
            }
        }
    }

    void MainWindow::ToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
    {
        auto rootElement = this->Content().as<FrameworkElement>();
        if (rootElement != nullptr) {
            // SRS-UI-01: Implementasi perubahan tema aplikasi (Light/Dark) [cite: 51]
            if (ThemeToggleSwitch().IsOn()) {
                rootElement.RequestedTheme(ElementTheme::Dark);
            }
            else {
                rootElement.RequestedTheme(ElementTheme::Light);
            }
        }
    }
}