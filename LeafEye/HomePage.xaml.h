#pragma once
#include "App.xaml.h"
#include "HomePage.g.h"
#include "Utils/AppSession.h"
#include "Utils/LeafEyeDetector.h"

namespace winrt::LeafEye::implementation
{
    struct HomePage : HomePageT<HomePage>
    {
    private:

        inline static std::shared_ptr<winrt::LeafEye::Utils::LeafEyeDetector> ai_detector = nullptr;

        winrt::LeafEyeCore::HistoryModel m_historyInfo;
        winrt::Windows::Foundation::Collections::IVectorView<winrt::Windows::Storage::StorageFile> m_files;
		winrt::Windows::Foundation::Collections::IObservableVector<winrt::LeafEye::FileItemsHomePageModel> m_filesMetaInfo;

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        bool m_isProcessButtonEnabled{ false };
        hstring m_processButtonInfo;

		
        void RaisedPropertyChanged(hstring const& property_name);
    public:

        
		static hstring ConvertToDateTimeString(uint64_t fileTime);
		static hstring ConvertToFileSizeString(uint64_t fileSize);

        winrt::LeafEyeCore::HistoryModel HistoryInfo();

        HomePage();
        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        winrt::fire_and_forget DropTarget_DragOver(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e);
        winrt::fire_and_forget DropTarget_DragLeave(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e);
        winrt::fire_and_forget DropTarget_Drop(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e);
        
        void ProcessButtonInfo(const hstring& value);
        hstring ProcessButtonInfo();
        void IsProcessButtonEnabled(bool value);
        bool IsProcessButtonEnabled();
        void DeleteSelectedFilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget ProcessButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget FilePicker_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget SelectFolderOutputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<float>> PreprocessImageAsync(winrt::Windows::Storage::StorageFile file);
        
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct HomePage : HomePageT<HomePage, implementation::HomePage>
    {
    };
}
