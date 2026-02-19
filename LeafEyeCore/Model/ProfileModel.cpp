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
        m_fullname = value;
    }

    hstring ProfileModel::AvatarPath()
    {
        return m_avatarPath;
    }
    void ProfileModel::AvatarPath(hstring const& value)
    {
        m_avatarPath = value;
    }

    int32_t ProfileModel::Role()
    {
        return m_role;
    }
    void ProfileModel::Role(int32_t value)
    {
        m_role = value;
    }
}