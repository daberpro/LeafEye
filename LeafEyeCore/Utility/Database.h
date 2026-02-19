#pragma once
#include "Database.g.h"
#include "Result.h"
#include "Model/FileHistoryModel.h"
#include "Model/HistoryModel.h"
#include "Model/UserModel.h"
#include "Model/ProfileModel.h"
#define OBX_CPP_FILE
#include <objectbox.hpp>
#include "Schema/objectbox-model.h"
#include "Schema/model.obx.hpp"

namespace winrt::LeafEyeCore::implementation
{
    struct Database : DatabaseT<Database>
    {
        Database(hstring const& database_location, int32_t max_db_size);

        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> InitializeAsync();
        void PrintInfo();

        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable data);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> ClearEntriesAsync(winrt::LeafEyeCore::BoxType box);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> DeleteEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable data);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> UpdateEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable oldData, winrt::Windows::Foundation::IInspectable newData);

        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddRelationAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable from, winrt::Windows::Foundation::IInspectable to);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> RemoveRelationAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable from, winrt::Windows::Foundation::IInspectable to);

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable filter);
        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> GetAllEntriesAsync(winrt::LeafEyeCore::BoxType box, int32_t limit, int32_t offset);

        winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> GetHistoryByDateRangeAsync(int64_t startUnix, int64_t endUnix);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetHistoryCountByDateRangeAsync(int64_t startUnix, int64_t endUnix);

        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetCountAsync(winrt::LeafEyeCore::BoxType box);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetCountWithFilterAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable filter);

    private:
        OBX_model* m_model{ nullptr };
        std::unique_ptr<obx::Options> m_options{ nullptr };
        std::unique_ptr<obx::Store> m_store{ nullptr };
        std::unique_ptr<obx::Box<User>> m_user_box{ nullptr };
        std::unique_ptr<obx::Box<FileHistory>> m_file_box{ nullptr };
        std::unique_ptr<obx::Box<History>> m_history_box{ nullptr };
        std::unique_ptr<obx::Box<Profile>> m_profile_box{ nullptr };

        winrt::hstring m_database_location;
        int32_t m_max_db_size{ 0 };
    };
}
namespace winrt::LeafEyeCore::factory_implementation
{
    struct Database : DatabaseT<Database, implementation::Database>
    {
    };
}