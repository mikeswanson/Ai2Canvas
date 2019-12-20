// Ai2CanvasSuites.cpp
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
#include "Ai2CanvasSuites.h"

extern "C"
{
	AIUnicodeStringSuite*	sAIUnicodeString = NULL;
	SPBlocksSuite*			sSPBlocks = NULL;
	AIFileFormatSuite*		sAIFileFormat = NULL;
	AIDocumentSuite*		sAIDocument = NULL;
	AITextFrameSuite*		sAITextFrame = NULL;
	AIArtSuite*				sAIArt = NULL;
	AIPathSuite*			sAIPath = NULL;
	AIMatchingArtSuite*		sAIMatchingArt = NULL;
	AIMdMemorySuite*		sAIMdMemory = NULL;
	EXTERN_TEXT_SUITES

	// Added for Ai2Canvas functionality
	AIPathStyleSuite *sAIPathStyle = NULL;
	AIHardSoftSuite *sAIHardSoft = NULL;
	AIRealMathSuite *sAIRealMath = NULL;
	AIGradientSuite *sAIGradient = NULL;
	AIMaskSuite *sAIMask = NULL;
	AIPluginGroupSuite *sAIPluginGroup = NULL;
	AICustomColorSuite *sAICustomColor = NULL;
	AIColorConversionSuite *sAIColorConversion = NULL;
	AIBlendStyleSuite *sAIBlendStyle = NULL;
	AILayerSuite *sAILayer = NULL;
	AIATEPaintSuite *sATEPaint = NULL;
	AIFontSuite *sAIFont = NULL;
	AIATETextUtilSuite *sATETextUtil = NULL;
	AIDataFilterSuite *sAIDataFilter = NULL;
	AISymbolSuite *sAISymbol = NULL;
	AIPatternSuite *sAIPattern = NULL;
	AIPlacedSuite *sAIPlaced = NULL;
	AIRasterSuite *sAIRaster = NULL;
	AIImageOptSuite *sAIImageOpt = NULL;
	AIArtStyleSuite *sAIArtStyle = NULL;
	AIArtStyleParserSuite *sAIArtStyleParser = NULL;
	AILiveEffectSuite *sAILiveEffect = NULL;
	AIDictionarySuite *sAIDictionary = NULL;
	AIDictionaryIteratorSuite *sAIDictionaryIterator = NULL;
	AIEntrySuite *sAIEntry = NULL;
	AIRealBezierSuite *sAIRealBezier = NULL;
};

ImportSuite gImportSuites[] = 
{
	kAIUnicodeStringSuite, kAIUnicodeStringSuiteVersion, &sAIUnicodeString,
	kSPBlocksSuite, kSPBlocksSuiteVersion, &sSPBlocks,
	kAIFileFormatSuite, kAIFileFormatVersion, &sAIFileFormat,
	kAIDocumentSuite, kAIDocumentVersion, &sAIDocument,
	kAITextFrameSuite, kAITextFrameVersion, &sAITextFrame,
	kAIArtSuite, kAIArtSuiteVersion, &sAIArt,
	kAIPathSuite, kAIPathVersion, &sAIPath,
	kAIMatchingArtSuite, kAIMatchingArtVersion, &sAIMatchingArt,
	kAIMdMemorySuite, kAIMdMemorySuiteVersion, &sAIMdMemory,

	// Added for Ai2Canvas functionality
	kAIPathStyleSuite, kAIPathStyleVersion, &sAIPathStyle,
	kAIHardSoftSuite, kAIHardSoftVersion, &sAIHardSoft,
	kAIRealMathSuite, kAIRealMathVersion, &sAIRealMath,
	kAIGradientSuite, kAIGradientVersion, &sAIGradient,
	kAIMaskSuite, kAIMaskVersion, &sAIMask,
	kAIPluginGroupSuite, kAIPluginGroupVersion, &sAIPluginGroup,
	kAICustomColorSuite, kAICustomColorVersion, &sAICustomColor,
	kAIColorConversionSuite, kAIColorConversionVersion, &sAIColorConversion,
	kAIBlendStyleSuite, kAIBlendStyleVersion, &sAIBlendStyle,
	kAILayerSuite, kAILayerVersion, &sAILayer,
	kAIATEPaintSuite, kAIATEPaintSuiteVersion, &sATEPaint,
	kAIFontSuite, kAIFontSuiteVersion, &sAIFont,
	kAIATETextUtilSuite, kAIATETextUtilSuiteVersion, &sATETextUtil,
	kAIDataFilterSuite, kAIDataFilterSuiteVersion, &sAIDataFilter,
	kAISymbolSuite, kAISymbolSuiteVersion, &sAISymbol,
	kAIPatternSuite, kAIPatternSuiteVersion, &sAIPattern,
	kAIPlacedSuite, kAIPlacedSuiteVersion, &sAIPlaced,
	kAIRasterSuite, kAIRasterSuiteVersion, &sAIRaster,
	kAIArtStyleSuite, kAIArtStyleSuiteVersion, &sAIArtStyle,
    kAIArtStyleParserSuite, kAIArtStyleParserSuiteVersion, &sAIArtStyleParser,
	kAILiveEffectSuite, kAILiveEffectSuiteVersion, &sAILiveEffect,
	kAIDictionarySuite, kAIDictionarySuiteVersion, &sAIDictionary,
	kAIDictionaryIteratorSuite, kAIDictionaryIteratorSuiteVersion, &sAIDictionaryIterator,
	kAIEntrySuite, kAIEntrySuiteVersion, &sAIEntry,
	kAIImageOptSuite, kAIImageOptSuiteVersion, &sAIImageOpt,
	kAIRealBezierSuite, kAIRealBezierSuiteVersion, &sAIRealBezier,

	IMPORT_TEXT_SUITES
	nil, 0, nil
};


// End Ai2CanvasSuites.cpp
