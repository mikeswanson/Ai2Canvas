// Ai2CanvasPlugin.cpp
//
// Copyright (c) 2010-2021 Mike Swanson (http://blog.mikeswanson.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "IllustratorSDK.h"
#include "AIScriptMessage.h"
#include "Ai2CanvasPlugin.h"

// CanvasExport stuff
#include "Utility.h"
#include "Document.h"
#include "Image.h"
#include "State.h"
#include "Canvas.h"

#ifdef MAC_ENV
    #include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef WIN_ENV
	#include "shellapi.h"
#endif 

#define kSelectorAIScriptExport	"Export"

using namespace CanvasExport;

namespace CanvasExport
{
	// Globals
	ofstream outFile;
	bool debug;
}

/*
*/
Plugin* AllocatePlugin(SPPluginRef pluginRef)
{
	return new Ai2CanvasPlugin(pluginRef);
}

/*
*/
void FixupReload(Plugin* plugin)
{
	Ai2CanvasPlugin::FixupVTable((Ai2CanvasPlugin*) plugin);
}

/*
*/
Ai2CanvasPlugin::Ai2CanvasPlugin(SPPluginRef pluginRef) 
	: Plugin(pluginRef)
{
	strncpy(fPluginName, kAi2CanvasPluginName, kMaxStringLength);
}

/*
*/
ASErr Ai2CanvasPlugin::Message(char* caller, char* selector, void *message) 
{
	ASErr error = kNoErr;

	// Is this command being sent from an Illustrator script?
	if (strcmp(caller, kCallerAIScriptMessage) == 0)
	{
		bool isRecognizedCommand = false;

		AIScriptMessage* msg = (AIScriptMessage*)message;
		ai::UnicodeString outParam("");

		// Export command?
		if (strcmp(selector, kSelectorAIScriptExport) == 0)
		{
			isRecognizedCommand = true;
		}
		// Unrecognized command
		else
		{
			isRecognizedCommand = false;

			outParam.append(ai::UnicodeString("Unrecognized command: '"));
			outParam.append(ai::UnicodeString(selector));
			outParam.append(ai::UnicodeString("'"));
			outParam.append(ai::UnicodeString(" (only valid command is '"));
			outParam.append(ai::UnicodeString(kSelectorAIScriptExport));
			outParam.append(ai::UnicodeString("')"));
		}

		if (isRecognizedCommand)
		{
			if (msg->inParam.empty())
			{
				outParam.append(ai::UnicodeString("No output path provided"));
			}
			else
			{
				char pathName[300];
				msg->inParam.as_Roman(pathName, 300);

				error = WriteText(pathName, false);
				if (error == kNoErr)
				{
					outParam.append(ai::UnicodeString("Exported to: '"));
				}
				else
				{
					outParam.append(ai::UnicodeString("Error exporting to: '"));
				}

				outParam.append(msg->inParam);
				outParam.append(ai::UnicodeString("'"));
			}
		}

		// Return the output message as a parameter
		msg->outParam = outParam;
	}
	else
	{
		try {
			error = Plugin::Message(caller, selector, message);
		}
		catch (ai::Error& ex) {
			error = ex;
		}
		catch (...) {
			error = kCantHappenErr;
		}
		if (error) {
			if (error == kUnhandledMsgErr) {
				// Defined by Plugin.hpp and used in Plugin::Message - ignore.
				error = kNoErr;
			}
			else {
				Plugin::ReportError(error, caller, selector, message);
			}
		}
	}

	return error;
}


/*
*/
ASErr Ai2CanvasPlugin::StartupPlugin(SPInterfaceMessage* message)
{
	ASErr error = kNoErr;
	error = Plugin::StartupPlugin(message);
    if (error) { return error;  }
	error = this->AddMenus(message);
    if (error) { return error;  }
	error = this->AddFileFormats(message);
	
	return error;
}

