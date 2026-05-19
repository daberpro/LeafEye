#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <string_view>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>
#include "HomePage.xaml.h"
#include "HistoryPage.xaml.h"
#include "ProfilePage.xaml.h"
#include "SettingsPage.xaml.h"
#include "LoginPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace Microsoft::UI::Xaml::Media::Animation;

namespace winrt::LeafEye::implementation
{

	winrt::LeafEyeCore::ProfileModel MainWindow::Profile() { return m_profile; }
	void MainWindow::Profile(winrt::LeafEyeCore::ProfileModel const& value)
	{
		if (m_profile != value) {
			m_profile = value;
			RaisePropertyChanged(L"Profile");
		}
	}

	winrt::LeafEyeCore::UserModel MainWindow::User() { return m_user; }
	void MainWindow::User(winrt::LeafEyeCore::UserModel const& value)
	{
		if (m_user != value) {
			m_user = value;
			RaisePropertyChanged(L"User");
		}
	}

	void MainWindow::SetStatusLog(const winrt::LeafEyeCore::Result& result) {}

	void MainWindow::OnClosed(IInspectable const&, WindowEventArgs const&)
	{
		winrt::LeafEye::implementation::App::ClearWindow();
	}

	MainWindow::MainWindow()
	{
		InitializeComponent();
		ExtendsContentIntoTitleBar(true);
		SetTitleBar(AppTitleBar());

		
		auto resources = Application::Current().Resources();

		auto profileFlyout = MenuFlyout{};
		profileFlyout.Placement(Primitives::FlyoutPlacementMode::Bottom);

		auto profileMenuItem = MenuFlyoutItem{};
		auto logoutMenuItem = MenuFlyoutItem{};

		profileMenuItem.Text(L"Profile");
		profileMenuItem.Icon(SymbolIcon{ Symbol::Contact });
		profileMenuItem.Click([this](auto&&, auto&&) {
			contentFrame().Navigate(xaml_typename<LeafEye::ProfilePage>());
			AppNavigationView().Header(box_value(L"Profile"));
		});

		auto LogoutBrushForeground = resources.Lookup(box_value(L"LogoutBrushForeground")).as<Media::Brush>();
		auto LogoutBrushBackground = resources.Lookup(box_value(L"LogoutBrushBackground")).as<Media::Brush>();

		logoutMenuItem.Text(L"Logout");
		auto logoutIcon = FontIcon{};
		logoutIcon.Glyph(L"\uF3B1");
		logoutMenuItem.Foreground(LogoutBrushForeground);
		logoutMenuItem.Background(LogoutBrushBackground);
		logoutIcon.Foreground(LogoutBrushForeground);
		logoutMenuItem.Icon(logoutIcon);

		logoutMenuItem.Click([this](auto&&, auto&&) { Logout(); });

		profileFlyout.Items().Append(profileMenuItem);
		profileFlyout.Items().Append(logoutMenuItem);
		Primitives::FlyoutBase::SetAttachedFlyout(ProfilePic(), profileFlyout);

		auto app_local_path = winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path();
		winrt::LeafEye::Utils::AppSession::SetDatabase(
			winrt::LeafEyeCore::Database(app_local_path + L"\\database", 1024 * 1024)
		);

		Init();
	}

	winrt::fire_and_forget MainWindow::Init() {
		auto app_local_path = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
		co_await app_local_path.CreateFolderAsync(
			L"profile_photo",
			winrt::Windows::Storage::CreationCollisionOption::OpenIfExists
		);
		co_return;
	}

