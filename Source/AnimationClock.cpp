// AnimationClock.cpp
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

#include "IllustratorSDK.h"
#include "AnimationClock.h"

using namespace CanvasExport;

AnimationClock::AnimationClock()
{
	// Initialize AnimationClock
	this->name = "animationClock";
	this->duration = 5.0f;
	this->delay = 0.0f;
	this->direction = AnimationClock::kNone;
	this->reverses = false;
	this->iterations = 0;
	this->timingFunction = "linear";
	this->rangeExpression = "";
	this->multiplier = 1.0f;
	this->offset = 0.0f;
}

AnimationClock::~AnimationClock()
{
	// Clear triggers
	for (unsigned int i = 0; i < triggers.size(); i++)
	{
		// Remove instance
		delete triggers[i];
	}
}

// Output JavaScript to create a clock with the given name
void AnimationClock::JSClockInit(const std::string& objectName)
{
	// Only initialize if we have animation
	if (direction != kNone)
	{
		outFile <<   "\n      " << objectName << "." << name << " = new clock(" <<
			setiosflags(ios::fixed) << setprecision(2) << duration << ", " <<
			delay << ", " <<
			direction << ", " <<
			(reverses ? "true" : "false") << ", " <<
			iterations << ", " <<
			timingFunction << ", " <<
			rangeExpression << ", " <<
			multiplier << ", " <<
			setprecision(4) <<				// Increase precision to accomodate Firefox 3.x scaling issue
			offset <<
			");";

		if (debug)
		{
			// Debug animation clock
			outFile <<   "\n      " << objectName << "." << name << ".timeProvider = debug; // Debug animation clock (comment out for normal animation)";
		}
	}
}

void AnimationClock::JSClockTriggerInit(const std::string& objectName)
{
	// Only initialize if we have animation
	if (direction != kNone)
	{
		// Do we have any triggers?
		if (triggers.size() > 0)
		{
			// Loop through triggers
			// animations[0].pathClock.finished.subscribe(function() { animations[0].pathClock.reset(); });
			for (unsigned int i = 0; i < triggers.size(); i++)
			{
				triggers[i]->JSTriggerInit(objectName, name);
			}
		}
	}
}

// Outputs correct JavaScript to start a clock immediately
void AnimationClock::JSClockStart(const std::string& objectName)
{
	// Only start if we have animation
	if (direction != kNone)
	{
		// And only if we don't have another valid start trigger
		if (!HasValidStartTrigger())
		{
			outFile <<   "\n      " << objectName << "." << name << ".start();";
		}
	}
}

// Output JavaScript to tick a clock
void AnimationClock::JSClockTick(const std::string& objectName)
{
	// Only tick if we have animation
	if (direction != kNone)
	{
		// Output tick
		outFile <<   "\n      " << objectName << "." << name << ".update();";
	}
}

// Does this clock have any valid triggers?
bool AnimationClock::HasValidTriggers()
{
	bool result = false;

	// Loop through triggers
	for (unsigned int i = 0; i < triggers.size(); i++)
	{
		// Is this a valid trigger?
		if (triggers[i]->parsedOkay)
		{
			// Found at least one
			result = true;
			break;
		}
	}

	return result;
}

// Does this clock have a valid start trigger?
bool AnimationClock::HasValidStartTrigger()
{
	bool result = false;

	// Loop through triggers
	for (unsigned int i = 0; i < triggers.size(); i++)
	{
		// Is this a valid start trigger?
		if (triggers[i]->parsedOkay &&
			triggers[i]->triggeredFunction == "start")
		{
			// Found one
			result = true;
			break;
		}
	}

	return result;
}

