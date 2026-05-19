#include "pch.h"
#include "UserRoleHelper.h"
#include "UserRoleHelper.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    hstring UserRoleHelper::ConvertUserRoleToString(winrt::LeafEyeCore::UserRole const& role)
    {
        switch (role) {
            case winrt::LeafEyeCore::UserRole::Manager:
				return L"Manager";
			case winrt::LeafEyeCore::UserRole::Staff:
                return L"Staff";
			case winrt::LeafEyeCore::UserRole::Coordinator:
                return L"Coordinator";
			case winrt::LeafEyeCore::UserRole::Guest:
                return L"Guest";
			default:
				return L"Unknown";
        }
    }

	hstring UserRoleHelper::ConvertUserIntRoleToString(int32_t role)
	{
		switch (role) {
		case 0:
			return L"Manager";
		case 1:
			return L"Staff";
		case 2:
			return L"Coordinator";
		case 3:
			return L"Guest";
		default:
			return L"Unknown";
		}
	}
}
