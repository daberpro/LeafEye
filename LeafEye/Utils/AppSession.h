#pragma once
#include "../pch.h"
#include <winrt/LeafEyeCore.h>
#include <shared_mutex>

namespace winrt::LeafEye::Utils {
    class AppSession {
    public:
        AppSession() = delete;

        // --- DATABASE ---
        static winrt::LeafEyeCore::Database GetDatabase() {
            std::shared_lock lock(m_mutex); // Multiple threads bisa read bersamaan
            return m_db;
        }

        static void SetDatabase(winrt::LeafEyeCore::Database const& db) {
            std::unique_lock lock(m_mutex); // Lock eksklusif saat write
            m_db = db;
        }

        // --- PROFILE ---
        static winrt::LeafEyeCore::ProfileModel GetProfile() {
            std::shared_lock lock(m_mutex);
            return m_profile;
        }

        static void SetProfile(winrt::LeafEyeCore::ProfileModel const& profile) {
            std::unique_lock lock(m_mutex);
            m_profile = profile;
        }

        // --- USER ---
        static winrt::LeafEyeCore::UserModel GetUser() {
            std::shared_lock lock(m_mutex);
            return m_user;
        }

        static void SetUser(winrt::LeafEyeCore::UserModel const& user) {
            std::unique_lock lock(m_mutex);
            m_user = user;
        }

        // --- TEARDOWN ---
        // Panggil ini saat aplikasi akan ditutup (misal di event MainWindow::Closed)
        static void ClearSession() {
            std::unique_lock lock(m_mutex);
            m_db = nullptr;
            m_profile = nullptr;
            m_user = nullptr;
        }

    private:
        inline static winrt::LeafEyeCore::Database m_db{ nullptr };
        inline static winrt::LeafEyeCore::ProfileModel m_profile{ nullptr };
        inline static winrt::LeafEyeCore::UserModel m_user{ nullptr };

        // Satu mutex untuk melindungi semua state dalam sesi ini
        inline static std::shared_mutex m_mutex;
    };
}