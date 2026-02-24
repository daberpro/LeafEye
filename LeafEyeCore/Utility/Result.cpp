#include "pch.h"
#include "Result.h"
#include "Result.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    Result::Result(bool is_error, bool is_value_exists, int32_t error_code, hstring const& message, winrt::Windows::Foundation::IInspectable const& result_value):
    m_is_error(is_error), m_error_code(error_code), m_message(message), m_result_value(result_value), m_is_value_exists(is_value_exists){}
    
    bool Result::IsError()
    {
        return m_is_error;
    }

    void Result::IsError(bool value)
    {
		m_is_error = value;
    }

    bool Result::IsValueExists()
    {
        return m_is_value_exists;
    }

    void Result::IsValueExists(bool value)
    {
        m_is_value_exists = value;
    }

    int32_t Result::ErrorCode()
    {
		return m_error_code;
    }
    
    void Result::ErrorCode(int32_t value)
    {
		m_error_code = value;
    }
    
    hstring Result::Message()
    {
		return m_message;
    }
    
    void Result::Message(hstring const& value)
    {
		m_message = value;
    }
    
    winrt::Windows::Foundation::IInspectable Result::ResultValue()
    {
        return m_result_value;
    }
    
    void Result::ResultValue(winrt::Windows::Foundation::IInspectable const& value)
    {
		m_result_value = value;
    }
}
