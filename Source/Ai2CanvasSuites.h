// Ai2CanvasSuites.h
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

#ifndef __AI2CANVASSUITES_H__
#define __AI2CANVASSUITES_H__

#include "IllustratorSDK.h"
#include "Suites.hpp"
#include "ATETextSuitesImportHelper.h"

// Added for Ai2Canvas functionality
#include "AIColorConversion.h"
#include "AIATEPaint.h"
#include "AIATETextUtil.h"
#include "AIImageOptimization.h"
#include "AISymbol.h"
#include "AIArtStyleParser.h"
#include "AIPattern.h"
#include "AIPathStyle.h"
#include "AIGradient.h"

extern	"C"	AIUnicodeStringSuite*	sAIUnicodeString;
extern  "C" SPBlocksSuite*			sSPBlocks;
extern	"C" AIFileFormatSuite*		sAIFileFormat;
extern	"C" AIDocumentSuite*		sAIDocument;
extern	"C" AITextFrameSuite*		sAITextFrame;
extern	"C" AIArtSuite*				sAIArt;
extern	"C" AIPathSuite*			sAIPath;
extern	"C" AIMatchingArtSuite*		sAIMatchingArt;
extern	"C" AIMdMemorySuite*		sAIMdMemory;

// Added for Ai2Canvas functionality
extern "C" AIATEPaintSuite *sATEPaint;
extern "C" AIFontSuite *sAIFont;
extern "C" AIATETextUtilSuite *sATETextUtil;
extern "C" AIDataFilterSuite *sAIDataFilter;
extern "C" AISymbolSuite *sAISymbol;
extern "C" AIPatternSuite *sAIPattern;
extern "C" AIPlacedSuite *sAIPlaced;
extern "C" AIRasterSuite *sAIRaster;
extern "C" AIImageOptSuite *sAIImageOpt;
extern "C" AIArtStyleSuite *sAIArtStyle;
extern "C" AIArtStyleParserSuite *sAIArtStyleParser;
extern "C" AILiveEffectSuite *sAILiveEffect;
extern "C" AIDictionarySuite *sAIDictionary;
extern "C" AIDictionaryIteratorSuite *sAIDictionaryIterator;
extern "C" AIEntrySuite *sAIEntry;
extern "C" AIPathStyleSuite *sAIPathStyle;
extern "C" AIHardSoftSuite *sAIHardSoft;
extern "C" AIRealMathSuite *sAIRealMath;
extern "C" AIGradientSuite *sAIGradient;
extern "C" AIMaskSuite *sAIMask;
extern "C" AIPluginGroupSuite *sAIPluginGroup;
extern "C" AICustomColorSuite *sAICustomColor;
extern "C" AIColorConversionSuite *sAIColorConversion;
extern "C" AIBlendStyleSuite *sAIBlendStyle;
extern "C" AILayerSuite *sAILayer;
extern "C" AIRealBezierSuite *sAIRealBezier;

#endif // End Ai2CanvasSuites.h
