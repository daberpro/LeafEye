#include "pch.h"
#include "ProfilePage.xaml.h"
#if __has_include("ProfilePage.g.cpp")
#include "ProfilePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::LeafEye::implementation
{
	hstring ProfilePage::GetUserNameAndFullNameString(hstring const& username, hstring const& fullname) {
		return winrt::to_hstring(
			winrt::to_string(std::format(L"{} ({})", fullname, username))
		);
	};

	ProfilePage::ProfilePage()
	{
		InitializeComponent();
	}

	winrt::LeafEyeCore::ProfileModel ProfilePage::Profile() {
		return winrt::LeafEye::Utils::AppSession::GetProfile();
	}

	winrt::LeafEyeCore::UserModel ProfilePage::User() {
		return winrt::LeafEye::Utils::AppSession::GetUser();
	}

	void ProfilePage::Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
	{
		if (auto mainWindow = App::Window().try_as<winrt::LeafEye::implementation::MainWindow>())
			mainWindow->Logout();
	}

	winrt::fire_and_forget ProfilePage::EditPhoto_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e) {
		auto lifetime = get_strong();

		winrt::Windows::Storage::Pickers::FileOpenPicker picker;
		picker.ViewMode(winrt::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
		picker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);

		picker.FileTypeFilter().Append(L".jpg");
		picker.FileTypeFilter().Append(L".jpeg");
		picker.FileTypeFilter().Append(L".png");

		auto window = App::Window();
		HWND hwnd;
		winrt::check_hresult(window.as<IWindowNative>()->get_WindowHandle(&hwnd));

		auto initializeWithWindow{ picker.as<::IInitializeWithWindow>() };
		initializeWithWindow->Initialize(hwnd);

		m_profile_photo = co_await picker.PickSingleFileAsync();

		if (m_profile_photo)
		{
			// 1. Buka file sebagai RandomAccessStream
			winrt::Windows::Storage::Streams::IRandomAccessStream stream = co_await m_profile_photo.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read);

			// 2. Buat instance BitmapImage
			winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage;

			// 3. Set source gambar dari stream secara asinkron
			co_await bitmapImage.SetSourceAsync(stream);

			// 4. Terapkan ke kontrol Image di XAML sebagai preview
			ImageProfile().Source(bitmapImage);
		}
	}

	void ProfilePage::RaisePropertyChanged(hstring const& propertyName)
	{
		m_propertyChanged(*this, Data::PropertyChangedEventArgs{ propertyName });
	}

	event_token ProfilePage::PropertyChanged(Data::PropertyChangedEventHandler const& handler)
	{
		return m_propertyChanged.add(handler);
	}

	void ProfilePage::PropertyChanged(event_token const& token) noexcept
	{
		m_propertyChanged.remove(token);
	}

	bool ProfilePage::IsChanged() {
		return m_is_changed;
	}

	void ProfilePage::IsChanged(bool is_changed) {
		if (is_changed != m_is_changed) {
			m_is_changed = is_changed;
			RaisePropertyChanged(L"IsChanged");
		}
	}

	void ProfilePage::EvaluateSaveState()
	{
		auto profile = winrt::LeafEye::Utils::AppSession::GetProfile();
		auto user = winrt::LeafEye::Utils::AppSession::GetUser();
		bool hasChanges = false;

		// 1. Cek apakah ada foto baru yang di-pick
		if (m_profile_photo != nullptr) {
			hasChanges = true;
		}

		// 2. Cek perubahan informasi dasar (TextBox)
		if (FullNameInput().Text() != profile.Fullname()) hasChanges = true;
		if (UsernameInput().Text() != user.Username()) hasChanges = true;

		// 3. Cek perubahan password (jika pengguna mengetik walau 1 karakter)
		if (!OldPasswordInput().Password().empty() ||
			!NewPasswordInput().Password().empty() ||
			!ConfirmPasswordInput().Password().empty())
		{
			hasChanges = true;
		}

		IsChanged(hasChanges);
	}

	winrt::fire_and_forget ProfilePage::Save_Click(winrt::Windows::Foundation::IInspectable const& /*sender*/, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& /*e*/)
	{
		auto lifetime = get_strong(); // Amankan instance page
		auto profile = winrt::LeafEye::Utils::AppSession::GetProfile();
		auto user = winrt::LeafEye::Utils::AppSession::GetUser();
		auto database = winrt::LeafEye::Utils::AppSession::GetDatabase();

		hstring fatalErrorMessage;
		bool isPasswordUpdated = false;

		try {
			// ==========================================
			// 0. BACKUP STATE ASLI (Untuk Rollback)
			// ==========================================
			hstring originalUsername = user.Username();
			hstring originalPassword = user.Password();

			// ==========================================
			// 1. LOGIKA UPDATE FOTO
			// ==========================================
			if (m_profile_photo) {
				auto app_local_path = winrt::Windows::Storage::ApplicationData::Current().LocalFolder();
				auto target_folder = co_await app_local_path.CreateFolderAsync(L"profile_photo", winrt::Windows::Storage::CreationCollisionOption::OpenIfExists);

				hstring oldAvatarPath = profile.AvatarPath();

				winrt::Windows::Storage::StorageFile copiedFile = co_await m_profile_photo.CopyAsync(
					target_folder,
					m_profile_photo.Name(),
					winrt::Windows::Storage::NameCollisionOption::ReplaceExisting
				);

				if (!oldAvatarPath.empty()) {
					std::filesystem::path oldPath{ oldAvatarPath.c_str() };
					std::error_code ec;
					if (std::filesystem::exists(oldPath, ec)) {
						std::filesystem::remove(oldPath, ec);
					}
				}

				profile.AvatarPath(copiedFile.Path());
				m_profile_photo = nullptr; // Kosongkan state
			}

			// ==========================================
			// 2. LOGIKA UPDATE MEMORY (Password, Nama, Username)
			// ==========================================
			hstring old_pwd = OldPasswordInput().Password();
			hstring new_pwd = NewPasswordInput().Password();
			hstring confirm_pwd = ConfirmPasswordInput().Password();

			bool isAttemptingPasswordChange = !old_pwd.empty() || !new_pwd.empty() || !confirm_pwd.empty();

			if (isAttemptingPasswordChange) {
				if (new_pwd != confirm_pwd) {
					winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog{};
					dialog.Title(box_value(L"Peringatan"));
					dialog.Content(box_value(L"Konfirmasi kata sandi tidak cocok."));
					dialog.CloseButtonText(L"Ok");
					dialog.XamlRoot(this->XamlRoot());
					co_await dialog.ShowAsync();
					co_return; // Batalkan proses simpan
				}

				if (old_pwd.empty() || new_pwd.empty()) {
					winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog{};
					dialog.Title(box_value(L"Peringatan"));
					dialog.Content(box_value(L"Kata sandi lama dan baru tidak boleh kosong."));
					dialog.CloseButtonText(L"Ok");
					dialog.XamlRoot(this->XamlRoot());
					co_await dialog.ShowAsync();
					co_return; // Batalkan proses simpan
				}

				winrt::apartment_context ui_thread;
				co_await winrt::resume_background();
				auto auth_result = co_await database.ValidateUserCredentials(originalUsername, old_pwd);
				co_await ui_thread;

				if (auth_result.IsError()) {
					winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog{};
					dialog.Title(box_value(L"Autentikasi Gagal"));
					dialog.Content(box_value(auth_result.Message()));
					dialog.CloseButtonText(L"Ok");
					dialog.XamlRoot(this->XamlRoot());
					co_await dialog.ShowAsync();
					co_return; // Batalkan proses simpan
				}

				// Validasi sukses, update state model memori
				user.Password(new_pwd);
				isPasswordUpdated = true;
			}

			profile.Fullname(FullNameInput().Text());
			user.Username(UsernameInput().Text());

			// ==========================================
			// 3. LOGIKA SIMPAN KE DATABASE (USER DAHULU)
			// ==========================================
			winrt::apartment_context final_ui_thread;
			co_await winrt::resume_background();
			auto user_result = co_await database.UpdateUser(user);
			co_await final_ui_thread;

			// Cek hasil Update User
			if (user_result.IsError()) {
				// ROLLBACK MEMORY STATE
				user.Username(originalUsername);
				user.Password(originalPassword);

				winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog{};
				dialog.Title(box_value(L"Gagal Memperbarui Akun"));
				dialog.Content(box_value(user_result.Message())); // Menampilkan error seperti "Username sudah diambil"
				dialog.CloseButtonText(L"Ok");
				dialog.XamlRoot(this->XamlRoot());
				co_await dialog.ShowAsync();
				co_return; // BERHENTI, jangan lanjutkan ke Update Profile
			}

			// ==========================================
			// 4. LOGIKA SIMPAN KE DATABASE (PROFILE KEMUDIAN)
			// ==========================================
			co_await winrt::resume_background();
			auto profile_result = co_await database.UpdateUserProfile(profile);
			co_await final_ui_thread;

			// Semuanya sukses
			winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog{};
			dialog.CloseButtonText(L"Ok");
			dialog.XamlRoot(this->XamlRoot());

			if (!profile_result.IsError()) {
				dialog.Title(box_value(L"Info"));
				dialog.Content(box_value(std::format(L"{} & {}", profile_result.Message(), user_result.Message())));

				// Jika password berhasil diubah dan disimpan ke DB, update Vault
				if (isPasswordUpdated || originalUsername != user.Username()) {
					// Gunakan Username terbaru dan Password terbaru (jika tidak diubah, pass kosong akan ditolak vault, tapi vault butuh pass asli, namun karena vault kita menggunakan VaultResource, cukup update jika password berubah)
					// Jika hanya username yang berubah, Anda harus mensyaratkan user input password lama atau mengelola credential ulang.
					// Untuk amannya, panggil SaveCredentials dengan data terbaru.
					winrt::LeafEye::LoginPage::SaveCredentials(user.Username(), user.Password().empty() ? originalPassword : user.Password());
				}

				if (isPasswordUpdated) {
					// Kosongkan form input
					OldPasswordInput().Password(L"");
					NewPasswordInput().Password(L"");
					ConfirmPasswordInput().Password(L"");
				}
			}
			else {
				// Jika profile gagal, user tetap terupdate di database, namun profile memori dan db berbeda.
				// Sesuai instruksi, tidak ada rollback untuk profile.
				dialog.Title(box_value(L"Peringatan Sebagian"));
				dialog.Content(box_value(std::format(L"User terupdate, tetapi profil gagal: {}", profile_result.Message())));
			}

			co_await dialog.ShowAsync();

		}
		catch (winrt::hresult_error const& ex) {
			// Tangkap pesan error ke variabel string lokal
			fatalErrorMessage = ex.message();
		}

		// ==========================================
		// 5. PENANGANAN ERROR FATAL (Di Luar Catch)
		// ==========================================
		if (!fatalErrorMessage.empty()) {
			winrt::Microsoft::UI::Xaml::Controls::ContentDialog errorDialog{};
			errorDialog.Title(box_value(L"Fatal System Error"));
			errorDialog.Content(box_value(fatalErrorMessage));
			errorDialog.CloseButtonText(L"Ok");
			errorDialog.XamlRoot(this->XamlRoot());
			co_await errorDialog.ShowAsync();
		}
	}
	void winrt::LeafEye::implementation::ProfilePage::FullNameInput_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
	{
		EvaluateSaveState();
	}

	void winrt::LeafEye::implementation::ProfilePage::UsernameInput_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e)
	{
		EvaluateSaveState();
	}

	void winrt::LeafEye::implementation::ProfilePage::OldPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
	{
		EvaluateSaveState();
	}

	void winrt::LeafEye::implementation::ProfilePage::NewPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
	{
		EvaluateSaveState();
	}

	void winrt::LeafEye::implementation::ProfilePage::ConfirmPasswordInput_PasswordChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
	{
		EvaluateSaveState();
	}
}

