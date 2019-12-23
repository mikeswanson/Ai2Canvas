// Ai2CanvasPlugin.h
//
// Copyright (c) 2010-2020 Mike Swanson (http://blog.mikeswanson.com)
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

#ifndef __AI2CANVASPLUGIN_H__
#define __AI2CANVASPLUGIN_H__

#include "IllustratorSDK.h"
#include "Plugin.hpp"
#include "Ai2CanvasID.h"
#include "Ai2CanvasSuites.h"
#include "SDKAboutPluginsHelper.h"

#define kMaxStringLength 256

/**	Creates a new Ai2CanvasPlugin.
	@param pluginRef IN unique reference to this plugin.
	@return pointer to new Ai2CanvasPlugin.
*/
Plugin* AllocatePlugin(SPPluginRef pluginRef);

/**	Reloads the Ai2CanvasPlugin class state when the plugin is 
	reloaded by the application.
	@param plugin IN pointer to plugin being reloaded.
*/
void FixupReload(Plugin* plugin);

/**	Provides a plugin which demonstrates adding new file formats to open and save to.
*/
class Ai2CanvasPlugin : public Plugin
{
public:
	/**	Constructor.
		@param pluginRef IN reference to this plugin.
	*/
	Ai2CanvasPlugin(SPPluginRef pluginRef);

	/**	Destructor.
	*/
	virtual ~Ai2CanvasPlugin(){}

	/**	Restores state of Ai2CanvasPlugin during reload.
	*/
	FIXUP_VTABLE_EX(Ai2CanvasPlugin, Plugin);

protected:
	/** Calls Plugin::Message and handles any errors returned.
		@param caller IN sender of the message.
		@param selector IN nature of the message.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	virtual ASErr Message(char* caller, char* selector, void *message);

	/**	Calls Plugin::Startup and initialisation functions, such as 
		AddMenus and AddNotifiers.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	virtual ASErr StartupPlugin(SPInterfaceMessage* message);

	/**	Performs actions required for menu item selected.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	virtual ASErr GoMenuItem(AIMenuMessage* message);
	
	/**	Performs actions required for file format selected.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	virtual ASErr GoFileFormat(AIFileFormatMessage* message);

private:
	/**	File format handle for Selected Text as Text.
	*/
	AIFileFormatHandle fFileFormatCanvas;
	
	/**	Menu item handle for this plugins About menu.
	*/
	AIMenuItemHandle fAboutPluginMenu;

	/**	Adds the menu items for this plugin to the application UI.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	ASErr AddMenus(SPInterfaceMessage* message);

	/**	Adds the file formats for this plugin to the application.
		@param message IN pointer to plugin and call information.
		@return kNoErr on success, other ASErr otherwise.
	*/
	ASErr AddFileFormats(SPInterfaceMessage* message);

	/**	Writes the text to the file.
		@param pathName IN path to file.
		@param selectedTextOnly IN true to write only selected text, false to write all.
		@return kNoErr on success, other ASErr otherwise.
	*/
	ASErr WriteText(const char* pathName);
};
#endif // End Ai2CanvasPlugin.h
