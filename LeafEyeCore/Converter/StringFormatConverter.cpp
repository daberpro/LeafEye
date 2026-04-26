#include "pch.h"
#include "StringFormatConverter.h"
#include "StringFormatConverter.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    winrt::Windows::Foundation::IInspectable StringFormatConverter::Convert(
        winrt::Windows::Foundation::IInspectable const& value,
        winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/,
        winrt::Windows::Foundation::IInspectable const& parameter,
        winrt::hstring const& /*language*/)
    {
        winrt::hstring textValue = L"";

        if (auto str = value.try_as<winrt::hstring>()) {
            textValue = *str;
        }

        winrt::hstring formatStr = L"{0}";

        if (auto paramStr = parameter.try_as<winrt::hstring>()) {
            formatStr = *paramStr;
        }

        std::wstring result{ formatStr };
        size_t pos = result.find(L"{0}");
        if (pos != std::wstring::npos) {
            result.replace(pos, 3, textValue);
        }

        return winrt::box_value(result);
    }

    winrt::Windows::Foundation::IInspectable StringFormatConverter::ConvertBack(
        winrt::Windows::Foundation::IInspectable const& /*value*/,
        winrt::Windows::UI::Xaml::Interop::TypeName const& /*targetType*/,
        winrt::Windows::Foundation::IInspectable const& /*parameter*/,
        winrt::hstring const& /*language*/)
    {
        throw winrt::hresult_not_implemented(); // Biasanya tidak dipakai untuk Mode OneWay
    }
}
