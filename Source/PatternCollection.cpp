// PatternCollection.cpp
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
#include "PatternCollection.h"

using namespace CanvasExport;

PatternCollection::PatternCollection()
{
	// Initialize PatternCollection
	this->hasPatterns = false;
	this->hasSymbols = false;
	this->canvasIndex = 0;
}

PatternCollection::~PatternCollection()
{
	// Clear patterns
	for (unsigned int i = 0; i < patterns.size(); i++)
	{
		// Remove instance
		delete patterns[i];
	}
}

std::vector<CanvasExport::Pattern*>& PatternCollection::Patterns()
{
	return patterns;
}

bool PatternCollection::HasPatterns()
{
	return hasPatterns;
}

bool PatternCollection::HasSymbols()
{
	return hasSymbols;
}

// TODO: Should only add a new pattern if there is no existing pattern
// Returns true if a new pattern was added, or false if it already exists
bool PatternCollection::Add(AIPatternHandle patternHandle, bool isSymbol)
{
	// Track whether or not this pattern exists
	bool patternExists = (Find(patternHandle) != NULL);

	// Does it already exist?
	if (!patternExists)
	{
		// Create a new pattern
		Pattern* pattern = new Pattern();

		// Initialize default pattern values
		pattern->patternHandle = patternHandle;
		pattern->isSymbol = isSymbol;

		// If this is a pattern, track canvas index
		if (!isSymbol)
		{
			canvasIndex++;
			pattern->canvasIndex = canvasIndex;
		}

		// Get pattern name
		// TODO: Do we need to make this unique? Does Illustrator allow duplicates?
		ai::UnicodeString patternName;
		sAIPattern->GetPatternName(pattern->patternHandle, patternName);
		std::string name = patternName.as_Platform();
		CleanString(name, true);
		pattern->name = name;

		// Add to vector
		patterns.push_back(pattern);

		// Track this collection
		this->hasPatterns |= (!isSymbol);
		this->hasSymbols |= isSymbol;
	}

	return !patternExists;
}

// Find a pattern, returns NULL if not found
CanvasExport::Pattern* PatternCollection::Find(AIPatternHandle patternHandle)
{
	Pattern* result = NULL;

	// Loop through patterns
	for (unsigned int i = 0; i < patterns.size(); i++)
	{
		// Do the handles match?
		if (patterns[i]->patternHandle == patternHandle)
		{
			// Found a match
			result = patterns[i];
			break;
		}
	}

	// Return result
	return result;
}



