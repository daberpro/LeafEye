# White-Box Testing: Basis Path Analysis untuk LeafEye Application

**Tanggal:** 2026-05-19  
**Aplikasi:** LeafEye (WinUI3 C++/WinRT + LibTorch)  
**Metodologi:** White-Box Testing dengan Basis Path Method  
**Tester:** Basis Path Coverage Analysis

---

## 📑 Daftar Isi

1. [Ringkasan Eksekutif](#ringkasan-eksekutif)
2. [1. User Management Operations](#1-user-management-operations)
3. [2. Profile Management Operations](#2-profile-management-operations)
4. [3. History Tracking Operations](#3-history-tracking-operations)
5. [4. File History Operations](#4-file-history-operations)
6. [Total Basis Path Summary](#total-basis-path-summary)

---

## Ringkasan Eksekutif

### Definisi Basis Path Testing
Basis Path Testing adalah teknik white-box testing yang mengidentifikasi jalur logis independen dalam program dengan menghitung **Cyclomatic Complexity (CC)**. Setiap basis path merepresentasikan kombinasi unik dari kondisi yang harus diuji.

### Formula Cyclomatic Complexity
```
CC = E - N + 2P
```
Dimana:
- **E** = Jumlah edge (koneksi antar node)
- **N** = Jumlah node (titik keputusan)
- **P** = Jumlah komponen terhubung (1 untuk single program)

**Atau alternatif:**
```
CC = (Jumlah Decision Points) + 1
```

### Notasi Alur Path
- **[D]** = Decision Point (Kondisi IF/ELSE, SWITCH, LOOP)
- **[A]** = Action/Proses
- **→** = Flow direction
- **║** = Parallel decision paths

---

## 1. User Management Operations

### 📊 Cyclomatic Complexity Calculation

#### Fungsi: `AddUser(UserModel user)`
```
IF user.Username.IsEmpty()
    RETURN Error("Username cannot be empty")
IF user.Password.IsEmpty()
    RETURN Error("Password cannot be empty")
Persist to Database
RETURN Success()
```

**Kalkulasi:**
- Decision Points: 2
- CC = 2 + 1 = **3**

#### Fungsi: `ValidateUserCredentials(String username, String password)`
```
IF username.IsEmpty() OR password.IsEmpty()
    RETURN Error
Lookup user in database
    IF user not found
        RETURN ValueNotExists
    ELSE
        IF password matches
            RETURN User
        ELSE
            RETURN ValueNotExists
RETURN Success or Error
```

**Kalkulasi:**
- Decision Points: 4 (2 IF + 1 nested IF + 1 nested IF)
- CC = 4 + 1 = **5**

#### Fungsi: `GetAllUsers(Int64 offset, Int64 limit)`
```
IF offset < 0 OR limit <= 0
    RETURN Error("Invalid pagination parameters")
Query database with offset and limit
IF query returns no results
    RETURN ValueNotExists
IF results.Size() > 0
    RETURN Results
RETURN Error()
```

**Kalkulasi:**
- Decision Points: 3
- CC = 3 + 1 = **4**

#### Fungsi: `UpdateUser(UserModel user)`
```
IF user.Id == 0
    RETURN Error("Invalid user ID")
IF user.Username.IsEmpty()
    RETURN Error("Username cannot be empty")
Fetch existing user
IF user not found
    RETURN Error("User not found")
Update password or admin status
IF update succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `DeleteUser(UInt64 id)`
```
IF id == 0
    RETURN Error("Invalid ID")
Verify user exists
IF user not found
    RETURN ValueNotExists
Remove from database
IF delete succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

### 🔄 Basis Path List untuk User Management

| ID | Path | Flow | Input/Condition | Expected Output | CC |
|---|---|---|---|---|---|
| **BP1.1** | Add User (Success) | [D1: Empty?] → NO → [A: Persist] → [D2: Success?] → YES | Valid username & password | User created, Return Success | 1 |
| **BP1.2** | Add User (Empty Username) | [D1: Empty?] → YES → [A: Error] | Username = "", Password = "pass" | Error message returned | 1 |
| **BP1.3** | Add User (Empty Password) | [D1: Empty?] → NO → [D2: Empty?] → YES | Username = "admin", Password = "" | Error message returned | 1 |
| **BP1.4** | Validate Credentials (Valid) | [D1: Empty?] → NO → [D2: Found?] → YES → [D3: Match?] → YES | Valid username & password | Return UserModel | 1 |
| **BP1.5** | Validate Credentials (Wrong Password) | [D1: Empty?] → NO → [D2: Found?] → YES → [D3: Match?] → NO | Username exist, wrong password | ValueNotExists | 1 |
| **BP1.6** | Validate Credentials (User Not Found) | [D1: Empty?] → NO → [D2: Found?] → NO | Non-existent username | ValueNotExists | 1 |
| **BP1.7** | Validate Credentials (Empty Input) | [D1: Empty?] → YES | Empty username or password | Error returned | 1 |
| **BP1.8** | Get All Users (With Results) | [D1: Invalid Params?] → NO → [D2: Empty?] → NO → [D3: Size > 0?] → YES | Valid offset/limit, users exist | IVector with users | 1 |
| **BP1.9** | Get All Users (No Results) | [D1: Invalid Params?] → NO → [D2: Empty?] → YES | Valid params, no users | ValueNotExists | 1 |
| **BP1.10** | Get All Users (Invalid Params) | [D1: Invalid Params?] → YES | Offset < 0 or limit ≤ 0 | Error message | 1 |
| **BP1.11** | Update User (Success) | [D1: Valid ID?] → YES → [D2: Valid Username?] → YES → [D3: Found?] → YES → [D4: Update?] → YES | Valid ID, exists in DB | UserModel updated | 1 |
| **BP1.12** | Update User (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP1.13** | Update User (Empty Username) | [D1: Valid ID?] → YES → [D2: Valid Username?] → NO | ID valid, username empty | Error message | 1 |
| **BP1.14** | Update User (Not Found) | [D1: Valid ID?] → YES → [D2: Valid Username?] → YES → [D3: Found?] → NO | ID valid, user doesn't exist | Error "Not found" | 1 |
| **BP1.15** | Delete User (Success) | [D1: Valid ID?] → YES → [D2: Exists?] → YES → [D3: Delete Success?] → YES | Valid ID, user exists | Success, user deleted | 1 |
| **BP1.16** | Delete User (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP1.17** | Delete User (Not Found) | [D1: Valid ID?] → YES → [D2: Exists?] → NO | Valid ID, user doesn't exist | ValueNotExists | 1 |

### 📈 Summary - User Management

```
Total CC = 3 + 5 + 4 + 6 + 5 = 23
Total Basis Paths = 17
Average CC per function = 23 ÷ 5 = 4.6
```

---

## 2. Profile Management Operations

### 📊 Cyclomatic Complexity Calculation

#### Fungsi: `AddUserProfile(ProfileModel profile)`
```
IF profile == null
    RETURN Error("Profile cannot be null")
IF profile.UserId == 0
    RETURN Error("Invalid UserId")
Persist profile
IF persist succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 3
- CC = 3 + 1 = **4**

#### Fungsi: `GetUserProfileByLink(UInt64 userId)`
```
IF userId == 0
    RETURN Error("Invalid UserId")
Query profile by userId
IF result is empty
    RETURN ValueNotExists
IF result found
    RETURN ProfileModel
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `UpdateUserProfile(ProfileModel profile)`
```
IF profile == null
    RETURN Error
IF profile.Id == 0
    RETURN Error("Invalid profile ID")
Verify profile exists
IF not exists
    RETURN ValueNotExists
Update profile
IF update succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `DeleteUserProfile(UInt64 id)`
```
IF id == 0
    RETURN Error("Invalid ID")
Verify profile exists
IF not exists
    RETURN ValueNotExists
Delete from database
IF delete succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `LinkUserProfile(UInt64 userId, UInt64 profileId)`
```
IF userId == 0 OR profileId == 0
    RETURN Error("Invalid IDs")
Verify user exists
IF user not found
    RETURN Error("User not found")
Verify profile exists
IF profile not found
    RETURN Error("Profile not found")
Create relationship
IF relationship created
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 6
- CC = 6 + 1 = **7**

### 🔄 Basis Path List untuk Profile Management

| ID | Path | Flow | Input/Condition | Expected Output | CC |
|---|---|---|---|---|---|
| **BP2.1** | Add Profile (Success) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Persist?] → YES | Valid profile with userId | ProfileModel persisted | 1 |
| **BP2.2** | Add Profile (Null Check) | [D1: Null?] → YES | profile = null | Error "Profile cannot be null" | 1 |
| **BP2.3** | Add Profile (Invalid UserId) | [D1: Null?] → NO → [D2: Valid ID?] → NO | UserId = 0 | Error "Invalid UserId" | 1 |
| **BP2.4** | Get Profile (Found) | [D1: Valid ID?] → YES → [D2: Empty?] → NO → [D3: Found?] → YES | Valid userId, profile exists | Return ProfileModel | 1 |
| **BP2.5** | Get Profile (Not Found) | [D1: Valid ID?] → YES → [D2: Empty?] → YES | Valid userId, no profile | ValueNotExists | 1 |
| **BP2.6** | Get Profile (Invalid UserId) | [D1: Valid ID?] → NO | UserId = 0 | Error "Invalid UserId" | 1 |
| **BP2.7** | Update Profile (Success) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Exists?] → YES → [D4: Update?] → YES | Valid profile ID, exists | Profile updated | 1 |
| **BP2.8** | Update Profile (Null Check) | [D1: Null?] → YES | profile = null | Error | 1 |
| **BP2.9** | Update Profile (Invalid ID) | [D1: Null?] → NO → [D2: Valid ID?] → NO | profile.Id = 0 | Error "Invalid ID" | 1 |
| **BP2.10** | Update Profile (Not Exists) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Exists?] → NO | Valid ID, profile doesn't exist | ValueNotExists | 1 |
| **BP2.11** | Delete Profile (Success) | [D1: Valid ID?] → YES → [D2: Exists?] → YES → [D3: Delete?] → YES | Valid ID, profile exists | Profile deleted | 1 |
| **BP2.12** | Delete Profile (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP2.13** | Delete Profile (Not Exists) | [D1: Valid ID?] → YES → [D2: Exists?] → NO | Valid ID, profile doesn't exist | ValueNotExists | 1 |
| **BP2.14** | Link User-Profile (Success) | [D1: Valid IDs?] → YES → [D2: User Exists?] → YES → [D3: Profile Exists?] → YES → [D4: Link?] → YES | Valid IDs, both exist | Link created | 1 |
| **BP2.15** | Link User-Profile (Invalid IDs) | [D1: Valid IDs?] → NO | userId=0 or profileId=0 | Error "Invalid IDs" | 1 |
| **BP2.16** | Link User-Profile (User Not Found) | [D1: Valid IDs?] → YES → [D2: User Exists?] → NO | Valid profileId, user missing | Error "User not found" | 1 |
| **BP2.17** | Link User-Profile (Profile Not Found) | [D1: Valid IDs?] → YES → [D2: User Exists?] → YES → [D3: Profile Exists?] → NO | Valid userId, profile missing | Error "Profile not found" | 1 |

### 📈 Summary - Profile Management

```
Total CC = 4 + 5 + 6 + 5 + 7 = 27
Total Basis Paths = 17
Average CC per function = 27 ÷ 5 = 5.4
```

---

## 3. History Tracking Operations

### 📊 Cyclomatic Complexity Calculation

#### Fungsi: `AddHistory(HistoryModel history)`
```
IF history == null
    RETURN Error
IF history.UserId == 0
    RETURN Error("Invalid UserId")
IF history.Status < 0
    RETURN Error("Invalid Status")
Persist history
IF persist succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `GetHistoryByStatus(Int64 status, Int64 offset, Int64 limit)`
```
IF status < 0
    RETURN Error("Invalid Status")
IF offset < 0 OR limit <= 0
    RETURN Error("Invalid pagination")
Query by status with offset/limit
IF query empty
    RETURN ValueNotExists
IF results found
    RETURN Results
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `GetHistoryByDateRange(UInt64 start, UInt64 end, Int64 offset, Int64 limit)`
```
IF start > end
    RETURN Error("Invalid date range")
IF offset < 0 OR limit <= 0
    RETURN Error("Invalid pagination")
IF end - start > MAX_RANGE
    RETURN Error("Range too large")
Query database with date range
IF query empty
    RETURN ValueNotExists
IF results found
    RETURN Results
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 6
- CC = 6 + 1 = **7**

#### Fungsi: `UpdateHistory(HistoryModel history)`
```
IF history == null
    RETURN Error
IF history.Id == 0
    RETURN Error("Invalid ID")
Verify history exists
IF not exists
    RETURN ValueNotExists
Update history fields
IF update succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `DeleteHistory(UInt64 id)`
```
IF id == 0
    RETURN Error("Invalid ID")
Verify history exists
IF not exists
    RETURN ValueNotExists
Delete from database
IF delete succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `LinkHistoryToUser(UInt64 historyId, UInt64 userId)`
```
IF historyId == 0 OR userId == 0
    RETURN Error("Invalid IDs")
Verify history exists
IF not exists
    RETURN Error("History not found")
Verify user exists
IF not exists
    RETURN Error("User not found")
Create relationship
IF relationship created
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 6
- CC = 6 + 1 = **7**

### 🔄 Basis Path List untuk History Tracking

| ID | Path | Flow | Input/Condition | Expected Output | CC |
|---|---|---|---|---|---|
| **BP3.1** | Add History (Success) | [D1: Null?] → NO → [D2: Valid User?] → YES → [D3: Valid Status?] → YES → [D4: Persist?] → YES | Valid history | HistoryModel persisted | 1 |
| **BP3.2** | Add History (Null Check) | [D1: Null?] → YES | history = null | Error | 1 |
| **BP3.3** | Add History (Invalid UserId) | [D1: Null?] → NO → [D2: Valid User?] → NO | UserId = 0 | Error "Invalid UserId" | 1 |
| **BP3.4** | Add History (Invalid Status) | [D1: Null?] → NO → [D2: Valid User?] → YES → [D3: Valid Status?] → NO | Status < 0 | Error "Invalid Status" | 1 |
| **BP3.5** | Get History by Status (Found) | [D1: Valid Status?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → NO | Valid status & pagination, results exist | IVector of histories | 1 |
| **BP3.6** | Get History by Status (Not Found) | [D1: Valid Status?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → YES | Valid params, no history | ValueNotExists | 1 |
| **BP3.7** | Get History by Status (Invalid Status) | [D1: Valid Status?] → NO | status < 0 | Error "Invalid Status" | 1 |
| **BP3.8** | Get History by Status (Invalid Pagination) | [D1: Valid Status?] → YES → [D2: Valid Pagination?] → NO | offset < 0 or limit ≤ 0 | Error "Invalid pagination" | 1 |
| **BP3.9** | Get History by DateRange (Found) | [D1: Valid Range?] → YES → [D2: Valid Pagination?] → YES → [D3: Not Exceeded?] → YES → [D4: Empty?] → NO | start ≤ end, results exist | IVector of histories | 1 |
| **BP3.10** | Get History by DateRange (Not Found) | [D1: Valid Range?] → YES → [D2: Valid Pagination?] → YES → [D3: Not Exceeded?] → YES → [D4: Empty?] → YES | Valid range, no history | ValueNotExists | 1 |
| **BP3.11** | Get History by DateRange (Invalid Range) | [D1: Valid Range?] → NO | start > end | Error "Invalid range" | 1 |
| **BP3.12** | Get History by DateRange (Invalid Pagination) | [D1: Valid Range?] → YES → [D2: Valid Pagination?] → NO | offset < 0 or limit ≤ 0 | Error "Invalid pagination" | 1 |
| **BP3.13** | Get History by DateRange (Range Exceeded) | [D1: Valid Range?] → YES → [D2: Valid Pagination?] → YES → [D3: Not Exceeded?] → NO | end - start > MAX_RANGE | Error "Range too large" | 1 |
| **BP3.14** | Update History (Success) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Exists?] → YES → [D4: Update?] → YES | Valid ID, history exists | History updated | 1 |
| **BP3.15** | Update History (Null Check) | [D1: Null?] → YES | history = null | Error | 1 |
| **BP3.16** | Update History (Invalid ID) | [D1: Null?] → NO → [D2: Valid ID?] → NO | history.Id = 0 | Error "Invalid ID" | 1 |
| **BP3.17** | Update History (Not Exists) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Exists?] → NO | Valid ID, history doesn't exist | ValueNotExists | 1 |
| **BP3.18** | Delete History (Success) | [D1: Valid ID?] → YES → [D2: Exists?] → YES → [D3: Delete?] → YES | Valid ID, history exists | History deleted | 1 |
| **BP3.19** | Delete History (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP3.20** | Delete History (Not Exists) | [D1: Valid ID?] → YES → [D2: Exists?] → NO | Valid ID, history doesn't exist | ValueNotExists | 1 |
| **BP3.21** | Link History to User (Success) | [D1: Valid IDs?] → YES → [D2: History Exists?] → YES → [D3: User Exists?] → YES → [D4: Link?] → YES | Valid IDs, both exist | Link created | 1 |
| **BP3.22** | Link History to User (Invalid IDs) | [D1: Valid IDs?] → NO | historyId=0 or userId=0 | Error "Invalid IDs" | 1 |
| **BP3.23** | Link History to User (History Not Found) | [D1: Valid IDs?] → YES → [D2: History Exists?] → NO | Valid userId, history missing | Error "History not found" | 1 |
| **BP3.24** | Link History to User (User Not Found) | [D1: Valid IDs?] → YES → [D2: History Exists?] → YES → [D3: User Exists?] → NO | Valid historyId, user missing | Error "User not found" | 1 |

### 📈 Summary - History Tracking

```
Total CC = 5 + 6 + 7 + 6 + 5 + 7 = 36
Total Basis Paths = 24
Average CC per function = 36 ÷ 6 = 6.0
```

---

## 4. File History Operations

### 📊 Cyclomatic Complexity Calculation

#### Fungsi: `AddFileHistory(FileHistoryModel fileHistory)`
```
IF fileHistory == null
    RETURN Error
IF fileHistory.FileName.IsEmpty()
    RETURN Error("FileName cannot be empty")
IF fileHistory.FileSize <= 0
    RETURN Error("Invalid file size")
IF fileHistory.Confidence < 0 OR Confidence > 1
    RETURN Error("Invalid confidence")
Persist file history
IF persist succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `GetFileHistoriesByConfidenceThreshold(Double minConfidence, Int64 page, Int64 pageSize)`
```
IF minConfidence < 0 OR minConfidence > 1
    RETURN Error("Invalid threshold")
IF page <= 0 OR pageSize <= 0
    RETURN Error("Invalid pagination")
Query database by confidence
IF query empty
    RETURN ValueNotExists
IF results found
    RETURN Results
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 5
- CC = 5 + 1 = **6**

#### Fungsi: `GetFileHistoryById(UInt64 id)`
```
IF id == 0
    RETURN Error("Invalid ID")
Query database by id
IF query empty
    RETURN ValueNotExists
IF result found
    RETURN FileHistoryModel
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `GetFileHistoriesByHistoryLink(UInt64 historyId, Int64 page, Int64 pageSize)`
```
IF historyId == 0
    RETURN Error("Invalid historyId")
IF page <= 0 OR pageSize <= 0
    RETURN Error("Invalid pagination")
Query database by historyId
IF query empty
    RETURN ValueNotExists
IF results found
    RETURN Results
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `UpdateFileHistory(FileHistoryModel fileHistory)`
```
IF fileHistory == null
    RETURN Error
IF fileHistory.Id == 0
    RETURN Error("Invalid ID")
IF fileHistory.FileName.IsEmpty()
    RETURN Error("FileName cannot be empty")
Verify file history exists
IF not exists
    RETURN ValueNotExists
Update file history
IF update succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 6
- CC = 6 + 1 = **7**

#### Fungsi: `DeleteFileHistory(UInt64 id)`
```
IF id == 0
    RETURN Error("Invalid ID")
Verify file history exists
IF not exists
    RETURN ValueNotExists
Delete from database
IF delete succeeds
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 4
- CC = 4 + 1 = **5**

#### Fungsi: `LinkFileHistoryToHistory(UInt64 fileHistoryId, UInt64 historyId)`
```
IF fileHistoryId == 0 OR historyId == 0
    RETURN Error("Invalid IDs")
Verify file history exists
IF not exists
    RETURN Error("File history not found")
Verify history exists
IF not exists
    RETURN Error("History not found")
Create relationship
IF relationship created
    RETURN Success
ELSE
    RETURN Error
```

**Kalkulasi:**
- Decision Points: 6
- CC = 6 + 1 = **7**

### 🔄 Basis Path List untuk File History Operations

| ID | Path | Flow | Input/Condition | Expected Output | CC |
|---|---|---|---|---|---|
| **BP4.1** | Add FileHistory (Success) | [D1: Null?] → NO → [D2: Valid FileName?] → YES → [D3: Valid Size?] → YES → [D4: Valid Confidence?] → YES → [D5: Persist?] → YES | Valid file history | FileHistoryModel persisted | 1 |
| **BP4.2** | Add FileHistory (Null Check) | [D1: Null?] → YES | fileHistory = null | Error | 1 |
| **BP4.3** | Add FileHistory (Empty FileName) | [D1: Null?] → NO → [D2: Valid FileName?] → NO | FileName = "" | Error "FileName empty" | 1 |
| **BP4.4** | Add FileHistory (Invalid Size) | [D1: Null?] → NO → [D2: Valid FileName?] → YES → [D3: Valid Size?] → NO | FileSize ≤ 0 | Error "Invalid size" | 1 |
| **BP4.5** | Add FileHistory (Invalid Confidence) | [D1: Null?] → NO → [D2: Valid FileName?] → YES → [D3: Valid Size?] → YES → [D4: Valid Confidence?] → NO | Confidence < 0 or > 1 | Error "Invalid confidence" | 1 |
| **BP4.6** | Get by Confidence (Found) | [D1: Valid Threshold?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → NO | Valid threshold & pagination, results exist | IVector of file histories | 1 |
| **BP4.7** | Get by Confidence (Not Found) | [D1: Valid Threshold?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → YES | Valid params, no results | ValueNotExists | 1 |
| **BP4.8** | Get by Confidence (Invalid Threshold) | [D1: Valid Threshold?] → NO | minConfidence < 0 or > 1 | Error "Invalid threshold" | 1 |
| **BP4.9** | Get by Confidence (Invalid Pagination) | [D1: Valid Threshold?] → YES → [D2: Valid Pagination?] → NO | page ≤ 0 or pageSize ≤ 0 | Error "Invalid pagination" | 1 |
| **BP4.10** | Get by Id (Found) | [D1: Valid ID?] → YES → [D2: Empty?] → NO | Valid ID, file exists | Return FileHistoryModel | 1 |
| **BP4.11** | Get by Id (Not Found) | [D1: Valid ID?] → YES → [D2: Empty?] → YES | Valid ID, no file | ValueNotExists | 1 |
| **BP4.12** | Get by Id (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP4.13** | Get by HistoryLink (Found) | [D1: Valid HistoryId?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → NO | Valid history ID, files exist | IVector of file histories | 1 |
| **BP4.14** | Get by HistoryLink (Not Found) | [D1: Valid HistoryId?] → YES → [D2: Valid Pagination?] → YES → [D3: Empty?] → YES | Valid historyId, no files | ValueNotExists | 1 |
| **BP4.15** | Get by HistoryLink (Invalid HistoryId) | [D1: Valid HistoryId?] → NO | historyId = 0 | Error "Invalid historyId" | 1 |
| **BP4.16** | Get by HistoryLink (Invalid Pagination) | [D1: Valid HistoryId?] → YES → [D2: Valid Pagination?] → NO | page ≤ 0 or pageSize ≤ 0 | Error "Invalid pagination" | 1 |
| **BP4.17** | Update FileHistory (Success) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Valid FileName?] → YES → [D4: Exists?] → YES → [D5: Update?] → YES | Valid ID & FileName, exists | File history updated | 1 |
| **BP4.18** | Update FileHistory (Null Check) | [D1: Null?] → YES | fileHistory = null | Error | 1 |
| **BP4.19** | Update FileHistory (Invalid ID) | [D1: Null?] → NO → [D2: Valid ID?] → NO | fileHistory.Id = 0 | Error "Invalid ID" | 1 |
| **BP4.20** | Update FileHistory (Empty FileName) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Valid FileName?] → NO | FileName = "" | Error "FileName empty" | 1 |
| **BP4.21** | Update FileHistory (Not Exists) | [D1: Null?] → NO → [D2: Valid ID?] → YES → [D3: Valid FileName?] → YES → [D4: Exists?] → NO | Valid ID, doesn't exist | ValueNotExists | 1 |
| **BP4.22** | Delete FileHistory (Success) | [D1: Valid ID?] → YES → [D2: Exists?] → YES → [D3: Delete?] → YES | Valid ID, file exists | File deleted | 1 |
| **BP4.23** | Delete FileHistory (Invalid ID) | [D1: Valid ID?] → NO | ID = 0 | Error "Invalid ID" | 1 |
| **BP4.24** | Delete FileHistory (Not Exists) | [D1: Valid ID?] → YES → [D2: Exists?] → NO | Valid ID, doesn't exist | ValueNotExists | 1 |
| **BP4.25** | Link FileHistory to History (Success) | [D1: Valid IDs?] → YES → [D2: FileHistory Exists?] → YES → [D3: History Exists?] → YES → [D4: Link?] → YES | Valid IDs, both exist | Link created | 1 |
| **BP4.26** | Link FileHistory to History (Invalid IDs) | [D1: Valid IDs?] → NO | fileHistoryId=0 or historyId=0 | Error "Invalid IDs" | 1 |
| **BP4.27** | Link FileHistory to History (FileHistory Not Found) | [D1: Valid IDs?] → YES → [D2: FileHistory Exists?] → NO | Valid historyId, file missing | Error "File not found" | 1 |
| **BP4.28** | Link FileHistory to History (History Not Found) | [D1: Valid IDs?] → YES → [D2: FileHistory Exists?] → YES → [D3: History Exists?] → NO | Valid fileHistoryId, history missing | Error "History not found" | 1 |

### 📈 Summary - File History Operations

```
Total CC = 6 + 6 + 5 + 5 + 7 + 5 + 7 = 41
Total Basis Paths = 28
Average CC per function = 41 ÷ 7 = 5.86
```

---

## Total Basis Path Summary

### 🎯 Comprehensive Coverage Matrix

| Module | Functions | CC | Basis Paths | Avg CC | Complexity Level |
|---|---|---|---|---|---|
| **User Management** | 5 | 23 | 17 | 4.6 | Medium |
| **Profile Management** | 5 | 27 | 17 | 5.4 | Medium-High |
| **History Tracking** | 6 | 36 | 24 | 6.0 | High |
| **File History** | 7 | 41 | 28 | 5.86 | High |
| **TOTAL** | **23** | **127** | **86** | **5.52** | **Medium-High** |

### 📊 Kompleksitas Distribusi

```
Cyclomatic Complexity Distribution:
┌─────────────────────────────────────────────┐
│ User Management:    23 (18.1%)              │
│ Profile Management: 27 (21.3%)              │
│ History Tracking:   36 (28.3%)              │
│ File History:       41 (32.3%)              │
└─────────────────────────────────────────────┘

Total: 127 CC / 86 Basis Paths
Average CC per Path: 1.48
Overall Complexity: MEDIUM-HIGH
Recommended Test Coverage: 80-100%
```

### 🧪 Test Execution Strategy

#### Phase 1: Unit Testing (Basis Path Coverage)
```
Priority 1 (Critical Paths):
  - User validation & credential checking (BP1.4, BP1.5, BP1.6)
  - Database initialization (All Add operations)
  - Error handling paths (Invalid ID, Null checks)
  
Priority 2 (Important Paths):
  - Pagination & data retrieval (Get operations)
  - Update & delete operations
  - Relationship linking
  
Priority 3 (Edge Cases):
  - Boundary condition testing
  - Large dataset handling
  - Concurrent operation testing
```

#### Phase 2: Integration Testing (Component Interaction)
```
Test Scenarios:
1. LoginPage → Database Initialization → User Validation
2. MainWindow Navigation → Page Loading → Data Retrieval
3. CRUD Operations → Database State → Verification
4. Error Scenarios → Error Handling → User Notification
```

#### Phase 3: System Testing (WinUI3 Application)
```
UI Interaction Paths:
1. InitDb_Click → SeedDb_Click → LoginButton_Click
2. MainNav_SelectionChanged → Page Navigation → Data Loading
3. AddUserButton_Click → Dialog Input → Database Operation
4. EditUser_Click → Update Dialog → Verification
5. DeleteUser_Click → Confirmation → Deletion
```

---

## 📋 Implementasi Test Cases

### Test Case Template

```cpp
// Format: TEST_METHOD(Module_Functionality_ExpectedResult)
TEST_METHOD(User_ValidateCredentials_ValidUser_ShouldReturnUser)
{
    // Arrange
    winrt::LeafEyeCore::UserModel testUser(L"testuser", L"password", false);
    m_db.AddUser(testUser).get();
    
    // Act
    auto result = m_db.ValidateUserCredentials(L"testuser", L"password").get();
    
    // Assert
    Assert::IsFalse(result.IsError());
    Assert::IsTrue(result.IsValueExists());
    auto retrievedUser = unbox_value<winrt::LeafEyeCore::UserModel>(result.ResultValue());
    Assert::AreEqual(winrt::hstring(L"testuser"), retrievedUser.Username());
}
```

### Existing Test Coverage (dari UnitTests.cpp)
- ✅ 59 test methods sudah mengcover mayoritas basis path
- ✅ CRUD operations fully tested
- ✅ Pagination tested
- ✅ Error scenarios covered

---

## 🔗 Alur Path Visual

### User Management Flow Diagram
```
┌─────────────────────────────────────────────────────────┐
│                    USER MANAGEMENT FLOW                 │
└─────────────────────────────────────────────────────────┘

AddUser Path:
  START → [D1: Validate Input] 
    ├─ YES → [A: Persist] → [D2: Success?]
    │          ├─ YES → RETURN Success (BP1.1)
    │          └─ NO → RETURN Error (not in basis paths)
    └─ NO → RETURN Error (BP1.2, BP1.3)

ValidateCredentials Path:
  START → [D1: Input Valid?]
    ├─ NO → RETURN Error (BP1.7)
    └─ YES → [D2: User Found?]
              ├─ NO → RETURN ValueNotExists (BP1.6)
              └─ YES → [D3: Password Match?]
                        ├─ YES → RETURN User (BP1.4)
                        └─ NO → RETURN ValueNotExists (BP1.5)
```

### Profile Management Flow Diagram
```
┌─────────────────────────────────────────────────────────┐
│                  PROFILE MANAGEMENT FLOW                │
└─────────────────────────────────────────────────────────┘

LinkUserProfile Path:
  START → [D1: IDs Valid?]
    ├─ NO → RETURN Error (BP2.15)
    └─ YES → [D2: User Exists?]
              ├─ NO → RETURN Error (BP2.16)
              └─ YES → [D3: Profile Exists?]
                        ├─ NO → RETURN Error (BP2.17)
                        └─ YES → [D4: Create Link?]
                                  ├─ YES → RETURN Success (BP2.14)
                                  └─ NO → RETURN Error
```

### History Tracking Flow Diagram
```
┌─────────────────────────────────────────────────────────┐
│              HISTORY TRACKING FLOW                      │
└─────────────────────────────────────────────────────────┘

GetHistoryByDateRange Path:
  START → [D1: Range Valid?]
    ├─ NO → RETURN Error (BP3.11)
    └─ YES → [D2: Pagination Valid?]
              ├─ NO → RETURN Error (BP3.12)
              └─ YES → [D3: Range within limits?]
                        ├─ NO → RETURN Error (BP3.13)
                        └─ YES → [D4: Query Results?]
                                  ├─ YES → RETURN Results (BP3.9)
                                  └─ NO → RETURN ValueNotExists (BP3.10)
```

### File History Operations Flow Diagram
```
┌─────────────────────────────────────────────────────────┐
│            FILE HISTORY OPERATIONS FLOW                 │
└─────────────────────────────────────────────────────────┘

UpdateFileHistory Path:
  START → [D1: Null Check?]
    ├─ YES → RETURN Error (BP4.18)
    └─ NO → [D2: ID Valid?]
             ├─ NO → RETURN Error (BP4.19)
             └─ YES → [D3: FileName Valid?]
                       ├─ NO → RETURN Error (BP4.20)
                       └─ YES → [D4: Exists?]
                                 ├─ NO → RETURN ValueNotExists (BP4.21)
                                 └─ YES → [D5: Update?]
                                           ├─ YES → RETURN Success (BP4.17)
                                           └─ NO → RETURN Error
```

---

## 📝 WinUI3 UI Testing Paths

### LoginPage Flow
```
OnNavigatedTo Path:
  ┌─ [D1: LoadCredentials] → [D2: Auto-Login] → [D3: ValidateCredentials]
  │    ├─ Success → [A: SetUser & SetProfile] → [A: DismissOverlay]
  │    └─ Fail → [A: ClearCredentials]
  └─ [D4: Return]

Button_Click (Manual Login):
  ┌─ [D1: Input Empty?] → [D2: ValidateCredentials]
  │    ├─ Error → [A: ShowErrorBar] → [D3: Return]
  │    ├─ Success → [A: SaveCredentials] → [A: DismissOverlay]
  │    └─ Invalid → [A: ShowErrorMessage]
  └─ [D4: Return]
```

### MainWindow Flow
```
InitDb_Click:
  [A: GetDbPath] → [A: CreateDatabase] → [D: InitializeAsync]
    ├─ Success → [A: ShowSuccessMessage]
    └─ Error → [A: ShowErrorMessage]

LoginButton_Click:
  [D1: DbNull?] → [D2: ValidateCredentials]
    ├─ Success → [A: SaveUser] → [A: HideLogin] → [A: ShowMainNav]
    └─ Fail → [A: ShowErrorMessage]

MainNav_SelectionChanged:
  [D1: SettingsSelected?] → [D2: GetTag]
    ├─ Users → [A: Navigate to UsersPage]
    ├─ Profiles → [A: Navigate to ProfilesPage]
    ├─ History → [A: Navigate to HistoryPage]
    └─ FileHistory → [A: Navigate to FileHistoryPage]
```

### UsersPage Flow
```
OnNavigatedTo:
  [A: GetDatabase] → [A: SetItemsSource] → [A: LoadData]

LoadDataAsync:
  [A: GetAllUsers] → [D1: Success?]
    ├─ YES → [A: PopulateList] → [A: SetPaginationButtons]
    └─ NO → [A: HandleError]

AddUserButton_Click:
  [A: ShowDialog] → [D1: ConfirmClicked?]
    ├─ YES → [A: CreateUser] → [A: GetUserByUsername] → [A: AddToList]
    └─ NO → [A: Cancel]

EditUser_Click:
  [A: FindUser] → [A: ShowDialog] → [D1: ConfirmClicked?]
    ├─ YES → [A: UpdateUser] → [A: UpdateListItem]
    └─ NO → [A: Cancel]

DeleteUser_Click:
  [A: ShowConfirmDialog] → [D1: ConfirmClicked?]
    ├─ YES → [A: DeleteUser] → [A: RemoveFromList]
    └─ NO → [A: Cancel]
```

---

## 🎯 Kesimpulan

### White-Box Testing Coverage untuk LeafEye

1. **Total Basis Paths**: 86 paths
2. **Total Cyclomatic Complexity**: 127
3. **Average Complexity per Path**: 1.48
4. **Overall Complexity Level**: Medium-High

### Rekomendasi Untuk Skripsi

1. **Fokus pada 4 komponen utama** sebagaimana terdapat di dokumentasi ini
2. **Implementasikan 80+ test cases** mengikuti struktur basis path
3. **Dokumentasikan alur path** dengan visual flow diagram
4. **Verifikasi coverage** menggunakan code coverage tools
5. **Uji integration** antara UI dan database layer

### Test Execution Priority
- **Phase 1**: Unit tests untuk semua CRUD operations (High Priority)
- **Phase 2**: Integration tests untuk komponen interaction
- **Phase 3**: UI tests untuk WinUI3 application flow
- **Phase 4**: System tests untuk end-to-end scenarios

---

**Dokumen ini dibuat untuk keperluan pendidikan dan riset akademis.**  
**Repository**: [daberpro/LeafEye](https://github.com/daberpro/LeafEye)  
**Diperbarui**: 2026-05-19
