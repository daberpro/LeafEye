#include "pch.h"
#include "ProfilePage.xaml.h"
#if __has_include("ProfilePage.g.cpp")
#include "ProfilePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::LeafEye::implementation
{
    int32_t ProfilePage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ProfilePage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
