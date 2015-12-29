# NetworkActiv IP Filtering Plugin

This project creates a Windows DLL file that enables programs such as NetworkActiv AUTAPF to filter clients by IP address using an external server. Currently both HTTP and HTTPS are supported using Microsoft's open-source [cpprestsdk]. Because AUTAPF is static-built, this plugin has been designed to be built static as well. Static-compiled binaries of the cpprestsdk are included in the Zip file found in *packages/cpprestsdk-master*.

### Features

* Exports a simple IsIPAllowed() function that takes an IP address as a string and returns a boolean indicating whether the response from the server for the given IP address matches those filters as specified in this plugin's JSON-based settings file.
* Supports HTTP and HTTPS.
* Supports HTTP Basic Authentication (preferably over HTTPS).
* Uses JSON for plugin settings; accepts JSON from server.
* Written in C++ for maximum performance. Both 32-bit and 64-bit builds are supported.
* Statically-built for minimal post-build dependency.
* Uses an open and extensible filtering method that can essentially filter based on any value at any depth of the JSON returned from the server, regardless of whether contained in an Object or an Array. This allows support for pretty much any Web server that provides a JSON response for an IP address (or hostname).
* Can filter on any number of fields using any number of servers.

### Licence

This plugin is distributed under the standard MIT licence. See LICENCE.TXT for details. As a result of this licence, you may use this plugin in your own projects for which a Web-interfacing, IP-filtering plugin is needed.

The linked packages have their own respective licences, so be sure to look into these licences as appropriate.

### Copyright

The initial commit of this plugin (minus dependencies) was designed and written entirely by Michael J. Kowalski of [NetworkActiv Software].

### Dependencies

* [cpprestsdk] -- used to provide HTTP and HTTPS communication with servers.
* [rapidjson] -- used to parse JSON loaded from local settings file or as returned from servers.

### Building the DLL

This project was designed to be opened and built using [Microsoft Visual Studio 2015]. You will need to make sure that rapidjson is added to the project, such as through NuGet. **This project must be built statically if it will be used with NetworkActiv AUTAPF or PortImport**. Because cpprestsdk does not provide static builds in its standard NuGet package (as of 2015-12-29), you will need to link manually to the cpprestsdk LIBs and Headers. These headers have been included in the *packages/cpprestsdk-master/Release/include* folder. Static builds of cpprestsdk have been included in *packages/cpprestsdk-master/Binaries_(extract_my_contents_here).zip*. These builds were too large to include uncompressed in this repo.

### Contributions

Additions are welcome as long as they are well-coded, not too specific to a particular product or service, and appropriately licensed. Any contributions must be made under the MIT licence or in Public Domain. Copy-left contributions cannot be accepted. No guarantee is given as to whether a particular contribution will be accepted.

   [cpprestsdk]: <https://github.com/Microsoft/cpprestsdk>
   [Microsoft Visual Studio 2015]: <https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx>
   [rapidjson]: <https://github.com/miloyip/rapidjson>
   [NetworkActiv Software]: <https://www.networkactiv.com/>