	winrt::fire_and_forget MainWindow::DismissOverlay()
	{
		auto lifetime = get_strong();
		winrt::apartment_context ui_thread;

		auto frame = overlayFrame();

		// Pisahkan type alias dulu agar compiler tidak bingung parse template
		using TranslateTransform = winrt::Microsoft::UI::Xaml::Media::TranslateTransform;

		auto translate = frame.RenderTransform().try_as<TranslateTransform>();
		if (!translate)
		{
			translate = TranslateTransform{};
			frame.RenderTransform(translate);
		}

		auto storyboard = Storyboard{};

		auto slideAnim = DoubleAnimation{};
		slideAnim.From(0.0);
		slideAnim.To(-static_cast<double>(frame.ActualHeight()));
		slideAnim.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(400)));
		auto slideEase = CubicEase{};
		slideEase.EasingMode(EasingMode::EaseIn);
		slideAnim.EasingFunction(slideEase);
		Storyboard::SetTarget(slideAnim, translate);
		Storyboard::SetTargetProperty(slideAnim, L"Y");

		storyboard.Children().Append(slideAnim);

		winrt::handle completedEvent{ CreateEvent(nullptr, true, false, nullptr) };
		auto token = storyboard.Completed([&completedEvent](auto&&, auto&&)
			{
				SetEvent(completedEvent.get());
			}
		);

		storyboard.Begin();
		co_await winrt::resume_on_signal(completedEvent.get());
		storyboard.Completed(token);

		co_await ui_thread;

		translate.Y(0.0);
		frame.Visibility(Visibility::Collapsed);
		frame.Content(nullptr);
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

	Windows::Foundation::IAsyncAction MainWindow::NavigationView_Loaded(IInspectable const&, RoutedEventArgs const&)
	{
		auto db_status = co_await winrt::LeafEye::Utils::AppSession::GetDatabase().InitializeAsync();
		this->DispatcherQueue().TryEnqueue([this, db_status]() {
			SetStatusLog(db_status);
		});

		overlayFrame().Navigate(xaml_typename<LeafEye::LoginPage>());
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

				if (tag == L"HomePage")
					contentFrame().Navigate(xaml_typename<LeafEye::HomePage>());
				else if (tag == L"HistoryPage")
					contentFrame().Navigate(xaml_typename<LeafEye::HistoryPage>());
				else if (tag == L"ProfilePage")
					contentFrame().Navigate(xaml_typename<LeafEye::ProfilePage>());
				else if (tag == L"UsersPage")
					contentFrame().Navigate(xaml_typename<LeafEye::UsersPage>());
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
		AppTitleBar().IsBackButtonEnabled(contentFrame().CanGoBack());

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
						if (!tag.empty() && currentFullName.find(tag) != std::wstring_view::npos) {
							AppNavigationView().SelectedItem(item);
							AppNavigationView().Header(item.Content());
							return true;
						}
					}
				}
				return false;
			};

			if (!findAndSetItem(AppNavigationView().MenuItems())) {
				findAndSetItem(AppNavigationView().FooterMenuItems());
			}
		}
	}

	void MainWindow::ToggleSwitch_Toggled(IInspectable const&, RoutedEventArgs const&)
	{
		auto rootElement = this->Content().as<FrameworkElement>();
		if (rootElement != nullptr) {
			if (ThemeToggleSwitch().IsOn())
				rootElement.RequestedTheme(ElementTheme::Dark);
			else
				rootElement.RequestedTheme(ElementTheme::Light);
		}
	}

	void MainWindow::Logout()
	{
		// 1. Hapus kredensial dari Windows Password Vault menggunakan fungsi statis
		winrt::LeafEye::LoginPage::ClearCredentials();

		// 2. Bersihkan state memori AppSession agar data user lama tidak bocor
		winrt::LeafEye::Utils::AppSession::SetUser(nullptr);
		winrt::LeafEye::Utils::AppSession::SetProfile(nullptr);

		// 3. Reset TranslateTransform
		using TranslateTransform = winrt::Microsoft::UI::Xaml::Media::TranslateTransform;
		if (auto translate = overlayFrame().RenderTransform().try_as<TranslateTransform>()) {
			translate.Y(0.0);
		}

		// 4. Bersihkan frame utama dan riwayatnya
		contentFrame().Navigate(xaml_typename<winrt::LeafEye::HomePage>());
		contentFrame().BackStack().Clear();
		contentFrame().ForwardStack().Clear();

		// 5. Tampilkan overlay login kembali
		overlayFrame().Opacity(1.0);
		overlayFrame().Visibility(Microsoft::UI::Xaml::Visibility::Visible);
		overlayFrame().Navigate(xaml_typename<winrt::LeafEye::LoginPage>());
	}
}