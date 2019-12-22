// AnimationFunction.h
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

#ifndef ANIMATIONFUNCTION_H
#define ANIMATIONFUNCTION_H

#include "IllustratorSDK.h"
#include "Function.h"
#include "Canvas.h"
#include "Utility.h"
#include "AnimationClock.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	struct BezierInfo
	{
		AIRealBezier	b;							// Bezier
		AIReal			length;						// Segment length
	};

	/// Represents a JavaScript animation function
	class AnimationFunction : public Function
	{
	private:

	public:

		AnimationFunction();
		~AnimationFunction();

		AnimationClock		pathClock;						// Clock for the animation path

		unsigned int		index;							// JavaScript animation array index
		AIArtHandle			artHandle;						// Handle to art tree
		std::vector<BezierInfo>	beziers;					// Bezier segments (for arc-length calculations)
		float				segmentLength;					// Computed linear segment length

		void				RenderInit(const AIRealRect& documentBounds);			// Initialize animation
		virtual void		RenderClockInit();		// Initialize animation clocks
		virtual void		RenderTriggerInit();	// Initialize animation clock triggers
		virtual void		RenderClockStart();		// Start animation clocks
		virtual void		RenderClockTick();		// Tick animation clocks
		virtual void		SetParameter(const std::string& parameter, const std::string& value);
		virtual bool const	HasValidTriggers();

		void				RenderArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderGroupArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderCompoundPathArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderPathArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderPathFigure(AIArtHandle artHandle, unsigned int depth);
		void				RenderSegment(AIPathSegment& previousSegment, AIPathSegment& segment, unsigned int depth);
		void				TransformPoint(AIRealPoint& point);
		void				Bezier(const AIRealBezier& b, AIReal u, AIReal& x, AIReal& y);
		void				ArcLength(unsigned int depth);
	};
}
#endif
