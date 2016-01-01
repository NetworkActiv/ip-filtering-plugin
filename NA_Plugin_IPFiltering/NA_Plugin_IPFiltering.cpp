
#include "stdafx.h"
#include "NA_Plugin_IPFiltering.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object
CWinApp theApp;

using namespace std;

int main()
{
    int nRetCode = 0;
    HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr) {
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
		return nRetCode;
	}
    // initialize MFC and print and error on failure
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: MFC initialization failed\n");
        nRetCode = 1;
		return nRetCode;
	}
	// Return (close):
    return nRetCode;
}

void MessageBox(std::string Message, std::string Title)
{
	MessageBoxA(NULL, Message.c_str(), Title.c_str(), MB_OK);
}

void MessageBox(std::wstring Message, std::wstring Title)
{
	MessageBoxW(NULL, Message.c_str(), Title.c_str(), MB_OK);
}

void DoLogEntry(std::wstring InText) {
	DoLogEntry(InText.c_str());
}

void DoLogEntry(const WCHAR *InText) {
	DoLogEntry(UTF16ToUTF8(InText).c_str());
}

void DoLogEntry(const char *InText) {
	WCHAR ModuleFilePath[MAX_PATH + 1];
	memset(ModuleFilePath, 0, sizeof(ModuleFilePath));
	const WCHAR *InFilePath = GetDLLPathW();
	if (InFilePath == NULL)
		return;
	wcscpy_s(ModuleFilePath, MAX_PATH, InFilePath);
	long long loc = wcslen(ModuleFilePath) - 1;
	while (loc >= 0 && ModuleFilePath[loc] != '.')
		loc--;
	ModuleFilePath[loc < 0 ? 0 : loc] = NULL;
	WCHAR LogFilePath[MAX_PATH + 1 + 24];
	swprintf_s(LogFilePath, MAX_PATH + 24, L"%s_DebugLog.txt", ModuleFilePath);
	HANDLE OutFile = CreateFile(LogFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (OutFile == INVALID_HANDLE_VALUE)
		return;
	SetFilePointer(OutFile, 0, 0, FILE_END);
	DWORD BytesInOut = 0;
	WriteFile(OutFile, "[", 1, &BytesInOut, NULL);
	const char *CurTime = GetCurrentDateTimeA();
	WriteFile(OutFile, CurTime, strlen(CurTime), &BytesInOut, NULL);
	WriteFile(OutFile, "]: ", 3, &BytesInOut, NULL);
	WriteFile(OutFile, InText, strlen(InText), &BytesInOut, NULL);
	WriteFile(OutFile, "\r\n", 2, &BytesInOut, NULL);
	CloseHandle(OutFile);
}

const WCHAR *GetDLLPathW() {
	static WCHAR InDLLPath[MAX_PATH + 1];
	HMODULE FoundModule = NULL;
	if (0 == GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (WCHAR *)&GetDLLPathW, &FoundModule))
		return NULL;
	GetModuleFileName(FoundModule, InDLLPath, MAX_PATH);
	return InDLLPath;
}

const char *GetDLLPathA() {
	static char InDLLPath[MAX_PATH + 1];
	HMODULE FoundModule = NULL;
	if (0 == GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (WCHAR *)&GetDLLPathA, &FoundModule))
		return NULL;
	GetModuleFileNameA(FoundModule, InDLLPath, MAX_PATH);
	return InDLLPath;
}

const char *GetCurrentDateTimeA() {// Returns a string similar to ISO 8601;
	SYSTEMTIME Now;
	GetLocalTime(&Now);
	static char CurrentDateTimeA[24];
	sprintf_s(CurrentDateTimeA, 24 - 1, "%02d-%02d-%02d %02d:%02d:%02d", Now.wYear, Now.wMonth, Now.wDay, Now.wHour, Now.wMinute, Now.wSecond);
	return CurrentDateTimeA;
}

const WCHAR *GetCurrentDateTimeW() {// Returns a string similar to ISO 8601;
	SYSTEMTIME Now;
	GetLocalTime(&Now);
	static WCHAR CurrentDateTimeW[24];
	wnsprintfW(CurrentDateTimeW, 24 - 1, L"%02d-%02d-%02d %02d:%02d:%02d", Now.wYear, Now.wMonth, Now.wDay, Now.wHour, Now.wMinute, Now.wSecond);
	return CurrentDateTimeW;
}

