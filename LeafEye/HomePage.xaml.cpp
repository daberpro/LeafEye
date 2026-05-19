#include <windows.h>
#include <filesystem>
#include "pch.h"
#include "HomePage.xaml.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif

namespace winrt::LeafEye::implementation
{
    // =========================================================================
    // HELPER: Win2D untuk Menggambar Kotak Deteksi (Standalone Function)
    // =========================================================================
    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Storage::StorageFile> DrawAndSaveImageAsync(
        winrt::Windows::Storage::StorageFile originalFile,
        winrt::Windows::Storage::StorageFolder destinationFolder,
        std::vector<winrt::LeafEye::Utils::DetectionResult> detections,
        winrt::hstring newFileName)
    {
        winrt::Microsoft::Graphics::Canvas::CanvasDevice device = winrt::Microsoft::Graphics::Canvas::CanvasDevice::GetSharedDevice();
        winrt::Microsoft::Graphics::Canvas::CanvasBitmap bitmap = co_await winrt::Microsoft::Graphics::Canvas::CanvasBitmap::LoadAsync(device, originalFile.Path());

        winrt::Microsoft::Graphics::Canvas::CanvasRenderTarget renderTarget(device, bitmap.Size().Width, bitmap.Size().Height, bitmap.Dpi());
        {
            winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession ds = renderTarget.CreateDrawingSession();

            ds.DrawImage(bitmap);

            float scaleX = bitmap.Size().Width / 256.0f;
            float scaleY = bitmap.Size().Height / 256.0f;

            // Disease names as wide strings
            static const std::string class_names[] = {
                "Healthy",              // 0
                "Leaf Algal",           // 1
                "Leaf Blight",          // 2
                "Leaf Colletotrichum",  // 3
                "Leaf Phomopsis",       // 4
                "Leaf Rhizoctonia"      // 5
            };

            for (const auto& det : detections) {
                winrt::Windows::Foundation::Rect rect{
                    det.x1 * scaleX,
                    det.y1 * scaleY,
                    (det.x2 - det.x1) * scaleX,
                    (det.y2 - det.y1) * scaleY
                };

                ds.DrawRectangle(rect, winrt::Windows::UI::Colors::Red(), 3.0f);

                // Build label: "ClassName (82%)"
                std::string name = (det.class_id >= 0 && det.class_id <= 5)
                    ? class_names[det.class_id]
                    : "Unknown";
                int percent = static_cast<int>(std::round(det.confidence * 100.0f));
                winrt::hstring label = winrt::to_hstring(name + " (" + std::to_string(percent) + "%)");

                ds.DrawText(label, det.x1 * scaleX,
                    (det.y1 * scaleY) - 20.0f,
                    winrt::Windows::UI::Colors::Yellow());
            }
        }

        winrt::Windows::Storage::StorageFile newFile = co_await destinationFolder.CreateFileAsync(newFileName, winrt::Windows::Storage::CreationCollisionOption::GenerateUniqueName);
        winrt::Windows::Storage::Streams::IRandomAccessStream stream = co_await newFile.OpenAsync(winrt::Windows::Storage::FileAccessMode::ReadWrite);

        co_await renderTarget.SaveAsync(stream, winrt::Microsoft::Graphics::Canvas::CanvasBitmapFileFormat::Jpeg);

        co_return newFile;
    }

    // =========================================================================
    // METODE KELAS HOMEPAGE
    // =========================================================================

