// State.cpp
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
#include "State.h"

using namespace CanvasExport;

State::State()
{
	// Default color string
	const std::string defaultColor("\"rgb(0, 0, 0)\"");

	// Initialize State
	// NOTE: These are the HTML5 canvas defaults: http://www.whatwg.org/specs/web-apps/current-work/multipage/the-canvas-element.html#2dcontext
	this->globalAlpha = 1.0f;
	this->fillStyle = defaultColor;
	this->strokeStyle = defaultColor;
	this->lineWidth = 1.0f;
	this->lineCap = kAIButtCap;
	this->lineJoin = kAIMiterJoin;
	this->miterLimit = 10.0f;
	this->fontSize = 10.0f;
	this->fontName = "sans-serif";
	this->fontStyleName = "Regular";
	this->isProcessingSymbol = false;
	sAIRealMath->AIRealMatrixSetIdentity(&this->internalTransform);
}

State::~State()
{
}

// Report state information
void State::DebugInfo()
{
	outFile << "\n\n// State Info";
	outFile << "\n//   globalAlpha = " << setiosflags(ios::fixed) << setprecision(2) << this->globalAlpha;
	outFile << "\n//   fillStyle = " << this->fillStyle;
	outFile << "\n//   strokeStyle = " << this->strokeStyle;
	outFile << "\n//   lineWidth = " << setiosflags(ios::fixed) << setprecision(1) << this->lineWidth;
	outFile << "\n//   lineCap = " << this->lineCap;
	outFile << "\n//   lineJoin = " << this->lineJoin;
	outFile << "\n//   miterLimit = " << setiosflags(ios::fixed) << setprecision(1) << this->miterLimit;
	outFile << "\n//   fontSize = " << setiosflags(ios::fixed) << setprecision(1) << this->fontSize;
	outFile << "\n//   fontName = " << this->fontName;
	outFile << "\n//   fontStyleName = " << this->fontStyleName;
	outFile << "\n//   isProcessingSymbol = " << this->isProcessingSymbol;
	outFile << "\n//   internalTransform = ";
	//RenderTransform(state.internalTransform);
}