SettingsBase::SettingsBase(){
	WCHAR ModuleFilePath[MAX_PATH + 1];
	memset(ModuleFilePath, 0, sizeof(ModuleFilePath));
	const WCHAR *InFilePath = GetDLLPathW();
	if (InFilePath == NULL)
		return;
	wcscpy_s(ModuleFilePath, MAX_PATH, InFilePath);
	long long loc = wcslen(ModuleFilePath) - 1;
	while (loc >= 0 && ModuleFilePath[loc] != '.')
		loc--;
	ModuleFilePath[loc < 0 ? 0 : loc] = NULL;
	SettingsPath = std::wstring(ModuleFilePath) + L"_Settings.JSON";
	Load();
}

using namespace rapidjson;

bool SettingsBase::Load() {
	// Read and parse the settings file:
	std::wstring SettingsText = ReadAllText(SettingsPath);
	if (SettingsText == L"") {
		DoLogEntry(L"Settings could not be loaded (file inaccessible or not found). Path=" + SettingsPath);
		return false;
	}
	SetDoc.Parse(UTF16ToUTF8(SettingsText.c_str()).c_str());
	// Fetch the settings from the Document:
	do {
		try {
			// Process the text:
			if (!SetDoc.HasMember("servers")) {
				DoLogEntry(L"No 'servers' member found.");
				break;
			}
			std::wstring Indent = L"";
			Value& Servers = SetDoc["servers"];
			Type BigType = Servers.GetType();
			if (BigType != Type::kArrayType) {
				DoLogEntry(L"'servers' is not an Array: " + GetTypeText(BigType) + std::wstring(L""));
				break;
			}
			HasLoaded = true;
			for (int x = 0; x < Servers.Size(); x++) {
				Indent = L"";
				Value& ThisMember = Servers[x];
				if (ThisMember.GetType() != Type::kObjectType) {
					DoLogEntry(Indent + L"A member of 'servers' was not an Object: " + GetTypeText(ThisMember.GetType()) + std::wstring(L" (fail)"));
					continue;
				}
				SettingsBase::Server *NewServer = new SettingsBase::Server();
				if (NewServer == NULL) {
					DoLogEntry(Indent + L"Unable to allocate a new server!");
					continue;
				}
				NewServer->URL = L"";
				if (!ThisMember.HasMember("url")) {
					DoLogEntry(Indent + L"No 'url' found (error).");
					continue;
				}
				else {
					if (ThisMember["url"].GetType() != Type::kStringType) {
						DoLogEntry(Indent + L"'url' is not a String: " + GetTypeText(ThisMember["url"].GetType()) + L" (error).");
						continue;
					}
					NewServer->URL = UTF8ToUTF16(ThisMember["url"].GetString());
					DoLogEntry(L"'url' found: " + NewServer->URL + std::wstring(L""));
				}
				NewServer->Username = L"";
				if (!ThisMember.HasMember("username")) {
					DoLogEntry(Indent + L"No 'username' found (warning).");
					//continue;
				}
				else {
					if (ThisMember["username"].GetType() != Type::kStringType) {
						DoLogEntry(Indent + L"'username' is not a String: " + GetTypeText(ThisMember["username"].GetType()) + L" (error).");
						continue;
					}
					NewServer->Username = UTF8ToUTF16(ThisMember["username"].GetString());
					//DoLogEntry(Indent + L"'username' found: " + NewServer.Username + std::wstring(L""));
				}
				NewServer->Password = L"";
				if (!ThisMember.HasMember("password")) {
					DoLogEntry(Indent + L"No 'password' found (warning).");
					//continue;
				}
				else {
					if (ThisMember["password"].GetType() != Type::kStringType) {
						DoLogEntry(Indent + L"'password' is not a String: " + GetTypeText(ThisMember["password"].GetType()) + L" (error).");
						continue;
					}
					NewServer->Password = UTF8ToUTF16(ThisMember["password"].GetString());
					//DoLogEntry(Indent + L"'password' found: " + NewServer.Password + std::wstring(L""));
				}
				// Server entry appears to be valid; add to list:
				this->Servers.push_back(NewServer);
				// Initialise the Web client for this server:
				{
					// Get the URL:
					NewServer->URL = CString(NewServer->URL.c_str()).Trim(L"\t ");
					// Parse the URL components:
					if (NewServer->URL.find(L"https://") == 0)
						NewServer->Protocol = L"https://";
					else if (NewServer->URL.find(L"http://") == 0)
						NewServer->Protocol = L"http://";
					NewServer->Domain = NewServer->URL;
					NewServer->Domain = ReplaceOne(NewServer->Domain, NewServer->Protocol, L"");
					NewServer->Domain = NewServer->Domain.find(L"/") == -1 ? NewServer->Domain : NewServer->Domain.substr(0, NewServer->Domain.find(L"/"));
					// Configure the client:
					http_client_config  ClientConfig;
					credentials         ClientCredentials;
					if (NewServer->Password != L"") {
						ClientCredentials = credentials(string_t(NewServer->Username), string_t(NewServer->Password));
						ClientConfig.set_credentials(ClientCredentials);
					}
					web::uri Webbie(NewServer->Protocol + NewServer->Domain);
					if (NewServer->WebClient != NULL) {
						delete NewServer->WebClient;
						NewServer->WebClient = NULL;
					}
					// Create the http_client with the correct Protocol, Domain, and Credentials:
					NewServer->WebClient = new http_client(Webbie, ClientConfig);
					if(NewServer->WebClient != NULL)
						DoLogEntry(L"Web client created for " + NewServer->URL);
					else
						DoLogEntry(L"Error creating Web client created for " + NewServer->URL);
				}
				// Collect filters for current server entry:
				if (!ThisMember.HasMember("filters")) {
					DoLogEntry(Indent + L"No 'filters' found (error).");
					continue;
				}
				Value& Filters = ThisMember["filters"];
				if (Filters.GetType() != Type::kArrayType) {
					DoLogEntry(Indent + L"'filters' is not an array: " + GetTypeText(Filters.GetType()) + std::wstring(L" (fail)"));
					continue;
				}
				for (int y = 0; y < Filters.Size(); y++) {
					Indent = L"  ";
					Value& ThisMember = Filters[y];
					if (ThisMember.GetType() != Type::kObjectType) {
						DoLogEntry(Indent + L"A member of 'filters' was not an Object: " + GetTypeText(ThisMember.GetType()) + std::wstring(L" (fail)"));
						continue;
					}
					SettingsBase::Server::Filter *NewFilter = new SettingsBase::Server::Filter();
					if (NewFilter == NULL) {
						DoLogEntry(L"Error allocating new filter!");
						continue;
					}
					NewFilter->Path = L"";
					if (!ThisMember.HasMember("path")) {
						DoLogEntry(Indent + L"A member of 'filters' has no 'path' field (fail)");
						continue;
					}
					else {
						if (ThisMember["path"].GetType() != Type::kStringType) {
							DoLogEntry(Indent + L"'path' is not a String: " + GetTypeText(ThisMember["path"].GetType()) + L" (error).");
							continue;
						}
						NewFilter->Path = UTF8ToUTF16(ThisMember["path"].GetString());
						DoLogEntry(Indent + L"'path' found: " + NewFilter->Path.c_str());
						// Filter entry appears to be valid; add to list:
						NewServer->Filters.push_back(NewFilter);
					}
					if (!ThisMember.HasMember("allow_only")) {
						DoLogEntry(Indent + L"'allow_only' is missing (warning)");
						//continue;
					}
					else {
						if (ThisMember["allow_only"].GetType() != Type::kArrayType) {
							DoLogEntry(Indent + L"'allow_only' is not an Array: " + GetTypeText(ThisMember["allow_only"].GetType()) + L" (error).");
							continue;
						}
						else {
							Value& AllowOnly = ThisMember["allow_only"];
							for (int o = 0; o < AllowOnly.Size(); o++) {
								Indent = L"    ";
								Value& ThisMember = AllowOnly[o];
								if (ThisMember.GetType() != Type::kObjectType) {
									DoLogEntry(Indent + L"A member of 'allow_only' was not an Object: " + GetTypeText(ThisMember.GetType()) + std::wstring(L" (fail)"));
									continue;
								}
								std::wstring Val = L"";
								if (!ThisMember.HasMember("val")) {
									DoLogEntry(Indent + L"A member of 'allow_only' has no 'val' field (fail)");
									continue;
								}
								else {
									// Type is unrestricted;
									Val = GetValueString(ThisMember["val"]);
									// Add this 'allow_only' value to the internal list:
									NewFilter->AllowOnly.push_back(std::wstring(GetValueString(ThisMember["val"])));
									DoLogEntry(Indent + std::wstring(L"allow_only val = ") + Val);
								}
							}
						}
					}
					if (!ThisMember.HasMember("allow_none")) {
						DoLogEntry(Indent + L"'allow_none' is missing (warning)");
						//continue;
					}
					else {
						if (ThisMember["allow_none"].GetType() != Type::kArrayType) {
							DoLogEntry(Indent + L"'allow_none' is not an Array: " + GetTypeText(ThisMember["allow_none"].GetType()) + L" (error).");
							continue;
						}
						else {
							Value& AllowOnly = ThisMember["allow_none"];
							for (int o = 0; o < AllowOnly.Size(); o++) {
								Indent = L"    ";
								Value& ThisMember = AllowOnly[o];
								if (ThisMember.GetType() != Type::kObjectType) {
									DoLogEntry(Indent + L"A member of 'allow_none' was not an Object: " + GetTypeText(ThisMember.GetType()) + std::wstring(L" (fail)"));
									continue;
								}
								std::wstring Val = L"";
								if (!ThisMember.HasMember("val")) {
									DoLogEntry(Indent + L"A member of 'allow_none' has no 'val' field (fail)");
									continue;
								}
								else {
									// Type is unrestricted;
									Val = GetValueString(ThisMember["val"]);
									// Add this 'allow_none' value to the internal list:
									NewFilter->AllowNone.push_back(std::wstring(GetValueString(ThisMember["val"])));
									DoLogEntry(Indent + std::wstring(L"allow_none val = ") + Val);
								}
							}
						}
					}
				}
			}
		}
		catch (std::exception &ex) {
			DoLogEntry(L"Exception: " + UTF8ToUTF16(ex.what()));
			return false;
		}
	} while (false);
	return true;
}

