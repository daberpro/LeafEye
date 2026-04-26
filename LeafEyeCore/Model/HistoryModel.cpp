#include "pch.h"
#include "HistoryModel.h"
#include "HistoryModel.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    HistoryModel::HistoryModel(uint64_t const& id, int64_t const& date, uint32_t const& totalFiles, uint32_t const& status, hstring const& output_folder)
         : m_id(id), m_date(date), m_totalFiles(totalFiles), m_status(status), m_outputFolder(output_folder)
     {
	}
    uint64_t HistoryModel::Id()
    {
        return m_id;
    }
    void HistoryModel::Id(uint64_t value)
    {
        if (m_id != value) {
            m_id = value;
            RaisedPropertyChanged(L"Id");
        }
    }

    int64_t HistoryModel::Date()
    {
        return m_date;
    }
    void HistoryModel::Date(int64_t value)
    {
        m_date = value;
    }

    uint32_t HistoryModel::TotalFiles()
    {
        return m_totalFiles;
    }
    void HistoryModel::TotalFiles(uint32_t value)
    {
        if (m_totalFiles != value) {
            m_totalFiles = value;
            RaisedPropertyChanged(L"TotalFiles");
        }
    }

    uint32_t HistoryModel::Status()
    {
        return m_status;
    }
    void HistoryModel::Status(uint32_t value)
    {
        if (m_status != value) {
            m_status = value;
            RaisedPropertyChanged(L"Status");
        }
    }

    winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> HistoryModel::Files()
    {
        return m_files;
    }

    void HistoryModel::Files(winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> const& value)
    {
        m_files = value;
    }

    winrt::event_token HistoryModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }
    void HistoryModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void HistoryModel::RaisedPropertyChanged(const winrt::hstring& property_name) {
        if (m_propertyChanged)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
        }
    }

    void HistoryModel::OutputFolder(hstring const& value)
    {
        if (m_outputFolder != value) {
            m_outputFolder = value;
            RaisedPropertyChanged(L"OutputFolder");
        }
    }

    hstring HistoryModel::OutputFolder()
    {
        return m_outputFolder;
    }
}