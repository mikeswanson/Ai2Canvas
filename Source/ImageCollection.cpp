// ImageCollection.cpp
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
#include "ImageCollection.h"

using namespace CanvasExport;

ImageCollection::ImageCollection()
{
}

ImageCollection::~ImageCollection()
{
	// Clear images
	for (unsigned int i = 0; i < images.size(); i++)
	{
		// Remove instance
		delete images[i];
	}
}

void ImageCollection::Render()
{
	// Render images
	for (unsigned int i = 0; i < images.size(); i++)
	{
		// Render
		images[i]->Render();
	}
}

// Won't add an image if it already exists
Image* ImageCollection::Add(const std::string& path)
{
	// Does this image already exist?
	Image* image = Find(path);

	// Did we find anything?
	if (!image)
	{
		// Create image ID
		std::ostringstream id;
		id << "image" << (images.size() + 1);

		// Create a new image
		image = new Image(id.str(), path);

		// Add to document
		images.push_back(image);
	}

	// Return result
	return image;
}

// Find an image, returns NULL if not found
Image* ImageCollection::Find(const std::string& path)
{
	Image* result = NULL;

	// Loop through images
	for (unsigned int i = 0; i < images.size(); i++)
	{
		// Do the paths match?
		if (images[i]->path == path)
		{
			// Found a match
			result = images[i];
			break;
		}
	}

	// Return result
	return result;
}

void ImageCollection::DebugInfo()
{
	// Image debug info
	outFile <<   "\n<p>Bitmap images: " << images.size() << "</p>";

	// Anything to list?
	if (images.size() > 0)
	{
		// Start unordered list
		outFile <<   "\n<ul>";

		// Loop through each image
		for (unsigned int i = 0; i < images.size(); i++)
		{
			outFile <<   "\n  <li>ID: " << images[i]->id <<
						 ", path: <a href=\"" << images[i]->Uri() << "\" target=\"_blank\">" << images[i]->path << "</a></li>";
		}

		// End unordered list
		outFile <<   "\n</ul>";
	}
}
