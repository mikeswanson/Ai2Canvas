// Image.h
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

#ifndef IMAGE_H
#define IMAGE_H

#include "IllustratorSDK.h"
#include "Utility.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a bitmap image
	class Image
	{
	private:


	public:

		Image(const std::string& id, const std::string& path);		
		~Image();

		std::string				id;					// Image element ID
		std::string				path;				// File path to the image
		std::string				name;				// Name of the image (to be used for the alt attribute)
		bool					pathIsAbsolute;		// Is this an absolute image path?

		void					Render();
		void					RenderDrawImage(const std::string& contextName, const AIReal x, const AIReal y);
		void					DebugBounds(const std::string& contextName, const AIRealRect& bounds);
		std::string				Uri();

	};
}

#endif