bool SettingsBase::Save() {
	// Not implemented;
	return false;
}

std::wstring GetTypeText(Type InType) {
	if (InType == Type::kArrayType)
		return L"kArrayType";
	if (InType == Type::kFalseType)
		return L"kFalseType";
	if (InType == Type::kNullType)
		return L"kNullType";
	if (InType == Type::kNumberType)
		return L"kNumberType";
	if (InType == Type::kObjectType)
		return L"kObjectType";
	if (InType == Type::kStringType)
		return L"kStringType";
	if (InType == Type::kTrueType)
		return L"kTrueType";
	return L"Unknown";
}

std::wstring GetValueString(Value& Val) {
	Type InType = Val.GetType();
	if (InType == Type::kArrayType)
		return L"[array]";
	if (InType == Type::kFalseType)
		return L"[false]";
	if (InType == Type::kNullType)
		return L"[null]";
	if (InType == Type::kNumberType) {
		std::wstring TempText = L"";
		if (Val.IsDouble())
			TempText = std::to_wstring(Val.GetDouble());
		else
			TempText = std::to_wstring(Val.GetInt64());
		return TempText;
	}
	if (InType == Type::kObjectType)
		return L"[object]";
	if (InType == Type::kStringType)
		return UTF8ToUTF16(Val.GetString());
	if (InType == Type::kTrueType)
		return L"[true]";
	return L"[unknown]";
}

