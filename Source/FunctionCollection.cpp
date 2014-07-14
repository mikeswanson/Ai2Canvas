// FunctionCollection.cpp
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
#include "FunctionCollection.h"

using namespace CanvasExport;

FunctionCollection::FunctionCollection()
{
	// Initialize FunctionCollection
	this->animationIndex = 0;
	this->hasAnimationFunctions = false;
	this->hasDrawFunctions = false;
}

FunctionCollection::~FunctionCollection()
{
	// Clear functions
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Remove instance
		delete functions[i];
	}
}

bool const FunctionCollection::HasAnimationFunctions()
{
	return hasAnimationFunctions;
}

bool const FunctionCollection::HasDrawFunctions()
{
	return hasDrawFunctions;
}

// Returns true if at least one of the draw functions includes animation
bool const FunctionCollection::HasDrawFunctionAnimation()
{
	bool hasAnimation = false;

	// Any draw functions to evaluate?
	if (hasDrawFunctions)
	{
		// Loop through functions
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			// Draw function?
			if (functions[i]->type == Function::kDrawFunction)
			{
				// Any animation?
				if (((DrawFunction*)functions[i])->HasAnimation())
				{
					hasAnimation = true;
					break;
				}
			}
		}
	}

	return hasAnimation;
}

// Initialize all animation clocks
void FunctionCollection::RenderClockInit()
{
	// Any draw function animation?
	if (HasDrawFunctionAnimation())
	{
		outFile << "\n\n      // Initialize animations";

		// Set default draw function animation values
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			// Draw function?
			if (functions[i]->type == Function::kDrawFunction)
			{
				// Render initialization for this function
				functions[i]->RenderClockInit();
			}
		}
	}
	
	// Configure animation clock triggers
	// NOTE: We do this *after* all clocks have been initialized. If we don't, we have ordering problems (e.g. clocks aren't initialized yet)
	if (HasValidTriggers())
	{
		outFile << "\n\n      // Configure animation triggers";

		// Configure function triggers
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			// Render trigger configuration
			functions[i]->RenderTriggerInit();
		}
	}
}

// Start all animation clocks
void FunctionCollection::RenderClockStart()
{
	// Start clocks
	outFile << "\n\n      // Start animation clocks";

	// Start animation clocks (if they have no "start" trigger defined)
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Start clocks
		functions[i]->RenderClockStart();
	}
}

// Tick all animation clocks
void FunctionCollection::RenderClockTick()
{
	// Any draw function animation?
	if (HasDrawFunctionAnimation())
	{
		outFile << "\n\n      // Update animation clocks";
		outFile <<   "\n      updateAllClocks();";

		//for (unsigned int i = 0; i < functions.size(); i++)
		//{
		//	// Draw function?
		//	if (functions[i]->type == Function::kDrawFunction)
		//	{
		//		// Tick animation
		//		functions[i]->RenderClockTick();
		//	}
		//}
	}

	// If we have animation functions, include JavaScript function to update each of them
	if (HasAnimationFunctions())
	{
		outFile << "\n\n      // Update animation paths";
		outFile << "  \n      var animationCount = animations.length;";
		outFile <<   "\n      for (var i = 0; i < animationCount; i++) {";
		outFile <<   "\n        animations[i].update();";
		outFile <<   "\n      }";
	}
}

void FunctionCollection::RenderDrawFunctionCalls(const AIRealRect& documentBounds)
{
	// Loop through all draw functions (they're already in order)
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Draw function?
		if (functions[i]->type == Function::kDrawFunction)
		{
			// Render draw function call
			((DrawFunction*)functions[i])->RenderDrawFunctionCall(documentBounds);
		}
	}
}

void FunctionCollection::RenderDrawFunctions(const AIRealRect& documentBounds)
{
	// Loop through all draw functions
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Draw function?
		if (functions[i]->type == Function::kDrawFunction)
		{
			// Render the function
			((DrawFunction*)functions[i])->RenderDrawFunction(documentBounds);
		}
	}
}

