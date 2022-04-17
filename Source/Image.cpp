// Image.cpp
//
// Copyright (c) 2010-2022 Mike Swanson (http://blog.mikeswanson.com)
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
#include "Image.h"

using namespace CanvasExport;

Image::Image(const std::string& id, const std::string& path)
{
	// Initialize Image
	this->id = id;
	this->path = path;
	this->name = "";
	this->pathIsAbsolute = false;
}

Image::~Image()
{
}

void Image::Render()
{
	// Output image tag
	outFile << "\n   <img alt=\"" << name << "\" id=\"" << id << "\" style=\"display: none\" src=\"" << Uri() << "\" />";
}

std::string Image::Uri()
{
	// Create file URI
	ai::UnicodeString usPath(path);
	ai::FilePath aiFilePath(usPath);
	std::string uri = aiFilePath.GetAsURL(false).as_Platform();

	// Firefox doesn't like local "file:" references
	if (!pathIsAbsolute && uri.length() >= 5 && uri.substr(0, 5) == "file:")
	{
		// Strip "file:" from string
		uri = uri.substr(5);
	}

	// Initial slashes?
	if (!pathIsAbsolute && uri.length() >= 3 && uri.substr(0, 3) == "///")
	{
		// Strip "///" from string
		uri = uri.substr(3);
	}

	return uri;
}

void Image::RenderDrawImage(const std::string& contextName, const AIReal x, const AIReal y)
{
	// Draw image
	outFile << "\n" << Indent(0) << contextName << ".drawImage(document.getElementById(\"" << id << "\"), " <<
		setiosflags(ios::fixed) << setprecision(1) <<
		x << ", " << y << ");";
}

void Image::DebugBounds(const std::string& contextName, const AIRealRect& bounds)
{
	if (debug)
	{
		// Stroke bounds	
		outFile << "\n" << Indent(0) << contextName << ".save();";
		outFile << "\n" << Indent(0) << contextName << ".lineWidth = 1.0;";
		outFile << "\n" << Indent(0) << contextName << ".strokeStyle = \"rgb(255, 0, 0)\";";
		outFile << "\n" << Indent(0) << contextName << ".strokeRect(" <<
			setiosflags(ios::fixed) << setprecision(1) <<
			bounds.left << ", " << bounds.top << ", " << (bounds.right - bounds.left) << ", " << (bounds.bottom - bounds.top) << ");";
		outFile << "\n" << Indent(0) << contextName << ".restore();";
	}
}
