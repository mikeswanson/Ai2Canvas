// AnimationFunction.cpp
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
#include "AnimationFunction.h"

using namespace CanvasExport;

AnimationFunction::AnimationFunction()
{
	// Initialize Function
	this->type = Function::kAnimationFunction;

	// Initialize AnimationFunction
	this->index = 0;
	this->artHandle = NULL;
	this->segmentLength = 0.0f;

	// Initialize path animation clock
	this->pathClock.name = "pathClock";
	this->pathClock.direction = AnimationClock::kForward;
	this->pathClock.rangeExpression = "this.linear.length - 1";
}

AnimationFunction::~AnimationFunction()
{
}

// Render the animation function/object initialization
void AnimationFunction::RenderInit(const AIRealRect& documentBounds)
{
	// Begin function block
	outFile << "\n\n    function " << name << "() {";

	outFile << "\n\n      // Control and anchor points";
	outFile <<   "\n      this.points = [";

	// Re-set matrix based on document
	// TODO: Need to make this more isolated/encapsulated
	sAIRealMath->AIRealMatrixSetIdentity(&canvas->currentState->internalTransform);
	sAIRealMath->AIRealMatrixConcatScale(&canvas->currentState->internalTransform, 1, -1);
	sAIRealMath->AIRealMatrixConcatTranslate(&canvas->currentState->internalTransform, - 1 * documentBounds.left, documentBounds.top);

	// Render animation
	RenderArt(artHandle, 1);

	outFile << "\n                    ];";

	// Add arc-length data
	ArcLength(1);

	// Other values
	outFile << "\n\n      this.lastValue = -1.0;";
	outFile <<   "\n      this.x = 0;";
	outFile <<   "\n      this.y = 0;";
	outFile <<   "\n      this.orientation = 0.0;";

	RenderClockInit();

	outFile << "\n\n      // Update function";
	outFile <<   "\n      this.update = updatePath;";
	//outFile << "\n\n      // Establish orientation";
	//outFile <<   "\n      this.update();";

	// End function block
	outFile << "\n    }";
}

void AnimationFunction::RenderClockInit()
{
	pathClock.JSClockInit("this");
}

void AnimationFunction::RenderTriggerInit()
{
	stringstream animation;
	animation << "animations[" << index << "]";
	pathClock.JSClockTriggerInit(animation.str());
}

void AnimationFunction::RenderClockStart()
{
	stringstream animation;
	animation << "animations[" << index << "]";
	pathClock.JSClockStart(animation.str());
}

void AnimationFunction::RenderClockTick()
{
	// Do nothing
}

// Crawl the art tree, but only look for paths
// The goal is to output just the path segments
// Also, we output in order...unlike rendering other artwork
void AnimationFunction::RenderArt(AIArtHandle artHandle, unsigned int depth)
{
	do
	{
		// Is this art visible?
		AIBoolean isArtVisible = false;
		ai::int32 attr = 0;
		sAIArt->GetArtUserAttr(artHandle, kArtHidden, &attr);
		isArtVisible = !((attr &kArtHidden) == kArtHidden);

		// Only render if art is visible
		if (isArtVisible)
		{
			// Get type
			short type = 0;
			sAIArt->GetArtType(artHandle, &type);

			// Process based on art type
			switch (type)
			{
				case kGroupArt:
				{
					// Render this sub-group
					RenderGroupArt(artHandle, depth);
					break;
				}
				case kCompoundPathArt:
				{
					RenderCompoundPathArt(artHandle, depth);
					break;
				}
				case kPathArt:
				{
					RenderPathArt(artHandle, depth);
					break;
				}
			}
		}

		// Find the next sibling
		sAIArt->GetArtSibling(artHandle, &artHandle);
	}
	while (artHandle != NULL);
}

void AnimationFunction::RenderGroupArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get the first art element in the group
	AIArtHandle childArtHandle = NULL;
	sAIArt->GetArtFirstChild(artHandle, &childArtHandle);

	// Render this sub-group
	RenderArt(childArtHandle, depth + 1);
}

void AnimationFunction::RenderCompoundPathArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get the first art element in the group
	AIArtHandle childArtHandle = NULL;
	sAIArt->GetArtFirstChild(artHandle, &childArtHandle);

	// Render this sub-group
	RenderPathArt(childArtHandle, depth + 1);
}

