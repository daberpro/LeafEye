#pragma once
#include "FileHistoryModel.g.h"

namespace winrt::LeafEyeCore::implementation
{
    struct FileHistoryModel : FileHistoryModelT<FileHistoryModel>
    {
		FileHistoryModel() = default;
        FileHistoryModel(uint64_t const& id, hstring const& fileName, uint32_t const& fileSize, int64_t const& dateCreated, int64_t const& dateModified, float const& confidenceScore, const hstring& filePath, const hstring& disease);

        uint64_t Id();
        void Id(uint64_t value);
        hstring FileName();
        void FileName(hstring const& value);
        hstring FilePath();
        void FilePath(hstring const& value);
        hstring Disease();
        void Disease(hstring const& value);
        uint32_t FileSize();
        void FileSize(uint32_t value);
        int64_t DateCreated();
        void DateCreated(int64_t value);
        int64_t DateModified();
        void DateModified(int64_t value);
        winrt::Windows::Foundation::Collections::IVector<uint32_t> DetectedDiseases();
        void DetectedDiseases(winrt::Windows::Foundation::Collections::IVector<uint32_t> const& value);
        float ConfidenceScore();
        void ConfidenceScore(float value);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    private:
        uint64_t m_id{ 0 };
        hstring m_fileName;
        hstring m_filePath;
        hstring m_disease;
        uint32_t m_fileSize{ 0 };
        int64_t m_dateCreated{ 0 };
        int64_t m_dateModified{ 0 };
        winrt::Windows::Foundation::Collections::IVector<uint32_t> m_detectedDiseases{ nullptr };
        float m_confidenceScore{ 0.0f };

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        void RaisedPropertyChanged(const winrt::hstring& property_name);
    };
}

namespace winrt::LeafEyeCore::factory_implementation
{
    struct FileHistoryModel : FileHistoryModelT<FileHistoryModel, implementation::FileHistoryModel>
    {
    };
}