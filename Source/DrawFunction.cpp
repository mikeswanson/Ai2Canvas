// DrawFunction.cpp
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
#include "DrawFunction.h"

using namespace CanvasExport;

DrawFunction::DrawFunction()
{
	// Initialize Function
	this->type = Function::kDrawFunction;

	// Initialize DrawFunction
	this->requestedName = "";
	this->hasGradients = false;
	this->hasPatterns = false;
	this->hasAlpha = false;
	this->animationFunction = nullptr;
	this->animationFunctionName = "";
	this->follow = false;
	this->followOrientation = 0.0f;
	this->rasterizeFileName = "";
	this->crop = false;

	// Initialize rotate animation clock
	this->rotateClock.name = "rotateClock";
	this->rotateClock.rangeExpression = "2.0 * Math.PI";

	// Initialize scale animation clock
	this->scaleClock.name = "scaleClock";
	this->scaleClock.rangeExpression = "1.0";

	// Initialize alpha animation clock
	this->alphaClock.name = "alphaClock";
	this->alphaClock.rangeExpression = "1.0";
}

DrawFunction::~DrawFunction()
{
}

bool const DrawFunction::HasValidTriggers()
{
	return (this->rotateClock.HasValidTriggers() ||
			this->scaleClock.HasValidTriggers() ||
			this->alphaClock.HasValidTriggers());
}

void DrawFunction::RenderClockInit()
{
	// Initialize rotation clock
	rotateClock.JSClockInit(name);

	// Initialize scale clock

	// Firefox 3.x won't allow a 0 scale value, so protect for that here by adding a very small offset
	if (scaleClock.offset <= 0)
	{
		// Set to very small offset (note that Number.MIN_VALUE will not work)
		scaleClock.offset = 0.0001f;
	}
	scaleClock.JSClockInit(name);

	// Initialize alpha clock
	alphaClock.JSClockInit(name);

	// Do we have a follow orientation?
	if (follow)
	{
		// Output follow orientation
		outFile <<   "\n      " << name << ".followOrientation = " << setiosflags(ios::fixed) << setprecision(2) <<
			followOrientation << " * Math.PI / 180.0;";
	}
}

void DrawFunction::RenderTriggerInit()
{
	// Configure rotation clock triggers
	rotateClock.JSClockTriggerInit(name);

	// Configure scale clock triggers
	scaleClock.JSClockTriggerInit(name);

	// Configure alpha clock triggers
	alphaClock.JSClockTriggerInit(name);
}

// Outputs correct JavaScript to start a clock immediately (if there is no "start" trigger defined)
void DrawFunction::RenderClockStart()
{
	// Start rotation clock
	rotateClock.JSClockStart(name);

	// Start scale clock
	scaleClock.JSClockStart(name);

	// Start alpha clock
	alphaClock.JSClockStart(name);
}

void DrawFunction::RenderClockTick()
{
	// Tick rotation
	rotateClock.JSClockTick(name);

	// Tick scale
	scaleClock.JSClockTick(name);

	// Tick alpha
	alphaClock.JSClockTick(name);
}

