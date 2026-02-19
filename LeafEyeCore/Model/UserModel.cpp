#include "pch.h"
#include "UserModel.h"
#include "UserModel.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    UserModel::UserModel(hstring const& username, hstring const& password, bool const& isAdmin)
        : m_username(username), m_password(password), m_isAdmin(isAdmin)
    {
    }

    uint64_t UserModel::Id()
    {
        return m_id;
    }
    void UserModel::Id(uint64_t value)
    {
        m_id = value;
    }

    hstring UserModel::Username()
    {
        return m_username;
    }
    void UserModel::Username(hstring const& value)
    {
        m_username = value;
    }

    hstring UserModel::Password()
    {
        return m_password;
    }
    void UserModel::Password(hstring const& value)
    {
        m_password = value;
    }

    bool UserModel::IsAdmin()
    {
        return m_isAdmin;
    }
    void UserModel::IsAdmin(bool value)
    {
        m_isAdmin = value;
    }
}