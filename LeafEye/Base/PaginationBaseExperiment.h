#pragma once
#include <unordered_map>
#include <functional>

namespace winrt::LeafEye::Utils {


	template <typename TEnum>
	class PaginationBase {
	protected:

		using FetchDataFuncDecl = std::function<winrt::fire_and_forget(int64_t offset, int64_t limit)>;
		using CountDataFuncDecl = std::function<winrt::Windows::Foundation::IAsyncOperation<uint64_t>()>;

		struct Callback {
			FetchDataFuncDecl GetData;
			CountDataFuncDecl GetDataCount;
		};

		std::map<TEnum, Callback> m_callbacks;
		TEnum m_current_filter;
		Callback m_filter_callback;

		void Reset() {
			m_pagination_total_page = 0;
			m_pagination_current_page = 0;
		}

		winrt::fire_and_forget PaginationSetUp() {

			Reset();

			if (m_callbacks.contains(m_current_filter)) {
				m_filter_callback = m_callbacks[m_current_filter];

				// mendapatkan jumlah page
				m_filter_callback.GetData(m_pagination_current_page * m_pagination_limit, m_pagination_limit);
				int64_t total_data = co_await m_filter_callback.GetDataCount();

				m_pagination_total_page = std::ceil(static_cast<double>(total_data) / m_pagination_limit);

				NextPageEnabled(m_pagination_current_page < m_pagination_total_page);
				PrevPageEnabled(m_pagination_current_page > 0);

				UpdatePaginationInfo();

			}
		}

	public:

		virtual void PrevPageEnabled(bool value) {};
		virtual void NextPageEnabled(bool value) {};
		virtual void UpdatePaginationInfo() {};
		
		int64_t m_pagination_total_page{ 0 };
		int64_t m_pagination_current_page{ 0 };
		int64_t m_pagination_limit{ 10 };

		void RegisterPaginationCallback(const TEnum& filter,FetchDataFuncDecl fetch_cb, CountDataFuncDecl count_cb) {
			if (m_callbacks.contains(filter)) {
				return;
			}
			m_callbacks[filter] = {fetch_cb,count_cb};
		}

		void SetPaginationFilter(const TEnum& filter) {
			m_current_filter = filter;
			PaginationSetUp();
		}

		void NextPage() {
			auto is_enabled = m_pagination_current_page + 1 < m_pagination_total_page;
			NextPageEnabled(is_enabled);
			if (is_enabled) {
				m_pagination_current_page++;
				PrevPageEnabled(m_pagination_current_page > 0);
				UpdatePaginationInfo();
				m_filter_callback.GetData(m_pagination_current_page * m_pagination_limit,m_pagination_limit);
			}
		}

		void PrevPage() {
			auto is_enabled = m_pagination_current_page > 0;
			PrevPageEnabled(is_enabled);
			if (is_enabled) {
				m_pagination_current_page--;
				NextPageEnabled(m_pagination_current_page + 1 < m_pagination_total_page);
				UpdatePaginationInfo();
				m_filter_callback.GetData(m_pagination_current_page * m_pagination_limit, m_pagination_limit);
			}
		}

	};
}
