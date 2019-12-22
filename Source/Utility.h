// Utility.h
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

#ifndef UTILITY_H
#define UTILITY_H

#include "IllustratorSDK.h"
#include "Ai2CanvasSuites.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	bool OpenFile(const std::string& filePath);
	void CloseFile();
	std::string Indent(size_t depth);
	void RenderTransform(const AIRealMatrix& matrix);
	void Replace(std::string& s, char find, char replace);
	void CleanString(std::string& s, AIBoolean camelCase);
	void CleanFunction(std::string& s);
	void CleanParameter(std::string& s);
	void ToLower(std::string& s);
	void MakeValidID(std::string& s);
	vector<string> Tokenize(const std::string& str, const std::string& delimiters);
	bool FileExists(const std::string& fileName);
	void UpdateBounds(const AIRealRect& newBounds, AIRealRect& bounds);
	std::string GetUniqueFileName(const std::string& path, const std::string& fileName, const std::string& extension);
	void WriteArtTree();
	void WriteArtTree(AIArtHandle artHandle, int depth);
}
#endif
