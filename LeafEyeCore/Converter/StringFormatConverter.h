#pragma once
#include "StringFormatConverter.g.h"

namespace winrt::LeafEyeCore::implementation
{
    struct StringFormatConverter : StringFormatConverterT<StringFormatConverter>
    {
        StringFormatConverter() = default;

        winrt::Windows::Foundation::IInspectable Convert(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
        winrt::Windows::Foundation::IInspectable ConvertBack(winrt::Windows::Foundation::IInspectable const& value, winrt::Windows::UI::Xaml::Interop::TypeName const& targetType, winrt::Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::LeafEyeCore::factory_implementation
{
    struct StringFormatConverter : StringFormatConverterT<StringFormatConverter, implementation::StringFormatConverter>
    {
    };
}