void DrawFunction::RenderDrawFunctionCall(const AIRealRect& documentBounds)
{
	// New line
	outFile << "\n";

	// Does this draw function have any animation?
	if (animationFunction ||
		rotateClock.direction != AnimationClock::kNone ||
		scaleClock.direction != AnimationClock::kNone ||
		alphaClock.direction != AnimationClock::kNone)
	{
		// Save drawing context state
		outFile << "\n      " << canvas->contextName << ".save();";

		// Do we have an animation path?
		if (animationFunction)
		{
			// Set transform for this animation path
			outFile <<   "\n      " << canvas->contextName << ".translate(animations[" << 
				animationFunction->index << "].x, animations[" << 
				animationFunction->index << "].y);";

			// Does this draw function have a follow orientation?
			if (follow)
			{
				outFile <<   "\n      " << canvas->contextName << ".rotate(" << name <<
							".followOrientation + animations[" << animationFunction->index <<
							"].orientation);";
			}
		}
		else
		{
			// Do we need to reposition?
			if (translateOrigin)
			{
				Reposition(documentBounds);
			}
		}

		// Do we have any rotation on the draw function?
		if (rotateClock.direction != AnimationClock::kNone)
		{
			outFile <<   "\n      " << canvas->contextName << ".rotate(" << name << "." << rotateClock.name << ".value);";
		}

		// Do we have any scale on the draw function?
		if (scaleClock.direction != AnimationClock::kNone)
		{
			outFile <<   "\n      " << canvas->contextName << ".scale(" << name << "." << scaleClock.name << ".value, " <<
				name << "." << scaleClock.name << ".value);";
		}

		// Do we have any alpha on the draw function?
		if (alphaClock.direction != AnimationClock::kNone)
		{
			outFile <<   "\n      " << canvas->contextName << ".globalAlpha = " << name << "." << alphaClock.name << ".value;";
		}

		// Call the draw function
		outFile <<   "\n      " << name << "(" << canvas->contextName << ");";

		// Show origin
		if (debug)
		{
			outFile <<   "\n      " << canvas->contextName << ".save();";
			outFile <<   "\n      " << canvas->contextName << ".fillStyle = \"rgb(0, 0, 255)\";";
			outFile <<   "\n      " << canvas->contextName << ".fillRect(-2.0, -2.0, 5, 5);";
			outFile <<   "\n      " << canvas->contextName << ".restore();";
		}

		// Restore drawing context state
		outFile <<   "\n      " << canvas->contextName << ".restore();";
	}
	else
	{
		// No animation

		// Do we need to reposition?
		if (translateOrigin)
		{
			// Save drawing context state
			outFile << "\n      " << canvas->contextName << ".save();";

			Reposition(documentBounds);
		}

		// Just call the function
		outFile <<   "\n      " << name << "(" << canvas->contextName << ");";

		// Show origin
		if (debug)
		{
			outFile <<   "\n      " << canvas->contextName << ".save();";
			outFile <<   "\n      " << canvas->contextName << ".fillStyle = \"rgb(0, 0, 255)\";";
			outFile <<   "\n      " << canvas->contextName << ".fillRect(-2.0, -2.0, 5, 5);";
			outFile <<   "\n      " << canvas->contextName << ".restore();";
		}

		// Do we need to restore?
		if (translateOrigin)
		{
			// Restore drawing context state
			outFile <<   "\n      " << canvas->contextName << ".restore();";
		}
	}
}

// Render a drawing function
void DrawFunction::RenderDrawFunction(const AIRealRect& documentBounds)
{
	// Begin function block
	outFile << "\n\n    function " << name << "(ctx) {";

	// Need a blank line?
	if (hasAlpha || hasGradients || hasPatterns)
	{
		outFile << "\n";
	}

	// Does this draw function have alpha changes?
	if (hasAlpha)
	{
		// Grab the alpha value (so we can use it to compute new globalAlpha values during this draw function)
		outFile << "\n" << Indent(0) << "var alpha = ctx.globalAlpha;";
	}

	// Will we be encountering gradients?
	if (hasGradients)
	{
		outFile << "\n" << Indent(0) << "var gradient;";
	}

	// Will we be encountering patterns?
	if (hasPatterns)
	{
		outFile << "\n" << Indent(0) << "var pattern;";
	}

	/// Re-set matrix based on document
	sAIRealMath->AIRealMatrixSetIdentity(&canvas->currentState->internalTransform);
	sAIRealMath->AIRealMatrixConcatScale(&canvas->currentState->internalTransform, 1, -1);
	sAIRealMath->AIRealMatrixConcatTranslate(&canvas->currentState->internalTransform, - 1 * documentBounds.left, documentBounds.top);

	// Do we need to move the origin?
	if (translateOrigin)
	{
		// Calculate offsets to move function/layer to 0, 0 of document
		AIReal offsetH = bounds.left - documentBounds.left;
		AIReal offsetV = bounds.top - documentBounds.top;

		// Calculate requested offsets based on percentages
		AIReal translateH = (bounds.right - bounds.left) * translateOriginH;
		AIReal translateV = (bounds.top - bounds.bottom) * translateOriginV;

		// Modify transformation matrix for this function (and set of layers)
		sAIRealMath->AIRealMatrixConcatTranslate(&canvas->currentState->internalTransform, (-1 * offsetH) - translateH, offsetV - translateV);
	}

	// Are we supposed to rasterize this function?
	if (!rasterizeFileName.empty())
	{
		// Rasterize the first layer
		// TODO: Note that this only rasterizes the first associated layer. What if this has multiple layers?

		// Output layer name
		outFile << "\n\n" << Indent(1) << "// " << name;

		canvas->RenderUnsupportedArt(layers[0]->artHandle, rasterizeFileName, 1);
	}
	else
	{
		// Render each layer in the function block (they're already in the correct order)
		for (unsigned int i = 0; i < layers.size(); i++)
		{
			// Render the art
			canvas->RenderArt(layers[i]->artHandle, 1);
	
			// Restore remaining state
			canvas->SetContextDrawingState(1);
		}
	}

	// End function block
	outFile << "\n    }";
}

