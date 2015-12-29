
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // remove support for MFC controls in dialogs

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
// Windows Header Files:
#include <windows.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <vector>

// Microsoft's cppRestSDK:

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <sstream>

using namespace utility;               // Common utilities like string conversions
using namespace web;                   // Common features like URIs.
using namespace web::http;             // Common HTTP functionality
using namespace web::http::client;     // HTTP client features
using namespace concurrency::streams;  // Asynchronous streams

//########## BUILDING AND USING THE *STATIC* cppRestSDK ##########
//
//(1) Download the latest cppRestSDK, such as from GitHub.
//
//(2) Open the project and delete any unneeded components.
//
//(3) Set all modes to Static build ["Static library (.lib)"].
//
//(4) Set the code generation to "Multi-threaded Debug (/MTd)" and "Multi-threaded //    (/ MT)", respectively. Note the "T" for STaTic.
//
//(5) Include the "_NO_ASYNCRTIMP;" preprocessor definition on all modes.
//
//(6) Build the lib, which should generate in cpprestsdk-master\Binaries.
//
//(7) In the consumer application (or DLL), go to the Project->Properties and set C/C++//    -> General -> Additional Include Directories to :
//    $(SolutionDir)packages\cpprestsdk-master\Release\include;
//
//(8) Then set "VC++ Directories -> Library Directories" to:
//    $(SolutionDir)packages\cpprestsdk-master\Binaries\$(Platform)\$(Configuration)\;
//
//(9) Go to the Preprocessor Definitions and include "_NO_ASYNCRTIMP;".
//
//(10) Finally, in Linker -> Input, set Additional Dependencies to :
//     winhttp.lib; cpprest140_2_7.lib; Crypt32.lib; Bcrypt.lib;
//
//################################################################

