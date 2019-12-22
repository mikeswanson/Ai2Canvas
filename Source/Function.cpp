// Function.cpp
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
#include "Function.h"

using namespace CanvasExport;

Function::Function()
{
	// Initialize Function
	this->type = kAnyFunction;
	this->name = "";
	this->canvas = NULL;
	this->translateOrigin = false;
	this->translateOriginH = 0.0f;
	this->translateOriginV = 0.0f;

	// Initialize bounds
	// Start with absolute maximums and minimums (these will be "trimmed")
	this->bounds.left = FLT_MAX;
	this->bounds.right = -FLT_MAX;
	this->bounds.top = -FLT_MAX;
	this->bounds.bottom = FLT_MAX;
}

Function::~Function()
{
}

void SetParameter(const std::string& parameter, const std::string& value)
{
	(void)parameter;
	(void)value;
}

void Function::RenderClockInit() { }
void Function::RenderTriggerInit() { }
void Function::RenderClockStart() { }
void Function::RenderClockTick() { }

bool const Function::HasValidTriggers() { return false; }
