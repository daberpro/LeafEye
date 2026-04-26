#include "pch.h"
#include "HomePage.xaml.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::LeafEye::implementation
{


    hstring HomePage::ConvertToDateTimeString(uint64_t fileTime)
    {
        if (fileTime <= 0) {
            return L"-";
        }
		auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(std::chrono::milliseconds(fileTime));
        auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), tp };
        return winrt::to_hstring(std::format("{:%d/%m/%Y %I:%M %p}", localTime));
	}

    hstring HomePage::ConvertToFileSizeString(uint64_t fileTime)
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
				EmptyFileHistoryListViewText().Visibility(Visibility::Visible);
                ProcessButtonInfo(L"Silahkan pilih input gambar");
                IsProcessButtonEnabled(false);
                return;
            }
			EmptyFileHistoryListViewText().Visibility(Visibility::Collapsed);
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

    void HomePage::OnNavigatedTo(Navigation::NavigationEventArgs const& e)
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

            auto now = std::chrono::system_clock::now();
		    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
		
            // we must take all the data from UI Thread so bg thread can consume it
            auto history_model = winrt::LeafEyeCore::HistoryModel(
                0, // objectbox id
                timestamp, // date (miliseconds)
                m_filesMetaInfo.Size(), // total files
                0, // status enum
                m_historyInfo.OutputFolder() // output folder
            );

			std::vector<winrt::LeafEyeCore::FileHistoryModel> files_history_model;
			files_history_model.reserve(m_filesMetaInfo.Size());
            
            auto outputFolder = co_await winrt::Windows::Storage::StorageFolder::GetFolderFromPathAsync(m_historyInfo.OutputFolder());
            auto new_folder = co_await outputFolder.CreateFolderAsync(winrt::to_hstring(timestamp), winrt::Windows::Storage::CreationCollisionOption::OpenIfExists);
            for (uint32_t i = 0; i < m_files.Size(); i++) {
                auto file = m_files.GetAt(i);
				auto meta = m_filesMetaInfo.GetAt(i);
                files_history_model.push_back(
                    winrt::LeafEyeCore::FileHistoryModel(
                        0, // objectbox id
                        meta.FileName(),
                        meta.FileSize(),
                        meta.DateCreated(),
                        meta.DateModified(),
                        0.0,
                        file.Path(),
                        L""
                    )
                );
			    co_await file.CopyAsync(new_folder, file.Name(), winrt::Windows::Storage::NameCollisionOption::GenerateUniqueName);
            }
    
            winrt::apartment_context ui_thread;
            co_await winrt::resume_background();

            auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();
            auto result_add_history = co_await database.AddHistory(history_model);
            if (!result_add_history.IsError() && result_add_history.IsValueExists()) {
				obx_id history_id = result_add_history.ResultValue().as<obx_id>();

                database.TxWrite();
                for (const auto& file: files_history_model) {
                   auto result_add_file_history = co_await database.AddFileHistory(file);
                   if (!result_add_file_history.IsError() && result_add_file_history.IsValueExists()) {
                       
                       obx_id file_id = result_add_file_history.ResultValue().as<obx_id>();
					   auto result_link_file_history_to_history = co_await database.LinkFileHistoryToHistory(file_id, history_id);
                       
                       if (result_link_file_history_to_history.IsError()) {
                           OutputDebugString(
                               std::format(
                                    L"\n[Process][Error]: {}\n",
                                    result_link_file_history_to_history.Message()
                               ).c_str()
                           );
                           
						   // do somenthing when link file history to history failed
                           co_return;
                       }
                   
                   }
                   else {
                       OutputDebugString(
                           std::format(
                                L"\n[Process][Error]: {}\n",
                                result_add_file_history.Message()
                           ).c_str()
                       );
					   // do something if add file history failed
                       co_return;
                   }
                }
                database.TxSuccess();
            }
            else {
                OutputDebugString(
                    std::format(
                        L"\n[Process]: {}\n",
                        result_add_history.Message()
                    ).c_str()
                );
                // do something if add history failed
                co_return;
            }

            co_await ui_thread;

            m_filesMetaInfo.Clear();
            
        }catch (const winrt::hresult_error& ex) {
            OutputDebugString(
                std::format(
                    L"\n[Error]: {}\n",
                    ex.message()
                ).c_str()
            );
            // show an error while copy files
        }
        catch (...) {
			// show an error while copy files
        }
    }

    winrt::event_token HomePage::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void HomePage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    void HomePage::RaisedPropertyChanged(hstring const& property_name){
        m_propertyChanged(*this, winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{ property_name });
    }

    void HomePage::IsProcessButtonEnabled(bool value) {
        if (m_isProcessButtonEnabled != value) {
            m_isProcessButtonEnabled = value;
            RaisedPropertyChanged(L"IsProcessButtonEnabled");
        }
    };
    
    bool HomePage::IsProcessButtonEnabled() {
        return m_isProcessButtonEnabled;
    };

    void HomePage::ProcessButtonInfo(const hstring& value) {
        if (m_processButtonInfo != value) {
            m_processButtonInfo = value;
            RaisedPropertyChanged(L"ProcessButtonInfo");
        }
    };
    
    hstring HomePage::ProcessButtonInfo() {
        return m_processButtonInfo;
    };
}