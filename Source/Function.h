// Function.h
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

#ifndef FUNCTION_H
#define FUNCTION_H

#include "IllustratorSDK.h"
#include "Utility.h"
#include "Canvas.h"

namespace CanvasExport
{
	// Represents the abstract base class for functions
	class Function
	{
	private:

	public:

		enum FunctionType { kAnyFunction, kDrawFunction, kAnimationFunction };

		Function();
		virtual ~Function();

		FunctionType		type;					// Type of this function (not very OO, but it works)
		std::string			name;					// Name of the function
		Canvas*				canvas;					// Pointer to targeted canvas
		AIRealRect			bounds;					// Bounds of the artwork
		AIBoolean			translateOrigin;		// Do we need to translate the origin?
		AIReal				translateOriginH;		// Horizontal origin translation as a percentage
		AIReal				translateOriginV;		// Vertical origin translation as a percentage

		// Set function parameters
		virtual void		SetParameter(const std::string& parameter, const std::string& value) = 0;

		virtual void		RenderClockInit() = 0;		// Initialize animation clocks
		virtual void		RenderTriggerInit() = 0;	// Initialize animation clock triggers
		virtual void		RenderClockStart() = 0;		// Start animation clocks
		virtual void		RenderClockTick() = 0;		// Tick animation clocks

		virtual bool const	HasValidTriggers() = 0;		// Does this function have any valid triggers defined?
	};
}
#endif
