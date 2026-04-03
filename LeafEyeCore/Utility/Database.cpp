#include "pch.h"
#include "Database.h"
#include "Database.g.cpp"

namespace winrt::LeafEyeCore::implementation
{
	// ─── Helpers ──────────────────────────────────────────────────────────────
	// Definisi Panjang Tetap
	const int ITERATIONS = 10000;
	const int SALT_SIZE = 16;          // 16 byte = 32 karakter hex
	const int HASH_SIZE = 32;          // 32 byte = 64 karakter hex
	const int ITER_CHARS = 6;          // Alokasi 6 digit untuk jumlah iterasi (max 999.999)

	// ─── Validation Constants ─────────────────────────────────────────────────
	constexpr size_t MAX_USERNAME_LEN = 64;
	constexpr size_t MIN_USERNAME_LEN = 3;
	constexpr size_t MAX_PASSWORD_LEN = 128;
	constexpr size_t MIN_PASSWORD_LEN = 6;
	constexpr size_t MAX_FULLNAME_LEN = 128;
	constexpr size_t MAX_PATH_LEN = 512;
	constexpr size_t MAX_FILENAME_LEN = 256;
	constexpr int32_t MAX_LIMIT = 500;
	constexpr int32_t MIN_LIMIT = 1;
	constexpr float MIN_CONFIDENCE = 0.0f;
	constexpr float MAX_CONFIDENCE = 1.0f;

	// ─── Validation Helpers ───────────────────────────────────────────────────

	static bool IsValidId(uint64_t id) {
		return id > 0;
	}

	static bool IsValidPagination(int32_t offset, int32_t limit) {
		return offset >= 0 && limit >= MIN_LIMIT && limit <= MAX_LIMIT;
	}

	static bool IsValidUsername(const hstring& username) {
		auto len = winrt::to_string(username).size();
		return len >= MIN_USERNAME_LEN && len <= MAX_USERNAME_LEN;
	}

	static bool IsValidPassword(const hstring& password) {
		auto len = winrt::to_string(password).size();
		return len >= MIN_PASSWORD_LEN && len <= MAX_PASSWORD_LEN;
	}

	static bool IsValidFullname(const hstring& fullname) {
		return !fullname.empty() && winrt::to_string(fullname).size() <= MAX_FULLNAME_LEN;
	}

	static bool IsValidAvatarPath(const hstring& path) {
		return winrt::to_string(path).size() <= MAX_PATH_LEN; // empty is allowed (no avatar)
	}

	static bool IsValidFilename(const hstring& filename) {
		return !filename.empty() && winrt::to_string(filename).size() <= MAX_FILENAME_LEN;
	}

	static bool IsValidConfidence(float confidence) {
		return confidence >= MIN_CONFIDENCE && confidence <= MAX_CONFIDENCE;
	}

	static bool IsValidDateRange(uint64_t start, uint64_t end) {
		return start > 0 && end > 0 && start <= end;
	}

	// Returns an error Result with given message (errorCode -2 = validation error)
	static winrt::LeafEyeCore::Result MakeValidationError(const winrt::hstring& message) {
		return winrt::make<implementation::Result>(true, false, -2, message, nullptr);
	}

	// ─── Hash Helpers ─────────────────────────────────────────────────────────

	std::string toHex(const unsigned char* data, size_t len) {
		std::stringstream ss;
		for (size_t i = 0; i < len; i++)
			ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
		return ss.str();
	}

	std::vector<unsigned char> fromHex(const std::string& hex) {
		std::vector<unsigned char> bytes;
		for (unsigned int i = 0; i < hex.length(); i += 2) {
			bytes.push_back((unsigned char)strtol(hex.substr(i, 2).c_str(), NULL, 16));
		}
		return bytes;
	}

	// 1. Membuat Hash (Output: [Iterasi(6)][Salt(32)][Hash(64)])
	std::string createSecureHash(const std::string& password) {
		unsigned char salt[SALT_SIZE];
		RAND_bytes(salt, sizeof(salt));

		unsigned char outHash[HASH_SIZE];
		PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt, SALT_SIZE,
			ITERATIONS, EVP_sha256(), HASH_SIZE, outHash);

		// Gunakan setfill('0') agar panjang iterasi selalu tetap 6 karakter
		std::stringstream ss;
		ss << std::setw(ITER_CHARS) << std::setfill('0') << ITERATIONS;
		ss << toHex(salt, SALT_SIZE);
		ss << toHex(outHash, HASH_SIZE);