void AnimationFunction::RenderPathArt(AIArtHandle artHandle, unsigned int depth)
{
	// Skip if this path is a "guide"
	AIBoolean isGuide = false;
	sAIPath->GetPathGuide(artHandle, &isGuide);
	if (!isGuide)
	{
		// Is this art part of a compound path?
		AIBoolean isCompound = false;
		ai::int32 attr = 0;
		sAIArt->GetArtUserAttr(artHandle, kArtPartOfCompound, &attr);
		isCompound = ((attr &kArtPartOfCompound) == kArtPartOfCompound);

		do
		{
			// Write each path as a figure
			RenderPathFigure(artHandle, depth);

			// If this is a compound path, get the next sibling
			if (isCompound)
			{
				sAIArt->GetArtSibling(artHandle, &artHandle);
			}

		}
		while (isCompound && (artHandle != NULL));
	}
}

// Output a single path and its segments (call multiple times for a compound path)
// There is a GetPathBezier command that might make this easier/more straightforward
void AnimationFunction::RenderPathFigure(AIArtHandle artHandle, unsigned int depth)
{
	// Is this a closed path?
	AIBoolean pathClosed = false;
	sAIPath->GetPathClosed(artHandle, &pathClosed);

	// Get the path starting point
	AIPathSegment segment;
	sAIPath->GetPathSegments(artHandle, 0, 1, &segment);

	// Remember the first segment, in case we have to create an extra segment to close the figure
	AIPathSegment firstSegment = segment;

	// Transform starting point
	TransformPoint(segment.p);
	TransformPoint(segment.in);
	TransformPoint(segment.out);

	// Move to the first point
	//fprintf(m_fp, "\n%s%s.moveTo(%.1f, %.1f);", Indent(depth), canvas->contextName.c_str(), x, y);

	// How many segments are in this path?
	short segmentCount = 0;
	sAIPath->GetPathSegmentCount(artHandle, &segmentCount);

	// Track the last out point
	AIPathSegment previousSegment = segment;

	// Loop through each segment
	for (short segmentIndex = 1; segmentIndex < segmentCount; segmentIndex++)
	{
		sAIPath->GetPathSegments(artHandle, segmentIndex, 1, &segment);

		RenderSegment(previousSegment, segment, depth);

		previousSegment = segment;
	}

	// Handle closing segment
	if (pathClosed)
	{
		// Create "phantom" extra segment to accomodate curve
		RenderSegment(previousSegment, firstSegment, depth);
	}
}

void AnimationFunction::RenderSegment(AIPathSegment& previousSegment, AIPathSegment& segment, unsigned int depth)
{
	// Transform points
	TransformPoint(segment.p);
	TransformPoint(segment.in);
	TransformPoint(segment.out);

	AIReal x1 = previousSegment.out.h;
	AIReal y1 = previousSegment.out.v;
	AIReal x2 = segment.in.h;
	AIReal y2 = segment.in.v;

	// Is this a straight line segment?
	AIBoolean isLine = ((previousSegment.p.h == previousSegment.out.h && previousSegment.p.v == previousSegment.out.v) &&
						(segment.p.h == segment.in.h && segment.p.v == segment.in.v));
	if (isLine)
	{
		// Optimize point locations
		x1 = ((segment.p.h - previousSegment.p.h) * 0.33f) + previousSegment.p.h;
		y1 = ((segment.p.v - previousSegment.p.v) * 0.33f) + previousSegment.p.v;
		x2 = ((segment.p.h - previousSegment.p.h) * 0.66f) + previousSegment.p.h;
		y2 = ((segment.p.v - previousSegment.p.v) * 0.66f) + previousSegment.p.v;
	}

	// If this isn't the first segment, include separator
	if (beziers.size() > 0)
	{
		outFile << ",";
	}

	// Output Bezier segment
	outFile << "\n" << Indent(depth) << "              [ " <<
		"[" << setiosflags(ios::fixed) << setprecision(1) << previousSegment.p.h << ", " << previousSegment.p.v << "]" <<
		", [" << x1 << ", " << y1 << "]" <<
		", [" << x2 << ", " << y2 << "]" <<
		", [" << segment.p.h << ", " << segment.p.v << "] ]";

	AIRealPoint p1;
	AIRealPoint p2;
	p1.h = x1;
	p1.v = y1;
	p2.h = x2;
	p2.v = y2;

	const AIReal FLATNESS = 1e-2f; // Adobe recommended value
	AIRealBezier b;
	sAIRealBezier->Set(&b, &previousSegment.p, &p1, &p2, &segment.p);
	AIReal segmentLength = sAIRealBezier->Length(&b, FLATNESS);
	//outFile << "\n" << Indent(depth) << "              // Length = " << setiosflags(ios::fixed) << setprecision(2) << segmentLength;

	// Remember for later
	BezierInfo bi;
	bi.b = b;
	bi.length = segmentLength;
	beziers.push_back(bi);
}

