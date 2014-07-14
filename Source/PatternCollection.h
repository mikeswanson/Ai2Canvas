// PatternCollection.h
//
// Copyright (c) 2010-2014 Mike Swanson (http://blog.mikeswanson.com)
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

#ifndef PATTERNCOLLECTION_H
#define PATTERNCOLLECTION_H

#include "IllustratorSDK.h"
#include "Ai2CanvasSuites.h"
#include "Pattern.h"
#include "Utility.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a collection of patterns (which includes symbols)
	class PatternCollection
	{
	private:

		std::vector<Pattern*>	patterns;			// Collection of patterns
		bool					hasPatterns;		// Does the collection include at least one pattern?
		bool					hasSymbols;			// Does the collection include at least one symbol?
		unsigned int			canvasIndex;		// Track next available canvas index

	public:

		PatternCollection();
		~PatternCollection();

		bool					Add(AIPatternHandle patternHandle, bool isSymbol);
		Pattern*				Find(AIPatternHandle patternHandle);
		std::vector<Pattern*>&	Patterns();
		bool					HasPatterns();
		bool					HasSymbols();
	};
}

#endif