void FunctionCollection::RenderAnimationFunctionInits(const AIRealRect& documentBounds)
{
	// Any animations to render?
	if (HasAnimationFunctions())
	{
		outFile << "\n\n    // Animations";
		outFile <<   "\n    var animations = [";
		
		// Loop through animation functions
		bool addComma = false;
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			// Animation function?
			if (functions[i]->type == Function::kAnimationFunction)
			{
				// Do we need a comma?
				if (addComma)
				{
					outFile << ",";
				}

				// Add animation
				outFile << " new " << functions[i]->name << "()";

				addComma = true;
			}
		}

		// Close array initialization
		outFile << " ];";

		// Loop through animations
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			// Animation function?
			if (functions[i]->type == Function::kAnimationFunction)
			{
				// Initialize
				((AnimationFunction*)functions[i])->RenderInit(documentBounds);
			}
		}
	}
}

// Bind string animation function names to actual objects
void FunctionCollection::BindAnimationFunctions()
{
	// Bind animation functions names (as string) to actual animations
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Draw function?
		if (functions[i]->type == Function::kDrawFunction)
		{
			// Convenience
			DrawFunction* drawFunction = (DrawFunction*)functions[i];

			// Is there an animation name to resolve?
			if (!drawFunction->animationFunctionName.empty())
			{
				// Resolve and assign actual animation function
				drawFunction->animationFunction = (AnimationFunction*)Find(drawFunction->animationFunctionName, Function::kAnimationFunction);
			}
		}
	}
}

// Bind all function triggers
void FunctionCollection::BindTriggers()
{
	// Bind all functions
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		// Bind based on type
		switch (functions[i]->type)
		{
			case Function::kAnimationFunction:
			{
				// Convenience
				AnimationFunction* animationFunction = (AnimationFunction*)functions[i];

				// Resolve triggers
				ResolveTriggers(animationFunction->pathClock.triggers);
				break;
			}
			case Function::kDrawFunction:
			{
				// Convenience
				DrawFunction* drawFunction = (DrawFunction*)functions[i];

				// Resolve triggers
				ResolveTriggers(drawFunction->rotateClock.triggers);
				ResolveTriggers(drawFunction->scaleClock.triggers);
				ResolveTriggers(drawFunction->alphaClock.triggers);
				break;
			}
            case Function::kAnyFunction:
            {
                break;
            }
		}
	}
}

// Pass an array of triggers to be resolved (late-bound)
void FunctionCollection::ResolveTriggers(std::vector<Trigger*>& triggers)
{
	// Loop through triggers
	for (unsigned int i = 0; i < triggers.size(); i++)
	{
		// Convenient reference
		Trigger* trigger = triggers[i];

		// Try to resolve
		Function* function = NULL;
		if (ResolveTriggerFunction(*trigger, &function))
		{
			// Bind
			if (function->type == Function::kDrawFunction)
			{
				// TODO: Should this be the "used" name?
				trigger->sourceObject = function->name;
			}
			else if (function->type == Function::kAnimationFunction)
			{
				stringstream animation;
				animation << "animations[" << ((AnimationFunction*)function)->index << "]";
				trigger->sourceObject = animation.str();
			}
			trigger->parsedOkay = true;
		}
		else
		{
			// Can't bind
			// TODO: Should we just remove it?
			trigger->parsedOkay = false;
		}
	}
}

// Pass in a trigger, resolves function and animation clock
// Returns true if successful, false if not
bool FunctionCollection::ResolveTriggerFunction(Trigger& trigger, Function** function)
{
	// Default return value
	bool result = false;

	// Is this an animation function?
	Function* searchFunction = NULL;
	searchFunction = Find(trigger.sourceObject, Function::kAnimationFunction);

	// Find anything?
	if (searchFunction != NULL)
	{
		// Return function
		*function = searchFunction;

		// Clock name should be empty, since there's only one clock on an animation function
		if (trigger.sourceClock.empty())
		{
			// Path clock
			trigger.sourceClock = "pathClock";

			// Note a successful resolution
			result = true;
		}
	}
	else
	{
		// Is this a draw function?
		bool isLast = false;
		searchFunction = FindDrawFunction(trigger.sourceObject, isLast);

		// Find anything?
		if (searchFunction != NULL)
		{
			// Return function
			*function = searchFunction;

			// Resolve clock names
			if (trigger.sourceClock == "rotate" ||
				trigger.sourceClock == "r")
			{
				// Rotate clock
				trigger.sourceClock = "rotateClock";

				// Note a successful resolution
				result = true;
			}
			else if (trigger.sourceClock == "scale" ||
					 trigger.sourceClock == "s")
			{
				// Scale clock
				trigger.sourceClock = "scaleClock";

				// Note a successful resolution
				result = true;
			}
			else if (trigger.sourceClock == "alpha" ||
					 trigger.sourceClock == "a")
			{
				// Alpha clock
				trigger.sourceClock = "alphaClock";

				// Note a successful resolution
				result = true;
			}
		}
	}

	return result;
}