void AnimationFunction::ArcLength(unsigned int depth)
{
	const AIReal FLATNESS = 1e-2f; // Adobe recommended value

	// First, calculate total length of all segments
	AIReal totalLength = 0;
	AIReal shortestLength = 65535.0f;	// Track the shortest length, so we can split up at least each segment once
	for (unsigned int i = 0; i < beziers.size(); i++)
	{
		totalLength += beziers[i].length;

		if (beziers[i].length < shortestLength)
		{
			shortestLength = beziers[i].length;
		}
	}

	outFile << "\n\n      // Linear motion index";
	outFile <<   "\n      this.linear = [";

	// Next, pick equally-spaced points along the total path that are at least shorter than the shortest segment
	unsigned int spacing = ((shortestLength * 0.9f) < 50.0f) ? (unsigned int)(shortestLength * 0.9f) : 50;
	if (spacing < 1) { spacing = 1; }

	// Remember length for debugging
	segmentLength = (float)spacing;

	const unsigned int TOTAL_POINTS = (unsigned int)(totalLength / spacing);
	for (unsigned int i = 0; i < (TOTAL_POINTS + 1.0f); i++)
	{
		// Find out which index contains the correct "length"
		AIReal totalS = (AIReal)i / (AIReal)TOTAL_POINTS;
		AIReal searchLength = totalS * totalLength;
		AIReal remainingSearchLength = searchLength;

		// Find correct segment
		size_t s = 0;
		for (s = 0; s < beziers.size(); s++)
		{
			if (remainingSearchLength <= beziers[s].length)
			{
				// In this segment
				break;
			}

			// Remove this segment's length from search
			remainingSearchLength -= beziers[s].length;
		}

		AIReal t;

		// If math didn't work out perfectly, protect against it
		if (s >= beziers.size())
		{
			s = beziers.size() - 1;
			t = 1.0f;
		}
		else
		{
			// Now that we found the segment, find the t value within the segment
			sAIRealBezier->TAtLength(&beziers[s].b, remainingSearchLength, beziers[s].length, FLATNESS, &t);
			//outFile << "\n" << Indent(depth) << "                // t at length " << setiosflags(ios::fixed) << setprecision(2) << length << " = " << t;
		}

		// Separator
		if (i > 0)
		{
			outFile << ", ";
		}

		// New line every once and awhile
		if (i % 4 == 0)
		{
			outFile << "\n" << Indent(depth) << "              ";
		}

		outFile << "[" << setiosflags(ios::fixed) << setprecision(2) <<
			s << ", " << t << ", " << totalS << "]";
	}

	// End function block
	outFile << "\n                    ];";

	outFile << "\n\n      // Segment T boundaries";
	outFile <<   "\n      this.segmentT = [";

	AIReal runningLength = 0.0f;
	for (unsigned int i = 0; i < beziers.size(); i++)
	{
		// Track running length
		runningLength += beziers[i].length;

		if (i > 0)
		{
			outFile << ", ";
		}
		outFile << setiosflags(ios::fixed) << setprecision(2) << (runningLength / totalLength);
	}

	// End block
	outFile << "];";
}

void AnimationFunction::Bezier(const AIRealBezier& b, AIReal u, AIReal& x, AIReal& y)
{
	x = pow(u, 3) * (b.p3.h + 3 * (b.p1.h - b.p2.h) - b.p0.h) + 3 * pow(u, 2) * (b.p0.h - 2 * b.p1.h + b.p2.h) + 3 * u * (b.p1.h - b.p0.h) + b.p0.h;
	y = pow(u, 3) * (b.p3.v + 3 * (b.p1.v - b.p2.v) - b.p0.v) + 3 * pow(u, 2) * (b.p0.v - 2 * b.p1.v + b.p2.v) + 3 * u * (b.p1.v - b.p0.v) + b.p0.v;
}

void AnimationFunction::TransformPoint(AIRealPoint& point)
{
	sAIRealMath->AIRealMatrixXformPoint(&canvas->currentState->internalTransform, &point, &point);
}

void AnimationFunction::SetParameter(const std::string& parameter, const std::string& value)
{
	// Allow path clock to parse its own parameters
	pathClock.SetParameter(parameter, value);
}

bool const AnimationFunction::HasValidTriggers()
{
	return this->pathClock.HasValidTriggers();
}