// Output repositioning translation for a draw function
void DrawFunction::Reposition(const AIRealRect& documentBounds)
{
	// Calculate offsets to move function/layer to 0, 0 of document
	AIReal offsetH = bounds.left - documentBounds.left;
	AIReal offsetV = bounds.top - documentBounds.top;

	// Calculate requested offsets based on percentages
	AIReal translateH = (bounds.right - bounds.left) * this->translateOriginH;
	AIReal translateV = (bounds.top - bounds.bottom) * this->translateOriginV;

	// Re-positioning translation
	AIReal x = offsetH + translateH;
	AIReal y = (-1 * offsetV) + translateV;

	// Render the repositioning translation for this function
	// NOTE: This needs to happen, even if it's just "identity," since other functions may have already changed the transformation
	outFile <<   "\n      " << canvas->contextName << ".translate(" << setiosflags(ios::fixed) << setprecision(1) << x << ", " << y << ");";
}

void DrawFunction::SetParameter(const std::string& parameter, const std::string& value)
{
	// Origin
	if (parameter == "origin" ||
		parameter == "o")
	{
		if (debug)
		{
			outFile << "\n//     Found origin parameter";
		}

		// Short-cut values?
		if (value == "normal" ||
			value == "n")
		{
			// Normal (do nothing special)
			this->translateOrigin = false;
		}
		else if (value == "center" ||
				 value == "c")
		{
			// Center
			this->translateOrigin = true;
			this->translateOriginH = 0.5f;
			this->translateOriginV = 0.5f;
		}
		else if (value == "upper-left" ||
				 value == "ul")
		{
			// Upper left
			this->translateOrigin = true;
			this->translateOriginH = 0.0f;
			this->translateOriginV = 0.0f;
		}
		else if (value == "upper-right" ||
				 value == "ur")
		{
			// Upper right
			this->translateOrigin = true;
			this->translateOriginH = 1.0f;
			this->translateOriginV = 0.0f;
		}
		else if (value == "lower-right" ||
				 value == "lr")
		{
			// Lower right
			this->translateOrigin = true;
			this->translateOriginH = 1.0f;
			this->translateOriginV = 1.0f;
		}
		else if (value == "lower-left" ||
				 value == "ll")
		{
			// Lower left
			this->translateOrigin = true;
			this->translateOriginH = 0.0f;
			this->translateOriginV = 1.0f;
		}
		else
		{
			// Can we parse two float values?
			std::vector<std::string> originOffsets = Tokenize(value, ",");

			// Did we find two values?
			if (originOffsets.size() == 2)
			{
				// Indicate that we want to translate
				this->translateOrigin = true;

				// Set translation values
				this->translateOriginH = (AIReal)strtod(originOffsets[0].c_str(), NULL);
				this->translateOriginV = (AIReal)strtod(originOffsets[1].c_str(), NULL);

				if (debug)
				{
					outFile << "\n//     translateH = " << setiosflags(ios::fixed) << setprecision(1) <<
						this->translateOriginH << ", translateV = " << this->translateOriginV;
				}
			}
		}
	}

	// Animation path association
	if (parameter == "animation" ||
		parameter == "a")
	{
		if (debug)
		{
			outFile << "\n//     Found animation parameter";
		}

		// Get the associated animation function name
		std::string cleanAnimationFunctionName = value;

		// Clean the name for a function
		CleanString(cleanAnimationFunctionName, true);

		if (debug)
		{
			outFile << "\n//     Animation function name = " << cleanAnimationFunctionName;
		}

		// Store the function name as a string that we will bind after all layers have been defined
		this->animationFunctionName = cleanAnimationFunctionName;
	}

	// Rotation related?
	if (parameter.substr(0, 7) == "rotate-" ||
		parameter.substr(0, 2) == "r-")
	{
		if (debug)
		{
			outFile << "\n//     Found rotation parameters";
		}

		// Replace with clock parameter
		std::string clockParameter = parameter.substr((parameter.find_first_of('-') + 1));

		// Replace direction convenience values with clock values
		std::string clockValue = value;
		if (clockValue == "clockwise" ||
			clockValue == "cw")
		{
			// Same as forward
			clockValue = "forward";
		}
		else if (clockValue == "counterclockwise" ||
				 clockValue == "ccw")
		{
			// Same as backward
			clockValue = "backward";
		}

		// Set the clock parameter
		rotateClock.SetParameter(clockParameter, clockValue);
	}

	// Scale related?
	if (parameter.substr(0, 6) == "scale-" ||
		parameter.substr(0, 2) == "s-")
	{
		if (debug)
		{
			outFile << "\n//     Found scale parameters";
		}

		// Replace with clock parameter
		std::string clockParameter = parameter.substr((parameter.find_first_of('-') + 1));

		// Replace direction convenience values with clock values
		std::string clockValue = value;
		if (clockValue == "grow" ||
			clockValue == "g")
		{
			// Same as forward
			clockValue = "forward";
		}
		else if (clockValue == "shrink" ||
				 clockValue == "s")
		{
			// Same as backward
			clockValue = "backward";
		}

		// Set the clock parameter
		scaleClock.SetParameter(clockParameter, clockValue);
	}

	// Alpha related?
	if (parameter.substr(0, 6) == "alpha-" ||
		parameter.substr(0, 2) == "a-")
	{
		if (debug)
		{
			outFile << "\n//     Found alpha parameters";
		}

		// Replace with clock parameter
		std::string clockParameter = parameter.substr((parameter.find_first_of('-') + 1));

		// Replace direction convenience values with clock values
		std::string clockValue = value;
		if (clockValue == "fade-in" ||
			clockValue == "f-i")
		{
			// Same as forward
			clockValue = "forward";
		}
		else if (clockValue == "fade-out" ||
				 clockValue == "f-o")
		{
			// Same as backward
			clockValue = "backward";
		}

		// Set the clock parameter
		alphaClock.SetParameter(clockParameter, clockValue);
	}

	// Follow orientation
	if (parameter == "follow-orientation" ||
		parameter == "f-o")
	{
		if (debug)
		{
			outFile << "\n//     Found follow orientation parameter";
		}

		// Is this disabling following?
		if (value == "none" ||
			value == "n")
		{
			// Disable follow
			this->follow = false;
		}
		else
		{
			// Enable following
			this->follow = true;

			// Parse the follow orientation "offset" in degrees
			this->followOrientation = (AIReal)(strtod(value.c_str(), NULL));

			if (debug)
			{
				outFile << "\n//     Follow orientation = " << setiosflags(ios::fixed) << setprecision(2) <<
					this->followOrientation << " degrees";
			}
		}
	}

	// Rasterize
	if (parameter == "rasterize" ||
		parameter == "rast")
	{
		if (debug)
		{
			outFile << "\n//     Found rasterize parameter";
		}

		if (value == "no" ||
			value == "n")
		{
			// No rasterization
			this->rasterizeFileName = "";
		}
		else
		{
			// If we have a value
			if (!value.empty())
			{
				std::string fileName = value;

				// Does the file name already have an extension?
				size_t index = value.find_last_of('.');
				if (index != string::npos)
				{
					// Remove the extension
					fileName = name.substr(0, index);
				}

				// Since we'll rasterize to a PNG file, add a ".png" extension
				fileName += ".png";

				// Store file name
				this->rasterizeFileName = fileName;

				if (debug)
				{
					outFile << "\n//     Rasterize file name = " << fileName;
				}
			}
		}
	}

	// Crop
	if (parameter == "crop" ||
		parameter == "c")
	{
		if (debug)
		{
			outFile << "\n//     Found crop parameter";
		}

		if (value == "yes" ||
			value == "y")
		{
			// Crop
			this->crop = true;

			// Set first layer to crop
			// TODO: This only works on the first layer
			this->layers[0]->crop = true;
		}
		else if (value == "no" ||
			value == "n")
		{
			// Don't crop
			this->crop = false;

			// Set first layer to NOT crop
			// TODO: This only works on the first layer
			this->layers[0]->crop = false;
		}
	}
}

bool const DrawFunction::HasAnimation()
{
	return	(this->follow ||
			(this->rotateClock.direction != AnimationClock::kNone) ||
			(this->scaleClock.direction != AnimationClock::kNone) ||
			(this->alphaClock.direction != AnimationClock::kNone));
}
