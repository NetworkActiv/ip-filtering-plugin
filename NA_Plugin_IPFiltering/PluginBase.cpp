#include "stdafx.h"
#include "PluginBase.h"
#include "NA_Plugin_IPFiltering.h"

PluginBase::PluginBase()
{
}

PluginBase::~PluginBase()
{
}

const WCHAR * PluginBase::GetPluginType()
{
	return GetPluginTypeInternal();
}

void PluginBase::GetPluginTypeCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow)
{
	MessageBox(NULL, GetPluginTypeInternal(), L"My Type Is", MB_OK);
}

const WCHAR * PluginBase::GetPluginName()
{
	return GetPluginNameInternal();
}

void PluginBase::GetPluginNameCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow)
{
	MessageBox(NULL, GetPluginNameInternal(), L"My Name Is", MB_OK);
}

const WCHAR * PluginBase::GetPluginDescription()
{
	return GetPluginDescriptionInternal();
}

void PluginBase::GetPluginDescriptionCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow)
{
	MessageBox(NULL, GetPluginDescriptionInternal(), L"My Description Is", MB_OK);
}

const WCHAR * PluginBase::GetPluginVersion()
{
	return GetPluginVersionInternal();
}

void PluginBase::GetPluginVersionCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow)
{
	MessageBox(NULL, GetPluginVersionInternal(), L"My Version Is", MB_OK);
}

const WCHAR * PluginBase::GetPluginTypeInternal()
{
	return L"ClientIPFiltering";
}

const WCHAR * PluginBase::GetPluginNameInternal()
{
	return L"HTTP-Based Client IP Filter";
}

const WCHAR * PluginBase::GetPluginDescriptionInternal()
{
	return L"This plugin filters client IP addresses using an API from a third-party. Communication is accepted in JSON.";
}

const WCHAR * PluginBase::GetPluginVersionInternal()
{
	return L"1.0.5";
}

bool PluginBase::IsIPAllowed(const WCHAR *IPAddress)
{
	return IsIPAllowedInternal(IPAddress);
}

void CALLBACK PluginBase::IsIPAllowedCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow)
{
	MessageBox(NULL, IsIPAllowedInternal(CString(IPAddress)) ? L"VALID" : L"INVALID", L"The Provided IP Is", MB_OK);
}

SettingsBase Settings;

bool PluginBase::IsIPAllowedInternal(const WCHAR * IPAddress)
{
	if (!Settings.HasLoaded) {
		DoLogEntry(L"IP " + std::wstring(IPAddress) + L" FAILED: No settings are loaded");
		return false;
	}
	long long ChecksRemaining = Settings.Servers.size();
	long long *ChecksRemainingPtr = &ChecksRemaining;// Avoids closure issues;
	long long PassCount = 0, FailCount = 0;
	long long *PassCountPtr = &PassCount, *FailCountPtr = &FailCount;
	struct ReasonStruct {
		std::wstring Text = L"";
	};
	ReasonStruct *Reason = new ReasonStruct();
	if (Reason == NULL) {
		return false;
	}
	for each(SettingsBase::Server *ThisServer in Settings.Servers) {
		std::wstring PathText = ThisServer->URL.substr(ThisServer->Protocol.length() + ThisServer->Domain.length());
		if (PathText == L"")
			PathText = L"/";
		PathText = ReplaceOne(PathText, L"$IPAddress", IPAddress);
		uri_builder Query = uri_builder(uri(PathText));
		ThisServer->WebClient->request(methods::GET, Query.to_string())
			.then([=](http_response ActualResponse) {
			// Called upon initial response from server;
			stringstreambuf *ReceiveBuffer = new stringstreambuf();
			if (ReceiveBuffer != NULL) {
				ActualResponse.body().read_to_end(*ReceiveBuffer)
					.then([=](size_t BytesRead) {
					// Called once all data has been received;
					std::string StringBuffer = ReceiveBuffer->collection();
					char BOMChars[5] = { (char)0xEF, (char)0xBB, (char)0xBF, (char)0x00, (char)0x00 };
					if (StringBuffer.find(BOMChars) == 0)// EF BB BF = UTF8 Byte-Order Mark (BOM);
						StringBuffer = StringBuffer.substr(3);
					rapidjson::Document InDoc;
					InDoc.Parse(StringBuffer.data());
					// [Important] Free memory used by the stream and buffer:
					ReceiveBuffer->close();
					delete ReceiveBuffer;
					// See if IP address passes these filters:
					for each (SettingsBase::Server::Filter *ThisFilter in ThisServer->Filters){
						// Obtain the value in the JSON for this filter's "path":
						std::wstring ThisValue = GetValueFromPath(ThisFilter->Path, InDoc);
						if (ThisValue == L"[null]") {
							(*FailCountPtr)++;// Fail type 1: Value not found;
							Reason->Text += L" JSON value not found (for path=" + ThisFilter->Path + L", bytes_read=" + std::to_wstring(BytesRead) + L");";
						}
						else {
							// Check for blocked (disallowed) values:
							for each (std::wstring BadEntry in ThisFilter->AllowNone) {
								if(ThisValue == BadEntry){
									(*FailCountPtr)++;// Fail type 2: Explicitly blocked;
									Reason->Text += L" Explicitly blocked (" + ThisValue + L");";
									break;
								}
							}
							// Check for permissible (allowed) values:
							bool HadAPass = false;
							for each (std::wstring GoodEntry in ThisFilter->AllowOnly) {
								if (ThisValue == GoodEntry) {
									(*PassCountPtr)++;
									HadAPass = true;
									Reason->Text += L" Explicitly allowed (" + ThisValue + L");";
									break;
								}
							}
							if (!HadAPass && ThisFilter->AllowOnly.size() > 0) {
								(*FailCountPtr)++;// Fail type 3: Not explicitly allowed;
								Reason->Text += L" Not allowed (" + ThisValue + L");";
							}
						}
					}
					(*ChecksRemainingPtr)--;
				});
			}
			else {// Bad buffer;
				(*ChecksRemainingPtr)--;
			}
		});
	}
	// Wait for all servers to respond:
	int x = 0;
	for (int x = 0; x < 1000; x++) {
		if (ChecksRemaining <= 0)
			break;
		Sleep(10);
	}
	if (x >= 1000) {
		DoLogEntry(L"IP " + std::wstring(IPAddress) + L" FAILED: Server timeout");
		delete Reason;
		return false;// Fail type 4: Server timeout (prevents exploit via flooding);
	}
	if (FailCount == 0) {
		DoLogEntry(L"IP " + std::wstring(IPAddress) + L" PASSED:" + Reason->Text);
		delete Reason;
		return true;
	}
	else {
		DoLogEntry(L"IP " + std::wstring(IPAddress) + L" FAILED:" + Reason->Text);
		delete Reason;
		return false;
	}
}

