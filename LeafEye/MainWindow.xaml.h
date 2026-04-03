#pragma once
#include "MainWindow.g.h"
#include "App.xaml.h"
#include <winrt/LeafEyeCore.h>

namespace winrt::LeafEye::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
    private:

		winrt::hstring m_myStatusText{ L"Hello, World!" };
		winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    public:
        MainWindow();

        void OnClosed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowEventArgs const& args);

        hstring MyStatusText();
        void MyStatusText(hstring const& value);
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        void SetStatusLog(const winrt::LeafEyeCore::Result& result);
		Windows::Foundation::IAsyncAction NavigationView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
		void NavigationView_ItemInvoked(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs const& args);

        void RaisePropertyChanged(hstring const& propertyName);
        void AppTitleBar_PaneToggleRequested(winrt::Microsoft::UI::Xaml::Controls::TitleBar const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void ProfilePic_Tapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const& e);
        void AppTitleBar_BackRequested(winrt::Microsoft::UI::Xaml::Controls::TitleBar const& sender, winrt::Windows::Foundation::IInspectable const& args);
        void contentFrame_Navigated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        void ToggleSwitch_Toggled(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    private:

        winrt::LeafEyeCore::Database m_db;
    
    };
}
namespace winrt::LeafEye::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
