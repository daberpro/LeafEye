#include "pch.h"
#include "FileHistoryDetailDialog.xaml.h"
#if __has_include("FileHistoryDetailDialog.g.cpp")
#include "FileHistoryDetailDialog.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <chrono>
#include <format>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

namespace winrt::LeafEye::implementation
{
    hstring FormatFileSize(uint32_t sizeInBytes)
    {
        if (sizeInBytes < 1024)
            return hstring(std::to_wstring(sizeInBytes) + L" B");
        else if (sizeInBytes < 1024 * 1024)
            return hstring(std::to_wstring(sizeInBytes / 1024) + L" KB");
        else
        {
            double mb = sizeInBytes / (1024.0 * 1024.0);
            std::wstringstream wss;
            wss << std::fixed << std::setprecision(2) << mb << L" MB";
            return hstring(wss.str());
        }
    }

    hstring FormatFileTime(int64_t fileTime)
    {
        if (fileTime <= 0)
            return L"N/A";

        // Konversi dari epoch milliseconds menggunakan chrono
        std::chrono::sys_time<std::chrono::milliseconds> tp{ std::chrono::milliseconds{fileTime} };

        // Format menjadi string "YYYY-MM-DD HH:MM" menggunakan std::format dari C++20
        // winrt::to_hstring akan secara otomatis menangani konversi dari std::string ke hstring
        return winrt::to_hstring(std::format("{:%Y-%m-%d %H:%M}", tp));
    }

    FileHistoryDetailDialog::FileHistoryDetailDialog(LeafEyeCore::FileHistoryModel const& model)
        : m_model(model)
    {
        InitializeComponent();
        txtFileName().Text(model.FileName());
        txtFilePath().Text(model.FilePath());
        txtDisease().Text(model.Disease());
        txtFileSize().Text(FormatFileSize(model.FileSize()));

        // Memanggil konverter waktu yang baru
        txtDateCreated().Text(FormatFileTime(model.DateCreated()));
        txtDateModified().Text(FormatFileTime(model.DateModified()));

        std::wstringstream confStream;
        confStream << std::fixed << std::setprecision(2)
            << (model.ConfidenceScore() * 100.0f) << L"%";
        txtConfidence().Text(hstring(confStream.str()));

        // Set gambar dari file path
        try
        {
            auto uri = Windows::Foundation::Uri(model.FilePath());
            auto bitmap = Microsoft::UI::Xaml::Media::Imaging::BitmapImage(uri);
            imgPreview().Source(bitmap);
        }
        catch (...) { /* gambar tidak tersedia, biarkan kosong */ }
    }
}