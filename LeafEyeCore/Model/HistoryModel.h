#pragma once
#include "HistoryModel.g.h"
#include "FileHistoryModel.h"

namespace winrt::LeafEyeCore::implementation
{
    struct HistoryModel : HistoryModelT<HistoryModel>
    {
		HistoryModel() = default;
        HistoryModel(uint64_t const& id, int64_t const& date, uint32_t const& totalFiles, uint32_t const& status, hstring const& output_folder);

        uint64_t Id();
        void Id(uint64_t value);
        int64_t Date();
        void Date(int64_t value);
        uint32_t TotalFiles();
        void TotalFiles(uint32_t value);
        uint32_t Status();
        void Status(uint32_t value);
        void OutputFolder(hstring const& value);
        hstring OutputFolder();
        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> Files();
        void Files(winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> const& value);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    private:
        uint64_t m_id{ 0 };
        int64_t m_date{ 0 };
        uint32_t m_totalFiles{ 0 };
        uint32_t m_status{ 0 };
        hstring m_outputFolder;
        winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> m_files{ nullptr };

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        void RaisedPropertyChanged(const winrt::hstring& property_name);
    };
}

namespace winrt::LeafEyeCore::factory_implementation
{
    struct HistoryModel : HistoryModelT<HistoryModel, implementation::HistoryModel>
    {
    };
}