    winrt::hstring HomePage::ConvertToDateTimeString(uint64_t fileTime)
    {
        if (fileTime <= 0) {
            return L"-";
        }
        auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(std::chrono::milliseconds(fileTime));
        auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), tp };
        return winrt::to_hstring(std::format("{:%d/%m/%Y %I:%M %p}", localTime));
    }

    winrt::hstring HomePage::ConvertToFileSizeString(uint64_t fileTime)
    {
        return winrt::to_hstring(std::format("{} KB", fileTime));
    }

    HomePage::HomePage()
    {
        InitializeComponent();

        m_processButtonInfo = L"Silahkan pilih output folder dan ambil input gambar";
        m_historyInfo = winrt::LeafEyeCore::HistoryModel(
            0, // objectbox id
            0, // date (miliseconds)
            0, // total files
            0, // status enum
            L"Silahkan pilih folder output" // output folder
        );

        m_filesMetaInfo = winrt::single_threaded_observable_vector<winrt::LeafEye::FileItemsHomePageModel>();
        m_filesMetaInfo.VectorChanged([this](auto&&, auto&&) {

            if (m_filesMetaInfo.Size() <= 0) {
                EmptyFileHistoryListViewText().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Visible);
                ProcessButtonInfo(L"Silahkan pilih input gambar");
                IsProcessButtonEnabled(false);
                return;
            }
            EmptyFileHistoryListViewText().Visibility(winrt::Microsoft::UI::Xaml::Visibility::Collapsed);
            });
        SelectAllFilesCheckBox().Unchecked([this](auto&&, auto&&) {
            for (const auto& item : m_filesMetaInfo) {
                item.Selected(false);
            }
            });
        SelectAllFilesCheckBox().Checked([this](auto&&, auto&&) {
            for (const auto& item : m_filesMetaInfo) {
                item.Selected(true);
            }
            });
        FileHistoryListView().ItemsSource(m_filesMetaInfo);
    }

    winrt::LeafEyeCore::HistoryModel HomePage::HistoryInfo() {
        return m_historyInfo;
    };

    void HomePage::OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {

    }

    winrt::fire_and_forget HomePage::DropTarget_DragOver(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
    {
        e.AcceptedOperation(winrt::Windows::ApplicationModel::DataTransfer::DataPackageOperation::Copy);
        e.DragUIOverride().Caption(L"Lepaskan untuk menyalin file");
        e.DragUIOverride().IsContentVisible(true);
        co_return;
    }

    winrt::fire_and_forget HomePage::DropTarget_DragLeave(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
    {
        co_return;
    }

    winrt::fire_and_forget HomePage::DropTarget_Drop(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::DragEventArgs const& e)
    {
        if (m_filesMetaInfo.Size() > 0) {
            m_filesMetaInfo.Clear();
        }

        auto defferal = e.GetDeferral();
        if (e.DataView().Contains(winrt::Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems())) {
            auto items = co_await e.DataView().GetStorageItemsAsync();
            if (items.Size() > 0) {
                for (auto const& item : items) {
                    auto storageItem = item.as<winrt::Windows::Storage::StorageFile>();
                    auto basicProperties = co_await storageItem.GetBasicPropertiesAsync();
                    auto fileName = storageItem.Name();
                    m_filesMetaInfo.Append(
                        winrt::LeafEye::FileItemsHomePageModel(
                            storageItem.Path(),
                            false,
                            fileName,
                            basicProperties.Size() / 1024, // Convert to KB
                            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(winrt::clock::to_sys(basicProperties.ItemDate()).time_since_epoch()).count()),
                            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(winrt::clock::to_sys(basicProperties.DateModified()).time_since_epoch()).count())
                        )
                    );
                }
            }
        }
        defferal.Complete();
    }

    void HomePage::DeleteSelectedFilesButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        for (int32_t i = m_filesMetaInfo.Size() - 1; i >= 0; --i)
        {
            auto file = m_filesMetaInfo.GetAt(i);
            if (file.Selected())
            {
                m_filesMetaInfo.RemoveAt(i);
            }
        }
        SelectAllFilesCheckBox().IsChecked(false);
    }

    winrt::fire_and_forget HomePage::FilePicker_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto lifetime = get_strong();
        winrt::Windows::Storage::Pickers::FileOpenPicker filePicker;
        auto mainWindow = winrt::LeafEye::implementation::App::Window();
        auto native{ mainWindow.as<::IWindowNative>() };
        HWND hwnd{ 0 };
        native->get_WindowHandle(&hwnd);
        auto init{ filePicker.as<::IInitializeWithWindow>() };
        init->Initialize(hwnd);

        filePicker.ViewMode(winrt::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
        filePicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);
        filePicker.FileTypeFilter().Append(L".jpg");
        filePicker.FileTypeFilter().Append(L".png");
        filePicker.FileTypeFilter().Append(L".jpeg");

        m_files = co_await filePicker.PickMultipleFilesAsync();
        if (m_files.Size() > 0) {
            m_filesMetaInfo.Clear();
            for (const auto& file : m_files) {
                if (file) {
                    auto basicProperties = co_await file.GetBasicPropertiesAsync();
                    auto fileName = file.Name();
                    m_filesMetaInfo.Append(
                        winrt::LeafEye::FileItemsHomePageModel(
                            file.Path(),
                            false,
                            fileName,
                            basicProperties.Size() / 1024, // Convert to KB
                            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(winrt::clock::to_sys(basicProperties.ItemDate()).time_since_epoch()).count()),
                            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(winrt::clock::to_sys(basicProperties.DateModified()).time_since_epoch()).count())
                        )
                    );
                }
            }
        }

        if (m_filesMetaInfo.Size() > 0) {
            if (m_historyInfo.OutputFolder() != L"Silahkan pilih folder output") {
                ProcessButtonInfo(L"Siap memproses");
                IsProcessButtonEnabled(true);
            }
            else {
                ProcessButtonInfo(L"Silahkan pilih output folder");
                IsProcessButtonEnabled(false);
            }
        }
    }

    winrt::fire_and_forget HomePage::SelectFolderOutputButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        auto lifetime = get_strong();
        winrt::Windows::Storage::Pickers::FolderPicker folderPicker;
        auto mainWindow = winrt::LeafEye::implementation::App::Window();
        auto native{ mainWindow.as<::IWindowNative>() };
        HWND hwnd{ 0 };
        native->get_WindowHandle(&hwnd);
        auto init{ folderPicker.as<::IInitializeWithWindow>() };
        init->Initialize(hwnd);

        folderPicker.ViewMode(winrt::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
        folderPicker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);

        winrt::Windows::Storage::StorageFolder folder_output = co_await folderPicker.PickSingleFolderAsync();
        if (folder_output) {
            m_historyInfo.OutputFolder(folder_output.Path());

            if (m_filesMetaInfo.Size() > 0) {
                ProcessButtonInfo(L"Siap memproses");
                IsProcessButtonEnabled(true);
            }
            else {
                ProcessButtonInfo(L"Silahkan pilih input gambar");
                IsProcessButtonEnabled(false);
            }
        }
    }

    winrt::fire_and_forget HomePage::ProcessButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        try {
            // 1. Persiapan metadata awal di UI Thread
            auto now = std::chrono::system_clock::now();
            uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            auto history_model = winrt::LeafEyeCore::HistoryModel(
                0, timestamp, m_filesMetaInfo.Size(), 0, m_historyInfo.OutputFolder()
            );

            std::vector<winrt::Windows::Storage::StorageFile> files_to_process;
            std::vector<winrt::LeafEye::FileItemsHomePageModel> meta_to_process;
            for (uint32_t i = 0; i < m_files.Size(); i++) {
                files_to_process.push_back(m_files.GetAt(i));
                meta_to_process.push_back(m_filesMetaInfo.GetAt(i));
            }

            winrt::Windows::Storage::StorageFolder outputRootFolder = co_await winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(m_historyInfo.OutputFolder());
            winrt::Windows::Storage::StorageFolder new_folder = co_await outputRootFolder.CreateFolderAsync(winrt::to_hstring(timestamp), winrt::Windows::Storage::CreationCollisionOption::OpenIfExists);

            // --- TAMBAHAN: Ambil DispatcherQueue & Setup Progress Bar ---
            auto dispatcher = this->DispatcherQueue();
            ProgressBar().Maximum(files_to_process.size());
            ProgressBar().Value(0);
            ProcessButtonInfo(L"Menyiapkan model AI...");
            // -------------------------------------------------------------

            // Simpan konteks UI thread untuk kembali nanti
            winrt::apartment_context ui_thread;

            // ==========================================
            // PINDAH KE BACKGROUND THREAD (AI & Database)
            co_await winrt::resume_background();
            // ==========================================

            if (!ai_detector) {
                ai_detector = std::make_shared<winrt::LeafEye::Utils::LeafEyeDetector>(
                    L"Assets\\leafeye_pipeline_v1.onnx"
                );
            }

            std::vector<winrt::LeafEyeCore::FileHistoryModel> files_history_model;
            files_history_model.reserve(files_to_process.size());

            for (size_t i = 0; i < files_to_process.size(); i++) {
                winrt::Windows::Storage::StorageFile file = files_to_process[i];
                winrt::LeafEye::FileItemsHomePageModel meta = meta_to_process[i];

                // A - E: Preprocessing, Inferensi, Ekstrak, dan Menggambar Kotak...
                winrt::Windows::Foundation::Collections::IVector<float> ivec_tensor = co_await PreprocessImageAsync(file);

                std::vector<float> input_tensor(ivec_tensor.Size());
                ivec_tensor.GetMany(0, input_tensor);

                std::vector<winrt::LeafEye::Utils::DetectionResult> detections = ai_detector->Detect(input_tensor);

                static const std::wstring class_names[] = {
                    L"Leaf_Healthy", L"Leaf_Algal", L"Leaf_Blight", L"Leaf_Colletotrichum", L"Leaf_Phomopsis", L"Leaf_Rhizoctonia"
                };

                float max_conf = 0.0f;
                winrt::hstring top_disease = L"Tidak terdeteksi";
                winrt::Windows::Foundation::Collections::IVector<uint32_t> detected_diseases_vector = winrt::single_threaded_vector<uint32_t>();

                for (const auto& det : detections) {
                    detected_diseases_vector.Append(det.class_id);
                    if (det.confidence > max_conf) {
                        max_conf = det.confidence;
                        top_disease = (det.class_id >= 0 && det.class_id <= 5) ? winrt::hstring(class_names[det.class_id]) : winrt::hstring(L"Unknown");
                    }
                }

                winrt::Windows::Storage::StorageFile saved_annotated_file = co_await DrawAndSaveImageAsync(file, new_folder, detections, file.Name());

                // --- TAMBAHAN: Update UI via DispatcherQueue ---
                dispatcher.TryEnqueue([this, i, total = files_to_process.size()]() {
                    ProgressBar().Value(i + 1);
                    ProcessButtonInfo(winrt::to_hstring(std::format("Memproses {} dari {} file...", i + 1, total)));
                    });
                // -----------------------------------------------

                // F. Bungkus ke Model File History untuk Database
                winrt::LeafEyeCore::FileHistoryModel fileModel = winrt::LeafEyeCore::FileHistoryModel(
                    0, saved_annotated_file.Name(), meta.FileSize(), meta.DateCreated(), meta.DateModified(),
                    max_conf, saved_annotated_file.Path(), top_disease
                );
                fileModel.DetectedDiseases(detected_diseases_vector);
                files_history_model.push_back(fileModel);
            }

            // 5. PENYIMPANAN KE DATABASE OBJECTBOX ...
            // (Biarkan kode ObjectBox Anda berjalan seperti biasa di sini)
            auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
            auto result_add_history = co_await database.AddHistory(history_model);

            if (!result_add_history.IsError() && result_add_history.IsValueExists()) {
                obx_id history_id = result_add_history.ResultValue().as<obx_id>();
                database.TxWrite();
                for (const auto& fileModel : files_history_model) {
                    auto result_add_file = co_await database.AddFileHistory(fileModel);
                    if (!result_add_file.IsError() && result_add_file.IsValueExists()) {
                        obx_id file_id = result_add_file.ResultValue().as<obx_id>();
                        auto result_link = co_await database.LinkFileHistoryToHistory(file_id, history_id);
                        if (result_link.IsError()) { co_return; }
                    }
                    else { co_return; }
                }
                database.TxSuccess();
            }

            // 6. KEMBALI KE UI THREAD UNTUK UPDATE TAMPILAN
            co_await ui_thread;
            m_filesMetaInfo.Clear();
            ProgressBar().Value(0); // Reset progress bar setelah selesai
            ProcessButtonInfo(L"Proses selesai!");

        }
        catch (const winrt::hresult_error& ex) { /*...*/ }
        catch (const std::exception& ex_std) { /*...*/ }
        catch (...) { /*...*/ }
    }

    winrt::event_token HomePage::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void HomePage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void HomePage::RaisedPropertyChanged(winrt::hstring const& property_name)
    {
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
    }

    void HomePage::IsProcessButtonEnabled(bool value) {
        if (m_isProcessButtonEnabled != value) {
            m_isProcessButtonEnabled = value;
            RaisedPropertyChanged(L"IsProcessButtonEnabled");
        }
    }

    bool HomePage::IsProcessButtonEnabled() {
        return m_isProcessButtonEnabled;
    }

    void HomePage::ProcessButtonInfo(const winrt::hstring& value) {
        if (m_processButtonInfo != value) {
            m_processButtonInfo = value;
            RaisedPropertyChanged(L"ProcessButtonInfo");
        }
    }

    winrt::hstring HomePage::ProcessButtonInfo() {
        return m_processButtonInfo;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<float>> HomePage::PreprocessImageAsync(winrt::Windows::Storage::StorageFile file)
    {
        winrt::Windows::Storage::Streams::IRandomAccessStream stream = co_await file.OpenReadAsync();
        winrt::Windows::Graphics::Imaging::BitmapDecoder decoder = co_await winrt::Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream);

        winrt::Windows::Graphics::Imaging::BitmapTransform transform;
        transform.ScaledWidth(256);
        transform.ScaledHeight(256);
        transform.InterpolationMode(winrt::Windows::Graphics::Imaging::BitmapInterpolationMode::Linear);

        winrt::Windows::Graphics::Imaging::PixelDataProvider pixelData = co_await decoder.GetPixelDataAsync(
            winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Rgba8,
            winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied,
            transform,
            winrt::Windows::Graphics::Imaging::ExifOrientationMode::RespectExifOrientation,
            winrt::Windows::Graphics::Imaging::ColorManagementMode::ColorManageToSRgb
        );

        winrt::com_array<uint8_t> pixels = pixelData.DetachPixelData();

        // Gunakan std::vector untuk manipulasi data yang sangat cepat
        std::vector<float> tensor_values(3 * 256 * 256);
        size_t image_size = 256 * 256;

        // Mean and std for ImageNet normalization (RGB)
        const float mean_r = 0.485f, mean_g = 0.456f, mean_b = 0.406f;
        const float std_r = 0.229f, std_g = 0.224f, std_b = 0.225f;

        for (size_t i = 0; i < image_size; ++i)
        {
            // Convert from [0,255] to [0,1], then normalize: (x - mean) / std
            tensor_values[i] = (pixels[i * 4] / 255.0f - mean_r) / std_r;   // Red
            tensor_values[i + image_size] = (pixels[i * 4 + 1] / 255.0f - mean_g) / std_g; // Green
            tensor_values[i + 2 * image_size] = (pixels[i * 4 + 2] / 255.0f - mean_b) / std_b; // Blue
        }

        // Ubah std::vector menjadi IVector WinRT saat dikembalikan
        co_return winrt::single_threaded_vector<float>(std::move(tensor_values));
    }
}