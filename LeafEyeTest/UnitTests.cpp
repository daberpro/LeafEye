#include "pch.h"
#include <winrt/LeafEyeCore.h>
#include "CppUnitTest.h"
#include <format>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// ── hstring formatter for Assert::AreEqual ──────────────────────────────────
namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template<>
    inline std::wstring ToString<winrt::hstring>(const winrt::hstring& t)
    {
        return std::wstring(t.c_str());
    }
}

// ── Helpers ──────────────────────────────────────────────────────────────────

// Build an assertion message string from a Result's hstring Message.
// Cannot use std::format with hstring directly; concatenate wstrings instead.
static std::wstring Msg(const winrt::LeafEyeCore::Result& r, const wchar_t* prefix)
{
    return std::wstring(prefix) + L": " + r.Message().c_str();
}

// Unbox a typed IVector out of a Result's IInspectable ResultValue.
template<typename T>
winrt::Windows::Foundation::Collections::IVector<T>
UnboxVector(const winrt::LeafEyeCore::Result& r)
{
    return r.ResultValue().try_as<winrt::Windows::Foundation::Collections::IVector<T>>();
}

// ────────────────────────────────────────────────────────────────────────────
namespace LeafEyeTest
{
    TEST_CLASS(DatabaseTests)
    {
    private:
        winrt::hstring m_db_path{
            winrt::Windows::Storage::ApplicationData::Current().LocalFolder().Path()
            + L"\\db_unit_test"
        };
        winrt::LeafEyeCore::Database m_db{ nullptr };

        // ── Lifecycle ─────────────────────────────────────────────────────────

        TEST_METHOD_INITIALIZE(Setup)
        {
            m_db = winrt::LeafEyeCore::Database(m_db_path, 1024 * 20 /*20 MB*/);
            auto r = m_db.InitializeAsync().get();
            Assert::IsFalse(r.IsError(), Msg(r, L"InitializeAsync failed").c_str());
        }

        TEST_METHOD_CLEANUP(Teardown)
        {
            // Tests use unique string values to avoid cross-test contamination.
        }

    public:

        // ════════════════════════════════════════════════════════════════════
        //  DATABASE INIT
        // ════════════════════════════════════════════════════════════════════

        TEST_METHOD(Database_Initialize_ShouldSucceed)
        {
            Assert::IsTrue(true); // Setup() already asserts this
        }

        // ════════════════════════════════════════════════════════════════════
        //  USER — CRUD
        // ════════════════════════════════════════════════════════════════════

        TEST_METHOD(User_Add_ShouldSucceed)
        {
            winrt::LeafEyeCore::UserModel user(L"t_add_user", L"pass123", false);
            auto r = m_db.AddUser(user).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"AddUser failed").c_str());
        }

        TEST_METHOD(User_GetByUsername_ShouldReturnUser)
        {
            winrt::LeafEyeCore::UserModel user(L"t_getbyname", L"pass123", false);
            m_db.AddUser(user).get();

            auto r = m_db.GetUserByUsername(L"t_getbyname").get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetUserByUsername failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected a user to be returned");

            auto fetched = winrt::unbox_value<winrt::LeafEyeCore::UserModel>(r.ResultValue());
            Assert::AreEqual(winrt::hstring(L"t_getbyname"), fetched.Username(), L"Username should match");
        }

        TEST_METHOD(User_GetById_ShouldReturnUser)
        {
            winrt::LeafEyeCore::UserModel user(L"t_getbyid", L"pass123", true);
            m_db.AddUser(user).get();

            // Get the auto-assigned id via username lookup first
            auto byName = m_db.GetUserByUsername(L"t_getbyid").get();
            Assert::IsTrue(byName.IsValueExists(), L"Prerequisite: user must exist by username");
            uint64_t id = winrt::unbox_value<winrt::LeafEyeCore::UserModel>(byName.ResultValue()).Id();
            Assert::IsTrue(id != 0, L"Id should have been assigned by ObjectBox");

            auto r = m_db.GetUserById(id).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetUserById failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected a user to be returned");

            auto fetched = winrt::unbox_value<winrt::LeafEyeCore::UserModel>(r.ResultValue());
            Assert::AreEqual(winrt::hstring(L"t_getbyid"), fetched.Username(), L"Username should match");
        }

        TEST_METHOD(User_ContainsUsername_ShouldReturnMatch)
        {
            winrt::LeafEyeCore::UserModel user(L"t_containstest_abc", L"pass", false);
            m_db.AddUser(user).get();

            auto r = m_db.GetUserContainsUsername(L"t_contains", 0, 10).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetUserContainsUsername failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected a matching user");
        }

        TEST_METHOD(User_GetAll_ShouldReturnAtLeastOneEntry)
        {
            winrt::LeafEyeCore::UserModel user(L"t_getall_user", L"pass", false);
            m_db.AddUser(user).get();

            auto r = m_db.GetAllUsers(0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetAllUsers failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected at least one user");

            auto vec = UnboxVector<winrt::LeafEyeCore::UserModel>(r);
            Assert::IsTrue(vec != nullptr && vec.Size() > 0, L"GetAllUsers should return at least 1 entry");
        }

        TEST_METHOD(User_GetAll_Pagination_LimitOne_ShouldReturnExactlyOne)
        {
            m_db.AddUser(winrt::LeafEyeCore::UserModel(L"t_page_u1", L"p", false)).get();
            m_db.AddUser(winrt::LeafEyeCore::UserModel(L"t_page_u2", L"p", false)).get();

            auto r = m_db.GetAllUsers(0, 1).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetAllUsers(0,1) failed").c_str());
            auto vec = UnboxVector<winrt::LeafEyeCore::UserModel>(r);
            Assert::IsTrue(vec != nullptr && vec.Size() == 1, L"Limit=1 should return exactly 1 user");
        }

        TEST_METHOD(User_ValidateCredentials_ValidUser_ShouldSucceed)
        {
            winrt::LeafEyeCore::UserModel user(L"t_validate_user", L"secret", false);
            m_db.AddUser(user).get();

            auto r = m_db.ValidateUserCredentials(L"t_validate_user", L"secret").get();
            Assert::IsFalse(r.IsError(), Msg(r, L"ValidateUserCredentials failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Valid credentials should return a user");
        }

        TEST_METHOD(User_ValidateCredentials_WrongPassword_ShouldNotReturnUser)
        {
            winrt::LeafEyeCore::UserModel user(L"t_validate_wrong", L"correctpass", false);
            m_db.AddUser(user).get();

            auto r = m_db.ValidateUserCredentials(L"t_validate_wrong", L"wrongpass").get();
            Assert::IsFalse(r.IsValueExists(), L"Wrong password should not return a user");
        }

        TEST_METHOD(User_Update_ShouldUpdatePassword)
        {
            winrt::LeafEyeCore::UserModel user(L"t_update_user", L"oldpass", false);
            m_db.AddUser(user).get();

            auto byName = m_db.GetUserByUsername(L"t_update_user").get();
            Assert::IsTrue(byName.IsValueExists(), L"User must exist before update");
            auto existing = winrt::unbox_value<winrt::LeafEyeCore::UserModel>(byName.ResultValue());

            // UpdateUser internally preserves the username; only password/isAdmin change.
            winrt::LeafEyeCore::UserModel updated(L"t_update_user", L"newpass", true);
            updated.Id(existing.Id());

            auto r = m_db.UpdateUser(updated).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"UpdateUser failed").c_str());

            auto validate = m_db.ValidateUserCredentials(L"t_update_user", L"newpass").get();
            Assert::IsTrue(validate.IsValueExists(), L"New password should be accepted after update");
        }

        TEST_METHOD(User_Delete_ShouldRemoveEntry)
        {
            winrt::LeafEyeCore::UserModel user(L"t_delete_user", L"pass", false);
            m_db.AddUser(user).get();

            auto byName = m_db.GetUserByUsername(L"t_delete_user").get();
            Assert::IsTrue(byName.IsValueExists(), L"User must exist before delete");
            uint64_t id = winrt::unbox_value<winrt::LeafEyeCore::UserModel>(byName.ResultValue()).Id();

            auto r = m_db.DeleteUser(id).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"DeleteUser failed").c_str());

            auto after = m_db.GetUserById(id).get();
            Assert::IsFalse(after.IsValueExists(), L"User should not exist after deletion");
        }

        TEST_METHOD(User_GetById_NotFound_ShouldNotHaveValue)
        {
            auto r = m_db.GetUserById(0xDEADBEEFDEADBEEFULL).get();
            Assert::IsFalse(r.IsValueExists(), L"Non-existent id should not return a value");
        }

        TEST_METHOD(User_GetByUsername_NotFound_ShouldNotHaveValue)
        {
            auto r = m_db.GetUserByUsername(L"__no_such_user__").get();
            Assert::IsFalse(r.IsValueExists(), L"Non-existent username should not return a value");
        }

        // ════════════════════════════════════════════════════════════════════
        //  PROFILE — CRUD
        // ════════════════════════════════════════════════════════════════════

        TEST_METHOD(Profile_Add_ShouldSucceed)
        {
            winrt::LeafEyeCore::ProfileModel profile(0ULL, L"Test User", L"/avatars/test.png", 1);
            auto r = m_db.AddUserProfile(profile).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"AddUserProfile failed").c_str());
        }

        TEST_METHOD(Profile_GetByUserLink_ShouldNotError)
        {
            // Without a wired-up User→Profile relation the result is empty,
            // but the call must not throw.
            auto r = m_db.GetUserProfileByLink(9999999ULL).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetUserProfileByLink returned error").c_str());
        }

        TEST_METHOD(Profile_Delete_ShouldSucceed)
        {
            winrt::LeafEyeCore::ProfileModel profile(0ULL, L"ToDelete Profile", L"/avatars/del.png", 99);
            m_db.AddUserProfile(profile).get();

            // ObjectBox remove(0) is a no-op; confirms no exception surfaces through Result.
            auto r = m_db.DeleteUserProfile(0ULL).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"DeleteUserProfile failed").c_str());
        }

        // ════════════════════════════════════════════════════════════════════
        //  HISTORY — CRUD
        // ════════════════════════════════════════════════════════════════════

        TEST_METHOD(History_Add_ShouldSucceed)
        {
            winrt::LeafEyeCore::HistoryModel history(0ULL, 1700000000LL, 5u, 1u);
            auto r = m_db.AddHistory(history).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"AddHistory failed").c_str());
        }

        TEST_METHOD(History_GetByStatus_ShouldReturnMatchingEntries)
        {
            winrt::LeafEyeCore::HistoryModel history(0ULL, 1700000000LL, 3u, 42u /*unique status*/);
            m_db.AddHistory(history).get();

            auto r = m_db.GetHistoryByStatus(42, 0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetHistoryByStatus failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected at least one history with status 42");

            auto vec = UnboxVector<winrt::LeafEyeCore::HistoryModel>(r);
            Assert::IsTrue(vec != nullptr && vec.Size() > 0, L"Should return histories matching status");
            Assert::AreEqual(42u, vec.GetAt(0).Status(), L"Status should be 42");
        }

        TEST_METHOD(History_GetByStatus_Pagination_OffsetSkipsFirstResult)
        {
            m_db.AddHistory(winrt::LeafEyeCore::HistoryModel(0ULL, 1700000010LL, 1u, 55u)).get();
            m_db.AddHistory(winrt::LeafEyeCore::HistoryModel(0ULL, 1700000020LL, 2u, 55u)).get();

            auto all = m_db.GetHistoryByStatus(55, 0, 100).get();
            auto vecAll = UnboxVector<winrt::LeafEyeCore::HistoryModel>(all);
            Assert::IsTrue(vecAll != nullptr && vecAll.Size() >= 2, L"Should find at least 2 histories");

            auto paged = m_db.GetHistoryByStatus(55, 1, 100).get();
            auto vecPaged = UnboxVector<winrt::LeafEyeCore::HistoryModel>(paged);
            Assert::IsTrue(vecPaged != nullptr && vecPaged.Size() == vecAll.Size() - 1,
                L"Offset=1 should skip the first result");
        }

        TEST_METHOD(History_GetByDateRange_ShouldReturnMatchingEntries)
        {
            int64_t ts = 1710000000LL;
            winrt::LeafEyeCore::HistoryModel history(0ULL, ts, 2u, 1u);
            m_db.AddHistory(history).get();

            auto r = m_db.GetHistoryByDateRange(
                static_cast<uint64_t>(ts - 1000),
                static_cast<uint64_t>(ts + 1000),
                0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetHistoryByDateRange failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected at least one history in date range");
        }

        TEST_METHOD(History_GetByUserLink_ShouldNotError)
        {
            auto r = m_db.GetHistoryByUserLink(9999999ULL, 0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetHistoryByUserLink returned error").c_str());
        }

        TEST_METHOD(History_Update_ShouldReflectNewValues)
        {
            // Use a rare status value so we can reliably find this entry
            winrt::LeafEyeCore::HistoryModel history(0ULL, 1700000000LL, 5u, 101u);
            m_db.AddHistory(history).get();

            auto getR = m_db.GetHistoryByStatus(101, 0, 1).get();
            Assert::IsTrue(getR.IsValueExists(), L"History with status 101 must exist");
            auto fetched = UnboxVector<winrt::LeafEyeCore::HistoryModel>(getR).GetAt(0);

            winrt::LeafEyeCore::HistoryModel updated(fetched.Id(), 1700009999LL, 10u, 102u);
            auto r = m_db.UpdateHistory(updated).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"UpdateHistory failed").c_str());

            auto afterR = m_db.GetHistoryByStatus(102, 0, 10).get();
            Assert::IsTrue(afterR.IsValueExists(), L"Should find history with updated status 102");
            auto afterVec = UnboxVector<winrt::LeafEyeCore::HistoryModel>(afterR);
            Assert::AreEqual(10u, afterVec.GetAt(0).TotalFiles(), L"TotalFiles should be updated");
        }

        TEST_METHOD(History_Delete_ShouldRemoveEntry)
        {
            winrt::LeafEyeCore::HistoryModel history(0ULL, 1700000000LL, 1u, 77u /*unique status*/);
            m_db.AddHistory(history).get();

            auto getR = m_db.GetHistoryByStatus(77, 0, 1).get();
            Assert::IsTrue(getR.IsValueExists(), L"History must exist before deletion");
            uint64_t id = UnboxVector<winrt::LeafEyeCore::HistoryModel>(getR).GetAt(0).Id();

            auto r = m_db.DeleteHistory(id).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"DeleteHistory failed").c_str());

            auto afterR = m_db.GetHistoryByStatus(77, 0, 100).get();
            auto afterVec = UnboxVector<winrt::LeafEyeCore::HistoryModel>(afterR);
            Assert::IsTrue(afterVec == nullptr || afterVec.Size() == 0,
                L"Deleted history should not be found");
        }

        // ════════════════════════════════════════════════════════════════════
        //  FILE HISTORY — CRUD
        // ════════════════════════════════════════════════════════════════════

        TEST_METHOD(FileHistory_Add_ShouldSucceed)
        {
            winrt::LeafEyeCore::FileHistoryModel fh(0ULL, L"t_leaf_add.jpg", 2048u, 1700000000LL, 1700003600LL, 0.95f);
            auto r = m_db.AddFileHistory(fh).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"AddFileHistory failed").c_str());
        }

        TEST_METHOD(FileHistory_GetByConfidenceThreshold_ShouldReturnMatchingEntries)
        {
            winrt::LeafEyeCore::FileHistoryModel fh(0ULL, L"t_leaf_conf.jpg", 512u, 1700000002LL, 1700000002LL, 0.99f);
            m_db.AddFileHistory(fh).get();

            auto r = m_db.GetFileHistoriesByConfidenceThreshold(0.98, 0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetFileHistoriesByConfidenceThreshold failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected at least one entry with conf >= 0.98");

            auto vec = UnboxVector<winrt::LeafEyeCore::FileHistoryModel>(r);
            bool found = false;
            for (uint32_t i = 0; i < vec.Size(); ++i) {
                if (vec.GetAt(i).FileName() == winrt::hstring(L"t_leaf_conf.jpg")) { found = true; break; }
            }
            Assert::IsTrue(found, L"t_leaf_conf.jpg (conf 0.99) should appear in threshold query");
        }

        TEST_METHOD(FileHistory_GetById_ShouldReturnEntry)
        {
            winrt::LeafEyeCore::FileHistoryModel fh(0ULL, L"t_leaf_getbyid.jpg", 1024u, 1700000001LL, 1700000001LL, 0.80f);
            m_db.AddFileHistory(fh).get();

            // Locate the entry first via confidence threshold to get the assigned id
            auto byConf = m_db.GetFileHistoriesByConfidenceThreshold(0.79, 0, 100).get();
            Assert::IsTrue(byConf.IsValueExists(), L"FileHistory must exist");
            auto vec = UnboxVector<winrt::LeafEyeCore::FileHistoryModel>(byConf);

            winrt::LeafEyeCore::FileHistoryModel target{ nullptr };
            for (uint32_t i = 0; i < vec.Size(); ++i) {
                if (vec.GetAt(i).FileName() == winrt::hstring(L"t_leaf_getbyid.jpg")) {
                    target = vec.GetAt(i); break;
                }
            }
            Assert::IsTrue(target != nullptr, L"Could not locate inserted file history by filename");

            auto r = m_db.GetFileHistoryById(target.Id()).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetFileHistoryById failed").c_str());
            Assert::IsTrue(r.IsValueExists(), L"Expected a file history to be returned");
        }

        TEST_METHOD(FileHistory_GetByHistoryLink_ShouldNotError)
        {
            auto r = m_db.GetFileHistoriesByHistoryLink(9999999ULL, 0, 100).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"GetFileHistoriesByHistoryLink returned error").c_str());
        }

        TEST_METHOD(FileHistory_Update_ShouldReflectNewValues)
        {
            winrt::LeafEyeCore::FileHistoryModel fh(0ULL, L"t_leaf_upd_orig.jpg", 1024u, 1700000003LL, 1700000003LL, 0.60f);
            m_db.AddFileHistory(fh).get();

            // Find the entry by confidence to get its assigned id
            auto byConf = m_db.GetFileHistoriesByConfidenceThreshold(0.59, 0, 100).get();
            Assert::IsTrue(byConf.IsValueExists(), L"Should find file to update");
            auto vec = UnboxVector<winrt::LeafEyeCore::FileHistoryModel>(byConf);

            winrt::LeafEyeCore::FileHistoryModel target{ nullptr };
            for (uint32_t i = 0; i < vec.Size(); ++i) {
                if (vec.GetAt(i).FileName() == winrt::hstring(L"t_leaf_upd_orig.jpg")) {
                    target = vec.GetAt(i); break;
                }
            }
            Assert::IsTrue(target != nullptr, L"Could not locate file history to update");

            winrt::LeafEyeCore::FileHistoryModel updated(
                target.Id(), L"t_leaf_upd_new.jpg", 4096u, 1700000003LL, 1700009999LL, 0.75f);
            auto r = m_db.UpdateFileHistory(updated).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"UpdateFileHistory failed").c_str());

            auto afterR = m_db.GetFileHistoryById(target.Id()).get();
            Assert::IsTrue(afterR.IsValueExists(), L"Updated file history should still exist");
            auto afterFH = afterR.ResultValue().try_as<winrt::LeafEyeCore::FileHistoryModel>();
            Assert::AreEqual(winrt::hstring(L"t_leaf_upd_new.jpg"), afterFH.FileName(),
                L"FileName should reflect the update");
            Assert::AreEqual(4096u, afterFH.FileSize(), L"FileSize should reflect the update");
        }

        TEST_METHOD(FileHistory_Delete_ShouldRemoveEntry)
        {
            winrt::LeafEyeCore::FileHistoryModel fh(0ULL, L"t_leaf_del.jpg", 256u, 1700000004LL, 1700000004LL, 0.50f);
            m_db.AddFileHistory(fh).get();

            auto byConf = m_db.GetFileHistoriesByConfidenceThreshold(0.49, 0, 100).get();
            auto vec = UnboxVector<winrt::LeafEyeCore::FileHistoryModel>(byConf);

            winrt::LeafEyeCore::FileHistoryModel target{ nullptr };
            for (uint32_t i = 0; i < vec.Size(); ++i) {
                if (vec.GetAt(i).FileName() == winrt::hstring(L"t_leaf_del.jpg")) {
                    target = vec.GetAt(i); break;
                }
            }
            Assert::IsTrue(target != nullptr, L"File to delete must exist");

            auto r = m_db.DeleteFileHistory(target.Id()).get();
            Assert::IsFalse(r.IsError(), Msg(r, L"DeleteFileHistory failed").c_str());

            auto afterR = m_db.GetFileHistoryById(target.Id()).get();
            Assert::IsFalse(afterR.IsValueExists(), L"Deleted file history should not be found");
        }

        TEST_METHOD(FileHistory_GetById_NotFound_ShouldNotHaveValue)
        {
            auto r = m_db.GetFileHistoryById(0xDEADBEEFDEADBEEFULL).get();
            Assert::IsFalse(r.IsValueExists(), L"Non-existent file history id should not return a value");
        }
    };
}