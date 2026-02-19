#pragma once

#include <winrt/LeafEyeCore.h>
#include "MainWindow.g.h"
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::LeafEyeTest::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        // Event Handlers Utama (Sinkron/Void untuk XAML)
        void BoxSelector_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void AddDummyData_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearBox_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PrevButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NextButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        // Event Handlers Baru
        void SearchButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ResetButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SortToggle_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        // State Management
        bool m_isInitialized{ false };
        winrt::LeafEyeCore::Database m_db{ nullptr };
        int32_t m_currentPage{ 1 };
        int32_t m_pageSize{ 10 };
        winrt::LeafEyeCore::BoxType m_currentBox{ winrt::LeafEyeCore::BoxType::User };

        // Data Binding
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::hstring> m_displayItems;

        // Async Background Workers
        winrt::fire_and_forget InitializeDatabaseAsync();
        winrt::fire_and_forget LoadDataAsync();
        winrt::fire_and_forget SearchDataAsync();
        winrt::fire_and_forget AddDummyDataAsync();
        winrt::fire_and_forget ClearBoxAsync();

        // Helper Functions
        void ShowStatus(winrt::hstring const& message, bool isError);
        winrt::LeafEyeCore::BoxType GetSelectedBoxType();
        winrt::hstring FormatModelData(winrt::Windows::Foundation::IInspectable const& item);
        int64_t DateTimeToUnix(winrt::Windows::Foundation::DateTime dt);
    };
}

namespace winrt::LeafEyeTest::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}