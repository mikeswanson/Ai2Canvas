// FunctionCollection.h
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

#ifndef FUNCTIONCOLLECTION_H
#define FUNCTIONCOLLECTION_H

#include "IllustratorSDK.h"
#include "Function.h"
#include "AnimationFunction.h"
#include "DrawFunction.h"
#include "Utility.h"

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	/// Represents a collection of functions
	class FunctionCollection
	{
	private:

		unsigned int				animationIndex;			// Track JavaScript array index for animation functions
		bool						hasAnimationFunctions;	// Does the collection include at least one animation function?
		bool						hasDrawFunctions;		// Does the collection include at least one draw function?

	public:

		FunctionCollection();
		~FunctionCollection();

		std::vector<Function*>		functions;				// Collection of functions

		bool const					HasAnimationFunctions();
		bool const					HasDrawFunctions();
		bool const					HasDrawFunctionAnimation();
		Function*					Find(const std::string& name, const Function::FunctionType& functionType);
		std::string					CreateUniqueName(const std::string& name);
		DrawFunction*				AddDrawFunction(const std::string& name);
		DrawFunction*				FindDrawFunction(const std::string& name, bool& isLast);
		AnimationFunction*			AddAnimationFunction(const std::string& name);
		bool const					HasValidTriggers();

		void						RenderClockInit();
		void						RenderClockStart();
		void						RenderClockTick();
		void						RenderDrawFunctionCalls(const AIRealRect& documentBounds);
		void						RenderDrawFunctions(const AIRealRect& documentBounds);
		void						RenderAnimationFunctionInits(const AIRealRect& documentBounds);

		void						BindAnimationFunctions();
		void						BindTriggers();
		void						ResolveTriggers(std::vector<Trigger*>& triggers);
		bool						ResolveTriggerFunction(Trigger& trigger, Function** function);
		void						DebugInfo();

	};
}

#endif
