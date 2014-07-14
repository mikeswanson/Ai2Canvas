// Trigger.cpp
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

#include "IllustratorSDK.h"
#include "Trigger.h"

using namespace CanvasExport;

Trigger::Trigger()
{
	// Initialize Trigger
	this->parsedOkay = false;
	this->sourceObject = "";
	this->sourceClock = "";
	this->sourceEvent = "";
	this->triggeredFunction = "";
}

Trigger::~Trigger()
{
}

void Trigger::SetParameter(const std::string& parameter, const std::string& value)
{
	std::string parameterName = parameter;

	// Need to modify any "short-cuts"?
	if (parameterName == "fast-forward")
	{
		// Change to correct JavaScript function name
		parameterName = "fastForward";
	}

	// Parameter should be the name of the trigger
	this->triggeredFunction = parameterName;

	// Parse value into object-clock-event
	// Store as strings for now, since they'll have to be bound after everything has been parsed

	// Tokenize the values
	std::vector<std::string> values = Tokenize(value, "-");

	// Process based on whether we have 2 or 3 values
	switch (values.size())
	{
		case 2:
		{
			// Is this a valid event?
			if (IsValidEvent(values[1]))
			{
				// object-event (object has a single clock...like an animation path function)
				this->sourceObject = values[0];
				this->sourceClock = "";
				this->sourceEvent = values[1];

				// Note that we parsed
				this->parsedOkay = true;
			}

			break;
		}
		case 3:
		{
			// Is this a valid event?
			if (IsValidEvent(values[2]))
			{
				// object-clock-event (object has multiple clocks...like a draw function)
				this->sourceObject = values[0];
				this->sourceClock = values[1];
				this->sourceEvent = values[2];

				// Note that we parsed
				this->parsedOkay = true;
			}

			break;
		}
	}

	if (debug)
	{
		outFile << "\n//     triggeredFunction = " << this->triggeredFunction;
		outFile << "\n//     sourceObject = " << this->sourceObject;
		outFile << "\n//     sourceClock = " << this->sourceClock;
		outFile << "\n//     sourceEvent = " << this->sourceEvent;
	}
}

// Is the value a valid event name?
bool Trigger::IsValidEvent(const std::string& value)
{
	return (value == "started" ||
			value == "stopped" ||
			value == "iterated" ||
			value == "finished");
}

void Trigger::JSTriggerInit(const std::string& objectName, const std::string& clockName)
{
	if (this->parsedOkay)
	{
		// Output JavaScript initialization
		// animations[0].pathClock.finished.subscribe(function() { animations[0].pathClock.reset(); });
		outFile <<   "\n      " << this->sourceObject <<
			"." << this->sourceClock <<
			"." << this->sourceEvent << ".subscribe(function() { " << 
			objectName << "." <<
			clockName << "." <<
			this->triggeredFunction << "(); });";
	}
}