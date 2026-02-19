#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.h>
#include <format>
#include <random>
#include <chrono>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::LeafEyeTest::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        m_displayItems = winrt::single_threaded_observable_vector<winrt::hstring>();
        DataListView().ItemsSource(m_displayItems);

        m_isInitialized = true;
        InitializeDatabaseAsync();
    }

    // --- INITIALIZATION ---

    winrt::fire_and_forget MainWindow::InitializeDatabaseAsync()
    {
        auto ui_thread = this->DispatcherQueue();
        winrt::hstring dbPath{ winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path() };
        m_db = winrt::LeafEyeCore::Database{ dbPath, 1024 * 10 };

        auto result = co_await m_db.InitializeAsync();

        ui_thread.TryEnqueue([this, result]() {
            ShowStatus(result.Message(), result.IsError());
            if (!result.IsError()) LoadDataAsync();
            });
    }

    // --- HELPER: UNIX TO HUMAN DATE ---

    winrt::hstring UnixToDateString(int64_t unixTime)
    {
        try {
            auto tp = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(unixTime));
            return std::format(L"{:%d %b %Y, %H:%M}", tp).c_str();
        }
        catch (...) {
            return L"Format Tanggal Error";
        }
    }

    // --- DATA LOADING ---

    winrt::fire_and_forget MainWindow::LoadDataAsync()
    {
        if (!m_db || !m_isInitialized) co_return;

        auto ui_thread = this->DispatcherQueue();
        LoadingRing().IsActive(true);

        int32_t offset = (m_currentPage - 1) * m_pageSize;
        auto currentBox = m_currentBox;

        auto countResult = co_await m_db.GetCountAsync(currentBox);
        auto pagedEntries = co_await m_db.GetAllEntriesAsync(currentBox, m_pageSize, offset);

        std::vector<winrt::hstring> formattedStrings;
        for (auto const& item : pagedEntries) {
            formattedStrings.push_back(FormatModelData(item));
        }

        ui_thread.TryEnqueue([this, countResult, formattedStrings = std::move(formattedStrings)]() {
            if (LoadingRing() == nullptr) return;
            LoadingRing().IsActive(false);

            if (countResult.IsError()) {
                ShowStatus(L"Gagal memuat: " + countResult.Message(), true);
                return;
            }

            int64_t totalData = winrt::unbox_value<int64_t>(countResult.ResultValue());
            int64_t totalPages = (totalData == 0) ? 1 : (totalData + m_pageSize - 1) / m_pageSize;

            m_displayItems.Clear();
            for (auto const& str : formattedStrings) m_displayItems.Append(str);

            PageInfoText().Text(std::format(L"Halaman {} dari {} (Total: {})", m_currentPage, totalPages, totalData).c_str());
            PrevButton().IsEnabled(m_currentPage > 1);
            NextButton().IsEnabled(m_currentPage < totalPages);
            });
    }

    // --- DUMMY DATA (FIXED: PROFILE INCLUDED) ---

    winrt::fire_and_forget MainWindow::AddDummyDataAsync()
    {
        if (!m_db) co_return;
        LoadingRing().IsActive(true);

        // Ambil waktu sekarang sebagai titik akhir
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        // Rentang 3 tahun dalam detik (3 thn * 365 hari * 24 jam * 3600 detik)
        int64_t threeYearsInSeconds = 3LL * 365LL * 24LL * 3600LL;

        for (int i = 0; i < 5; i++) {
            // Buat offset acak antara 0 hingga 3 tahun yang lalu
            // Menggunakan kombinasi rand() agar persebaran lebih luas
            int64_t randomOffset = (static_cast<int64_t>(rand()) * rand()) % threeYearsInSeconds;
            int64_t randomTimestamp = now - randomOffset;

            int r = rand() % 1000;

            switch (m_currentBox) {
            case winrt::LeafEyeCore::BoxType::User:
                co_await m_db.AddEntryAsync(m_currentBox, winrt::LeafEyeCore::UserModel(std::format(L"user_{}", r).c_str(), L"pass123", false));
                break;

            case winrt::LeafEyeCore::BoxType::Profile:
                co_await m_db.AddEntryAsync(m_currentBox, winrt::LeafEyeCore::ProfileModel(0, std::format(L"Petani {}", r).c_str(), L"/assets/avatar.png", (int32_t)(r % 3)));
                break;

            case winrt::LeafEyeCore::BoxType::History:
                // Menggunakan randomTimestamp agar tahunnya acak
                co_await m_db.AddEntryAsync(m_currentBox, winrt::LeafEyeCore::HistoryModel(0, randomTimestamp, (uint32_t)(r % 10), 0));
                break;

            case winrt::LeafEyeCore::BoxType::FileHistory:
                // Menggunakan randomTimestamp agar tanggal pembuatan file acak
                co_await m_db.AddEntryAsync(m_currentBox, winrt::LeafEyeCore::FileHistoryModel(0, std::format(L"daun_durian_{}.jpg", r).c_str(), 2048, randomTimestamp, randomTimestamp, 0.95f));
                break;
            }
        }

        // Refresh tampilan setelah data masuk
        LoadDataAsync();
    }
    // --- SEARCH LOGIC ---

    winrt::fire_and_forget MainWindow::SearchDataAsync()
    {
        if (!m_db) co_return;
        auto ui_thread = this->DispatcherQueue();
        LoadingRing().IsActive(true);

        winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable> results{ nullptr };

        if (m_currentBox == winrt::LeafEyeCore::BoxType::History && StartDatePicker().Date() && EndDatePicker().Date())
        {
            int64_t start = DateTimeToUnix(StartDatePicker().Date().Value());
            int64_t end = DateTimeToUnix(EndDatePicker().Date().Value());
            results = co_await m_db.GetHistoryByDateRangeAsync(start, end);
        }
        else
        {
            winrt::hstring keyword = SearchBox().Text();
            if (m_currentBox == winrt::LeafEyeCore::BoxType::User) {
                results = co_await m_db.GetEntriesWithFilterAsync(m_currentBox, winrt::LeafEyeCore::UserModel(keyword, L"", false));
            }
            else if (m_currentBox == winrt::LeafEyeCore::BoxType::Profile) {
                // Opsional: Cari profile berdasarkan role (integer)
                try { results = co_await m_db.GetEntriesWithFilterAsync(m_currentBox, winrt::LeafEyeCore::ProfileModel(0, L"", L"", std::stoi(keyword.c_str()))); }
                catch (...) {}
            }
            else if (m_currentBox == winrt::LeafEyeCore::BoxType::FileHistory) {
                results = co_await m_db.GetEntriesWithFilterAsync(m_currentBox, winrt::LeafEyeCore::FileHistoryModel(0, keyword, 0, 0, 0, 0.0f));
            }
        }

        ui_thread.TryEnqueue([this, results]() {
            m_displayItems.Clear();
            if (results && results.Size() > 0) {
                for (auto const& item : results) m_displayItems.Append(FormatModelData(item));
                ShowStatus(std::format(L"Ditemukan {} data.", results.Size()).c_str(), false);
            }
            else {
                ShowStatus(L"Tidak ada data ditemukan.", true);
            }
            LoadingRing().IsActive(false);
            });
    }

    // --- UI EVENT HANDLERS ---

    void MainWindow::BoxSelector_SelectionChanged(IInspectable const&, Controls::SelectionChangedEventArgs const&)
    {
        if (!m_isInitialized) return;
        m_currentBox = GetSelectedBoxType();

        bool isHistory = (m_currentBox == winrt::LeafEyeCore::BoxType::History);
        DateRangePanel().Visibility(isHistory ? Visibility::Visible : Visibility::Collapsed);

        m_currentPage = 1;
        LoadDataAsync();
    }

    void MainWindow::SearchButton_Click(IInspectable const&, RoutedEventArgs const&) { SearchDataAsync(); }

    void MainWindow::ResetButton_Click(IInspectable const&, RoutedEventArgs const&) {
        SearchBox().Text(L""); StartDatePicker().Date(nullptr); EndDatePicker().Date(nullptr);
        m_currentPage = 1; LoadDataAsync();
    }

    void MainWindow::SortToggle_Click(IInspectable const&, RoutedEventArgs const&) {
        bool isDesc = SortToggle().IsChecked().Value();
        SortToggle().Content(winrt::box_value(isDesc ? L"Sort: Descending" : L"Sort: Ascending"));
        LoadDataAsync();
    }

    void MainWindow::AddDummyData_Click(IInspectable const&, RoutedEventArgs const&) { AddDummyDataAsync(); }
    void MainWindow::ClearBox_Click(IInspectable const&, RoutedEventArgs const&) { ClearBoxAsync(); }
    winrt::fire_and_forget MainWindow::ClearBoxAsync() { co_await m_db.ClearEntriesAsync(m_currentBox); LoadDataAsync(); }

    void MainWindow::PrevButton_Click(IInspectable const&, RoutedEventArgs const&) { if (m_currentPage > 1) { m_currentPage--; LoadDataAsync(); } }
    void MainWindow::NextButton_Click(IInspectable const&, RoutedEventArgs const&) { m_currentPage++; LoadDataAsync(); }

    // --- FORMATTER (FIXED: PROFILE FORMAT INCLUDED) ---

    winrt::hstring MainWindow::FormatModelData(IInspectable const& item)
    {
        if (auto u = item.try_as<winrt::LeafEyeCore::UserModel>())
            return std::format(L"[USER] ID: {} | Username: {}", u.Id(), u.Username().c_str()).c_str();

        // Perbaikan: Tambahkan formatter untuk Profile
        if (auto p = item.try_as<winrt::LeafEyeCore::ProfileModel>())
            return std::format(L"[PROFILE] ID: {} | Nama: {} | Role: {} | Avatar: {}", p.Id(), p.Fullname().c_str(), p.Role(), p.AvatarPath().c_str()).c_str();

        if (auto h = item.try_as<winrt::LeafEyeCore::HistoryModel>())
            return std::format(L"[HISTORY] ID: {} | Waktu: {} | Total File: {}", h.Id(), UnixToDateString(h.Date()).c_str(), h.TotalFiles()).c_str();

        if (auto f = item.try_as<winrt::LeafEyeCore::FileHistoryModel>())
            return std::format(L"[FILE] ID: {} | Nama: {} | Skor: {:.2f} | Dibuat: {}", f.Id(), f.FileName().c_str(), f.ConfidenceScore(), UnixToDateString(f.DateCreated()).c_str()).c_str();

        return L"Unknown Data Type";
    }

    // --- UTILS ---

    void MainWindow::ShowStatus(winrt::hstring const& message, bool isError) {
        StatusText().Text(message);
        StatusText().Foreground(isError ? SolidColorBrush(winrt::Microsoft::UI::Colors::Red()) : SolidColorBrush(winrt::Microsoft::UI::Colors::Green()));
    }

    winrt::LeafEyeCore::BoxType MainWindow::GetSelectedBoxType() {
        int index = BoxSelector().SelectedIndex();
        switch (index) {
        case 0: return winrt::LeafEyeCore::BoxType::User;
        case 1: return winrt::LeafEyeCore::BoxType::Profile;
        case 2: return winrt::LeafEyeCore::BoxType::History;
        case 3: return winrt::LeafEyeCore::BoxType::FileHistory;
        default: return winrt::LeafEyeCore::BoxType::User;
        }
    }

    int64_t MainWindow::DateTimeToUnix(winrt::Windows::Foundation::DateTime dt) {
        return (dt.time_since_epoch().count() - 116444736000000000LL) / 10000000LL;
    }
}