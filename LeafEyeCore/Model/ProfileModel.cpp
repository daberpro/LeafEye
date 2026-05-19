#include "pch.h"
#include "ProfileModel.h"
#include "ProfileModel.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    ProfileModel::ProfileModel(uint64_t const& id, hstring const& fullname, hstring const& avatarPath, int32_t const& role)
        : m_id(id), m_fullname(fullname), m_avatarPath(avatarPath), m_role(role)
    {
	}
    uint64_t ProfileModel::Id()
    {
        return m_id;
    }
    void ProfileModel::Id(uint64_t value)
    {
        m_id = value;
    }

    hstring ProfileModel::Fullname()
    {
        return m_fullname;
    }
    void ProfileModel::Fullname(hstring const& value)
    {
        if (m_fullname != value) {
            m_fullname = value;
            RaisedPropertyChanged(L"Fullname");
        }
    }

    hstring ProfileModel::AvatarPath()
    {
        return m_avatarPath;
    }
    void ProfileModel::AvatarPath(hstring const& value)
    {
        if (m_avatarPath != value) {
            m_avatarPath = value;
            RaisedPropertyChanged(L"AvatarPath");
        }
    }

    int32_t ProfileModel::Role()
    {
        return m_role;
    }
    void ProfileModel::Role(int32_t value)
    {
        if (m_role != value) {
            m_role = value;
            RaisedPropertyChanged(L"Role");
        }
    }
    winrt::event_token ProfileModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }
    void ProfileModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void ProfileModel::RaisedPropertyChanged(const winrt::hstring& property_name) {
        if (m_propertyChanged)
        {
            m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
        }
    }
}