std::wstring GetValueFromPath(std::wstring Path, Document &InDoc) {
	std::vector<std::wstring> Parts = StringSplit(Path, L"/");
	Value *CurVal = &InDoc;
	for each(std::wstring ThisPart in Parts) {
		if (!CurVal->IsObject() && !CurVal->IsArray())
			break;// Only Objects and Arrays can have further members;
		std::string ThisPartCS = UTF16ToUTF8(ThisPart.c_str());
		std::string TempText = CStringA(ThisPartCS.c_str()).SpanIncluding("0123456789");
		if (ThisPartCS.length() == TempText.length() && ThisPartCS.length() > 0) {// Must be a number (index);
			int Index = 0;
			sscanf_s(TempText.c_str(), "%ld", &Index);
			if (Index >= CurVal->Size()) {
				DoLogEntry(L"Path segment index out-of-range: " + ThisPart);
				return L"";
			}
			CurVal = &(*CurVal)[Index];
		}
		else {
			if (!CurVal->HasMember(ThisPartCS.c_str())) {
				DoLogEntry(L"Path segment not found: " + ThisPart);
				return L"";
			}
			CurVal = &(*CurVal)[ThisPartCS.c_str()];
		}
	}
	return std::wstring(GetValueString(*CurVal));
}

std::vector<std::wstring> StringSplit(std::wstring InString, std::wstring SplitOn) {
	std::vector<std::wstring> OutList;
	size_t Loc = 0, LastLoc = 0;
	while ((Loc = InString.find(SplitOn, Loc)) != -1) {
		OutList.push_back(InString.substr(LastLoc, Loc - LastLoc));
		Loc += SplitOn.length();
		LastLoc = Loc;
	}
	if (LastLoc <= InString.length())
		OutList.push_back(InString.substr(LastLoc));
	return OutList;
}

