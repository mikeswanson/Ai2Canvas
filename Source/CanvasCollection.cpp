// CanvasCollection.cpp
//
// Copyright (c) 2010-2018 Mike Swanson (http://blog.mikeswanson.com)
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
#include "CanvasCollection.h"

using namespace CanvasExport;

CanvasCollection::CanvasCollection()
{
}

CanvasCollection::~CanvasCollection()
{
	// Clear canvases
	for (unsigned int i = 0; i < canvases.size(); i++)
	{
		// Remove instance
		delete canvases[i];
	}

}

void CanvasCollection::Render()
{
	// Render canvases
	for (unsigned int i = 0; i < canvases.size(); i++)
	{
		// Render
		canvases[i]->Render();
	}
}

// Find a canvas, returns NULL if not found
Canvas* CanvasCollection::Find(const std::string& id)
{
	Canvas* result = NULL;

	// Loop through canvases
	for (unsigned int i = 0; i < canvases.size(); i++)
	{
		// Do the IDs match?
		if (canvases[i]->id == id)
		{
			// Found a match
			result = canvases[i];
			break;
		}
	}

	// Return result
	return result;
}

// Adds a new canvas with a given ID and context name
Canvas* CanvasCollection::Add(const std::string& id, const std::string& contextName, DocumentResources* documentResources)
{
	// Create new canvas with ID
	Canvas* canvas = new Canvas(id, documentResources);

	// Set context name
	canvas->contextName = contextName;

	// Add to array
	canvases.push_back(canvas);

	// Return
	return canvas;
}
