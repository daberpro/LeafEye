#include "pch.h"
#include "FileHistoryModel.h"
#include "FileHistoryModel.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    FileHistoryModel::FileHistoryModel(uint64_t const& id, hstring const& fileName, uint32_t const& fileSize, int64_t const& dateCreated, int64_t const& dateModified, float const& confidenceScore, const hstring& filePath, const hstring& disease)
        : m_id(id), m_fileName(fileName), m_fileSize(fileSize), m_dateCreated(dateCreated), m_dateModified(dateModified), m_confidenceScore(confidenceScore), m_filePath(filePath), m_disease(disease)
    {
	}
    uint64_t FileHistoryModel::Id()
    {
        return m_id;
    }
    void FileHistoryModel::Id(uint64_t value)
    {
        m_id = value;
    }

    hstring FileHistoryModel::Disease()
    {
        return m_disease;
    }
    void FileHistoryModel::Disease(hstring const& value)
    {
        m_disease = value;
    }

    hstring FileHistoryModel::FilePath()
    {
        return m_filePath;
    }
    void FileHistoryModel::FilePath(hstring const& value)
    {
        m_filePath = value;
    }

    hstring FileHistoryModel::FileName()
    {
        return m_fileName;
    }
    void FileHistoryModel::FileName(hstring const& value)
    {
        m_fileName = value;
    }

    uint32_t FileHistoryModel::FileSize()
    {
        return m_fileSize;
    }
    void FileHistoryModel::FileSize(uint32_t value)
    {
        m_fileSize = value;
    }

    int64_t FileHistoryModel::DateCreated()
    {
        return m_dateCreated;
    }
    void FileHistoryModel::DateCreated(int64_t value)
    {
        m_dateCreated = value;
    }

    int64_t FileHistoryModel::DateModified()
    {
        return m_dateModified;
    }
    void FileHistoryModel::DateModified(int64_t value)
    {
        m_dateModified = value;
    }

    winrt::Windows::Foundation::Collections::IVector<uint32_t> FileHistoryModel::DetectedDiseases()
    {
        return m_detectedDiseases;
    }
    void FileHistoryModel::DetectedDiseases(winrt::Windows::Foundation::Collections::IVector<uint32_t> const& value)
    {
        m_detectedDiseases = value;
    }

    float FileHistoryModel::ConfidenceScore()
    {
        return m_confidenceScore;
    }
    void FileHistoryModel::ConfidenceScore(float value)
    {
        m_confidenceScore = value;
    }
    winrt::event_token FileHistoryModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }
    void FileHistoryModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void FileHistoryModel::RaisedPropertyChanged(const winrt::hstring& property_name) {
        if (m_propertyChanged)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
        }
    }
}