// Do the functions have any valid clock triggers?
bool const FunctionCollection::HasValidTriggers()
{
	bool result = false;

	for (unsigned int i = 0; i < functions.size(); i++)
	{
		if (functions[i]->HasValidTriggers())
		{
			// Found at least one
			result = true;
			break;
		}
	}

	return result;
}

Function* FunctionCollection::Find(const std::string& name, const Function::FunctionType& functionType)
{
	Function* function = NULL;

	// Any functions to search?
	size_t i = functions.size();
	if (i > 0)
	{
		// Loop through functions
		do
		{
			i--;

			// Correct type?
			if ((functions[i]->type == functionType) ||
				(functions[i]->type == Function::kAnyFunction))
			{
				// Do the names match?
				if (functions[i]->name == name)
				{
					// Found a match
					function = functions[i];
					break;
				}
			}
		} while (i > 0);
	}

	// Return result
	return function;
}

// Returns a unique function name (which could be identical to what was passed in, if it's unique)
std::string FunctionCollection::CreateUniqueName(const std::string& name)
{
	// Since we may change the name, copy it first
	std::string usedName = name;

	// Does a function with this name already exist?
	Function* function = Find(usedName, Function::kAnyFunction);

	// Did we find anything?
	if (function)
	{
		// Find a unique name
		std::ostringstream uniqueName;
		int unique = 0;
		do
		{
			// Increment to make unique name
			unique++;

			// Generate a unique name
			uniqueName.str("");
			uniqueName << usedName << unique;
		} while (Find(uniqueName.str(), Function::kAnyFunction));

		// Assign unique name
		usedName = uniqueName.str();
	}

	// Return the unique name
	return usedName;
}

// Adds a new animation function
DrawFunction* FunctionCollection::AddDrawFunction(const std::string& name)
{
	// Since we may change the name, copy it first
	std::string uniqueName = name;

	// Does a function with this name already exist?
	bool isLast = false;
	DrawFunction* drawFunction = FindDrawFunction(uniqueName, isLast);

	// Did we find the function?
	if (drawFunction)
	{
		// We found the function. Is it the most recent drawing function we've added (don't consider animation layers)?
		// Since drawing order is bottoms-up, we can't "go back to" a prior function and add layers to it...it must be unique at that point
		if (!isLast)
		{
			// It's not the most recent function, so we have to make it unique

			// Need a new function
			drawFunction = NULL;

			// Generate unique name
			uniqueName = CreateUniqueName(uniqueName);
		}
	}
	else
	{
		// Matched another function type (perhaps animation), so we need a unique name
		uniqueName = CreateUniqueName(uniqueName);
	}

	// Do we have a function yet?
	if (!drawFunction)
	{
		// Create a new function
		drawFunction = new DrawFunction();

		// Assign names
		drawFunction->requestedName = name;
		drawFunction->name = uniqueName;

		// Add function
		functions.push_back(drawFunction);

		// Note that we have at least one draw function in the collection
		hasDrawFunctions = true;
	}

	// Return the function
	return drawFunction;
}

