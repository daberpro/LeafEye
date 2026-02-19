#include "pch.h"
#include "Database.h"
#include "Database.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
    Database::Database(hstring const& database_location, int32_t max_db_size)
        : m_database_location(database_location), m_max_db_size(max_db_size), m_model(create_obx_model()) {
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::InitializeAsync()
    {
        co_await winrt::resume_background();

        if (!m_model) {
            co_return winrt::LeafEyeCore::Result(
                true,
                static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::FailedCreateModule),
                L"Failed to create module",
                nullptr
            );
        }

        m_options = std::make_unique<obx::Options>(m_model);
        m_options->directory(winrt::to_string(m_database_location));
        m_options->maxDbSizeInKb(m_max_db_size);
        try {
            m_store = std::make_unique<obx::Store>(*m_options);
            m_user_box = std::make_unique<obx::Box<User>>(*m_store);
            m_file_box = std::make_unique<obx::Box<FileHistory>>(*m_store);
            m_history_box = std::make_unique<obx::Box<History>>(*m_store);
            m_profile_box = std::make_unique<obx::Box<Profile>>(*m_store);
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(),
                (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(),
                nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true,
                static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] Unknown Error: " + winrt::to_hstring(err.what())).c_str(),
                nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false,
            static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Database initialized successfully",
            nullptr
        );
    }

    void Database::PrintInfo()
    {
        winrt::hstring version = winrt::to_hstring(obx_version_string());
        OutputDebugString(L"\n");
        OutputDebugString(winrt::to_hstring(std::string(30, '=')).c_str());
        OutputDebugString(L"\n");
        OutputDebugString(std::format(L"ObjectBox VERSION: {}", version).c_str());
        OutputDebugString(L"\n");
        OutputDebugString(winrt::to_hstring(std::string(30, '=')).c_str());
        OutputDebugString(L"\n");
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable data)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::User: {
                auto user = data.try_as<winrt::LeafEyeCore::UserModel>();
                if (!user) throw std::invalid_argument("Invalid UserModel data");
                m_user_box->put(User{
                    .id = 0,
                    .username = winrt::to_string(user.Username()),
                    .password = winrt::to_string(user.Password()),
                    .is_admin = user.IsAdmin()
                    });
                break;
            }
            case BoxType::FileHistory: {
                auto file = data.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!file) throw std::invalid_argument("Invalid FileHistoryModel data");
                m_file_box->put(FileHistory{
                    .id = 0,
                    .filename = winrt::to_string(file.FileName()),
                    .date_created = file.DateCreated(),
                    .date_modified = file.DateModified(),
                    .confidence_score = file.ConfidenceScore(),
                    .file_size = file.FileSize()
                    });
                break;
            }
            case BoxType::History: {
                auto history = data.try_as<winrt::LeafEyeCore::HistoryModel>();
                if (!history) throw std::invalid_argument("Invalid HistoryModel data");
                m_history_box->put(History{
                    .id = 0,
                    .date = history.Date(),
                    .total_files = history.TotalFiles(),
                    .status = history.Status()
                    });
                break;
            }
            case BoxType::Profile: {
                auto profile = data.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!profile) throw std::invalid_argument("Invalid ProfileModel data");
                m_profile_box->put(Profile{
                    .id = 0,
                    .full_name = winrt::to_string(profile.Fullname()),
                    .avatar_path = winrt::to_string(profile.AvatarPath()),
                    .role = profile.Role()
                    });
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] AddEntry Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Entry added successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::ClearEntriesAsync(winrt::LeafEyeCore::BoxType box)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::User:        m_user_box->removeAll();    break;
            case BoxType::FileHistory: m_file_box->removeAll();    break;
            case BoxType::History:     m_history_box->removeAll(); break;
            case BoxType::Profile:     m_profile_box->removeAll(); break;
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] ClearEntries Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"All entries cleared successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::DeleteEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable data)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::User: {
                auto user = data.try_as<winrt::LeafEyeCore::UserModel>();
                if (!user) throw std::invalid_argument("Invalid UserModel data");
                m_user_box->remove(user.Id());
                break;
            }
            case BoxType::FileHistory: {
                auto file = data.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!file) throw std::invalid_argument("Invalid FileHistoryModel data");
                m_file_box->remove(file.Id());
                break;
            }
            case BoxType::History: {
                auto history = data.try_as<winrt::LeafEyeCore::HistoryModel>();
                if (!history) throw std::invalid_argument("Invalid HistoryModel data");
                m_history_box->remove(history.Id());
                break;
            }
            case BoxType::Profile: {
                auto profile = data.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!profile) throw std::invalid_argument("Invalid ProfileModel data");
                m_profile_box->remove(profile.Id());
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] DeleteEntry Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Entry deleted successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::UpdateEntryAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable oldData, winrt::Windows::Foundation::IInspectable newData)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::User: {
                auto oldUser = oldData.try_as<winrt::LeafEyeCore::UserModel>();
                auto newUser = newData.try_as<winrt::LeafEyeCore::UserModel>();
                if (!oldUser || !newUser) throw std::invalid_argument("Invalid UserModel data");
                m_user_box->put(User{
                    .id = oldUser.Id(),
                    .username = winrt::to_string(newUser.Username()),
                    .password = winrt::to_string(newUser.Password()),
                    .is_admin = newUser.IsAdmin()
                    });
                break;
            }
            case BoxType::FileHistory: {
                auto oldFile = oldData.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                auto newFile = newData.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!oldFile || !newFile) throw std::invalid_argument("Invalid FileHistoryModel data");
                m_file_box->put(FileHistory{
                    .id = oldFile.Id(),
                    .filename = winrt::to_string(newFile.FileName()),
                    .date_created = newFile.DateCreated(),
                    .date_modified = newFile.DateModified(),
                    .confidence_score = newFile.ConfidenceScore(),
                    .file_size = newFile.FileSize()
                    });
                break;
            }
            case BoxType::History: {
                auto oldHistory = oldData.try_as<winrt::LeafEyeCore::HistoryModel>();
                auto newHistory = newData.try_as<winrt::LeafEyeCore::HistoryModel>();
                if (!oldHistory || !newHistory) throw std::invalid_argument("Invalid HistoryModel data");
                m_history_box->put(History{
                    .id = oldHistory.Id(),
                    .date = newHistory.Date(),
                    .total_files = newHistory.TotalFiles(),
                    .status = newHistory.Status()
                    });
                break;
            }
            case BoxType::Profile: {
                auto oldProfile = oldData.try_as<winrt::LeafEyeCore::ProfileModel>();
                auto newProfile = newData.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!oldProfile || !newProfile) throw std::invalid_argument("Invalid ProfileModel data");
                m_profile_box->put(Profile{
                    .id = oldProfile.Id(),
                    .full_name = winrt::to_string(newProfile.Fullname()),
                    .avatar_path = winrt::to_string(newProfile.AvatarPath()),
                    .role = newProfile.Role()
                    });
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] UpdateEntry Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Entry updated successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddRelationAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable from, winrt::Windows::Foundation::IInspectable to)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::History: {
                auto history = from.try_as<winrt::LeafEyeCore::HistoryModel>();
                auto file = to.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!history || !file) throw std::invalid_argument("Invalid History/FileHistory relation data");
                m_history_box->standaloneRelPut<FileHistory>(History_::files, history.Id(), file.Id());
                break;
            }
            case BoxType::User: {
                auto user = from.try_as<winrt::LeafEyeCore::UserModel>();
                auto profile = to.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!user || !profile) throw std::invalid_argument("Invalid User/Profile relation data");
                m_user_box->standaloneRelPut<Profile>(User_::profile, user.Id(), profile.Id());
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Relation not supported for this BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] AddRelation Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Relation added successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::RemoveRelationAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable from, winrt::Windows::Foundation::IInspectable to)
    {
        co_await winrt::resume_background();

        try {
            switch (box) {
            case BoxType::History: {
                auto history = from.try_as<winrt::LeafEyeCore::HistoryModel>();
                auto file = to.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!history || !file) throw std::invalid_argument("Invalid History/FileHistory relation data");
                m_history_box->standaloneRelRemove<FileHistory>(History_::files, history.Id(), file.Id());
                break;
            }
            case BoxType::User: {
                auto user = from.try_as<winrt::LeafEyeCore::UserModel>();
                auto profile = to.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!user || !profile) throw std::invalid_argument("Invalid User/Profile relation data");
                m_user_box->standaloneRelRemove<Profile>(User_::profile, user.Id(), profile.Id());
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Relation not supported for this BoxType", nullptr
                );
            }
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] RemoveRelation Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }

        co_return winrt::LeafEyeCore::Result(
            false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
            L"Relation removed successfully", nullptr
        );
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> Database::GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable filter)
    {
        co_await winrt::resume_background();
        auto results = winrt::single_threaded_vector<winrt::Windows::Foundation::IInspectable>();

        switch (box) {
        case BoxType::User: {
            auto filterModel = filter.try_as<winrt::LeafEyeCore::UserModel>();
            if (!filterModel) co_return results;
            // FITUR: .contains(..., true) untuk case-insensitive partial match
            auto query = m_user_box->query(User_::username.contains(winrt::to_string(filterModel.Username()), true)).build();
            for (auto& u : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::UserModel>(
                    winrt::to_hstring(u.username), winrt::to_hstring(u.password), u.is_admin
                );
                model.Id(u.id);
                results.Append(model);
            }
            break;
        }
        case BoxType::Profile: {
            auto filterModel = filter.try_as<winrt::LeafEyeCore::ProfileModel>();
            if (!filterModel) co_return results;
            auto query = m_profile_box->query(Profile_::role.equals(filterModel.Role())).build();
            for (auto& p : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::ProfileModel>(
                    p.id, winrt::to_hstring(p.full_name), winrt::to_hstring(p.avatar_path), p.role
                );
                results.Append(model);
            }
            break;
        }
        case BoxType::History: {
            auto filterModel = filter.try_as<winrt::LeafEyeCore::HistoryModel>();
            if (!filterModel) co_return results;

            // FITUR: Ditambahkan order by DESCENDING date
            auto qb = m_history_box->query(History_::status.equals(filterModel.Status()));
            qb.order(History_::date, OBXOrderFlags_DESCENDING);
            auto query = qb.build();

            for (auto& h : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::HistoryModel>(
                    h.id, h.date, h.total_files, h.status
                );
                auto qbFiles = m_file_box->query();
                qbFiles.backlink<History>(History_::files).with(History_::id.equals(h.id));
                auto linkedFiles = qbFiles.build().find();
                auto fileVector = winrt::single_threaded_vector<winrt::LeafEyeCore::FileHistoryModel>();
                for (auto& f : linkedFiles) {
                    auto fileModel = winrt::make<winrt::LeafEyeCore::implementation::FileHistoryModel>(
                        f.id, winrt::to_hstring(f.filename), f.file_size, f.date_created, f.date_modified, f.confidence_score
                    );
                    fileVector.Append(fileModel);
                }
                model.Files(fileVector);
                results.Append(model);
            }
            break;
        }
        case BoxType::FileHistory: {
            auto filterModel = filter.try_as<winrt::LeafEyeCore::FileHistoryModel>();
            if (!filterModel) co_return results;
            // Sesuai permintaan: FileHistory tetap menggunakan equals / tidak diubah
            auto query = m_file_box->query(FileHistory_::filename.equals(winrt::to_string(filterModel.FileName()))).build();
            for (auto& f : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::FileHistoryModel>(
                    f.id, winrt::to_hstring(f.filename), f.file_size, f.date_created, f.date_modified, f.confidence_score
                );
                results.Append(model);
            }
            break;
        }
        }
        co_return results;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> Database::GetAllEntriesAsync(winrt::LeafEyeCore::BoxType box, int32_t limit, int32_t offset)
    {
        co_await winrt::resume_background();
        auto results = winrt::single_threaded_vector<winrt::Windows::Foundation::IInspectable>();

        switch (box) {
        case BoxType::User: {
            auto query = m_user_box->query().build();
            query.offset(offset); query.limit(limit);
            for (auto& u : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::UserModel>(
                    winrt::to_hstring(u.username), winrt::to_hstring(u.password), u.is_admin
                );
                model.Id(u.id);
                results.Append(model);
            }
            break;
        }
        case BoxType::Profile: {
            auto query = m_profile_box->query().build();
            query.offset(offset); query.limit(limit);
            for (auto& p : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::ProfileModel>(
                    p.id, winrt::to_hstring(p.full_name), winrt::to_hstring(p.avatar_path), p.role
                );
                results.Append(model);
            }
            break;
        }
        case BoxType::History: {
            // FITUR SORTING DITAMBAHKAN DI SINI (DESCENDING DATE)
            auto qb = m_history_box->query();
            qb.order(History_::date, OBXOrderFlags_DESCENDING);
            auto query = qb.build();

            query.offset(offset); query.limit(limit);
            for (auto& h : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::HistoryModel>(
                    h.id, h.date, h.total_files, h.status
                );
                auto qbFiles = m_file_box->query();
                qbFiles.backlink<History>(History_::files).with(History_::id.equals(h.id));
                auto linkedFiles = qbFiles.build().find();
                auto fileVector = winrt::single_threaded_vector<winrt::LeafEyeCore::FileHistoryModel>();
                for (auto& f : linkedFiles) {
                    auto fileModel = winrt::make<winrt::LeafEyeCore::implementation::FileHistoryModel>(
                        f.id, winrt::to_hstring(f.filename), f.file_size, f.date_created, f.date_modified, f.confidence_score
                    );
                    fileVector.Append(fileModel);
                }
                model.Files(fileVector);
                results.Append(model);
            }
            break;
        }
        case BoxType::FileHistory: {
            auto query = m_file_box->query().build();
            query.offset(offset); query.limit(limit);
            for (auto& f : query.find()) {
                auto model = winrt::make<winrt::LeafEyeCore::implementation::FileHistoryModel>(
                    f.id, winrt::to_hstring(f.filename), f.file_size, f.date_created, f.date_modified, f.confidence_score
                );
                results.Append(model);
            }
            break;
        }
        }
        co_return results;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::Collections::IVector<winrt::Windows::Foundation::IInspectable>> Database::GetHistoryByDateRangeAsync(int64_t startUnix, int64_t endUnix)
    {
        co_await winrt::resume_background();
        auto results = winrt::single_threaded_vector<winrt::Windows::Foundation::IInspectable>();

        // MENGGUNAKAN .between() UNTUK RENTANG WAKTU & DIURUTKAN DESCENDING
        auto qb = m_history_box->query(History_::date.between(startUnix, endUnix));
        qb.order(History_::date, OBXOrderFlags_DESCENDING);
        auto query = qb.build();

        for (auto& h : query.find()) {
            auto model = winrt::make<winrt::LeafEyeCore::implementation::HistoryModel>(
                h.id, h.date, h.total_files, h.status
            );

            auto qbFiles = m_file_box->query();
            qbFiles.backlink<History>(History_::files).with(History_::id.equals(h.id));
            auto linkedFiles = qbFiles.build().find();

            auto fileVector = winrt::single_threaded_vector<winrt::LeafEyeCore::FileHistoryModel>();
            for (auto& f : linkedFiles) {
                auto fileModel = winrt::make<winrt::LeafEyeCore::implementation::FileHistoryModel>(
                    f.id, winrt::to_hstring(f.filename), f.file_size, f.date_created, f.date_modified, f.confidence_score
                );
                fileVector.Append(fileModel);
            }
            model.Files(fileVector);
            results.Append(model);
        }
        co_return results;
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetHistoryCountByDateRangeAsync(int64_t startUnix, int64_t endUnix)
    {
        co_await winrt::resume_background();
        try {
            uint64_t count = m_history_box->query(History_::date.between(startUnix, endUnix)).build().count();
            co_return winrt::LeafEyeCore::Result(
                false,
                static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
                winrt::to_hstring(count),
                winrt::box_value(static_cast<int64_t>(count))
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true,
                static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] Date Range Error: " + winrt::to_hstring(err.what())).c_str(),
                nullptr
            );
        }
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetCountAsync(winrt::LeafEyeCore::BoxType box)
    {
        co_await winrt::resume_background();
        try {
            uint64_t count = 0;
            switch (box) {
            case BoxType::User:        count = m_user_box->count();    break;
            case BoxType::FileHistory: count = m_file_box->count();    break;
            case BoxType::History:     count = m_history_box->count(); break;
            case BoxType::Profile:     count = m_profile_box->count(); break;
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
            co_return winrt::LeafEyeCore::Result(
                false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
                winrt::to_hstring(count), winrt::box_value(static_cast<int64_t>(count))
            );
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] GetCount Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
    }

    winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetCountWithFilterAsync(winrt::LeafEyeCore::BoxType box, winrt::Windows::Foundation::IInspectable filter)
    {
        co_await winrt::resume_background();
        try {
            uint64_t count = 0;
            switch (box) {
            case BoxType::User: {
                auto filterModel = filter.try_as<winrt::LeafEyeCore::UserModel>();
                if (!filterModel) throw std::invalid_argument("Invalid UserModel filter");
                // MENGGUNAKAN .contains(..., true)
                count = m_user_box->query(User_::username.contains(winrt::to_string(filterModel.Username()), true)).build().count();
                break;
            }
            case BoxType::Profile: {
                auto filterModel = filter.try_as<winrt::LeafEyeCore::ProfileModel>();
                if (!filterModel) throw std::invalid_argument("Invalid ProfileModel filter");
                count = m_profile_box->query(Profile_::role.equals(filterModel.Role())).build().count();
                break;
            }
            case BoxType::History: {
                auto filterModel = filter.try_as<winrt::LeafEyeCore::HistoryModel>();
                if (!filterModel) throw std::invalid_argument("Invalid HistoryModel filter");
                count = m_history_box->query(History_::status.equals(filterModel.Status())).build().count();
                break;
            }
            case BoxType::FileHistory: {
                auto filterModel = filter.try_as<winrt::LeafEyeCore::FileHistoryModel>();
                if (!filterModel) throw std::invalid_argument("Invalid FileHistoryModel filter");
                count = m_file_box->query(FileHistory_::filename.equals(winrt::to_string(filterModel.FileName()))).build().count();
                break;
            }
            default:
                co_return winrt::LeafEyeCore::Result(
                    true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                    L"[DATABASE] Unknown BoxType", nullptr
                );
            }
            co_return winrt::LeafEyeCore::Result(
                false, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::None),
                winrt::to_hstring(count), winrt::box_value(static_cast<int64_t>(count))
            );
        }
        catch (const obx::Exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, err.code(), (L"[DATABASE] ObjectBox Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
        catch (const std::exception& err) {
            co_return winrt::LeafEyeCore::Result(
                true, static_cast<int32_t>(winrt::LeafEyeCore::DatabaseError::UnknownError),
                (L"[DATABASE] GetCountWithFilter Error: " + winrt::to_hstring(err.what())).c_str(), nullptr
            );
        }
    }
}