std::vector<std::string> StringSplit(std::string InString, std::string SplitOn) {
	std::vector<std::string> OutList;
	size_t Loc = 0, LastLoc = 0;
	while ((Loc = InString.find(SplitOn, Loc)) != -1) {
		OutList.push_back(InString.substr(LastLoc, Loc - LastLoc));
		Loc += SplitOn.length();
		LastLoc = Loc;
	}
	if (LastLoc <= InString.length())
		OutList.push_back(InString.substr(LastLoc));
	return OutList;
}

std::wstring ReadAllText(std::wstring FilePath) {
	std::wstring TheLoadedString;/* ReadAllText() returns string as UTF-8; */
	__int64 TheFileSize = GetFileSizeFromPath(FilePath);
	if (TheFileSize <= 0)
		return L"";
	HANDLE InFile = CreateFile(FilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (InFile == INVALID_HANDLE_VALUE)
		return L"";
	// Determine file format:
	WORD TheWord;
	DWORD BytesRead = 0;
	ReadFile(InFile, &TheWord, 2, &BytesRead, NULL);
	if (BytesRead < 2) {
		CloseHandle(InFile);
		return L"";
	}
	bool IsUnicode = (TheWord == 0xFEFF);
	bool IsBackwardUnicode = (TheWord == 0xFFFE);
	// Handle it:
	DWORD BytesInOut = 0;
	if (IsUnicode || IsBackwardUnicode) {// Is a Unicode-16 file
		WCHAR *TheBigBuffer = (WCHAR *)malloc((int)TheFileSize + 2);
		if (TheBigBuffer == 0) {
			CloseHandle(InFile);
			return L"";
		}
		ReadFile(InFile, TheBigBuffer, (int)(TheFileSize - 2), &BytesInOut, NULL);
		CloseHandle(InFile);
		TheBigBuffer[(TheFileSize - 2) / 2] = 0;
		if (IsBackwardUnicode) {// Big endian format, so reverse it
			for (int count = 0; count < ((TheFileSize - 2) / 2); count++) {
				TheBigBuffer[count] = htons(TheBigBuffer[count]);
			}
		}
		TheLoadedString = TheBigBuffer;
		delete TheBigBuffer;
	}
	else {// Is not a Unicode-16 file
		char *TheLittleBuffer = (char *)malloc((int)TheFileSize + 1);
		if (TheLittleBuffer == 0) {
			CloseHandle(InFile);
			return L"";
		}
		memcpy(TheLittleBuffer, &TheWord, 2);// Since it was not Unicode-16, we keep the first two chars
		ReadFile(InFile, TheLittleBuffer + 2, (int)(TheFileSize - 2), &BytesInOut, NULL);
		CloseHandle(InFile);
		TheLittleBuffer[TheFileSize] = 0;
		TheLoadedString = UTF8ToUTF16(TheLittleBuffer);
		delete TheLittleBuffer;
	}
	// Return it:
	return TheLoadedString;
}

std::wstring UTF8ToUTF16(const char *UTF8Text) {
	size_t UTF8StringLength = strlen(UTF8Text);
	size_t CharsNeeded = MultiByteToWideChar(CP_UTF8, NULL, UTF8Text, UTF8StringLength, NULL, NULL);
	if (CharsNeeded == 0)
		return L"";
	WCHAR *NewChars = (WCHAR *)malloc((CharsNeeded + 1)*sizeof(WCHAR));
	if (NewChars == NULL)
		return L"";
	NewChars[0] = 0;
	NewChars[CharsNeeded] = 0;
	UINT BytesWritten = MultiByteToWideChar(CP_UTF8, NULL, UTF8Text, UTF8StringLength, NewChars, CharsNeeded + 1);
	if (BytesWritten == 0) {
		delete NewChars;
		return L"";
	}
	std::wstring TheOutputString = NewChars;
	delete NewChars;
	return TheOutputString;
}

std::string UTF16ToUTF8(const WCHAR *UTF16Text) {
	// Converts special codes to standard UTF-8:
	UINT UnicodeStringLength = lstrlenW(UTF16Text);
	UINT CharsNeeded = WideCharToMultiByte(CP_UTF8, NULL, UTF16Text, UnicodeStringLength, NULL, NULL, NULL, NULL);
	if (CharsNeeded == 0)
		return "";
	char *NewChars = (char *)malloc(CharsNeeded + 1);
	if (NewChars == NULL)
		return "";
	NewChars[0] = 0;
	NewChars[CharsNeeded] = 0;
	UINT BytesWritten = WideCharToMultiByte(CP_UTF8, NULL, UTF16Text, UnicodeStringLength, NewChars, CharsNeeded + 1, NULL, NULL);
	if (BytesWritten == 0) {
		delete NewChars;
		return "";
	}
	std::string ReturnString = NewChars;
	delete NewChars;
	return ReturnString;
}

__int64 GetFileSizeFromPath(std::wstring FilePath) {
	WIN32_FIND_DATA TheFindData;
	HANDLE ReturnHandle = FindFirstFile(FilePath.c_str(), &TheFindData);
	if (ReturnHandle != INVALID_HANDLE_VALUE) {
		FindClose(ReturnHandle);
		if ((TheFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			return -1;
		else {
			G_ULARGE_INTEGER BigNumber;
			BigNumber.u.LowPart = TheFindData.nFileSizeLow;
			BigNumber.u.HighPart = TheFindData.nFileSizeHigh;
			return (__int64)BigNumber.QuadPart;
		}
	}
	return 0;
}

SettingsBase::Server::~Server()
{
	if (WebClient != NULL) {
		delete WebClient;
		WebClient = NULL;
	}
}

std::wstring ReplaceOne(std::wstring InString, std::wstring Find, std::wstring Replace)
{
	size_t Loc = InString.find(Find);
	if (Loc == -1)
		return InString;
	return InString.substr(0, Loc) + Replace + (Loc + Find.length() >= InString.length() ? L"" : InString.substr(Loc + Find.length()));
}

std::wstring ReplaceAll(std::wstring InString, std::wstring Find, std::wstring Replace)
{
	std::vector<std::wstring> StringPieces = StringSplit(InString, Find);
	std::wstring Result = L"";
	for each(std::wstring ThisPiece in StringPieces) {
		Result += ThisPiece + Replace;
	}
	return Result.substr(0, Result.length() - Replace.length());
}

std::string ReplaceOne(std::string InString, std::string Find, std::string Replace)
{
	size_t Loc = InString.find(Find);
	if (Loc == -1)
		return InString;
	return InString.substr(0, Loc) + Replace + (Loc + Find.length() >= InString.length() ? "" : InString.substr(Loc + Find.length()));
}

std::string ReplaceAll(std::string InString, std::string Find, std::string Replace)
{
	std::vector<std::string> StringPieces = StringSplit(InString, Find);
	std::string Result = "";
	for each(std::string ThisPiece in StringPieces) {
		Result += ThisPiece + Replace;
	}
	return Result.substr(0, Result.length() - Replace.length());
}