		return ss.str();
	}

	// 2. Validasi Password
	bool verifySecureHash(const std::string& password, const std::string& storedData) {
		// Ambil bagian berdasarkan posisi (substring)
		// Pos 0-5: Iterasi | Pos 6-37: Salt | Pos 38-101: Hash
		int iterations = std::stoi(storedData.substr(0, ITER_CHARS));
		std::string saltHex = storedData.substr(ITER_CHARS, SALT_SIZE * 2);
		std::string actualHashHex = storedData.substr(ITER_CHARS + (SALT_SIZE * 2));

		std::vector<unsigned char> salt = fromHex(saltHex);
		unsigned char checkHash[HASH_SIZE];

		PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(),
			iterations, EVP_sha256(), HASH_SIZE, checkHash);

		return toHex(checkHash, HASH_SIZE) == actualHashHex;
	}

	// ─── Model Converters ─────────────────────────────────────────────────────

	static winrt::LeafEyeCore::UserModel UserToModel(const User& u)
	{
		auto model = winrt::make<implementation::UserModel>();
		model.Id(u.id);
		model.Username(winrt::to_hstring(u.username));
		model.Password(winrt::to_hstring(u.password));
		model.IsAdmin(u.is_admin);
		return model;
	}

	static winrt::LeafEyeCore::ProfileModel ProfileToModel(const Profile& p)
	{
		auto model = winrt::make<implementation::ProfileModel>();
		model.Id(p.id);
		model.Fullname(winrt::to_hstring(p.full_name));
		model.AvatarPath(winrt::to_hstring(p.avatar_path));
		model.Role(p.role);
		return model;
	}

	static winrt::LeafEyeCore::FileHistoryModel FileHistoryToModel(const FileHistory& fh)
	{
		auto model = winrt::make<implementation::FileHistoryModel>();
		model.Id(fh.id);
		model.FileName(winrt::to_hstring(fh.filename));
		model.FileSize(fh.file_size);
		model.DateCreated(fh.date_created);
		model.DateModified(fh.date_modified);
		model.ConfidenceScore(fh.confidence_score);
		return model;
	}

	static winrt::LeafEyeCore::HistoryModel HistoryToModel(const History& h)
	{
		auto model = winrt::make<implementation::HistoryModel>();
		model.Id(h.id);
		model.Date(h.date);
		model.TotalFiles(h.total_files);
		model.Status(h.status);
		model.OutputFolder(winrt::to_hstring(h.output_folder));
		return model;
	}

	static User ModelToUser(const winrt::LeafEyeCore::UserModel& m)
	{
		User u{};
		u.id = m.Id();
		u.username = winrt::to_string(m.Username());
		u.password = winrt::to_string(m.Password());
		u.is_admin = m.IsAdmin();
		return u;
	}

	static Profile ModelToProfile(const winrt::LeafEyeCore::ProfileModel& m)
	{
		Profile p{};
		p.id = m.Id();
		p.full_name = winrt::to_string(m.Fullname());
		p.avatar_path = winrt::to_string(m.AvatarPath());
		p.role = m.Role();
		return p;
	}

	static History ModelToHistory(const winrt::LeafEyeCore::HistoryModel& m)
	{
		History h{};
		h.id = m.Id();
		h.date = m.Date();
		h.total_files = m.TotalFiles();
		h.status = m.Status();
		h.output_folder = winrt::to_string(m.OutputFolder());
		return h;
	}

	static FileHistory ModelToFileHistory(const winrt::LeafEyeCore::FileHistoryModel& m)
	{
		FileHistory fh{};
		fh.id = m.Id();
		fh.filename = winrt::to_string(m.FileName());
		fh.file_size = m.FileSize();
		fh.date_created = m.DateCreated();
		fh.date_modified = m.DateModified();
		fh.confidence_score = m.ConfidenceScore();
		return fh;
	}

	// ─── Database ─────────────────────────────────────────────────────────────

	Database::Database(hstring const& database_location, uint64_t max_db_size)
		: m_database_location(database_location), m_max_db_size(max_db_size), m_is_initialized(false)
	{
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::InitializeAsync()
	{
		// result construct with param isError, isValueExists, errorCode, message, value
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);
		try {

			// Validate database location is not empty
			if (m_database_location.empty()) {
				result.Message(L"[VALIDATION ERROR][InitializeAsync] Database location cannot be empty.");
				co_return result;
			}

			obx::Options options(create_obx_model());
			options.directory(winrt::to_string(m_database_location));
			options.maxDbSizeInKb(m_max_db_size);

			m_store = std::make_unique<obx::Store>(options);
			m_user_box = std::make_unique<obx::Box<User>>(*m_store);
			m_profile_box = std::make_unique<obx::Box<Profile>>(*m_store);
			m_history_box = std::make_unique<obx::Box<History>>(*m_store);
			m_filehistory_box = std::make_unique<obx::Box<FileHistory>>(*m_store);

			// Get By Id
			m_query_user_by_id = std::make_unique<obx::Query<User>>(
				m_user_box->query(
					User_::id.equals(0)
				).build()
			);
			m_query_profile_by_id = std::make_unique<obx::Query<Profile>>(
				m_profile_box->query(
					Profile_::id.equals(0)
				).build()
			);
			m_query_history_by_id = std::make_unique<obx::Query<History>>(
				m_history_box->query(
					History_::id.equals(0)
				).build()
			);
			m_query_filehistory_by_id = std::make_unique<obx::Query<FileHistory>>(
				m_filehistory_box->query(
					FileHistory_::id.equals(0)
				).build()
			);

			// User query
			m_query_user_by_username = std::make_unique<obx::Query<User>>(
				m_user_box->query(
					User_::username.equals("")
				).build()
			);

			m_query_user_by_access_level = std::make_unique<obx::Query<User>>(
				m_user_box->query(
					User_::is_admin.equals(true)
				).build()
			);

			m_query_user_contains_username = std::make_unique<obx::Query<User>>(
				m_user_box->query(
					User_::username.contains("", false)
				).build()
			);

			m_query_user_by_credentials = std::make_unique<obx::Query<User>>(
				m_user_box->query(
					User_::username.equals("") && User_::password.equals("")
				).build()
			);

			// History query
			m_query_history_by_status = std::make_unique<obx::Query<History>>(
				m_history_box->query(
					History_::status.equals(0)
				).build()
			);

			auto query_builder_history = m_history_box->query();
			query_builder_history
				.link(History_::user_id)
				.with(User_::id.equals(0));
			m_query_history_by_user_link = std::make_unique<obx::Query<History>>(query_builder_history.build());

			// FileHistory query
			m_query_filehistory_by_confidence = std::make_unique<obx::Query<FileHistory>>(
				m_filehistory_box->query(
					FileHistory_::confidence_score.greaterOrEq(0)
				).build()
			);

			auto query_builder_filehistory = m_filehistory_box->query();
			query_builder_filehistory
				.link(FileHistory_::history_id)
				.with(History_::id.equals(0));
			m_query_filehistory_by_history_link = std::make_unique<obx::Query<FileHistory>>(query_builder_filehistory.build());

			// Profile query
			auto query_builder_profile = m_profile_box->query();
			query_builder_profile
				.backlink(User_::profile_id)
				.with(User_::id.equals(0));
			m_query_profile_by_user_link = std::make_unique<obx::Query<Profile>>(query_builder_profile.build());

			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"Database initialized successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][InitializeAsync] Failed to initialize: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][InitializeAsync] Failed to initialize: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][InitializeAsync] An unknown error occurred during initialization.");
		}

		co_return result;
	}

	// ─── User ─────────────────────────────────────────────────────────────────

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserById(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][GetUserById] ID must be greater than 0.");
			co_return result;
		}

		try {
			m_query_user_by_id->setParameter(User_::id, id);
			auto user = m_query_user_by_id->findFirst();
			if (!user) {
				result.Message(L"[DATABASE ERROR][GetUserById] User Not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.ResultValue(winrt::box_value(UserToModel(*user)));
				result.Message(L"User fetched successfully");
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserById] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserById] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserById] An unknown error occurred while fetching user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserByAccessLevel(bool is_admin, int32_t offset, int32_t limit) {
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		try {
			m_query_user_by_access_level->setParameter(User_::is_admin, is_admin);
			auto user = m_query_user_by_access_level->offset(offset).limit(limit).find();

			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel> users = winrt::single_threaded_vector<winrt::LeafEyeCore::UserModel>();

			for (auto u : user) {
				users.Append(UserToModel(u));
			}

			result.ErrorCode(0);
			result.IsError(false);
			result.ResultValue(winrt::box_value(users));
			result.Message(L"User fetched successfully");
			result.IsValueExists(true);

		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserByAccessLevel] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserByAccessLevel] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserByAccessLevel] An unknown error occurred while fetching user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserByUsername(hstring username)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (username.empty()) {
			result.Message(L"[VALIDATION ERROR][GetUserByUsername] Username cannot be empty.");
			co_return result;
		}
		if (winrt::to_string(username).size() > MAX_USERNAME_LEN) {
			result.Message(L"[VALIDATION ERROR][GetUserByUsername] Username exceeds maximum length.");
			co_return result;
		}

		try {
			m_query_user_by_username->setParameter(User_::username, winrt::to_string(username));
			auto user = m_query_user_by_username->findFirst();
			if (!user) {
				result.Message(L"[DATABASE ERROR][GetUserByUsername] User Not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.ResultValue(winrt::box_value(UserToModel(*user)));
				result.Message(L"User fetched successfully");
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserByUsername] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserByUsername] Failed to fetch user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserByUsername] An unknown error occurred while fetching user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserContainsUsername(hstring username, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetUserContainsUsername] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}
		if (winrt::to_string(username).size() > MAX_USERNAME_LEN) {
			result.Message(L"[VALIDATION ERROR][GetUserContainsUsername] Username exceeds maximum length.");
			co_return result;
		}

		try {
			m_query_user_contains_username->setParameter(User_::username, winrt::to_string(username));
			auto user = m_query_user_contains_username->limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel> users = winrt::single_threaded_vector<winrt::LeafEyeCore::UserModel>();
			for (auto& u : user) {
				users.Append(UserToModel(u));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.ResultValue(users);
			result.Message(L"Users fetched successfully");
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserContainsUsername] Failed to fetch users: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserContainsUsername] Failed to fetch users: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserContainsUsername] An unknown error occurred while fetching users.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetAllUsers(int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetAllUsers] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			m_query_user_contains_username->setParameter(User_::username, ""); // Match all users
			auto raw_users = m_query_user_contains_username->limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::UserModel> users = winrt::single_threaded_vector<winrt::LeafEyeCore::UserModel>();
			for (auto& user : raw_users) {
				users.Append(UserToModel(user));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.ResultValue(users);
			result.Message(L"Users fetched successfully");
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetAllUsers] Failed to fetch users: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetAllUsers] Failed to fetch users: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetAllUsers] An unknown error occurred while fetching users.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::ValidateUserCredentials(hstring username, hstring password)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (username.empty() || password.empty()) {
			result.Message(L"[VALIDATION ERROR][ValidateUserCredentials] Username and password cannot be empty.");
			co_return result;
		}
		if (winrt::to_string(username).size() > MAX_USERNAME_LEN) {
			result.Message(L"[VALIDATION ERROR][ValidateUserCredentials] Username exceeds maximum length.");
			co_return result;
		}
		if (winrt::to_string(password).size() > MAX_PASSWORD_LEN) {
			result.Message(L"[VALIDATION ERROR][ValidateUserCredentials] Password exceeds maximum length.");
			co_return result;
		}

		try {
			m_query_user_by_username->setParameter(User_::username, winrt::to_string(username));
			auto user = m_query_user_by_username->findFirst();
			if (!user) {
				result.Message(L"[DATABASE ERROR][ValidateUserCredentials] User not found");
			}
			else {
				if (verifySecureHash(winrt::to_string(password), user->password)) {
					result.ErrorCode(0);
					result.IsError(false);
					result.Message(L"User credentials valid");
					result.ResultValue(winrt::box_value(UserToModel(*user)));
					result.IsValueExists(true);
				}
				else {
					result.Message(L"[DATABASE ERROR][ValidateUserCredentials] Password does not match");
				}
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][ValidateUserCredentials] Failed to validate user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][ValidateUserCredentials] Failed to validate user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][ValidateUserCredentials] An unknown error occurred while validating user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddUser(winrt::LeafEyeCore::UserModel user)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidUsername(user.Username())) {
			result.Message(L"[VALIDATION ERROR][AddUser] Username must be between 3 and 64 characters.");
			co_return result;
		}
		if (!IsValidPassword(user.Password())) {
			result.Message(L"[VALIDATION ERROR][AddUser] Password must be between 6 and 128 characters.");
			co_return result;
		}

		try {
			// Check if username is already taken
			m_query_user_by_username->setParameter(User_::username, winrt::to_string(user.Username()));
			auto existing = m_query_user_by_username->findFirst();
			if (existing) {
				result.Message(L"[VALIDATION ERROR][AddUser] Username is already taken.");
				co_return result;
			}

			user.Password(
				winrt::to_hstring(createSecureHash(winrt::to_string(user.Password())))
			);
			m_user_box->put(ModelToUser(user));
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User added successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][AddUser] Failed to add user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][AddUser] Failed to add user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][AddUser] An unknown error occurred while adding user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::UpdateUser(winrt::LeafEyeCore::UserModel user)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(user.Id())) {
			result.Message(L"[VALIDATION ERROR][UpdateUser] User ID must be greater than 0.");
			co_return result;
		}
		// Password is optional on update, but validate length if provided
		if (!user.Password().empty() && !IsValidPassword(user.Password())) {
			result.Message(L"[VALIDATION ERROR][UpdateUser] Password must be between 6 and 128 characters.");
			co_return result;
		}

		try {
			// get existing user to preserve username and ID, and prevent accidental username changes which can cause issues with linked data
			auto existing_user = m_user_box->get(user.Id());
			if (!existing_user) {
				result.Message(L"[DATABASE ERROR][UpdateUser] User not found.");
				co_return result;
			}
			auto updated_user = ModelToUser(user);
			updated_user.id = existing_user->id; // Ensure the ID remains unchanged
			updated_user.username = existing_user->username;
			updated_user.is_admin = user.IsAdmin();
			if (!user.Password().empty()) {
				updated_user.password = createSecureHash(winrt::to_string(user.Password()));
			}
			else {
				updated_user.password = existing_user->password;
			}
			m_user_box->put(updated_user);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User updated successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateUser] Failed to update user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateUser] Failed to update user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][UpdateUser] An unknown error occurred while updating user.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::DeleteUser(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][DeleteUser] ID must be greater than 0.");
			co_return result;
		}

		try {
			// objectbox when one of relation get delete the standalone rel also get delete, so we don't need to manually delete profile and history linked to user
			m_user_box->remove(id);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User deleted successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteUser] Failed to delete user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteUser] Failed to delete user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][DeleteUser] An unknown error occurred while deleting user.");
		}

		co_return result;
	}

	// ─── Profile ──────────────────────────────────────────────────────────────

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetProfileById(uint64_t profileId)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(profileId)) {
			result.Message(L"[VALIDATION ERROR][GetProfileById] Profile ID must be greater than 0.");
			co_return result;
		}

		try {
			auto profile = m_query_profile_by_id->setParameter(Profile_::id, profileId).findFirst();
			if (!profile) {
				result.Message(L"[DATABASE ERROR][GetProfileById] User profile not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.Message(L"Profile found");
				result.ResultValue(winrt::box_value(ProfileToModel(*profile)));
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetProfileById] Failed to fetch profile: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetProfileById] Failed to fetch profile: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetProfileById] An unknown error occurred while fetching profile.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserProfileByLink(uint64_t userId)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(userId)) {
			result.Message(L"[VALIDATION ERROR][GetUserProfileByLink] User ID must be greater than 0.");
			co_return result;
		}

		try {
			auto profile = m_query_profile_by_user_link->setParameter(User_::id, userId).findFirst();
			if (!profile) {
				result.Message(L"[DATABASE ERROR][GetUserProfileByLink] User profile not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.Message(L"User profile found");
				result.ResultValue(winrt::box_value(ProfileToModel(*profile)));
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserProfileByLink] Failed to fetch profile: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserProfileByLink] Failed to fetch profile: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserProfileByLink] An unknown error occurred while fetching profile.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddUserProfile(winrt::LeafEyeCore::ProfileModel userProfile)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidFullname(userProfile.Fullname())) {
			result.Message(L"[VALIDATION ERROR][AddUserProfile] Full name cannot be empty and must not exceed 128 characters.");
			co_return result;
		}
		if (!IsValidAvatarPath(userProfile.AvatarPath())) {
			result.Message(L"[VALIDATION ERROR][AddUserProfile] Avatar path exceeds maximum length of 512 characters.");
			co_return result;
		}

		try {
			m_profile_box->put(ModelToProfile(userProfile));
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User profile added successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][AddUserProfile] Failed to add profile: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][AddUserProfile] Failed to add profile: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][AddUserProfile] An unknown error occurred while adding profile.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::UpdateUserProfile(winrt::LeafEyeCore::ProfileModel userProfile)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(userProfile.Id())) {
			result.Message(L"[VALIDATION ERROR][UpdateUserProfile] Profile ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidFullname(userProfile.Fullname())) {
			result.Message(L"[VALIDATION ERROR][UpdateUserProfile] Full name cannot be empty and must not exceed 128 characters.");
			co_return result;
		}
		if (!IsValidAvatarPath(userProfile.AvatarPath())) {
			result.Message(L"[VALIDATION ERROR][UpdateUserProfile] Avatar path exceeds maximum length of 512 characters.");
			co_return result;
		}

		try {
			auto existing_profile = m_profile_box->get(userProfile.Id());
			if (!existing_profile) {
				result.Message(L"[DATABASE ERROR][UpdateUserProfile] Profile not found.");
				co_return result;
			}
			auto updated_profile = ModelToProfile(userProfile);
			updated_profile.id = existing_profile->id; // Ensure the ID remains unchanged
			m_profile_box->put(updated_profile);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User profile updated successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateUserProfile] Failed to update profile: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateUserProfile] Failed to update profile: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][UpdateUserProfile] An unknown error occurred while updating profile.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::DeleteUserProfile(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][DeleteUserProfile] ID must be greater than 0.");
			co_return result;
		}

		try {
			m_profile_box->remove(id);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"User profile deleted successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteUserProfile] Failed to delete profile: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteUserProfile] Failed to delete profile: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][DeleteUserProfile] An unknown error occurred while deleting profile.");
		}

		co_return result;
	}

	// ─── History ──────────────────────────────────────────────────────────────

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetHistoryById(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryById] ID must be greater than 0.");
			co_return result;
		}

		try {
			auto history = m_query_history_by_id->setParameter(History_::id, id).findFirst();
			if (!history) {
				result.Message(L"[DATABASE ERROR][GetHistoryById] History not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.Message(L"History fetched successfully");
				result.ResultValue(winrt::box_value(HistoryToModel(*history)));
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryById] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryById] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetHistoryById] An unknown error occurred while fetching history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetHistoryByUserLink(uint64_t userId, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(userId)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByUserLink] User ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByUserLink] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			auto histories = m_query_history_by_user_link->setParameter(User_::id, userId).limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::HistoryModel> historyModels = winrt::single_threaded_vector<winrt::LeafEyeCore::HistoryModel>();
			for (auto& history : histories) {
				historyModels.Append(HistoryToModel(history));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History fetched successfully");
			result.ResultValue(historyModels);
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByUserLink] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByUserLink] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetHistoryByUserLink] An unknown error occurred while fetching history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetHistoryByStatus(int32_t status, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (status < 0) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByStatus] Status cannot be negative.");
			co_return result;
		}
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByStatus] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			auto histories = m_query_history_by_status->setParameter(History_::status, status).limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::HistoryModel> historyModels = winrt::single_threaded_vector<winrt::LeafEyeCore::HistoryModel>();
			for (auto& history : histories) {
				historyModels.Append(HistoryToModel(history));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History fetched successfully");
			result.ResultValue(historyModels);
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByStatus] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByStatus] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetHistoryByStatus] An unknown error occurred while fetching history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetHistoryByDateRange(uint64_t start, uint64_t end, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidDateRange(start, end)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByDateRange] Invalid date range. Start and end must be > 0 and start must be <= end.");
			co_return result;
		}
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetHistoryByDateRange] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			auto query = m_history_box->query(History_::date.between(start, end)).build();
			auto histories = query.limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::HistoryModel> historyModels = winrt::single_threaded_vector<winrt::LeafEyeCore::HistoryModel>();
			for (auto& history : histories) {
				historyModels.Append(HistoryToModel(history));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History fetched successfully");
			result.ResultValue(historyModels);
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByDateRange] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetHistoryByDateRange] Failed to fetch history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetHistoryByDateRange] An unknown error occurred while fetching history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddHistory(winrt::LeafEyeCore::HistoryModel history)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (history.Date() <= 0) {
			result.Message(L"[VALIDATION ERROR][AddHistory] History date must be a positive timestamp.");
			co_return result;
		}

		try {
			obx_id data_id = m_history_box->put(ModelToHistory(history));
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History added successfully");
			result.ResultValue(winrt::box_value(data_id));
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][AddHistory] Failed to add history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][AddHistory] Failed to add history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][AddHistory] An unknown error occurred while adding history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::UpdateHistory(winrt::LeafEyeCore::HistoryModel history)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(history.Id())) {
			result.Message(L"[VALIDATION ERROR][UpdateHistory] History ID must be greater than 0.");
			co_return result;
		}
		if (history.Date() <= 0) {
			result.Message(L"[VALIDATION ERROR][UpdateHistory] History date must be a positive timestamp.");
			co_return result;
		}

		try {
			auto existing_history = m_history_box->get(history.Id());
			if (!existing_history) {
				result.Message(L"[DATABASE ERROR][UpdateHistory] History not found.");
				co_return result;
			}
			auto updated_history = ModelToHistory(history);
			updated_history.id = existing_history->id; // Ensure the ID remains unchanged
			m_history_box->put(updated_history);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History updated successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateHistory] Failed to update history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateHistory] Failed to update history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][UpdateHistory] An unknown error occurred while updating history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::DeleteHistory(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][DeleteHistory] ID must be greater than 0.");
			co_return result;
		}

		try {
			m_history_box->remove(id);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History deleted successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteHistory] Failed to delete history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteHistory] Failed to delete history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][DeleteHistory] An unknown error occurred while deleting history.");
		}

		co_return result;
	}

	// ─── FileHistory ──────────────────────────────────────────────────────────

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetFileHistoryById(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][GetFileHistoryById] ID must be greater than 0.");
			co_return result;
		}

		try {
			auto fh = m_query_filehistory_by_id->setParameter(FileHistory_::id, id).findFirst();
			if (!fh) {
				result.Message(L"[DATABASE ERROR][GetFileHistoryById] File history not found");
			}
			else {
				result.ErrorCode(0);
				result.IsError(false);
				result.ResultValue(winrt::box_value(FileHistoryToModel(*fh)));
				result.Message(L"File history fetched successfully");
				result.IsValueExists(true);
			}
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoryById] Failed to fetch file history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoryById] Failed to fetch file history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetFileHistoryById] An unknown error occurred while fetching file history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetFileHistoriesByHistoryLink(uint64_t historyId, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(historyId)) {
			result.Message(L"[VALIDATION ERROR][GetFileHistoriesByHistoryLink] History ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetFileHistoriesByHistoryLink] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			auto fileHistories = m_query_filehistory_by_history_link->setParameter(History_::id, historyId).limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> fhModels = winrt::single_threaded_vector<winrt::LeafEyeCore::FileHistoryModel>();
			for (auto& fh : fileHistories) {
				fhModels.Append(FileHistoryToModel(fh));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"File histories fetched successfully");
			result.ResultValue(fhModels);
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByHistoryLink] Failed to fetch file histories: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByHistoryLink] Failed to fetch file histories: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByHistoryLink] An unknown error occurred while fetching file histories.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetFileHistoriesByConfidenceThreshold(double minConfidence, int32_t offset, int32_t limit)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (minConfidence < MIN_CONFIDENCE || minConfidence > MAX_CONFIDENCE) {
			result.Message(L"[VALIDATION ERROR][GetFileHistoriesByConfidenceThreshold] Confidence must be between 0.0 and 1.0.");
			co_return result;
		}
		if (!IsValidPagination(offset, limit)) {
			result.Message(L"[VALIDATION ERROR][GetFileHistoriesByConfidenceThreshold] Invalid offset or limit. Offset >= 0, limit must be between 1 and 500.");
			co_return result;
		}

		try {
			auto fileHistories = m_query_filehistory_by_confidence->setParameter(FileHistory_::confidence_score, minConfidence).limit(limit).offset(offset).find();
			winrt::Windows::Foundation::Collections::IVector<winrt::LeafEyeCore::FileHistoryModel> fhModels = winrt::single_threaded_vector<winrt::LeafEyeCore::FileHistoryModel>();
			for (auto& fh : fileHistories) {
				fhModels.Append(FileHistoryToModel(fh));
			}
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"File histories fetched successfully");
			result.ResultValue(fhModels);
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByConfidenceThreshold] Failed to fetch file histories: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByConfidenceThreshold] Failed to fetch file histories: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetFileHistoriesByConfidenceThreshold] An unknown error occurred while fetching file histories.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::AddFileHistory(winrt::LeafEyeCore::FileHistoryModel fileHistory)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidFilename(fileHistory.FileName())) {
			result.Message(L"[VALIDATION ERROR][AddFileHistory] Filename cannot be empty and must not exceed 256 characters.");
			co_return result;
		}
		if (!IsValidConfidence(fileHistory.ConfidenceScore())) {
			result.Message(L"[VALIDATION ERROR][AddFileHistory] Confidence score must be between 0.0 and 1.0.");
			co_return result;
		}
		if (fileHistory.DateCreated() <= 0 || fileHistory.DateModified() <= 0) {
			result.Message(L"[VALIDATION ERROR][AddFileHistory] DateCreated and DateModified must be positive timestamps.");
			co_return result;
		}

		try {
			obx_id data_id = m_filehistory_box->put(ModelToFileHistory(fileHistory));
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"File history added successfully");
			result.ResultValue(winrt::box_value(data_id));
			result.IsValueExists(true);
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][AddFileHistory] Failed to add file history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][AddFileHistory] Failed to add file history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][AddFileHistory] An unknown error occurred while adding file history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::UpdateFileHistory(winrt::LeafEyeCore::FileHistoryModel fileHistory)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(fileHistory.Id())) {
			result.Message(L"[VALIDATION ERROR][UpdateFileHistory] File history ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidFilename(fileHistory.FileName())) {
			result.Message(L"[VALIDATION ERROR][UpdateFileHistory] Filename cannot be empty and must not exceed 256 characters.");
			co_return result;
		}
		if (!IsValidConfidence(fileHistory.ConfidenceScore())) {
			result.Message(L"[VALIDATION ERROR][UpdateFileHistory] Confidence score must be between 0.0 and 1.0.");
			co_return result;
		}
		if (fileHistory.DateCreated() <= 0 || fileHistory.DateModified() <= 0) {
			result.Message(L"[VALIDATION ERROR][UpdateFileHistory] DateCreated and DateModified must be positive timestamps.");
			co_return result;
		}

		try {
			auto existing_fh = m_filehistory_box->get(fileHistory.Id());
			if (!existing_fh) {
				result.Message(L"[DATABASE ERROR][UpdateFileHistory] File history not found.");
				co_return result;
			}
			auto updated_fh = ModelToFileHistory(fileHistory);
			updated_fh.id = existing_fh->id; // Ensure the ID remains unchanged
			m_filehistory_box->put(updated_fh);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"File history updated successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateFileHistory] Failed to update file history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][UpdateFileHistory] Failed to update file history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][UpdateFileHistory] An unknown error occurred while updating file history.");
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::DeleteFileHistory(uint64_t id)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(id)) {
			result.Message(L"[VALIDATION ERROR][DeleteFileHistory] ID must be greater than 0.");
			co_return result;
		}

		try {
			m_filehistory_box->remove(id);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"File history deleted successfully");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteFileHistory] Failed to delete file history: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][DeleteFileHistory] Failed to delete file history: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][DeleteFileHistory] An unknown error occurred while deleting file history.");
		}

		co_return result;
	}

	// ─── Relation Links ───────────────────────────────────────────────────────

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::LinkUserProfile(uint64_t userId, uint64_t profileId)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(userId)) {
			result.Message(L"[VALIDATION ERROR][LinkUserProfile] User ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidId(profileId)) {
			result.Message(L"[VALIDATION ERROR][LinkUserProfile] Profile ID must be greater than 0.");
			co_return result;
		}

		try {
			auto user = m_user_box->get(userId);
			if (!user) {
				result.Message(L"[DATABASE ERROR][LinkUserProfile] User not found to link profile.");
				co_return result;
			}
			user->profile_id = profileId;
			m_user_box->put(*user);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"Profile linked to User successfully");
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][LinkUserProfile] Failed to link: " + winrt::to_hstring(e.what()));
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::LinkHistoryToUser(uint64_t historyId, uint64_t userId)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(historyId)) {
			result.Message(L"[VALIDATION ERROR][LinkHistoryToUser] History ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidId(userId)) {
			result.Message(L"[VALIDATION ERROR][LinkHistoryToUser] User ID must be greater than 0.");
			co_return result;
		}

		try {
			auto history = m_history_box->get(historyId);
			if (!history) {
				result.Message(L"[DATABASE ERROR][LinkHistoryToUser] History not found to link to user.");
				co_return result;
			}
			history->user_id = userId;
			m_history_box->put(*history);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"History linked to User successfully");
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][LinkHistoryToUser] Failed to link: " + winrt::to_hstring(e.what()));
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::LinkFileHistoryToHistory(uint64_t fileHistoryId, uint64_t historyId)
	{
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		// Validation
		if (!IsValidId(fileHistoryId)) {
			result.Message(L"[VALIDATION ERROR][LinkFileHistoryToHistory] FileHistory ID must be greater than 0.");
			co_return result;
		}
		if (!IsValidId(historyId)) {
			result.Message(L"[VALIDATION ERROR][LinkFileHistoryToHistory] History ID must be greater than 0.");
			co_return result;
		}

		try {
			auto fileHistory = m_filehistory_box->get(fileHistoryId);
			if (!fileHistory) {
				result.Message(L"[DATABASE ERROR][LinkFileHistoryToHistory] FileHistory not found to link to history.");
				co_return result;
			}
			fileHistory->history_id = historyId;
			m_filehistory_box->put(*fileHistory);
			result.ErrorCode(0);
			result.IsError(false);
			result.Message(L"FileHistory linked to History successfully");
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][LinkFileHistoryToHistory] Failed to link: " + winrt::to_hstring(e.what()));
		}

		co_return result;
	}

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserCountByAccessLevel(bool is_admin) {
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		try {
			auto count = m_query_user_by_access_level->setParameter(User_::is_admin, is_admin).find();
			result.ErrorCode(0);
			result.IsError(false);
			result.IsValueExists(true);
			result.ResultValue(winrt::box_value(count.size()));
			result.Message(L"Get users count successfuly");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCountByAccessLevel] Failed to get count of all users with access level: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCountByAccessLevel] Failed to get count of all users with access level: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserCountByAccessLevel] An unknown error occurred while get count of all users with access level.");
		}

		co_return result;
	};

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserCountContainsUsername(hstring username) {
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		try {
			auto count = m_query_user_contains_username->setParameter(User_::username, winrt::to_string(username)).find();
			result.ErrorCode(0);
			result.IsError(false);
			result.IsValueExists(true);
			result.ResultValue(winrt::box_value(count.size()));
			result.Message(L"Get users count successfuly");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCountContainsUsername] Failed to get count of all users contains username: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCountContainsUsername] Failed to get count of all users contains username: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserCountContainsUsername] An unknown error occurred while get count of all users contains username.");
		}

		co_return result;
	};

	winrt::Windows::Foundation::IAsyncOperation<winrt::LeafEyeCore::Result> Database::GetUserCount() {
		winrt::LeafEyeCore::Result result(true, false, -1, L"", nullptr);

		try {
			auto count = m_user_box->getAll();
			result.ErrorCode(0);
			result.IsError(false);
			result.ResultValue(winrt::box_value(count.size()));
			result.IsValueExists(true);
			result.Message(L"Get users count successfuly");
		}
		catch (obx::Exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCount] Failed to get count of all user: " + winrt::to_hstring(e.what()));
		}
		catch (std::exception& e) {
			result.Message(L"[DATABASE ERROR][GetUserCount] Failed to get count of all user: " + winrt::to_hstring(e.what()));
		}
		catch (...) {
			result.Message(L"[DATABASE ERROR][GetUserCount] An unknown error occurred while get count of all user.");
		}

		co_return result;
	};


	void Database::TxWrite() {
		m_current_transaction = std::make_unique<obx::Transaction>(m_store->txWrite());
	};

	void Database::TxRead() {
		m_current_transaction = std::make_unique<obx::Transaction>(m_store->txRead());
	};

	void Database::TxSuccess() {
		m_current_transaction->success();
	};

}