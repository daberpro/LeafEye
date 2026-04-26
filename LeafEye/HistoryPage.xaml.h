#pragma once
#include "HistoryPage.g.h"
#include "Base/PaginationBaseExperiment.h"
#include "Utils/AppSession.h"

namespace winrt::LeafEye::implementation
{
    enum class HistoryFileType {
        NONE,
        DATE,
        STATUS
    };

    struct HistoryPage : HistoryPageT<HistoryPage>, winrt::LeafEye::Utils::PaginationBase<HistoryFileType>
    {
    private:

		winrt::LeafEyeCore::HistoryModel m_selectedHistory;
        // ================================= Pagination =========================================
        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        bool m_prevPageEnabled{ false };
        bool m_nextPageEnabled{ false };
        // ================================= Pagination =========================================

        void RaisedPropertyChanged(const winrt::hstring&);

    public:
        HistoryPage();

        winrt::LeafEyeCore::HistoryModel SelectedHistory();
        void SelectedHistory(winrt::LeafEyeCore::HistoryModel const& value);

		static winrt::hstring ConvertTotalFileToString(uint32_t totalFile);
		static winrt::hstring HistoryIdToStringConverter(uint64_t historyId);
        static winrt::hstring ConvertToDateTimeString(uint64_t fileTime);
        static winrt::Microsoft::UI::Xaml::Media::ImageSource CheckFilePath(const hstring& filePath);

        // ================================= Pagination =========================================
        void PrevPageEnabled(bool value) override;
        void NextPageEnabled(bool value) override;
        void UpdatePaginationInfo() override;
        hstring GetPaginationInfo();
        void TotalPage(int64_t value);
        int64_t TotalPage();
        int64_t CurrentPage();
        void CurrentPage(int64_t value);
        bool PrevPageEnabled();
        bool NextPageEnabled();
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
        // ================================= Pagination =========================================


        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);
        winrt::Windows::Foundation::Collections::IObservableVector<winrt::LeafEyeCore::HistoryModel> m_history;
		winrt::Windows::Foundation::Collections::IObservableVector<winrt::LeafEyeCore::FileHistoryModel> m_filesHistory;
        void PreviousPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PageSize_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        winrt::fire_and_forget HistoryListView_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e);
    };
}

namespace winrt::LeafEye::factory_implementation
{
    struct HistoryPage : HistoryPageT<HistoryPage, implementation::HistoryPage>
    {
    };
}
