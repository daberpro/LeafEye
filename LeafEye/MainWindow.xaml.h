#pragma once
#include "MainWindow.g.h"
#include "App.xaml.h"
#include "Utils/AppSession.h"
#include <winrt/LeafEyeCore.h>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>
#include "LoginPage.xaml.h"

namespace winrt::LeafEye::implementation
{
	struct MainWindow : MainWindowT<MainWindow>
	{
	private:
		winrt::hstring m_myStatusText{ L"Hello, World!" };
		winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
		winrt::LeafEyeCore::ProfileModel m_profile{ nullptr };
		
	public:
		MainWindow();

		winrt::LeafEyeCore::ProfileModel Profile();	
		void Profile(winrt::LeafEyeCore::ProfileModel const& value);

		winrt::fire_and_forget Init();
		void OnClosed(winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::WindowEventArgs const& args);

		hstring MyStatusText();
		void MyStatusText(hstring const& value);
		winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
		void PropertyChanged(winrt::event_token const& token) noexcept;

		void SetStatusLog(const winrt::LeafEyeCore::Result& result);
		void RaisePropertyChanged(hstring const& propertyName);

		winrt::fire_and_forget DismissOverlay();
		void Logout();

		Windows::Foundation::IAsyncAction NavigationView_Loaded(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void NavigationView_ItemInvoked(
			winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender,
			winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);
		void AppTitleBar_PaneToggleRequested(
			winrt::Microsoft::UI::Xaml::Controls::TitleBar const& sender,
			winrt::Windows::Foundation::IInspectable const& args);
		void ProfilePic_Tapped(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e);
		void AppTitleBar_BackRequested(
			winrt::Microsoft::UI::Xaml::Controls::TitleBar const& sender,
			winrt::Windows::Foundation::IInspectable const& args);
		void contentFrame_Navigated(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
		void ToggleSwitch_Toggled(
			winrt::Windows::Foundation::IInspectable const& sender,
			winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
	};
}

namespace winrt::LeafEye::factory_implementation
{
	struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
	{
	};
}