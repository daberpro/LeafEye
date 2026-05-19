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
		if (m_username != value) {
			m_username = value;
			RaisedPropertyChanged(L"Username");
		}
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
		if (m_isAdmin != value) {
			m_isAdmin = value;
			RaisedPropertyChanged(L"IsAdmin");
		}

	}
	winrt::event_token UserModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
	{
		return m_propertyChanged.add(handler);
	}
	void UserModel::PropertyChanged(winrt::event_token const& token) noexcept
	{
		m_propertyChanged.remove(token);
	}

	void UserModel::RaisedPropertyChanged(const winrt::hstring& property_name) {
		if (m_propertyChanged)
		{
			m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
		}
	}
}