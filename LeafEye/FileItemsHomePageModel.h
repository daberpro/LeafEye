#pragma once
#include "FileItemsHomePageModel.g.h"


namespace winrt::LeafEye::implementation
{
    struct FileItemsHomePageModel : FileItemsHomePageModelT<FileItemsHomePageModel>
    {
    private:

		void RaisedPropertyChanged(const winrt::hstring& propertyName);

		hstring m_filePath{ L"" };
		bool m_isSelected{ false };
		hstring m_fileName{ L"" };
		uint32_t m_fileSize{ 0 };
		int64_t m_dateCreated{ 0 };
		int64_t m_dateModified{ 0 };
		winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;

    public:
        FileItemsHomePageModel() = default;

        FileItemsHomePageModel(hstring const& filePath, bool selected, hstring const& fileName, uint32_t fileSize, uint64_t dateCreated, uint64_t dateModified);
        void FilePath(hstring const& value);
        hstring FilePath();
        void Selected(bool value);
        bool Selected();
        void FileName(hstring const& value);
        hstring FileName();
        void FileSize(uint32_t value);
        uint32_t FileSize();
        void DateCreated(int64_t value);
        int64_t DateCreated();
        void DateModified(int64_t value);
        int64_t DateModified();
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::LeafEye::factory_implementation
{
    struct FileItemsHomePageModel : FileItemsHomePageModelT<FileItemsHomePageModel, implementation::FileItemsHomePageModel>
    {
    };
}