ASErr Ai2CanvasPlugin::GoMenuItem(AIMenuMessage* message)
{
	ASErr error = kNoErr;
	if (message->menuItem == this->fAboutPluginMenu) {
		// Pop this plug-in's about box.
		SDKAboutPluginsHelper aboutPluginsHelper;

		#ifdef MAC_ENV
			aboutPluginsHelper.PopAboutBox(message, "Ai->Canvas Export Plug-In 1.6 (Mac)", "Copyright 2010-2021 Mike Swanson\nAll rights reserved\nhttp://blog.mikeswanson.com/");
		#endif 
		#ifdef WIN_ENV
		#ifdef _WIN64
			aboutPluginsHelper.PopAboutBox(message, "Ai->Canvas Export Plug-In 1.6 (PC/64)", "Copyright 2010-2021 Mike Swanson\nAll rights reserved\nhttp://blog.mikeswanson.com/");
		#else
			aboutPluginsHelper.PopAboutBox(message, "Ai->Canvas Export Plug-In 1.6 (PC/32)", "Copyright 2010-2021 Mike Swanson\nAll rights reserved\nhttp://blog.mikeswanson.com/");
		#endif
		#endif 
	}	
	return error;
}


ASErr Ai2CanvasPlugin::AddMenus(SPInterfaceMessage* message) {
	ASErr error = kNoErr;
	// Add a menu item to the About SDK Plug-ins menu group.
	SDKAboutPluginsHelper aboutPluginsHelper;
	error = aboutPluginsHelper.AddAboutPluginsMenuItem(message, 
				"AboutMikeSwansonPluginsGroupName", 
				ai::UnicodeString("About Mike Swanson Plug-Ins"),
				"Ai->Canvas...", 
				&this->fAboutPluginMenu);
	return error;
}

ASErr Ai2CanvasPlugin::AddFileFormats(SPInterfaceMessage* message) 
{
	ASErr error = kNoErr;

	PlatformAddFileFormatData affd;
	char pstrCanvas[kMaxStringLength] = "<canvas>";
	
	affd.title = ai::UnicodeString::FromRoman(pstrCanvas);
	affd.titleOrder = 0;
	affd.extension = ai::UnicodeString::FromRoman("html");
	
	error = sAIFileFormat->AddFileFormat( message->d.self, "<canvas>",
				                          &affd, kFileFormatExport,
										  &this->fFileFormatCanvas, kNoExtendedOptions );
	return error;
}

ASErr Ai2CanvasPlugin::GoFileFormat(AIFileFormatMessage* message) 
{
	ASErr error = kNoErr;
	char pathName[300];

	message->GetFilePath().GetFullPath().as_Roman( pathName, 300);
	
	if ( message->option & kFileFormatExport ) 
	{
		// Export our HTM canvas file
		error = WriteText(pathName, true);
	}
	
	return error;
}

ASErr Ai2CanvasPlugin::WriteText(const char* pathName, AIBoolean openFile)
{
	ASErr error = kNoErr;
	
	#ifdef MAC_ENV
		// Determine if shift key is being held down (can't distinguish between left/right shift keys using this method on OS X)
//		bool debugActivated = ((GetCurrentKeyModifiers() & (1 << shiftKeyBit)) != 0);
    
        bool debugActivated = (CGEventSourceFlagsState(kCGEventSourceStateHIDSystemState) & kCGEventFlagMaskShift);
	#endif 
	#ifdef WIN_ENV
		// Determine if the left shift key is being held down (to indicate previewing HTML file after export)
		bool debugActivated = ((GetKeyState(VK_LSHIFT) &0x1000) != 0);
	#endif 

	// Create file
	std::string file = std::string(pathName);
	if (OpenFile(file))
	{
		// Set debug mode
		//CanvasExport::debug = (openFile != 0);
		CanvasExport::debug = debugActivated;

		// Create a new document
		Document* document = new Document(file);

		// Render the document
		document->Render();

		// Close the file
		CloseFile();

		// Delete document
		delete document;
	}

	#ifdef MAC_ENV
		// Create file URI
		ai::UnicodeString usPath(file);
		ai::FilePath aiFilePath(usPath);
		std::string uri = aiFilePath.GetAsURL(false).as_Platform();

		// Launch the file
		if (openFile)
		{
			std::string command = "open " + uri;
			system(command.c_str());
		}
	#endif 
	#ifdef WIN_ENV
		// Launch the file
		if (openFile)
		{
			ShellExecute(NULL, "open", pathName, NULL, NULL, SW_SHOWNORMAL);
		}
	#endif 

	return error;
}

// End Ai2CanvasPlugin.cpp
