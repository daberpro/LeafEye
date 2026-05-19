#include "pch.h"
#include "FileItemsHomePageModel.h"
#include "FileItemsHomePageModel.g.cpp"


namespace winrt::LeafEye::implementation
{
    FileItemsHomePageModel::FileItemsHomePageModel(hstring const& filePath, bool selected, hstring const& fileName, uint32_t fileSize, uint64_t dateCreated, uint64_t dateModified)
    : m_filePath(filePath), m_isSelected(selected), m_fileName(fileName), m_fileSize(fileSize), m_dateCreated(dateCreated), m_dateModified(dateModified){
    }

    void FileItemsHomePageModel::FilePath(hstring const& value)
    {
        if (m_filePath != value) {
		    m_filePath = value;
			RaisedPropertyChanged(L"FilePath");
        }
    }

    hstring FileItemsHomePageModel::FilePath()
    {
		return m_filePath;
    }

    void FileItemsHomePageModel::Selected(bool value)
    {
        if (m_isSelected != value) {
		    m_isSelected = value;
			RaisedPropertyChanged(L"Selected");
        }
    }

    bool FileItemsHomePageModel::Selected()
    {
		return m_isSelected;
    }

    void FileItemsHomePageModel::FileName(hstring const& value)
    {
        if (m_fileName != value) {
		    m_fileName = value;
			RaisedPropertyChanged(L"FileName");
        }
    }

    hstring FileItemsHomePageModel::FileName()
    {
        return m_fileName;
    }

    void FileItemsHomePageModel::FileSize(uint32_t value)
    {
        if (m_fileSize != value) {
            m_fileSize = value;
			RaisedPropertyChanged(L"FileSize");
        }
    }

    uint32_t FileItemsHomePageModel::FileSize()
    {
		return m_fileSize;
    }

    void FileItemsHomePageModel::DateCreated(int64_t value)
    {
        if (m_dateCreated != value) {
		    m_dateCreated = value;
            RaisedPropertyChanged(L"DateCreated");
        }
    }

    int64_t FileItemsHomePageModel::DateCreated()
    {
		return m_dateCreated;
    }

    void FileItemsHomePageModel::DateModified(int64_t value)
    {
        if (m_dateModified != value) {
		    m_dateModified = value;
			RaisedPropertyChanged(L"DateModified");
        }
    }

    int64_t FileItemsHomePageModel::DateModified()
    {
		return m_dateModified;
    }

    winrt::event_token FileItemsHomePageModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return m_propertyChanged.add(handler);
    }

    void FileItemsHomePageModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
		m_propertyChanged.remove(token);
    }

    void FileItemsHomePageModel::RaisedPropertyChanged(const winrt::hstring& propertyName)
    {
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ propertyName });
	}
}
