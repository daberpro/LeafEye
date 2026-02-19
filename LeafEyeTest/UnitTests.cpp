#include "pch.h"
#include <winrt/LeafEyeCore.h>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Tell the testing framework how to format winrt::hstring for Assert::AreEqual
namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    inline std::wstring ToString<winrt::hstring>(const winrt::hstring& t)
    {
        return std::wstring(t.c_str());
    }
}

namespace LeafEyeTest
{
    TEST_CLASS(DatabaseTests)
    {
    private:
        winrt::hstring m_local_storage_path{ winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path() };
        winrt::LeafEyeCore::Database m_db{ nullptr };

        TEST_METHOD_INITIALIZE(Setup) {
            m_db = winrt::LeafEyeCore::Database{ m_local_storage_path + L"/database_test", 1024 * 10};
            // Gunakan .get() untuk memblokir dan menunggu hasil async di dalam Unit Test
            auto result = m_db.InitializeAsync().get();
            Assert::IsFalse(result.IsError(), std::format(L"DB Init Failed: {}", result.Message()).c_str());
        }

        TEST_METHOD_CLEANUP(Teardown) {
            m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::User).get();
            m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::Profile).get();
            m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::History).get();
            m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::FileHistory).get();
        }

    public:

        // ===================== DATABASE =====================

        TEST_METHOD(Database_Initialize_ShouldSucceed) {
            m_db.PrintInfo();
            Assert::IsTrue(true);
        }

        // ===================== USER =====================

        TEST_METHOD(User_Add_ShouldSucceed) {
            auto result = m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();
            Assert::IsFalse(result.IsError(), std::format(L"AddEntry User Failed: {}", result.Message()).c_str());
        }

        TEST_METHOD(User_GetAll_ShouldReturnEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();
            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::User, 100, 0).get();
            Assert::IsTrue(entries.Size() > 0, L"GetAllEntries User should return at least 1 entry");
        }

        TEST_METHOD(User_GetCount_ShouldBeGreaterThanZero) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();
            auto result = m_db.GetCountAsync(winrt::LeafEyeCore::BoxType::User).get();
            Assert::IsFalse(result.IsError(), std::format(L"GetCount User Failed: {}", result.Message()).c_str());
            auto count = winrt::unbox_value<int64_t>(result.ResultValue());
            Assert::IsTrue(count > 0, L"User count should be greater than 0");
        }

        TEST_METHOD(User_GetCountWithFilter_ShouldMatchUsername) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();

            winrt::LeafEyeCore::UserModel filter;
            filter.Username(L"daberdev");
            auto result = m_db.GetCountWithFilterAsync(winrt::LeafEyeCore::BoxType::User, filter).get();

            Assert::IsFalse(result.IsError(), std::format(L"GetCountWithFilter User Failed: {}", result.Message()).c_str());
            auto count = winrt::unbox_value<int64_t>(result.ResultValue());
            Assert::IsTrue(count > 0, L"Filtered user count should be greater than 0");
        }

        TEST_METHOD(User_GetWithFilter_ShouldReturnMatchingUser) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();

            winrt::LeafEyeCore::UserModel filter;
            filter.Username(L"daberdev");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, filter).get();

            Assert::IsTrue(entries.Size() > 0, L"Should find user with username 'daberdev'");

            auto user = entries.GetAt(0).try_as<winrt::LeafEyeCore::UserModel>();
            Assert::IsTrue(user != nullptr, L"Cast to UserModel should not be null");
            Assert::AreEqual(winrt::hstring(L"daberdev"), user.Username(), L"Username should match");
            Assert::IsTrue(user.IsAdmin(), L"User should be admin");
        }

        TEST_METHOD(User_Update_ShouldReflectNewValues) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();

            winrt::LeafEyeCore::UserModel filter;
            filter.Username(L"daberdev");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No user found to update");

            auto oldUser = entries.GetAt(0).try_as<winrt::LeafEyeCore::UserModel>();
            winrt::LeafEyeCore::UserModel newUser(L"daberdev_updated", L"newpassword", false);
            newUser.Id(oldUser.Id());

            auto result = m_db.UpdateEntryAsync(winrt::LeafEyeCore::BoxType::User, oldUser, newUser).get();
            Assert::IsFalse(result.IsError(), std::format(L"UpdateEntry User Failed: {}", result.Message()).c_str());

            winrt::LeafEyeCore::UserModel updatedFilter;
            updatedFilter.Username(L"daberdev_updated");
            auto updated = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, updatedFilter).get();
            Assert::IsTrue(updated.Size() > 0, L"Updated user should be retrievable");

            auto updatedUser = updated.GetAt(0).try_as<winrt::LeafEyeCore::UserModel>();
            Assert::AreEqual(winrt::hstring(L"daberdev_updated"), updatedUser.Username(), L"Username should be updated");
            Assert::IsFalse(updatedUser.IsAdmin(), L"IsAdmin should be updated to false");
        }

        TEST_METHOD(User_Delete_ShouldRemoveEntry) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"delete_me", L"123456", false)
            ).get();

            winrt::LeafEyeCore::UserModel filter;
            filter.Username(L"delete_me");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No user found to delete");

            auto result = m_db.DeleteEntryAsync(winrt::LeafEyeCore::BoxType::User, entries.GetAt(0)).get();
            Assert::IsFalse(result.IsError(), std::format(L"DeleteEntry User Failed: {}", result.Message()).c_str());

            auto remaining = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, filter).get();
            Assert::AreEqual(0u, remaining.Size(), L"Deleted user should no longer exist");
        }

        TEST_METHOD(User_Clear_ShouldRemoveAllEntries) {
            m_db.AddEntryAsync(winrt::LeafEyeCore::BoxType::User, winrt::LeafEyeCore::UserModel(L"user1", L"pass1", false)).get();
            m_db.AddEntryAsync(winrt::LeafEyeCore::BoxType::User, winrt::LeafEyeCore::UserModel(L"user2", L"pass2", false)).get();

            auto result = m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::User).get();
            Assert::IsFalse(result.IsError(), std::format(L"ClearEntries User Failed: {}", result.Message()).c_str());

            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::User, 100, 0).get();
            Assert::AreEqual(0u, entries.Size(), L"All users should be cleared");
        }

        // ===================== PROFILE =====================

        TEST_METHOD(Profile_Add_ShouldSucceed) {
            auto result = m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();
            Assert::IsFalse(result.IsError(), std::format(L"AddEntry Profile Failed: {}", result.Message()).c_str());
        }

        TEST_METHOD(Profile_GetAll_ShouldReturnEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();
            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::Profile, 100, 0).get();
            Assert::IsTrue(entries.Size() > 0, L"GetAllEntries Profile should return at least 1 entry");
        }

        TEST_METHOD(Profile_GetWithFilter_ShouldReturnMatchingRole) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();
            winrt::LeafEyeCore::ProfileModel filter;
            filter.Role(1);
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::Profile, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"Should find profile with role 1");

            auto profile = entries.GetAt(0).try_as<winrt::LeafEyeCore::ProfileModel>();
            Assert::IsTrue(profile != nullptr, L"Cast to ProfileModel should not be null");
            Assert::AreEqual(winrt::hstring(L"Daber Dev"), profile.Fullname(), L"Fullname should match");
            Assert::AreEqual(1, profile.Role(), L"Role should match");
        }

        TEST_METHOD(Profile_Update_ShouldReflectNewValues) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();
            winrt::LeafEyeCore::ProfileModel filter;
            filter.Role(1);
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::Profile, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No profile found to update");

            auto oldProfile = entries.GetAt(0).try_as<winrt::LeafEyeCore::ProfileModel>();
            auto newProfile = winrt::LeafEyeCore::ProfileModel(oldProfile.Id(), L"Daber Updated", L"/avatars/new.png", 2);

            auto result = m_db.UpdateEntryAsync(winrt::LeafEyeCore::BoxType::Profile, oldProfile, newProfile).get();
            Assert::IsFalse(result.IsError(), std::format(L"UpdateEntry Profile Failed: {}", result.Message()).c_str());

            winrt::LeafEyeCore::ProfileModel updatedFilter;
            updatedFilter.Role(2);
            auto updated = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::Profile, updatedFilter).get();
            Assert::IsTrue(updated.Size() > 0, L"Updated profile should be retrievable");

            auto updatedProfile = updated.GetAt(0).try_as<winrt::LeafEyeCore::ProfileModel>();
            Assert::AreEqual(winrt::hstring(L"Daber Updated"), updatedProfile.Fullname(), L"Fullname should be updated");
        }

        TEST_METHOD(Profile_Clear_ShouldRemoveAllEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();
            auto result = m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::Profile).get();
            Assert::IsFalse(result.IsError(), std::format(L"ClearEntries Profile Failed: {}", result.Message()).c_str());

            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::Profile, 100, 0).get();
            Assert::AreEqual(0u, entries.Size(), L"All profiles should be cleared");
        }

        // ===================== FILE HISTORY =====================

        TEST_METHOD(FileHistory_Add_ShouldSucceed) {
            auto result = m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"leaf_001.jpg", 2048, 1700000000LL, 1700003600LL, 0.95f)
            ).get();
            Assert::IsFalse(result.IsError(), std::format(L"AddEntry FileHistory Failed: {}", result.Message()).c_str());
        }

        TEST_METHOD(FileHistory_GetAll_ShouldReturnEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"leaf_001.jpg", 2048, 1700000000LL, 1700003600LL, 0.95f)
            ).get();
            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::FileHistory, 100, 0).get();
            Assert::IsTrue(entries.Size() > 0, L"GetAllEntries FileHistory should return at least 1 entry");
        }

        TEST_METHOD(FileHistory_GetWithFilter_ShouldReturnMatchingFile) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"leaf_001.jpg", 2048, 1700000000LL, 1700003600LL, 0.95f)
            ).get();
            winrt::LeafEyeCore::FileHistoryModel filter;
            filter.FileName(L"leaf_001.jpg");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"Should find file with name 'leaf_001.jpg'");

            auto file = entries.GetAt(0).try_as<winrt::LeafEyeCore::FileHistoryModel>();
            Assert::IsTrue(file != nullptr, L"Cast to FileHistoryModel should not be null");
            Assert::AreEqual(winrt::hstring(L"leaf_001.jpg"), file.FileName(), L"FileName should match");
            Assert::AreEqual(2048u, file.FileSize(), L"FileSize should match");
        }

        TEST_METHOD(FileHistory_Update_ShouldReflectNewValues) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"leaf_001.jpg", 2048, 1700000000LL, 1700003600LL, 0.95f)
            ).get();
            winrt::LeafEyeCore::FileHistoryModel filter;
            filter.FileName(L"leaf_001.jpg");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No file found to update");

            auto oldFile = entries.GetAt(0).try_as<winrt::LeafEyeCore::FileHistoryModel>();
            auto newFile = winrt::LeafEyeCore::FileHistoryModel(
                oldFile.Id(), L"leaf_001_updated.jpg", 4096, 1700000000LL, 1700007200LL, 0.88f
            );

            auto result = m_db.UpdateEntryAsync(winrt::LeafEyeCore::BoxType::FileHistory, oldFile, newFile).get();
            Assert::IsFalse(result.IsError(), std::format(L"UpdateEntry FileHistory Failed: {}", result.Message()).c_str());

            winrt::LeafEyeCore::FileHistoryModel updatedFilter;
            updatedFilter.FileName(L"leaf_001_updated.jpg");
            auto updated = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, updatedFilter).get();
            Assert::IsTrue(updated.Size() > 0, L"Updated file should be retrievable");

            auto updatedFile = updated.GetAt(0).try_as<winrt::LeafEyeCore::FileHistoryModel>();
            Assert::AreEqual(4096u, updatedFile.FileSize(), L"FileSize should be updated");
        }

        TEST_METHOD(FileHistory_Delete_ShouldRemoveEntry) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"delete_me.jpg", 1024, 1700000000LL, 1700000000LL, 0.5f)
            ).get();
            winrt::LeafEyeCore::FileHistoryModel filter;
            filter.FileName(L"delete_me.jpg");
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No file found to delete");

            auto result = m_db.DeleteEntryAsync(winrt::LeafEyeCore::BoxType::FileHistory, entries.GetAt(0)).get();
            Assert::IsFalse(result.IsError(), std::format(L"DeleteEntry FileHistory Failed: {}", result.Message()).c_str());

            auto remaining = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, filter).get();
            Assert::AreEqual(0u, remaining.Size(), L"Deleted file should no longer exist");
        }

        TEST_METHOD(FileHistory_Clear_ShouldRemoveAllEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"leaf_001.jpg", 2048, 1700000000LL, 1700003600LL, 0.95f)
            ).get();
            auto result = m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::FileHistory).get();
            Assert::IsFalse(result.IsError(), std::format(L"ClearEntries FileHistory Failed: {}", result.Message()).c_str());

            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::FileHistory, 100, 0).get();
            Assert::AreEqual(0u, entries.Size(), L"All file histories should be cleared");
        }

        // ===================== HISTORY =====================

        TEST_METHOD(History_Add_ShouldSucceed) {
            auto result = m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 5, 1)
            ).get();
            Assert::IsFalse(result.IsError(), std::format(L"AddEntry History Failed: {}", result.Message()).c_str());
        }

        TEST_METHOD(History_GetAll_ShouldReturnEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 5, 1)
            ).get();
            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::History, 100, 0).get();
            Assert::IsTrue(entries.Size() > 0, L"GetAllEntries History should return at least 1 entry");
        }

        TEST_METHOD(History_GetWithFilter_ShouldReturnMatchingStatus) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 5, 1)
            ).get();
            winrt::LeafEyeCore::HistoryModel filter;
            filter.Status(1);
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"Should find history with status 1");

            auto history = entries.GetAt(0).try_as<winrt::LeafEyeCore::HistoryModel>();
            Assert::IsTrue(history != nullptr, L"Cast to HistoryModel should not be null");
            Assert::AreEqual(1u, history.Status(), L"Status should match");
            Assert::AreEqual(5u, history.TotalFiles(), L"TotalFiles should match");
        }

        TEST_METHOD(History_Update_ShouldReflectNewValues) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 5, 1)
            ).get();
            winrt::LeafEyeCore::HistoryModel filter;
            filter.Status(1);
            auto entries = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, filter).get();
            Assert::IsTrue(entries.Size() > 0, L"No history found to update");

            auto oldHistory = entries.GetAt(0).try_as<winrt::LeafEyeCore::HistoryModel>();
            auto newHistory = winrt::LeafEyeCore::HistoryModel(oldHistory.Id(), 1700009999LL, 10, 2);

            auto result = m_db.UpdateEntryAsync(winrt::LeafEyeCore::BoxType::History, oldHistory, newHistory).get();
            Assert::IsFalse(result.IsError(), std::format(L"UpdateEntry History Failed: {}", result.Message()).c_str());

            winrt::LeafEyeCore::HistoryModel updatedFilter;
            updatedFilter.Status(2);
            auto updated = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, updatedFilter).get();
            Assert::IsTrue(updated.Size() > 0, L"Updated history should be retrievable");

            auto updatedHistory = updated.GetAt(0).try_as<winrt::LeafEyeCore::HistoryModel>();
            Assert::AreEqual(10u, updatedHistory.TotalFiles(), L"TotalFiles should be updated");
            Assert::AreEqual(2u, updatedHistory.Status(), L"Status should be updated");
        }

        TEST_METHOD(History_Clear_ShouldRemoveAllEntries) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 5, 1)
            ).get();
            auto result = m_db.ClearEntriesAsync(winrt::LeafEyeCore::BoxType::History).get();
            Assert::IsFalse(result.IsError(), std::format(L"ClearEntries History Failed: {}", result.Message()).c_str());

            auto entries = m_db.GetAllEntriesAsync(winrt::LeafEyeCore::BoxType::History, 100, 0).get();
            Assert::AreEqual(0u, entries.Size(), L"All histories should be cleared");
        }

        // ===================== RELATION: History <-> FileHistory =====================

        TEST_METHOD(Relation_History_FileHistory_AddAndVerify) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 1, 0)
            ).get();
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"relation_test.jpg", 1024, 1700000000LL, 1700000000LL, 0.88f)
            ).get();

            winrt::LeafEyeCore::HistoryModel hFilter;
            hFilter.Status(0);
            auto histories = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, hFilter).get();
            Assert::IsTrue(histories.Size() > 0, L"No history found for relation test");

            winrt::LeafEyeCore::FileHistoryModel fFilter;
            fFilter.FileName(L"relation_test.jpg");
            auto files = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, fFilter).get();
            Assert::IsTrue(files.Size() > 0, L"No file found for relation test");

            auto addResult = m_db.AddRelationAsync(
                winrt::LeafEyeCore::BoxType::History,
                histories.GetAt(0),
                files.GetAt(0)
            ).get();
            Assert::IsFalse(addResult.IsError(), std::format(L"AddRelation Failed: {}", addResult.Message()).c_str());

            auto linkedHistories = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, hFilter).get();
            Assert::IsTrue(linkedHistories.Size() > 0, L"No history found after relation");
            auto history = linkedHistories.GetAt(0).try_as<winrt::LeafEyeCore::HistoryModel>();
            Assert::IsTrue(history != nullptr, L"Cast to HistoryModel should not be null");
            Assert::IsTrue(history.Files().Size() > 0, L"History should have linked files");
            Assert::AreEqual(winrt::hstring(L"relation_test.jpg"), history.Files().GetAt(0).FileName(), L"Linked file name should match");
        }

        TEST_METHOD(Relation_History_FileHistory_Remove) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::History,
                winrt::LeafEyeCore::HistoryModel(0, 1700000000LL, 1, 0)
            ).get();
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::FileHistory,
                winrt::LeafEyeCore::FileHistoryModel(0, L"relation_test.jpg", 1024, 1700000000LL, 1700000000LL, 0.88f)
            ).get();

            winrt::LeafEyeCore::HistoryModel hFilter;
            hFilter.Status(0);
            auto histories = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, hFilter).get();

            winrt::LeafEyeCore::FileHistoryModel fFilter;
            fFilter.FileName(L"relation_test.jpg");
            auto files = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::FileHistory, fFilter).get();

            m_db.AddRelationAsync(winrt::LeafEyeCore::BoxType::History, histories.GetAt(0), files.GetAt(0)).get();

            auto removeResult = m_db.RemoveRelationAsync(
                winrt::LeafEyeCore::BoxType::History,
                histories.GetAt(0),
                files.GetAt(0)
            ).get();
            Assert::IsFalse(removeResult.IsError(), std::format(L"RemoveRelation Failed: {}", removeResult.Message()).c_str());

            auto linkedHistories = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::History, hFilter).get();
            auto history = linkedHistories.GetAt(0).try_as<winrt::LeafEyeCore::HistoryModel>();
            Assert::IsTrue(history != nullptr, L"Cast to HistoryModel should not be null");
            Assert::AreEqual(0u, history.Files().Size(), L"History should have no linked files after removal");
        }

        // ===================== RELATION: User <-> Profile =====================

        TEST_METHOD(Relation_User_Profile_AddAndRemove) {
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::User,
                winrt::LeafEyeCore::UserModel(L"daberdev", L"123456", true)
            ).get();
            m_db.AddEntryAsync(
                winrt::LeafEyeCore::BoxType::Profile,
                winrt::LeafEyeCore::ProfileModel(0, L"Daber Dev", L"/avatars/daber.png", 1)
            ).get();

            winrt::LeafEyeCore::UserModel uFilter;
            uFilter.Username(L"daberdev");
            auto users = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::User, uFilter).get();
            Assert::IsTrue(users.Size() > 0, L"No user found for relation test");

            winrt::LeafEyeCore::ProfileModel pFilter;
            pFilter.Role(1);
            auto profiles = m_db.GetEntriesWithFilterAsync(winrt::LeafEyeCore::BoxType::Profile, pFilter).get();
            Assert::IsTrue(profiles.Size() > 0, L"No profile found for relation test");

            auto addResult = m_db.AddRelationAsync(
                winrt::LeafEyeCore::BoxType::User,
                users.GetAt(0),
                profiles.GetAt(0)
            ).get();
            Assert::IsFalse(addResult.IsError(), std::format(L"AddRelation User/Profile Failed: {}", addResult.Message()).c_str());

            auto removeResult = m_db.RemoveRelationAsync(
                winrt::LeafEyeCore::BoxType::User,
                users.GetAt(0),
                profiles.GetAt(0)
            ).get();
            Assert::IsFalse(removeResult.IsError(), std::format(L"RemoveRelation User/Profile Failed: {}", removeResult.Message()).c_str());
        }
    };
}