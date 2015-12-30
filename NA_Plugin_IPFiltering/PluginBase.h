#pragma once

class PluginBase
{
public:
	PluginBase();
	~PluginBase();
	AFX_EXT_CLASS const WCHAR *GetPluginType();
	AFX_EXT_CLASS void GetPluginTypeCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow);
	AFX_EXT_CLASS const WCHAR *GetPluginName();
	AFX_EXT_CLASS void GetPluginNameCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow);
	AFX_EXT_CLASS const WCHAR *GetPluginDescription();
	AFX_EXT_CLASS void GetPluginDescriptionCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow);
	AFX_EXT_CLASS const WCHAR *GetPluginVersion();
	AFX_EXT_CLASS void GetPluginVersionCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow);
	// IsIPAllowed() takes an IPv4 or IPv6 address, possibly including a scope ID;
	// For example: "1.2.3.4" or "1111:2222:3333:4444:5555:6666:7777:8888%1234567890";
	AFX_EXT_CLASS bool IsIPAllowed(const WCHAR *IPAddress);
	// IsIPAllowedCmd() is like IsIPAllowed() but can be called directly using rundll32 at the command prompt;
	AFX_EXT_CLASS void CALLBACK IsIPAllowedCmd(HINSTANCE hinst, LPSTR IPAddress, int nCmdShow);
private:
	const WCHAR *GetPluginTypeInternal();
	const WCHAR *GetPluginNameInternal();
	const WCHAR *GetPluginVersionInternal();
	const WCHAR *GetPluginDescriptionInternal();
	// IsIPAllowedInternal() is where the actual processing takes place;
	bool IsIPAllowedInternal(const WCHAR * IPAddress);
};
