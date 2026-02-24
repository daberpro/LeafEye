#pragma once
#include "Database.g.h"
#include "Result.h"
#include "../Model/UserModel.h"
#include "../Model/ProfileModel.h"
#include "../Model/HistoryModel.h"
#include "../Model/FileHistoryModel.h"
#include <objectbox.hpp>
#include "Schema/objectbox-model.h"
#include "Schema/model.obx.hpp"

namespace winrt::LeafEyeCore::implementation
{
    struct Database : DatabaseT<Database>
    {
        Database() = default;

        Database(hstring const& database_location, uint64_t max_db_size);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> InitializeAsync();
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetUserById(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetUserByUsername(hstring username);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetUserContainsUsername(hstring username, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetAllUsers(int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> ValidateUserCredentials(hstring username, hstring password);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddUser(winrt::LeafEyeCore::UserModel user);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> UpdateUser(winrt::LeafEyeCore::UserModel user);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> DeleteUser(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetProfileById(uint64_t profileId);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetUserProfileByLink(uint64_t userId);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddUserProfile(winrt::LeafEyeCore::ProfileModel userProfile);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> UpdateUserProfile(winrt::LeafEyeCore::ProfileModel userProfile);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> DeleteUserProfile(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetHistoryById(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetHistoryByUserLink(uint64_t userId, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetHistoryByStatus(int32_t status, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetHistoryByDateRange(uint64_t start, uint64_t end, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddHistory(winrt::LeafEyeCore::HistoryModel history);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> UpdateHistory(winrt::LeafEyeCore::HistoryModel history);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> DeleteHistory(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetFileHistoryById(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetFileHistoriesByHistoryLink(uint64_t historyId, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> GetFileHistoriesByConfidenceThreshold(double minConfidence, int32_t offset, int32_t limit);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> AddFileHistory(winrt::LeafEyeCore::FileHistoryModel fileHistory);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> UpdateFileHistory(winrt::LeafEyeCore::FileHistoryModel fileHistory);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> DeleteFileHistory(uint64_t id);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> LinkUserProfile(uint64_t userId, uint64_t profileId);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> LinkHistoryToUser(uint64_t historyId, uint64_t userId);
        winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> LinkFileHistoryToHistory(uint64_t fileHistoryId, uint64_t historyId);


    private:

        std::unique_ptr<obx::Store> m_store;
        std::unique_ptr<obx::Box<User>> m_user_box;
        std::unique_ptr<obx::Box<Profile>> m_profile_box;
        std::unique_ptr<obx::Box<History>> m_history_box;
        std::unique_ptr<obx::Box<FileHistory>> m_filehistory_box;

        // Queries - User
        std::unique_ptr<obx::Query<User>> m_query_user_by_id; // done
        std::unique_ptr<obx::Query<User>> m_query_user_by_username; // done
        std::unique_ptr<obx::Query<User>> m_query_user_contains_username; // done
        std::unique_ptr<obx::Query<User>> m_query_user_by_credentials; // done

        // Queries - Profile
        std::unique_ptr<obx::Query<Profile>> m_query_profile_by_id; // done
        std::unique_ptr<obx::Query<Profile>> m_query_profile_by_user_link; // done

        // Queries - History
        std::unique_ptr<obx::Query<History>> m_query_history_by_id; // done
        std::unique_ptr<obx::Query<History>> m_query_history_by_status; // done
        std::unique_ptr<obx::Query<History>> m_query_history_by_user_link; // done

        // Queries - FileHistory
        std::unique_ptr<obx::Query<FileHistory>> m_query_filehistory_by_id; // done
        std::unique_ptr<obx::Query<FileHistory>> m_query_filehistory_by_confidence;  // done
        std::unique_ptr<obx::Query<FileHistory>> m_query_filehistory_by_history_link; // done

        winrt::hstring m_database_location;
        uint64_t m_max_db_size;
        bool m_is_initialized;
    };
}
namespace winrt::LeafEyeCore::factory_implementation
{
    struct Database : DatabaseT<Database, implementation::Database>
    {
    };
}