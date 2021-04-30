// DrawFunction.h
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

#ifndef DRAWFUNCTION_H
#define DRAWFUNCTION_H

#include "IllustratorSDK.h"
#include "Function.h"
#include "Canvas.h"
#include "AnimationFunction.h"
#include "Layer.h"
#include "Utility.h"
#include "AnimationClock.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a JavaScript drawing function
	class DrawFunction : public Function
	{
	private:

	public:

		enum RotateDirection { kRotateNone, kRotateClockwise, kRotateCounterclockwise };

		DrawFunction();
		~DrawFunction();

		std::string			requestedName;			// The requested function name (not always the same as what is used)
		std::vector<Layer*>	layers;					// Array of layers for this drawing function
		bool				hasGradients;			// Does this function use gradients?
		bool				hasPatterns;			// Does this function use pattern fills?
		bool				hasAlpha;				// Does this function have alpha changes?
		AnimationFunction*	animationFunction;		// Associated animation function
		std::string			animationFunctionName;	// Associated animation function name (capture as string so we can late-bind after all parsing is complete)
		bool				follow;					// Does this function follow an orientation for an animation path?
		AIReal				followOrientation;		// Follow orientation (in degrees)
		std::string			rasterizeFileName;		// File name if this function is to be rasterized (empty if not)
		bool				crop;					// Crop canvas to bounds of this drawing layer?

		AnimationClock		rotateClock;			// Rotation animation clock
		AnimationClock		scaleClock;				// Scale animation clock
		AnimationClock		alphaClock;				// Alpha animation clock

		virtual void		RenderClockInit();		// Initialize animation clocks
		virtual void		RenderTriggerInit();	// Initialize animation clock triggers
		virtual void		RenderClockStart();		// Start animation clocks
		virtual void		RenderClockTick();		// Tick animation clocks
		virtual void		SetParameter(const std::string& parameter, const std::string& value);
		virtual bool const	HasValidTriggers();

		void				RenderDrawFunctionCall(const AIRealRect& documentBounds);
		void				RenderDrawFunction(const AIRealRect& documentBounds);
		void				Reposition(const AIRealRect& documentBounds);
		bool const			HasAnimation();			// Does this draw function have any animation?

	};
}
#endif