void AnimationClock::SetParameter(const std::string& parameter, const std::string& value)
{
	// Duration
	if (parameter == "duration" ||
		parameter == "dur")
	{
		if (debug)
		{
			outFile << "\n//     Found animation duration parameter";
		}

		// Parse a float value for the duration parameter
		this->duration = (float)strtod(value.c_str(), NULL);

		if (debug)
		{
			outFile << "\n//     duration = " << setiosflags(ios::fixed) << setprecision(2) <<
				this->duration << " seconds";
		}
	}

	// Delay
	if (parameter == "delay" ||
		parameter == "del")
	{
		if (debug)
		{
			outFile << "\n//     Found animation delay parameter";
		}

		// Parse a float value for the delay parameter
		this->delay = (float)strtod(value.c_str(), NULL);

		if (debug)
		{
			outFile << "\n//     delay = " << setiosflags(ios::fixed) << setprecision(2) <<
				this->delay << " seconds";
		}
	}

	// Direction
	if (parameter == "direction" ||
		parameter == "dir")
	{
		if (debug)
		{
			outFile << "\n//     Found animation direction parameter";
		}

		// Parse value
		if (value == "none" ||
			value == "n")
		{
			this->direction = AnimationClock::kNone;
		}
		else if (value == "forward" ||
				 value == "f")
		{
			this->direction = AnimationClock::kForward;
		}
		else if (value == "backward" ||
				 value == "b")
		{
			this->direction = AnimationClock::kBackward;
		}
	}

	// Reverses?
	if (parameter == "reverses" ||
		parameter == "rev")
	{
		if (debug)
		{
			outFile << "\n//     Found animation reverses parameter";
		}

		if (value == "yes" ||
			value == "y")
		{
			this->reverses = true;
		}
		else if (value == "no" ||
				 value == "n")
		{
			this->reverses = false;
		}
	}

	// Iterations
	if (parameter == "iterations" ||
		parameter == "iter")
	{
		if (debug)
		{
			outFile << "\n//     Found animation iterations parameter";
		}

		// Infinite?
		if (value == "infinite" ||
			value == "i")
		{
			// 0 = infinite
			this->iterations = 0;
		}
		else
		{
			// Parse a unsigned long int value for the iteration parameter
			this->iterations = strtoul(value.c_str(), NULL, 0);

			if (debug)
			{
				outFile << "\n//     iterations = " << setiosflags(ios::fixed) << setprecision(2) <<
					this->iterations;
			}
		}
	}

	// Timing function
	if (parameter == "timing-function" ||
		parameter == "t-f")
	{
		if (debug)
		{
			outFile << "\n//     Found animation timing function parameter";
		}

		// Short-cut value?
		if (value == "linear" ||
			value == "l")
		{
			// Set to linear
			this->timingFunction = "linear";
		}
		else
		{
			// Get the associated timing function name
			std::string timingFunctionName = value;

			// Clean the name for a function
			// TODO: Should we convert to camel-case? Doesn't seem like we should, since this will be in the user's JavaScript
			CleanString(timingFunctionName, false);

			if (debug)
			{
				outFile << "\n//     Timing function name = " << timingFunctionName;
			}

			// Set function name
			this->timingFunction = timingFunctionName;
		}
	}

	// Multiplier
	if (parameter == "multiplier" ||
		parameter == "mult")
	{
		if (debug)
		{
			outFile << "\n//     Found animation multiplier parameter";
		}

		// Parse a float value for the multiplier parameter
		this->multiplier = (float)strtod(value.c_str(), NULL);

		if (debug)
		{
			outFile << "\n//     multiplier = " << setiosflags(ios::fixed) << setprecision(2) <<
				this->multiplier;
		}
	}

	// Offset (as a percentage)
	if (parameter == "offset" ||
		parameter == "off")
	{
		if (debug)
		{
			outFile << "\n//     Found animation offset parameter";
		}

		// Parse a float value for the offset parameter
		this->offset = (float)strtod(value.c_str(), NULL);

		if (debug)
		{
			outFile << "\n//     offset = " << setiosflags(ios::fixed) << setprecision(2) <<
				this->offset;
		}
	}

	// Trigger related?
	if (IsValidTrigger(parameter))
	{
		if (debug)
		{
			outFile << "\n//     Found trigger parameter";
		}

		// Create new trigger
		Trigger* trigger = new Trigger();

		// Set the trigger parameter
		trigger->SetParameter(parameter, value);

		// Did the trigger parse okay?
		if (trigger->parsedOkay)
		{
			// Add it to the list
			triggers.push_back(trigger);
		}
		else
		{
			// Delete it
			delete trigger;
		}
	}
}

// Is the parameter name a valid trigger?
bool AnimationClock::IsValidTrigger(const std::string& parameter)
{
	return (parameter == "reset" ||
			parameter == "start" ||
			parameter == "restart" ||
			parameter == "stop" ||
			parameter == "toggle" ||
			parameter == "rewind" ||
			parameter == "fast-forward" ||
			parameter == "reverse");
}
