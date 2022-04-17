// Canvas.h
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

#ifndef CANVAS_H
#define CANVAS_H

#include "IllustratorSDK.h"
#include "Ai2CanvasSuites.h"
#include "State.h"
#include "Utility.h"
#include <sstream>
#include <stdint.h>
#include "DocumentResources.h"

// Accommodate color component type based on SDK version
#if kPluginInterfaceVersion > kPluginInterfaceVersion16001
	typedef AIFloatSampleComponent SampleComponent;
#else
	typedef AISampleComponent SampleComponent;
#endif

namespace CanvasExport
{
	// Globals
	extern ofstream outFile;
	extern bool debug;

	// Drop shadow parameters
	// NOTE: Should this move to State? That's where it is in the canvas spec.
	struct DropShadow
	{
		AIReal			horz;						// Horizontal offset
		AIReal			vert;						// Vertical offset
		AIReal			blur;						// Blur amount
		AIFillStyle		shadowStyle;				// Fill style
		AIReal			opac;						// Opacity
	};

	// Handy structure to maintain glyph state
	// TODO: Evaluate a better (cleaner) way to do this
	struct GlyphState
	{
		AIReal			fontSize;
		AIReal			verticalScale;
		AIReal			horizontalScale;
		AIRealMatrix	glyphMatrix;
		AIBoolean		textFilled;
		AIBoolean		textStroked;
		AIColor			fillColor;
		std::string		fillStyle;
		std::string		strokeStyle;
		std::string		fontName;
		std::string		fontStyleName;
		AIStrokeStyle	strokeStyleValue;
	};

	/// Represents a HTML5 canvas element
	class Canvas
	{
	private:

	public:

		DocumentResources*					documentResources;		// Document resources
		std::string							id;						// Canvas element ID
		AIReal								width;					// Width
		AIReal								height;					// Height
		AIBoolean							isHidden;				// Is this canvas hidden (i.e. for patterns)?
		std::string							contextName;			// Name of the drawing context
		State*								currentState;			// Pointer to the current drawing state
		std::vector<State>					states;					// Stack of drawing states
		AIPathStyle							pathfinderStyle;		// Style for PathFinder artwork
		AIBoolean							usePathfinderStyle;		// Track special kPluginArt/Pathfinder style (seems "hacky")
		std::vector<std::string>			breadcrumbs;			// Path to the artwork

		Canvas(const std::string& id, DocumentResources* documentResources);
		~Canvas();

		void				PushState();
		void				PopState();
		void				DebugInfo();

		void				AddBreadcrumb(const std::string& artName, unsigned int depth);
		void				RemoveBreadcrumb();

		void				Render();
		void				RenderImages();

		void				RenderArt(AIArtHandle artHandle, unsigned int depth);
		void				ParseArtStyle(AIArtHandle artHandle, unsigned int depth, ASInt32& postEffectCount, 
										  AIBlendingMode& blendingMode, AIBoolean& hasDropShadow, DropShadow& dropShadow);
		void				SetContextDrawingState(unsigned int depth);
		void				RenderDropShadow(const DropShadow& dropShadow, unsigned int depth);
		void				RenderUnsupportedArt(AIArtHandle artHandle, const std::string& fileName, unsigned int depth);
		void				RasterizeArtToPNG(AIArtHandle artHandle, const std::string& path);
		void				GetPNGDimensions(const std::string& path, unsigned int& width, unsigned int& height);
		uint32_t			ReverseInt(uint32_t i);
		AIReal				GetJPGDPI(const std::string& path);
		uint16_t			ReverseInt(uint16_t i);
		void				ReportRasterRecordInfo(const AIRasterRecord& rasterRecord);
		void				ReportColorSpaceInfo(ai::int16 colorSpace);
		void				RenderGroupArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderPluginArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderSymbolArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderCompoundPathArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderPathArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderPathFigure(AIArtHandle artHandle, unsigned int depth);
		void				RenderSegment(AIPathSegment& previousSegment, AIPathSegment& segment, unsigned int depth);
		void				RenderPathStyle(const AIPathStyle& style, unsigned int depth);
		void				RenderPlacedArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderRasterArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderMidPointColor(const AIColor& color1, AIReal alpha1, const AIColor& color2, AIReal alpha2);
		void				RenderGradient(const AIGradientStyle& gradientStyle, unsigned int depth);
		void				RenderGradientStops(const AIGradientStyle& gradientStyle, unsigned int depth);
		void				RenderFillInfo(const AIColor& fillColor, unsigned int depth);
		void				GetFillStyle(const AIColor& color, AIReal alpha, std::string& fillStyle);
		void				ReportPatternStyleInfo(const AIPatternStyle& patternStyle);
		void				RenderStrokeInfo(const AIStrokeStyle& strokeStyle, unsigned int depth);
		void				RenderTextFrameArt(AIArtHandle artHandle, unsigned int depth);
		void				RenderGlyphRuns(AIArtHandle textFrameArt, unsigned int depth);
		void				RenderGlyphRun(char *contents, const GlyphState& glyphState, unsigned int depth);
		AIBoolean			GlyphStatesMatch(const GlyphState& state1, const GlyphState& state2);
		void				GetGlyphState(const ATE::IGlyphRun& glyphRun, GlyphState& glyphState, const AIRealMatrix& textFrameMatrix, unsigned int depth);
		void				ReportGlyphRunInfo(const ATE::IGlyphRun& glyphRun);
		void				ReportCharacterFeatures(const ATE::ICharFeatures& features);
		std::string			GetColor(const AIColor& color, AIReal alpha);
		void				ConvertColorToRGB(const AIColor& sourceColor, AIColor& rbgColor);
		void				TransformRect(AIRealRect& rect);
		void				TransformPoint(AIRealPoint& point);
		void				TransformPointWithMatrix(AIRealPoint& point, const AIRealMatrix& matrix);
	};
}

#endif
