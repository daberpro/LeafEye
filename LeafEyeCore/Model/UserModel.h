#pragma once
#include "UserModel.g.h"

namespace winrt::LeafEyeCore::implementation
{
    struct UserModel : UserModelT<UserModel>
    {
		UserModel() = default;
        UserModel(hstring const& username, hstring const& password, bool const& isAdmin);

        uint64_t Id();
        void Id(uint64_t value);
        hstring Username();
        void Username(hstring const& value);
        hstring Password();
        void Password(hstring const& value);
        bool IsAdmin();
        void IsAdmin(bool value);

    private:
        uint64_t m_id{ 0 };
        hstring m_username;
        hstring m_password;
        bool m_isAdmin{ false };
    };
}

namespace winrt::LeafEyeCore::factory_implementation
{
    struct UserModel : UserModelT<UserModel, implementation::UserModel>
    {
    };
}