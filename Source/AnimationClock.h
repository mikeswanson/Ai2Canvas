// AnimationClock.h
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


#ifndef ANIMATIONCLOCK_H
#define ANIMATIONCLOCK_H

#include "IllustratorSDK.h"
#include "Utility.h"
#include "Trigger.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	// Represents an animation clock
	class AnimationClock
	{
	private:

	public:

		AnimationClock();
		~AnimationClock();

		enum Direction { kBackward = -1, kNone = 0, kForward = 1 };

		std::string		name;				// Name of this clock
		float			duration;			// Clock duration (in seconds)
		float			delay;				// Initial delay (in seconds)
		Direction		direction;			// Direction
		bool			reverses;			// Does the clock automatically reverse?
		unsigned long	iterations;			// Number of iterations (0 = infinite)
		std::string		timingFunction;		// Name of the timing function
		std::string		rangeExpression;	// Upper range JavaScript expression (i.e. "(2.0 * Math.PI)")
		float			multiplier;			// Clock value multiplier
		float			offset;				// Offset after rangeExpression and multiplier (as a percentage of the range)

		// Triggers
		std::vector<Trigger*>	triggers;	// Reference to animation clock triggers

		void			JSClockInit(const std::string& objectName);
		void			JSClockTriggerInit(const std::string& objectName);
		void			JSClockStart(const std::string& objectName);
		void			JSClockTick(const std::string& objectName);
		void			SetParameter(const std::string& parameter, const std::string& value);
		bool			HasValidTriggers();
		bool			HasValidStartTrigger();
		bool			IsValidTrigger(const std::string& parameter);
	};
}
#endif
