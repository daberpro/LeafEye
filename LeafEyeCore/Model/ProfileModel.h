#pragma once
#include "ProfileModel.g.h"

namespace winrt::LeafEyeCore::implementation
{
    struct ProfileModel : ProfileModelT<ProfileModel>
    {
		ProfileModel() = default;
        ProfileModel(uint64_t const& id, hstring const& fullname, hstring const& avatarPath, int32_t const& role);

        uint64_t Id();
        void Id(uint64_t value);
        hstring Fullname();
        void Fullname(hstring const& value);
        hstring AvatarPath();
        void AvatarPath(hstring const& value);
        int32_t Role();
        void Role(int32_t value);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    private:
        uint64_t m_id{ 0 };
        hstring m_fullname;
        hstring m_avatarPath;
        int32_t m_role{ 0 };

        winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_propertyChanged;
        void RaisedPropertyChanged(const winrt::hstring& property_name);
    };
}

namespace winrt::LeafEyeCore::factory_implementation
{
    struct ProfileModel : ProfileModelT<ProfileModel, implementation::ProfileModel>
    {
    };
}