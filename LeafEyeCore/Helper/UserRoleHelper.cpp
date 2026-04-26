#include "pch.h"
#include "UserRoleHelper.h"
#include "UserRoleHelper.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    hstring UserRoleHelper::ConvertUserRoleToString(winrt::LeafEyeCore::UserRole const& role)
    {
        switch (role) {
            case winrt::LeafEyeCore::UserRole::Admin:
				return L"Admin";
			case winrt::LeafEyeCore::UserRole::User:
                return L"User";
			default:
				return L"Unknown";
        }
    }

	hstring UserRoleHelper::ConvertUserIntRoleToString(int32_t role)
	{
		switch (role) {
		case 0:
			return L"Admin";
		case 1:
			return L"User";
		default:
			return L"Unknown";
		}
	}
}
