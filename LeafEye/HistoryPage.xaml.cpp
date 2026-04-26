#include "pch.h"
#include "HistoryPage.xaml.h"
#if __has_include("HistoryPage.g.cpp")
#include "HistoryPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::LeafEye::implementation
{
    winrt::hstring HistoryPage::ConvertTotalFileToString(uint32_t totalFile) {
        return winrt::to_hstring(
            std::format("Total File : {}", totalFile)
        );
	};

    winrt::hstring HistoryPage::ConvertToDateTimeString(uint64_t fileTime) {
        std::chrono::sys_time<std::chrono::milliseconds> tp{ std::chrono::milliseconds{fileTime} };
        return winrt::to_hstring(
            std::format("Tanggal Prediksi : {:%Y-%m-%d}", tp)
		);
	};

    winrt::hstring HistoryPage::HistoryIdToStringConverter(uint64_t historyId) {
        return winrt::to_hstring(
            std::format("History ID #{}", historyId)
        );
    };

    winrt::Microsoft::UI::Xaml::Media::ImageSource HistoryPage::CheckFilePath(const hstring& filePath) {
        
        if (!std::filesystem::exists(winrt::to_string(filePath))) {
            return winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage{
				winrt::Windows::Foundation::Uri{ L"ms-appx:///Assets/PlaceholderImage.png" }
            };
        }
        return winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage{
            winrt::Windows::Foundation::Uri{ filePath }
        };
    };

    HistoryPage::HistoryPage() {
        InitializeComponent();
		m_history = winrt::single_threaded_observable_vector<winrt::LeafEyeCore::HistoryModel>();
		m_filesHistory = winrt::single_threaded_observable_vector<winrt::LeafEyeCore::FileHistoryModel>();

        m_filesHistory.VectorChanged([this](auto&&, auto&&) {
            EmptyFileHistoryListViewText().Visibility(m_filesHistory.Size() <= 0 ? Visibility::Visible : Visibility::Collapsed);
		});

        HistoryListView().ItemsSource(m_history);
        FileHistoryListView().ItemsSource(m_filesHistory);
    }


    void HistoryPage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e){

        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();

        RegisterPaginationCallback(
            HistoryFileType::NONE,
            [this,database](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto result = co_await database.GetAllHistory(offset, limit);
                co_await ui_thread;
                if (!result.IsError() && result.IsValueExists()) {
                    auto history = result.ResultValue().as<
                        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::HistoryModel>
                    >();
                    m_history.Clear();
                    for (auto item : history) {
                        m_history.Append(item);
                    }
                }
            },
            [this,database]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                auto result = co_await database.GetHistoryCount();
                if (!result.IsError() && result.IsValueExists()) {
                    co_return static_cast<uint64_t>(result.ResultValue().as<int64_t>());
                }
                co_return 0;
            }
        );

        RegisterPaginationCallback(
            HistoryFileType::DATE,
            [this,database](int64_t offset, int64_t limit) -> winrt::fire_and_forget {
                winrt::apartment_context ui_thread;
                co_await winrt::resume_background();
                auto result = co_await database.GetHistoryByDateRange(0, 0, offset, limit);
                co_await ui_thread;
                if (!result.IsError() && result.IsValueExists()) {
                    auto history = result.ResultValue().as<
                        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::HistoryModel>
                    >();
                    m_history.Clear();
                    for (auto item : history) {
                        m_history.Append(item);
                    }
                }
            },
            [this,database]() -> winrt::Windows::Foundation::IAsyncOperation<uint64_t> {
                auto result = co_await database.GetHistoryCount();
                if (!result.IsError() && result.IsValueExists()) {
                    co_return static_cast<uint64_t>(result.ResultValue().as<int64_t>());
                }
                co_return 0;
            }
        );

        SetPaginationFilter(HistoryFileType::NONE);

    }

    winrt::fire_and_forget HistoryPage::HistoryListView_ItemClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::ItemClickEventArgs const& e)
    {

        auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
        auto data = e.ClickedItem().as<winrt::LeafEyeCore::HistoryModel>();
        m_selectedHistory.Date(data.Date());
        m_selectedHistory.Id(data.Id());
        m_selectedHistory.Status(data.Status());
        m_selectedHistory.OutputFolder(data.OutputFolder());
        m_selectedHistory.TotalFiles(data.TotalFiles());
        obx_id history_id = data.Id();
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        auto result = co_await database.GetFileHistoriesByHistoryLink(history_id);
        co_await ui_thread;

        if (!result.IsError() && result.IsValueExists()) {
            auto files = result.ResultValue().as<
                winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel>
            >();

			m_filesHistory.Clear();
            for (const auto& file: files) {
				m_filesHistory.Append(file);
            }
        }

    }

    winrt::LeafEyeCore::HistoryModel HistoryPage::SelectedHistory() {
        return m_selectedHistory;
    };
    
    void HistoryPage::SelectedHistory(winrt::LeafEyeCore::HistoryModel const& value) {
        if (m_selectedHistory != value) {
            m_selectedHistory = value;
            RaisedPropertyChanged(L"SelectedHistory");
        }
    };


    // ================================= Pagination =========================================

    void HistoryPage::UpdatePaginationInfo() {
        RaisedPropertyChanged(L"GetPaginationInfo");
    }

    winrt::hstring HistoryPage::GetPaginationInfo() {

        return winrt::to_hstring(
            std::format(
                "Halaman ke {} dari {}",
                m_pagination_current_page + 1,
                m_pagination_total_page
            ));

    };

    void HistoryPage::TotalPage(int64_t value)
    {
        if (m_pagination_total_page != value) {
            m_pagination_total_page = value;
            RaisedPropertyChanged(L"TotalPage");
            RaisedPropertyChanged(L"GetPaginationInfo");

        }
    }

    int64_t HistoryPage::TotalPage()
    {
        return m_pagination_total_page;
    }

    int64_t HistoryPage::CurrentPage()
    {
        return m_pagination_current_page;
    }

    void HistoryPage::CurrentPage(int64_t value)
    {
        if (m_pagination_current_page != value) {
            m_pagination_current_page = value;
            RaisedPropertyChanged(L"CurrentPage");
        }
    }

    void HistoryPage::PrevPageEnabled(bool value)
    {
        if (m_prevPageEnabled != value) {
            m_prevPageEnabled = value;
            RaisedPropertyChanged(L"PrevPageEnabled");
        }
    }

    bool HistoryPage::PrevPageEnabled()
    {
        return m_prevPageEnabled;
    }

    void HistoryPage::NextPageEnabled(bool value)
    {
        if (m_nextPageEnabled != value) {
            m_nextPageEnabled = value;
            RaisedPropertyChanged(L"NextPageEnabled");
            RaisedPropertyChanged(L"GetPaginationInfo");
        }
    }

    bool HistoryPage::NextPageEnabled()
    {
        return m_nextPageEnabled;
    }

    winrt::event_token HistoryPage::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void HistoryPage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void HistoryPage::RaisedPropertyChanged(const winrt::hstring& property_name) {
        if (m_propertyChanged)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
        }
    }

    // ================================= Pagination =========================================

    void HistoryPage::PreviousPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        PrevPage();
    }

    void HistoryPage::NextPageButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        NextPage();
    }

    void HistoryPage::PageSize_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
    {
        auto item = PageSize().SelectedItem().as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
        auto content = winrt::unbox_value<winrt::hstring>(item.Content());
        int size = std::stoi(winrt::to_string(content));
        m_pagination_limit = size;
        PaginationSetUp();
    }
}

