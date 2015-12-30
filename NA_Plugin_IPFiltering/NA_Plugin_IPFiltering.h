
#ifdef NA_PLUGIN_IPFILTERING_EXPORTS
#define NA_PLUGIN_IPFILTERING_API __declspec(dllexport)
#else
#define NA_PLUGIN_IPFILTERING_API __declspec(dllimport)
#endif

// This class is exported from the NA_Plugin_IPFiltering.dll
class NA_PLUGIN_IPFILTERING_API CNA_Plugin_IPFiltering {
public:
	CNA_Plugin_IPFiltering(void);
};

extern NA_PLUGIN_IPFILTERING_API int nNA_Plugin_IPFiltering;

NA_PLUGIN_IPFILTERING_API int fnNA_Plugin_IPFiltering(void);

// The following functions are used internally:

void DoLogEntry(const char *InText);
void DoLogEntry(const WCHAR *InText);
void DoLogEntry(std::wstring InText);
const WCHAR *GetDLLPathW();
const char *GetDLLPathA();
const char *GetCurrentDateTimeA();
__int64 GetFileSizeFromPath(std::wstring FilePath);
std::wstring ReadAllText(std::wstring FilePath);
std::wstring UTF8ToUTF16(const char *UTF8Text);
std::string  UTF16ToUTF8(const WCHAR *UTF16Text);
std::wstring GetTypeText(rapidjson::Type InType);
std::wstring GetValueString(rapidjson::Value& Val);
std::wstring GetValueFromPath(std::wstring Path, rapidjson::Document &InDoc);/* Supports both value-pair names and array indices */
std::vector<std::wstring> StringSplit(std::wstring InString, std::wstring SplitOn);
std::wstring ReplaceOne(std::wstring InString, std::wstring Find, std::wstring Replace);

class SettingsBase {
public:
	// Functions (call as needed):
	SettingsBase();
	bool Load();
	bool Save();
	bool HasLoaded = false;
	// Settings (loaded from JSON file; strings as UTF-8):
	class Server {
	public:
		~Server();
		std::wstring URL;
		std::wstring Username;
		std::wstring Password;
		class Filter {
		public:
			std::wstring Path;
			std::vector<std::wstring> AllowOnly;
			std::vector<std::wstring> AllowNone;
		};
		std::vector<Filter *> Filters;// Keep these as pointers;
		// Web client components:
		http_client *WebClient = NULL;// Handles the session;
		std::wstring Protocol = L"http://";
		std::wstring Domain = L"";
	};
	std::vector<Server *> Servers;// Keep these as pointers;
private:
	// Internals:
	rapidjson::Document SetDoc;
	std::wstring SettingsPath = L"";
};

union G_ULARGE_INTEGER {
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;
	ULONGLONG QuadPart;
};

