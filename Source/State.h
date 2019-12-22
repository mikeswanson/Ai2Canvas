// State.h
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

#ifndef STATE_H
#define STATE_H

#include "IllustratorSDK.h"
#include "Utility.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a context drawing state
	class State
	{
	private:

	public:

		State();
		~State();

		AIReal				globalAlpha;			// Global canvas alpha value (0.0 - 1.0)
		std::string			fillStyle;				// String fill style (e.g. "rgb(0, 0, 0)")
		std::string			strokeStyle;			// String stroke style (e.g. "rgb(0, 0, 0)")
		AIReal				lineWidth;				// Stroke width (in pixels)
		AILineCap			lineCap;				// Cap type
		AILineJoin			lineJoin;				// Join type
		AIReal				miterLimit;				// Stroke miter limit
		AIReal				fontSize;				// Font size (in pixels)
		std::string			fontName;				// Font name
		std::string			fontStyleName;			// Style name
		AIBoolean			isProcessingSymbol;		// Is an Illustrator symbol being processed?
		AIRealMatrix		internalTransform;		// Internal transformation for adjustments from Illustrator to canvas coordinate space

		void				DebugInfo();
	};

}
#endif
