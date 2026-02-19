#include "pch.h"
#include "HistoryModel.h"
#include "HistoryModel.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    HistoryModel::HistoryModel(uint64_t const& id, int64_t const& date, uint32_t const& totalFiles, uint32_t const& status)
         : m_id(id), m_date(date), m_totalFiles(totalFiles), m_status(status)
     {
	}
    uint64_t HistoryModel::Id()
    {
        return m_id;
    }
    void HistoryModel::Id(uint64_t value)
    {
        m_id = value;
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
        m_totalFiles = value;
    }

    uint32_t HistoryModel::Status()
    {
        return m_status;
    }
    void HistoryModel::Status(uint32_t value)
    {
        m_status = value;
    }

    winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> HistoryModel::Files()
    {
        return m_files;
    }
    void HistoryModel::Files(winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> const& value)
    {
        m_files = value;
    }
}