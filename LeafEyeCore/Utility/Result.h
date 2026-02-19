#pragma once
#include "Result.g.h"
namespace winrt::LeafEyeCore::implementation
{
    struct Result : ResultT<Result>
    {
        Result() = default;

        Result(bool is_error, int32_t error_code, hstring const& message, winrt::Windows::Foundation::IInspectable const& result_value);
        bool IsError();
        void IsError(bool value);
        int32_t ErrorCode();
        void ErrorCode(int32_t value);
        hstring Message();
        void Message(hstring const& value);
        winrt::Windows::Foundation::IInspectable ResultValue();
        void ResultValue(winrt::Windows::Foundation::IInspectable const& value);

    private:

        bool m_is_error{ false };
        int32_t m_error_code{ 0 };
        winrt::hstring m_message;
		winrt::Windows::Foundation::IInspectable m_result_value{ nullptr };

    };
}
namespace winrt::LeafEyeCore::factory_implementation
{
    struct Result : ResultT<Result, implementation::Result>
    {
    };
}
