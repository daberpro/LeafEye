#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::LeafEye::implementation
{

    winrt::Microsoft::UI::Xaml::Window App::window{ nullptr };

    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {

        // ============================================================
        // Pre‑load OUR leafeye_onnxruntime.dll BEFORE WinUI activates
        // ============================================================
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::filesystem::path appDir = std::filesystem::path(exePath).parent_path();
        std::wstring dllPath = (appDir / L"leafeye_onnxruntime.dll").wstring();

        // Force Windows to search DLL dependencies in our folder first
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        AddDllDirectory(appDir.c_str());

        HMODULE hDll = LoadLibraryExW(dllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!hDll) {
            OutputDebugString(L"FATAL: Could not load leafeye_onnxruntime.dll. Is it in AppX?\n");
        }

        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
    }

    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        window = make<MainWindow>();
        window.Closed([this](auto const&, auto const&)
        {
            LeafEye::Utils::AppSession::ClearSession();
        });
        window.Activate();
    }
}
