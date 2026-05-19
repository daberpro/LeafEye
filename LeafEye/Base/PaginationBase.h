#pragma once
#include <map>
#include <functional>
#include <winrt/Windows.Foundation.h>

namespace winrt::LeafEye::Utils
{
    template <typename TEnum>
    class PaginationBase
    {
    protected:
        // Definisi tipe fungsi callback
        using FetchDataFunc = std::function<winrt::fire_and_forget(int32_t offset, int32_t limit)>;
        using FetchCountFunc = std::function<winrt::Windows::Foundation::IAsyncOperation<uint64_t>()>;

        // State pagination
        int32_t m_currentPage{ 1 };
        int32_t m_pageSize{ 10 };
        int32_t m_totalPages{ 1 };
        int32_t m_totalItems{ 0 };
        TEnum m_currentFilter;

        // Abstract method yang WAJIB diimplementasikan oleh kelas turunan (seperti UsersPage)
        virtual void OnPaginationUpdated() = 0;

        // Mendaftarkan fungsi logika untuk setiap filter
        void RegisterFilter(TEnum filterType, FetchCountFunc countCb, FetchDataFunc dataCb)
        {
            m_registry[filterType] = { countCb, dataCb };
        }

        // Pemicu aksi
        winrt::fire_and_forget SetFilterAsync(TEnum filterType)
        {
            m_currentFilter = filterType;
            m_currentPage = 1;
            co_await ExecuteCurrentStateAsync();
        }

        winrt::fire_and_forget NextPageAsync()
        {
            if (m_currentPage < m_totalPages) {
                m_currentPage++;
                co_await ExecuteCurrentStateAsync();
            }
        }

        winrt::fire_and_forget PrevPageAsync()
        {
            if (m_currentPage > 1) {
                m_currentPage--;
                co_await ExecuteCurrentStateAsync();
            }
        }

        winrt::fire_and_forget ChangePageSizeAsync(int32_t newSize)
        {
            m_pageSize = newSize;
            m_currentPage = 1;
            co_await ExecuteCurrentStateAsync();
        }

    private:
        struct Strategy {
            FetchCountFunc countFunc;
            FetchDataFunc dataFunc;
        };
        std::map<TEnum, Strategy> m_registry;

        // Otak utama yang mengeksekusi logika sesuai filter aktif
        winrt::Windows::Foundation::IAsyncAction ExecuteCurrentStateAsync()
        {
            if (m_registry.find(m_currentFilter) == m_registry.end()) {
                co_return; // Hindari eksekusi jika filter belum di-register
            }

            auto& strategy = m_registry[m_currentFilter];

            // 1. Ambil total data
            uint64_t totalItems = co_await strategy.countFunc();
            m_totalItems = static_cast<int32_t>(totalItems);

            // 2. Kalkulasi Halaman
            m_totalPages = static_cast<int32_t>(std::ceil(static_cast<double>(m_totalItems) / m_pageSize));
            if (m_totalPages < 1) m_totalPages = 1;

            // 3. Kalkulasi Offset
            int32_t offset = (m_currentPage - 1) * m_pageSize;

            // 4. Panggil fungsi pengambil data (fire_and_forget)
            strategy.dataFunc(offset, m_pageSize);

            // 5. Beritahu UI bahwa kalkulasi selesai
            OnPaginationUpdated();
        }
    };
}