// Find a draw function
// TODO: Any way we can clean this up? Seems a bit convoluted.
DrawFunction* FunctionCollection::FindDrawFunction(const std::string& name, bool& isLast)
{
	DrawFunction* drawFunction = NULL;

	// By default
	isLast = false;

	// Track if we've seen a draw function yet
	bool passedDrawFunction = false;

	// Any functions to search?
	size_t i = functions.size();
	if (i > 0)
	{
		// Loop through functions
		do
		{
			i--;

			if (functions[i]->type == Function::kDrawFunction)
			{
				// Cast to DrawFunction
				DrawFunction* searchDrawFunction = (DrawFunction*)functions[i];

				// Do the names match?
				if ((searchDrawFunction->name == name) ||
					(searchDrawFunction->requestedName == name))
				{
					// Found a match
					drawFunction = searchDrawFunction;
					break;
				}

				// Can't be the last, if we've made it here
				passedDrawFunction = true;
			}
		} while (i > 0);

		// Did we find something AND it's the last draw function?
		if (drawFunction && (!passedDrawFunction))
		{
			isLast = true;
		}
	}

	// Return result
	return drawFunction;
}

// Adds a new animation function
AnimationFunction* FunctionCollection::AddAnimationFunction(const std::string& name)
{
	// Since we may change the name, copy it first
	std::string usedName = name;

	// Does a function with this name already exist?
	Function* function = Find(usedName, Function::kAnyFunction);

	// Did we find anything?
	if (function)
	{
		// Find a unique name
		std::ostringstream uniqueName;
		int unique = 0;
		do
		{
			// Increment to make unique name
			unique++;

			// Generate a unique name
			uniqueName.str("");
			uniqueName << unique;
		} while (Find(uniqueName.str(), Function::kAnyFunction));

		// Assign unique name
		usedName = uniqueName.str();
	}

	// Now that we have a unique name, create a new animation function
	AnimationFunction* animationFunction = new AnimationFunction();

	// Assign properties
	animationFunction->name = usedName;
	animationFunction->index = animationIndex;

	// Increment index
	animationIndex++;

	// Add it to the functions array
	functions.push_back(animationFunction);

	// Note that we have at least one animation function in the collection
	hasAnimationFunctions = true;

	// Return new animation function
	return animationFunction;
}

void FunctionCollection::DebugInfo()
{
	// Gather info
	unsigned int drawFunctionCount = 0;
	unsigned int animationFunctionCount = 0;

	// Loop through functions
	for (unsigned int i = 0; i < functions.size(); i++)
	{
		switch (functions[i]->type)
		{
			case Function::kAnimationFunction:
			{
				++animationFunctionCount;
				break;
			}
			case Function::kDrawFunction:
			{
				++drawFunctionCount;
				break;
			}
            case Function::kAnyFunction:
            {
                break;
            }
		}
	}
	
	// Animation function debug info
	outFile <<   "\n<p>Animation functions: " << animationFunctionCount << "</p>";

	// Anything to list?
	if (animationFunctionCount > 0)
	{
		// Start unordered list
		outFile <<   "\n<ul>";

		// Loop through each animation function
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			if (functions[i]->type == Function::kAnimationFunction)
			{
				// For convenience
				AnimationFunction* animationFunction = (AnimationFunction*)functions[i];

				outFile <<   "\n  <li>name: " << animationFunction->name << ", index: " << animationFunction->index <<
							 ", segments: " << animationFunction->beziers.size() <<
							 ", linear segment length: " << setiosflags(ios::fixed) << setprecision(1) << animationFunction->segmentLength << "</li>";
			}
		}

		// End unordered list
		outFile <<   "\n</ul>";
	}

	// Draw function debug info
	outFile <<   "\n<p>Draw functions: " << drawFunctionCount << "</p>";

	// Anything to list?
	if (drawFunctionCount > 0)
	{
		// Start unordered list
		outFile <<   "\n<ul>";

		// Loop through each draw function
		for (unsigned int i = 0; i < functions.size(); i++)
		{
			if (functions[i]->type == Function::kDrawFunction)
			{
				// For convenience
				DrawFunction* drawFunction = (DrawFunction*)functions[i];

				outFile <<   "\n  <li>name: " << drawFunction->name << ", layers: " << drawFunction->layers.size() << "</li>";
			}
		}

		// End unordered list
		outFile <<   "\n</ul>";
	}
}