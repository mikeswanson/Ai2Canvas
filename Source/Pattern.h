// Pattern.h
//
// Copyright (c) 2010-2018 Mike Swanson (http://blog.mikeswanson.com)
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

#ifndef PATTERN_H
#define PATTERN_H

#include "IllustratorSDK.h"
#include "Utility.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a pattern
	class Pattern
	{
	private:

	public:

		Pattern();
		~Pattern();

		std::string			name;							// Name of the pattern/symbol
		AIPatternHandle		patternHandle;					// Handle to the pattern (for matching with artwork)
		int					canvasIndex;					// Index number for canvas
		bool				isSymbol;						// Is this pattern an Illustrator symbol?
		bool				hasGradients;					// Does this pattern use gradients?
		bool				hasPatterns;					// Does this pattern use pattern fills?
		bool				hasAlpha;						// Does this pattern use alpha?

	};
}
#endif