// Canvas.cpp
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
#include "Canvas.h"

#define MAX_BREADCRUMB_DEPTH 256

using namespace CanvasExport;

AIBoolean ProgressProc(ai::int32 current, ai::int32 total);

// Outside of namespace
// TODO: Fix this
AIBoolean ProgressProc(ai::int32 current, ai::int32 total)
{
	(void)current;
	(void)total;
	return true;
}

// Simple way to describe art types for debugging purposes
static const char *m_artTypes[] = 
{
	"kUnknownArt", "kGroupArt", "kPathArt", "kCompoundPathArt", "kTextArtUnsupported", "kTextPathArtUnsupported", "kTextRunArtUnsupported", "kPlacedArt", "kMysteryPathArt", "kRasterArt", "kPluginArt", "kMeshArt", "kTextFrameArt", "kSymbolArt", "kForeignArt", "kLegacyTextArt"
};

Canvas::Canvas(const std::string& id, DocumentResources* documentResources)
{
	// Initialize Canvas
	this->documentResources = documentResources;
	this->id = id;
	this->width = 0.0f;
	this->height = 0.0f;
	this->isHidden = false;
	this->contextName = "";
	this->currentState = nullptr;
	this->usePathfinderStyle = false;

	// Push the first drawing state
	PushState();
}

Canvas::~Canvas()
{
}

// Pushes a new drawing state onto the stack
// Copies values from prior state as defaults for new state
void Canvas::PushState()
{
	// New context drawing state
	State state;

	// Is there already a state on the stack?
	if (!states.empty())
	{
		// Copy current state
		state = states[(states.size() - 1)];
	}

	// Add state
	states.push_back(state);

	// Set "current" state
	currentState = &states.back();
}

void Canvas::PopState()
{
	// Remove last state
	states.pop_back();

	// Set "current" state
	currentState = &states.back();
}

// Report canvas information
void Canvas::DebugInfo()
{
	outFile << "\n\n// Canvas Info";
	outFile << "\n//   id = " << this->id;
	outFile << "\n//   width = " << setiosflags(ios::fixed) << setprecision(2) << this->width;
	outFile << "\n//   height = " << setiosflags(ios::fixed) << setprecision(2) << this->height;
	outFile << "\n//   isHidden = " << this->isHidden;
	outFile << "\n//   contextName = " << this->contextName;
	outFile << "\n//   states = " << this->states.size();

	// Report states
	for (unsigned int i = 0; i < states.size(); i++)
	{
		// Report state information
		states[i].DebugInfo();
	}
}

void Canvas::Render()
{
	// Start canvas tag
	outFile << "\n   <canvas id=\"" << id << "\" ";
			
	// Is this canvas hidden?
	if (isHidden)
	{
		if (!debug)
		{
			outFile << "style=\"display: none\" ";
		}
	}
			
	// Output ID and dimensions
	outFile << "width=\"" << (int)ceil(width) << "\" height=\"" << (int)ceil(height) << "\"></canvas>";
}

void Canvas::RenderImages()
{
	documentResources->images.Render();
}

// Render an Illustrator art object
// (including siblings and children)
void Canvas::RenderArt(AIArtHandle artHandle, unsigned int depth)
{
	// Simple way to describe blending modes for debugging purposes
	// See: http://help.adobe.com/en_US/Illustrator/14.0/WS714a382cdf7d304e7e07d0100196cbc5f-64e0a.html
	static const char *blendingModes[] =
	{
		"Normal", "Multiply", "Screen", "Overlay", "Soft Light", "Hard Light", "Color Dodge", "Color Burn", "Darken", "Lighten", "Difference",
		"Exclusion", "Hue", "Saturation", "Color", "Luminosity", "Num"
	};

	// Start by gathering art and its siblings
	std::vector<AIArtHandle> artHandles;
	bool hasClipIndex = false;
	size_t clipIndex = 0;

	do
	{
		// Get type (needed for kPluginArt/Pathfinder clip test)
		short type = 0;
		sAIArt->GetArtType(artHandle, &type);

		// Get the path style
		AIPathStyle style;
		AIBoolean outHasAdvFill = false;
		sAIPathStyle->GetPathStyle(artHandle, &style, &outHasAdvFill);

		// Is this kPluginArt?
		if (type == kPluginArt)
		{
			// Determine if this plug-in art is clipping
			AIBoolean clipping = false;
			sAIPluginGroup->GetPluginArtClipping(artHandle, &clipping);

			// Is it clipping?
			if (clipping)
			{
				// Remember where the clipping path is
				clipIndex = artHandles.size();
				hasClipIndex = true;
			}
		}
		else
		{
			// Not kPluginArt, so check style attribute
			if (style.clip)
			{
				// Remember where the clipping path is
				clipIndex = artHandles.size();
				hasClipIndex = true;
			}
		}

		// Add this art handle
		artHandles.push_back(artHandle);

		// Find the next sibling
		sAIArt->GetArtSibling(artHandle, &artHandle);
	}
	while (artHandle != nullptr);

	// Did we find a clipping path?
	if (hasClipIndex)
	{
		// Remember the clip art handle
		AIArtHandle clipArtHandle = artHandles[clipIndex];

		// Shift everything from the clip "up" by 1
		for (size_t i = clipIndex; i < (artHandles.size() - 1); i++)
		{
			artHandles[i] = artHandles[i + 1];
		}

		// Move the clip art handle to the end
		artHandles[(artHandles.size() - 1)] = clipArtHandle;
	}

	// Loop through all art in this layer
	// Do it backwards, because of canvas "painter model"
	for (size_t i = artHandles.size(); i > 0; i--)
	{
		artHandle = artHandles[(i - 1)];

		// Is this art visible?
		AIBoolean isArtVisible = false;
		ai::int32 attr = 0;
		sAIArt->GetArtUserAttr(artHandle, kArtHidden, &attr);
		isArtVisible = !((attr &kArtHidden) == kArtHidden);

		// Only render if art is visible
		if (isArtVisible)
		{
			// Get art name
			ai::UnicodeString artName;
			AIBoolean isDefaultName = false;
			sAIArt->GetArtName(artHandle, artName, &isDefaultName);

			// Add name to breadcrumbs
			AddBreadcrumb(artName.as_UTF8(), depth);

			// Do we need to rasterize this art?
			AIBoolean rasterizeArt = false;

			// Does this art have an associated opacity mask?
			AIMaskRef mask;
			sAIMask->GetMask(artHandle, &mask);

			// Did we find a mask?
			if (mask != nullptr)
			{
				// Output a warning
				outFile << "\n" << Indent(depth) << "// This artwork uses an unsupported opacity mask";

				// Rasterize the art
				rasterizeArt = true;
			}

			// Parse the art styles, including drop shadow information
			ASInt32 postEffectCount = 0;
			AIBlendingMode blendingMode = 0;
			AIBoolean hasDropShadow = false;
			DropShadow dropShadow;
			ParseArtStyle(artHandle, depth, postEffectCount, blendingMode, hasDropShadow, dropShadow);

			// Anything we can't convert and should rasterize?
			if (postEffectCount > 1)
			{
				// Rasterize this art
				rasterizeArt = true;

				// Don't bother with the drop shadow, since we can't convert the combination of effects
				hasDropShadow = false;
			}
			else if (postEffectCount == 1)
			{
				// Do we have anything other than a drop shadow?
				if (!hasDropShadow)
				{
					// Rasterize this art
					rasterizeArt = true;
				}
			}

			// If the blending mode is anything other than normal, we should output a warning
			if (blendingMode != kAINormalBlendingMode)
			{
				// Output a warning
				outFile << "\n" << Indent(depth) << "// This artwork uses an unsupported \"" << std::string(blendingModes[blendingMode]) << "\" blending mode";
			}

			// Do we need to increase depth because of a drop shadow?
			if (hasDropShadow)
			{
				// Increase depth so we can maintain shadow context
				depth++;
			}

			// Set state
			// TODO: Should this only be done if the art is visible?
			SetContextDrawingState(depth);

			// Render drop shadow info, if there is any
			if (hasDropShadow)
			{
				RenderDropShadow(dropShadow, depth);
			}

			// Get opacity
			AIReal opacity = sAIBlendStyle->GetOpacity(artHandle);

			// Are we rasterizing this art?
			if (rasterizeArt)
			{
				// Rasterize the art
				std::string fileName = GetUniqueFileName(documentResources->folderPath, "image", ".png");
				outFile << "\n" << Indent(depth) << "// This unsupported artwork has been rasterized";
				RenderUnsupportedArt(artHandle, fileName, depth);
			}
			else
			{
				// Is opacity different than current state?
				if (opacity != currentState->globalAlpha)
				{
					// Assign new global alpha
					currentState->globalAlpha = opacity;

					// Change global alpha (based on the "base" alpha value)
					outFile << "\n" << Indent(depth) << contextName << ".globalAlpha = alpha * " <<
						setiosflags(ios::fixed) << setprecision(2) << currentState->globalAlpha << ";";
				}

				// Get type
				short type = 0;
				sAIArt->GetArtType(artHandle, &type);
				if (debug)
				{
					outFile << "\n" << Indent(depth) << "// Art type = " << std::string(m_artTypes[type]) << " (" << type << ")";
				}

				// Process based on art type
				switch (type)
				{
					case kGroupArt:
					{
						// Render this sub-group
						RenderGroupArt(artHandle, depth);
						break;
					}
					case kPluginArt:
					{
						RenderPluginArt(artHandle, depth);
						break;
					}
					case kSymbolArt:
					{
						RenderSymbolArt(artHandle, depth);
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
					case kTextFrameArt:
					{
						RenderTextFrameArt(artHandle, depth);
						break;
					}
					case kPlacedArt:
					{
						RenderPlacedArt(artHandle, depth);
						break;
					}
					case kRasterArt:
					{
						RenderRasterArt(artHandle, depth);
						break;
					}
					case kMeshArt:
					{
						// Get a unique file name
						std::string fileName = GetUniqueFileName(documentResources->folderPath, "image", ".png");
						RenderUnsupportedArt(artHandle, fileName, depth);
						break;
					}
				}
			}

			// Were we rendering a drop shadow?
			if (hasDropShadow)
			{
				// Decrease depth to restore context
				depth--;
			}

			// Remove from breadcrumb
			RemoveBreadcrumb();
		}
	}
}

// Parse the art styles (including Live Effects) associated with this artwork
// Returns the number of effects, the blending mode, and drop shadow information
void Canvas::ParseArtStyle(AIArtHandle artHandle, unsigned int depth, ASInt32& postEffectCount, AIBlendingMode& blendingMode, AIBoolean& hasDropShadow, DropShadow& dropShadow)
{
	// Simple way to describe art types for debugging purposes
//	static const char *dictTypes[] = 
//	{
//		"UnknownType", "IntegerType", "BooleanType", "RealType", "StringType", "DictType", "ArrayType", "BinaryType", "PointType",
//		"MatrixType", "PatternRefType", "BrushPatternRefType", "CustomColorRefType", "GradientRefType", "PluginObjectRefType",
//		"FillStyleType", "StrokeStyleType", "UIDType", "UIDREFType", "XMLNodeType", "SVGFilterType", "ArtStyleType", "SymbolPatternRefType",
//		"GraphDesignRefType", "BlendStyleType", "GraphicObjectType"
//	};

	// Does this artwork have a drop shadow?
	hasDropShadow = false;

	// Get the style for this art handle
	AIArtStyleHandle artStyle = nullptr;
    sAIArtStyle->GetArtStyle(artHandle, &artStyle);

	// Create a new style parser 
	AIStyleParser parser(nullptr);
    sAIArtStyleParser->NewParser(&parser);
    
	// Parse the art style
    sAIArtStyleParser->ParseStyle(parser, artStyle);

	// Get blend field
	AIParserBlendField blendField;
	sAIArtStyleParser->GetStyleBlendField(parser, &blendField); 
	
	// Get the blending mode
	blendingMode = sAIBlendStyle->GetBlendingMode(artHandle);

	// How many post-effects are attached to this art style?
	postEffectCount = sAIArtStyleParser->CountPostEffects(parser);

	// Loop through all post-effect art styles
    for (ASInt32 postIndex = 0; (postIndex < postEffectCount); ++postIndex)
    {
		// Parse the Live Effects
        AIParserLiveEffect liveEffect;
        sAIArtStyleParser->GetNthPostEffect(parser, postIndex, &liveEffect);

		// Get the Live Effect handle
		AILiveEffectHandle liveEffectHandle;
		sAIArtStyleParser->GetLiveEffectHandle(liveEffect, &liveEffectHandle);
		
		// Get the name of the effect (function appears to allocate its own memory???)
		const char *liveEffectName = nullptr;
		// TODO: Do we need to release this memory somewhere? Or does the fact that we retrieved the handle via the AIArtStyleParser do the trick
		//       (since we clean-up the parser later)?
		sAILiveEffect->GetLiveEffectName(liveEffectHandle, &liveEffectName);
		if (debug)
		{
			outFile << "\n" << Indent(depth) << "// Live Effect name = " << liveEffectName;
		}

        // Check to see if the name is “Adobe Drop Shadow”
		if (strcmp(liveEffectName, "Adobe Drop Shadow") == 0)
		{
			// Set default drop shadow values
			dropShadow.horz = 0.0f;
			dropShadow.vert = 0.0f;
			dropShadow.blur = 0.0f;
			dropShadow.opac = 1.0f;

			// Note that this artwork has a drop shadow assigned
			hasDropShadow = true;

			// Obtain the parameters dictionary
			AILiveEffectParameters params;
			sAIArtStyleParser->GetLiveEffectParams(liveEffect, &params);

			// Do we have any parameters?
			if (params)
			{
				// Create an iterator for the parameters dictionary items
				AIDictionaryIterator dictionaryIter = nullptr;
				sAIDictionary->Begin(params, &dictionaryIter);

				// Iterate through the parameter dictionary entries
				int a = 0;
				while ( !sAIDictionaryIterator->AtEnd(dictionaryIter) )
				{
					// Get the dictionary key
					AIDictKey dictKey = sAIDictionaryIterator->GetKey(dictionaryIter);

					// Get the key string
					const char *keyString = nullptr;
					keyString = sAIDictionary->GetKeyString(dictKey);

					// Clean-up key string
					char betterKeyString[256];
					#ifdef MAC_ENV
					strcpy(betterKeyString, keyString);
					#endif
					#ifdef WIN_ENV
					strcpy_s(betterKeyString, keyString);
					#endif
					if (betterKeyString[0] == '-')
						betterKeyString[0] = ' ';

					// For matching keys, retrieve parameter values
					if (strcmp(betterKeyString, "horz") == 0)
					{
						// Get horizontal shadow offset
						sAIDictionary->GetRealEntry(params, dictKey, &dropShadow.horz);
					}
					else if (strcmp(betterKeyString, "vert") == 0)
					{
						// Get vertical shadow offset
						sAIDictionary->GetRealEntry(params, dictKey, &dropShadow.vert);
					}
					else if (strcmp(betterKeyString, "blur") == 0)
					{
						// Get vertical shadow offset
						sAIDictionary->GetRealEntry(params, dictKey, &dropShadow.blur);
					}
					else if (strcmp(betterKeyString, "opac") == 0)
					{
						// Get shadow opacity
						sAIDictionary->GetRealEntry(params, dictKey, &dropShadow.opac);
					}
					else if (strcmp(betterKeyString, "sclr") == 0)
					{
						// Get shadow color
						// TODO: Check to see if we have to do any reference counting for reading this key (and other keys)
						AIEntryRef entryRef = sAIDictionary->Get(params, dictKey);
						sAIEntry->ToFillStyle(entryRef, &dropShadow.shadowStyle);
					}

					// Move to the next dictionary entry
					sAIDictionaryIterator->Next(dictionaryIter);
					a++;
				}
				// Release the dictionary iterator
				sAIDictionaryIterator->Release(dictionaryIter);
			}
		}
		else
		{
			// A Live Effect we don't recognize
			if (debug)
			{
				outFile << "\n" << Indent(depth) << "//     Unsupported Live Effect: \"" << liveEffectName << "\"";
			}
		}
    }

	// Dispose the art style parser
	sAIArtStyleParser->DisposeParser(parser);
}

// Sets/restores the current state of the canvas
void Canvas::SetContextDrawingState(unsigned int depth)
{
	// Are we restoring state?
	if (depth < states.size())
	{
		// Restore canvas state back to requested depth
		for (size_t i = states.size(); i > depth; i--)
		{
			// Pop state off the stack
			PopState();

			// Restore canvas state
			outFile << "\n" << Indent((states.size() + 1)) << contextName << ".restore();";
		}
	}
	else if (depth > states.size())
	{
		// Save canvas state to requested depth
		for (size_t i = states.size(); i < depth; i++)
		{
			// Push state on the stack
			PushState();

			// Save canvas state
			outFile << "\n" << Indent((states.size() + 1)) << contextName << ".save();";
		}
	}
}

// Render drop shadow information
void Canvas::RenderDropShadow(const DropShadow& dropShadow, unsigned int depth)
{
	// Set the shadow paramters

	// Allocate memory for shadow fill color value string
	std::string shadowColor;
	shadowColor = GetColor(dropShadow.shadowStyle.color, dropShadow.opac);
	outFile << "\n" << Indent(depth) << contextName << ".shadowColor = " << shadowColor << ";";

	// Shadow offsets
	outFile << "\n" << Indent(depth) << contextName << ".shadowOffsetX = " << setiosflags(ios::fixed) << setprecision(1) << dropShadow.horz << ";";
	outFile << "\n" << Indent(depth) << contextName << ".shadowOffsetY = " << setiosflags(ios::fixed) << setprecision(1) << dropShadow.vert << ";";

	// Shadow blur
	// TODO: Note that it appears that we have to double the Illustrator value to achieve equivalent results with <canvas>
	outFile << "\n" << Indent(depth) << contextName << ".shadowBlur = " << setiosflags(ios::fixed) << setprecision(1) << (dropShadow.blur * 2.0f) << ";";
}

// There's no direct equivalent, so just rasterize to a bitmap
void Canvas::RenderUnsupportedArt(AIArtHandle artHandle, const std::string& fileName, unsigned int depth)
{
	(void)depth;

	// Full path to file
	std::string fullPath = documentResources->folderPath + fileName;

	// Rasterize to a 32-bit PNG that includes alpha
	RasterizeArtToPNG(artHandle, fullPath);

	// Get the actual dimensions of the rasterized PNG file
	// Note that the AIArtOptSuite functions seems to rasterize to different sizes, which is why we do this step
	unsigned int pngWidth = 0;
	unsigned int pngHeight = 0;
	GetPNGDimensions(fullPath, pngWidth, pngHeight);

	if (debug)
	{
		outFile << "\n// Actual PNG file dimensions, width = " << pngWidth << ", height = " << pngHeight;
	}

	// Add a new image
	Image* image = documentResources->images.Add(fileName);

	// Image is NOT an absolute path
	image->pathIsAbsolute = false;

	// Get image "alt" name
	ai::UnicodeString artName;
	AIBoolean isDefaultName = false;
	sAIArt->GetArtName(artHandle, artName, &isDefaultName);
	std::string cleanName = artName.as_Platform();
	CleanFunction(cleanName);
	CleanString(cleanName, false);
	image->name = cleanName;

	// Get the art bounding box (which includes transformations)
	AIRealRect bounds;
	sAIArt->GetArtBounds(artHandle, &bounds);

	// Transform the art bounding box
	TransformRect(bounds);

	// Since the PNG rasterize process doesn't always create images of bounds size, center the image inside of the bounds
	AIReal x = bounds.left + (((bounds.right - bounds.left) - pngWidth) / 2.0f);
	AIReal y = bounds.top + (((bounds.bottom - bounds.top) - pngHeight) / 2.0f);

	// Draw image
	image->RenderDrawImage(contextName, x, y);
	image->DebugBounds(contextName, bounds);
}

// Given an art handle, rasterizes to a file at the given path
// NOTE: While width and height are passed, the resulting file is often of a different size, which negatively affects positioning
// See discussion thread: http://forums.adobe.com/thread/603776?tstart=0
void Canvas::RasterizeArtToPNG(AIArtHandle artHandle, const std::string& path)
{
	ai::FilePath filePath;
	filePath.Set(ai::UnicodeString(path));

    AIRealRect bounds;
    sAIArt->GetArtBounds(artHandle, &bounds);
    AIReal artWidth = bounds.right - bounds.left;
    AIReal artHeight = bounds.top - bounds.bottom;

	AIErr result = kNoErr;
	AIDataFilter *dstFilter = nullptr;
	AIDataFilter *filter = nullptr;
	if (!result)
			result = sAIDataFilter->NewFileDataFilter(filePath, "write", 'prw', 'PNGf', &filter);
	if (!result) {
			result = sAIDataFilter->LinkDataFilter(dstFilter, filter);
			dstFilter = filter;
	}

	// Set PNG parameters
	AIImageOptPNGParams2 params;
    params.versionOneSuiteParams.interlaced = false;
    params.versionOneSuiteParams.numberOfColors = 16777216;
    params.versionOneSuiteParams.transparentIndex = 0;
    params.versionOneSuiteParams.resolution = 72.0f;
    params.versionOneSuiteParams.outAlpha = true;
    params.versionOneSuiteParams.outWidth = (ASInt32)artWidth;
    params.versionOneSuiteParams.outHeight = (ASInt32)artHeight;

    //We assume that the basic resolution of illustrator is 72 dpi
    AIReal resolutionRatio =  1.0f;
    AIReal minDim = min(artWidth,artHeight) * resolutionRatio;
    AIReal maxDim = max(artWidth,artHeight) * resolutionRatio;
    AIReal ratio = 1;

    if (minDim < 1)
    {
            ratio = 1 / minDim;
            minDim *= ratio;
            maxDim *= ratio;
    }

    if (maxDim > 65535)
    {
            ratio *= 65535 / maxDim;
    }

    //Here we tune the resolution parameter to comply to minRasterizationDimension and
    //maxRasterizationDimension constraints
    //We assume that the basic resolution of illustrator is 72 dpi
    params.versionOneSuiteParams.resolution *= (AIFloat)ratio;

    params.antialias = true;
    /* A cropping box for the art. If empty or degenerate, do not crop. */
	AIRealRect crop;
	crop.left = 0.0f;
	crop.top = 0.0f;
	crop.right = 0.0f;
	crop.bottom = 0.0f;
    params.cropBox = crop;
    params.backgroundIsTransparent = true;
    /* When backgroundIsTransparent is false, rasterize against this matte color. */
    /*params.matteColor.red = 1.0f;
    params.matteColor.green = 1.0f;
    params.matteColor.blue = 1.0f; */

	// Make PNG
    result = sAIImageOpt->MakePNG24 (artHandle, dstFilter, params, ProgressProc);

    if (dstFilter)
	{
		AIErr tmpresult = sAIDataFilter->UnlinkDataFilter(dstFilter, &dstFilter);
		if (!result)
				result = tmpresult;
	}
}

// Get PNG dimensions
// NOTE: Seems odd that we have to do this, but the rasterization suite in Illustrator doesn't seem to provide this information anywhere,
//       and with the unreliability of the PNG generation sizes, we have to resort to this.
// NOTE: PNG values are big endian, but we need little endian, so we have to reverse
void Canvas::GetPNGDimensions(const std::string& path, unsigned int& pngWidth, unsigned int& pngHeight)
{
	// Represents just enough of a PNG header for us to get its width and height
	// TODO: Will this always work? Or will structure memory layouts cause issues?
	struct PNGHeader
	{
		unsigned char signature[8];		// Should be 137, 80, 78, 71, 13, 10, 26, 10
		uint32_t length;				// Length of the IHDR chunk
		unsigned char chunkType[4];		// Should be 73, 72, 68, 82 (IHDR)
		uint32_t width;					// Image width (in pixels)
		uint32_t height;				// Image height (in pixels)
	};

#ifdef MAC_ENV
	// PNG file handle
	FILE *pngFile = NULL;
	
	// Open the PNG file for binary reading
	pngFile = fopen(path.c_str(), "rb");
#endif
#ifdef WIN_ENV
	// PNG file handle
	FILE *pngFile = nullptr;
	
	// Open the PNG file for binary reading
	fopen_s(&pngFile, path.c_str(), "rb");
#endif

	// Were we able to open the file?
	if (pngFile != nullptr)
	{
		// Read the header info
		PNGHeader header;
		size_t result = fread(&header, sizeof(PNGHeader), 1, pngFile);

		// Did we read anything?
		if (result == 1)
		{
			// Does the signature match?
			if (header.signature[0] == 137 &&
				header.signature[1] == 80 &&
				header.signature[2] == 78 &&
				header.signature[3] == 71 &&
				header.signature[4] == 13 &&
				header.signature[5] == 10 &&
				header.signature[6] == 26 &&
				header.signature[7] == 10)
			{
				// Is this the header?
				if (header.chunkType[0] == 73 &&
					header.chunkType[1] == 72 &&
					header.chunkType[2] == 68 &&
					header.chunkType[3] == 82)
				{
					// Flip "endianness" of the unsigned integer fields
					header.length = ReverseInt(header.length);
					header.width = ReverseInt(header.width);
					header.height = ReverseInt(header.height);

					// Assign return values
					pngWidth = (unsigned int)header.width;
					pngHeight = (unsigned int)header.height;
				}
			}
		}
	}

	// Close the PNG file
	fclose(pngFile);
}

// Flip "endianness" (for PNG files)
uint32_t Canvas::ReverseInt(uint32_t i)
{
    uint32_t c1, c2, c3, c4;

    c1 = i & 255;
    c2 = (i >> 8) & 255;
    c3 = (i >> 16) & 255;
    c4 = (i >> 24) & 255;

    return (c1 << 24) + (c2 << 16) + (c3 << 8) + c4;
}

// Get JPG DPI
// NOTE: Seems odd that we have to do this, but the rasterization suite in Illustrator doesn't seem to provide this information anywhere.
AIReal Canvas::GetJPGDPI(const std::string& path)
{
	// Default 72 DPI
	AIReal dpi = 72.0f;

	// Represents just enough of a JPG header for us to get DPI
	struct JPGHeader
	{
		unsigned char soi[2];			// SOI. Should be 0xff, 0xd8
		unsigned char app0HeaderID[2];	// APP0 segment header ID (0xFF, 0xE0)
		uint16_t app0Size;				// Size of the APP0 segment
		unsigned char identifier[5];	// JFIF identifier. Should be 0x4a, 0x46, 0x49, 0x46, 0x00
		unsigned char majorRevision;	// Major revision number (should be 1)
		unsigned char minorRevision;	// Minor revision number
		unsigned char units;			// Units (0 = no units, x/y-density specify the aspect ratio instead, 1 = x/y-density are dots/inch, 2 = x/y-density are dots/cm)
		uint16_t xDensity;				// Should not be 0
		uint16_t yDensity;				// Should not be 0
	};

#ifdef MAC_ENV
	// JPG file handle
	FILE *jpgFile = nullptr;
	
	// Open the JPG file for binary reading
	jpgFile = fopen(path.c_str(), "rb");
#endif
#ifdef WIN_ENV
	// JPG file handle
	FILE *jpgFile = nullptr;
	
	// Open the JPG file for binary reading
	fopen_s(&jpgFile, path.c_str(), "rb");
#endif

	// Were we able to open the file?
	if (jpgFile != nullptr)
	{
		// Read the header info
		JPGHeader header;
		size_t result = fread(&header, sizeof(JPGHeader), 1, jpgFile);

		// Did we read anything?
		if (result == 1)
		{
			// Does the signature match?
			if (header.soi[0] == 0xff &&
				header.soi[1] == 0xd8 &&
				header.app0HeaderID[0] == 0xff &&
				header.app0HeaderID[1] == 0xe0 &&
				header.identifier[0] == 0x4a &&
				header.identifier[1] == 0x46 &&
				header.identifier[2] == 0x49 &&
				header.identifier[3] == 0x46 &&
				header.identifier[4] == 0x00)
			{
				// Only bother if DPI
				if (header.units == 0x01)
				{
					// Flip "endianness" of the unsigned integer fields
					header.xDensity = ReverseInt(header.xDensity);
					header.yDensity = ReverseInt(header.yDensity);

					// Set DPI
					dpi = (AIReal)header.xDensity;
				}
			}
		}
	}

	// Close the JPG file
	fclose(jpgFile);

	// Return DPI
	return dpi;
}

// Flip "endianness" (for JPG files)
uint16_t Canvas::ReverseInt(uint16_t i)
{
    uint16_t c1, c2;

    c1 = i & 255;
    c2 = (i >> 8) & 255;

    return (c1 << 8) + c2;
}

void Canvas::ReportRasterRecordInfo(const AIRasterRecord& rasterRecord)
{
	outFile << "\n\n// Raster Record Info";
	outFile << "\n//   flags = " << rasterRecord.flags;
	outFile << "\n//   bounds = left:" << rasterRecord.bounds.left << ", top:" << rasterRecord.bounds.top <<
		", right:" << rasterRecord.bounds.right << ", bottom:" << rasterRecord.bounds.bottom;
	outFile << "\n//   byteWidth = " << rasterRecord.byteWidth;
	outFile << "\n//   colorSpace = ";
	ReportColorSpaceInfo(rasterRecord.colorSpace);
	outFile << "\n//   bitsPerPixel = " << rasterRecord.bitsPerPixel;
	outFile << "\n//   originalColorSpace = ";

	// If originalColorSpace = -1, then raster hasn't been through the color converter
	if (rasterRecord.originalColorSpace == -1)
	{
		outFile << "(hasn't been converted yet)";
	}
	else
	{
		ReportColorSpaceInfo(rasterRecord.originalColorSpace);
	}
}

void Canvas::ReportColorSpaceInfo(ai::int16 colorSpace)
{
	// Simple way to describe color space types for debugging purposes
	static const char *colorSpaces[] = 
	{
		"kGrayColorSpace", "kRGBColorSpace", "kCMYKColorSpace"
	};

	// Color space info
	outFile << std::string(colorSpaces[colorSpace]) << " (" << colorSpace << ")";

	// Alpha?
	if (colorSpace & kColorSpaceHasAlpha)
	{
		outFile << " with alpha";
	}
}

void Canvas::RenderGroupArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get the first art element in the group
	AIArtHandle childArtHandle = nullptr;
	sAIArt->GetArtFirstChild(artHandle, &childArtHandle);

	// Render this sub-group
	RenderArt(childArtHandle, depth + 1);
}

void Canvas::RenderPluginArt(AIArtHandle artHandle, unsigned int depth)
{
	// For Illustrator plug-in art types, like "Compound Shape" and "Blend" 
	// For simplicity, we render the "Result Group" (instead of the "Edit Group", which contains all of the original art)

	// What kind of plug-in art is this?
	// TODO: Do we really need to allocate memory here? Or does the call do it for us?
	char **pluginArtName = (char **)calloc(1024, sizeof(char));
	sAIPluginGroup->GetPluginArtName(artHandle, pluginArtName);
	if (debug)
	{
		outFile << "\n" << Indent(depth) << "// Plug-in art name = " << std::string(*pluginArtName);
	}

	// Is this the Pathfinder Suite? If so, we need to grab the style from this art handle
	if (strcmp(*pluginArtName, "Pathfinder Suite") == 0)
	{
		// Set pathfinder style
		AIBoolean outHasAdvFill = false;
		sAIPathStyle->GetPathStyle(artHandle, &pathfinderStyle, &outHasAdvFill);
		usePathfinderStyle = true;

		// Determine if this plug-in art is clipping
		AIBoolean clipping = false;
		sAIPluginGroup->GetPluginArtClipping(artHandle, &clipping);

		// Set clip on our "special style" so we know to clip later
		pathfinderStyle.clip = clipping;
	}

	AIArtHandle resultArtHandle = nullptr;

	// Get the result art handle
	sAIPluginGroup->GetPluginArtResultArt(artHandle, &resultArtHandle);

	// Get the first art element in the result group
	AIArtHandle childArtHandle = nullptr;
	sAIArt->GetArtFirstChild(resultArtHandle, &childArtHandle);

	// Render this sub-group
	// Stay at this depth, so we don't create a unique canvas context
	RenderArt(childArtHandle, depth);

	// Release memory
	free(pluginArtName);
	pluginArtName = nullptr;
}

void Canvas::RenderSymbolArt(AIArtHandle artHandle, unsigned int depth)
{
	// Save canvas state, so we can temporarily transform
	depth++;
	SetContextDrawingState(depth);

	// Get the symbol transformation
	AIRealMatrix transform;
	sAISymbol->GetSoftTransformOfSymbolArt(artHandle, &transform);

	// Concat by [1 0 0 -1 0 0] as coordinates are going positive the other direction.
	AIRealMatrix flipY = 
	{
		1, 0, 0,  - 1, 0, 0
	};
	sAIRealMath->AIRealMatrixConcat(&flipY, &transform, &transform);

	// Concatenate symbol matrix with current internal transform
	sAIRealMath->AIRealMatrixConcat(&transform, &currentState->internalTransform, &transform);
	
	// Render symbol transformation
	outFile << "\n" << Indent(depth) << contextName << ".transform(";
	RenderTransform(transform);
	outFile << ");";

	// Get the symbol pattern
	AIPatternHandle symbolPatternHandle = nullptr;
	sAISymbol->GetSymbolPatternOfSymbolArt(artHandle, &symbolPatternHandle);

	// Find the symbol pattern
	Pattern* symbolPattern = documentResources->patterns.Find(symbolPatternHandle);

	// Did we find it?
	// (we should always find it)
	if (symbolPattern)
	{
		// Call the symbol function
		outFile << "\n" << Indent(depth) << symbolPattern->name << "(" << contextName << ");";
	}

	// Restore canvas state
	depth--;
	SetContextDrawingState(depth);
}

void Canvas::RenderCompoundPathArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get the first art element in the group
	AIArtHandle childArtHandle = nullptr;
	sAIArt->GetArtFirstChild(artHandle, &childArtHandle);

	outFile << "\n" << Indent(depth) << contextName << ".beginPath();";

	// Render this sub-group
	RenderArt(childArtHandle, depth);

	// Get the "normal" style for this path
	AIPathStyle style;
	AIBoolean outHasAdvFill = false;
	sAIPathStyle->GetPathStyle(artHandle, &style, &outHasAdvFill);

	// Apply style
	RenderPathStyle(style, depth);

	// Render this sub-group
	//RenderPathArt(childArtHandle, depth + 1);
}

void Canvas::RenderPathArt(AIArtHandle artHandle, unsigned int depth)
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
		if (debug)
		{
			outFile << "\n\n" << Indent(depth) << "// Art is compound = " << isCompound;
		}

		// Get the "normal" style for this path
		AIPathStyle style;
		AIBoolean outHasAdvFill = false;
		sAIPathStyle->GetPathStyle(artHandle, &style, &outHasAdvFill);

		// Begin path
		if (!isCompound)
		{
			outFile << "\n" << Indent(depth) << contextName << ".beginPath();";
		}

		// Write each path as a figure
		RenderPathFigure(artHandle, depth);

		// Only output if this isn't compound
		if (!isCompound)
		{
			// Do we have a special pathfinder style?
			if (usePathfinderStyle)
			{
				// Apply special pathfinder style
				RenderPathStyle(pathfinderStyle, depth);

				// Stop using special style
				usePathfinderStyle = false;
			}
			else
			{
				// Apply style
				RenderPathStyle(style, depth);
			}
		}
	}
}

// Output a single path and its segments (call multiple times for a compound path)
void Canvas::RenderPathFigure(AIArtHandle artHandle, unsigned int depth)
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
	outFile << "\n" << Indent(depth) << contextName << ".moveTo(" <<
		setiosflags(ios::fixed) << setprecision(1) <<
		segment.p.h << ", " << segment.p.v << ");";

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

		// Close the path
		outFile << "\n" << Indent(depth) << contextName << ".closePath();";
	}
}

void Canvas::RenderSegment(AIPathSegment& previousSegment, AIPathSegment& segment, unsigned int depth)
{
	// Transform points
	TransformPoint(segment.p);
	TransformPoint(segment.in);
	TransformPoint(segment.out);

	// Is this a straight line segment?
	AIBoolean isLine = ((previousSegment.p.h == previousSegment.out.h && previousSegment.p.v == previousSegment.out.v) &&
						(segment.p.h == segment.in.h && segment.p.v == segment.in.v));

	// Draw a segment

	// Is this a line?
	if (isLine)
	{
		// Draw straight line
		outFile << "\n" << Indent(depth) << contextName << ".lineTo(" <<
			setiosflags(ios::fixed) << setprecision(1) << 
			segment.p.h << ", " << segment.p.v << ");";
	}
	else
	{
		// Output Bezier segment
		outFile << "\n" << Indent(depth) << contextName << ".bezierCurveTo(" <<
			setiosflags(ios::fixed) << setprecision(1)
			<< previousSegment.out.h << ", " << previousSegment.out.v << ", "
			<< segment.in.h << ", " << segment.in.v << ", "
			<< segment.p.h << ", " << segment.p.v << ");";
	}
}

void Canvas::RenderPathStyle(const AIPathStyle& style, unsigned int depth)
{
	// Is this clipping?
	if (style.clip)
	{
		outFile << "\n" << Indent(depth) << contextName << ".clip();";
	}
	else
	{
		// Output fill information
		if (style.fillPaint)
		{
			// Investigate style.evenodd
			// http://www.whatwg.org/specs/web-apps/current-work/multipage/the-canvas-element.html

			RenderFillInfo(style.fill.color, depth);
			if (style.evenodd)
			{
				outFile << "\n" << Indent(depth) << contextName << ".fill(\"evenodd\");";
			}
			else
			{
				// Non-zero is the default, so no need to specify
				outFile << "\n" << Indent(depth) << contextName << ".fill();";
			}
		}

		// Output stroke information
		if (style.strokePaint)
		{
			RenderStrokeInfo(style.stroke, depth);
			outFile << "\n" << Indent(depth) << contextName << ".stroke();";
		}
	}
}

void Canvas::RenderPlacedArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get type of placed art
	short placedType = 0;
	sAIPlaced->GetPlacedType(artHandle, &placedType);
	if (debug)
	{
		outFile << "\n" << Indent(depth) << "// Placed art type = " << placedType;
	}

	// Only bother if this isn't EPS art (should then be linked raster art)
	if (placedType != kEPSType)
	{
		// Get file path
		ai::UnicodeString path;
		sAIPlaced->GetPlacedFilePathFromArt(artHandle, path);
		if (debug)
		{
			outFile << "\n" << Indent(depth) << "// Placed art file path = " << path.as_Platform();
		}

		// Add a new image
		Image* image = documentResources->images.Add(path.as_Platform());

		// Image is an absolute path
		image->pathIsAbsolute = true;

		// Get image "alt" name
		ai::UnicodeString artName;
		AIBoolean isDefaultName = false;
		sAIArt->GetArtName(artHandle, artName, &isDefaultName);
		std::string cleanName = artName.as_Platform();
		CleanFunction(cleanName);
		CleanString(cleanName, false);
		image->name = cleanName;

		// Get placed dimensions
		AIRealPoint size;
		AIRealRect viewBounds;
		AIRealMatrix viewMatrix;
		AIRealRect imageBounds;
		AIRealMatrix imageMatrix;
		sAIPlaced->GetPlacedDimensions(artHandle, &size, &viewBounds, &viewMatrix, &imageBounds, &imageMatrix);

		// Get the art bounding box (which includes transformations)
		AIRealRect bounds;
		sAIArt->GetArtBounds(artHandle, &bounds);

		// Transform the art bounding box
		TransformRect(bounds);

		image->DebugBounds(contextName, bounds);

		// Get the transformation matrix for this placed art
		AIRealMatrix transform;
		sAIPlaced->GetPlacedMatrix(artHandle, &transform);

		// Flip the image
		transform.c *= -1.0f;
		transform.d *= -1.0f;

		// So that we transform around the center point, translate to the center point of the bounds
		transform.tx = (bounds.left + bounds.right) / 2.0f;
		transform.ty = (bounds.top + bounds.bottom) / 2.0f;

		// Get JPG DPI
		AIReal dpi = GetJPGDPI(path.as_Platform());

		// Modify transform values based on DPI setting
		AIReal ratio = 72.0f / dpi;
		transform.a *= ratio;
		transform.b *= ratio;
		transform.c *= ratio;
		transform.d *= ratio;

		// Save canvas state, so we can temporarily transform
		depth++;
		SetContextDrawingState(depth);

		// Render transform
		outFile << "\n" << Indent(depth) << contextName << ".transform(";
		RenderTransform(transform);
		outFile << ");";

		// Get actual image dimensions (files that aren't 72 DPI don't report real sizes, so need to do this)
		AIRasterRecord info;
		AIBoolean isRaster = true;
		sAIPlaced->GetRasterInfo(artHandle, &info, &isRaster);

		// Draw image
		// Draw so that the center point is position at 0, 0 (so transformation happens correctly)
		image->RenderDrawImage(contextName, (-1.0f * (info.bounds.right / 2.0f)), (-1.0f * (info.bounds.bottom / 2.0f)));

		// Restore canvas state
		depth--;
		SetContextDrawingState(depth);
	}
}

void Canvas::RenderRasterArt(AIArtHandle artHandle, unsigned int depth)
{
	// Get the raster record
	AIRasterRecord rasterRecord;
	sAIRaster->GetRasterInfo(artHandle, &rasterRecord);

	// Get the original file path
	// TODO: Is this *always* present? NO, it isn't. Need to add a base filename for empty paths.
	ai::UnicodeString path;
	sAIRaster->GetRasterFilePathFromArt(artHandle, path);
	if (debug)
	{
		outFile << "\n" << Indent(depth) << "// Raster file path from art = " << path.as_Platform();
	}

	// Did we get a filename?
	std::string fileName = path.as_UTF8();
	if (fileName.length() > 0)
	{
		// Construct a FilePath
		ai::UnicodeString usFileName(fileName);
		ai::FilePath aiFilePath(usFileName);

		// Extract file name
		fileName = aiFilePath.GetFileNameNoExt().as_Platform();
	}
	else
	{
		// Create a base filename
		fileName = "image";
	}

	// Get a unique file name
	std::string uniqueFileName = GetUniqueFileName(documentResources->folderPath, fileName, ".png");

	// Full path to file
	std::string fullPath = documentResources->folderPath + uniqueFileName;

	// NOTE: Remember that a single image/filename can be embedded multiple times using different
	//       transformations in a single Illustrator document. So, they need to be unique when they're rasterized anyway.
	RasterizeArtToPNG(artHandle, fullPath);

	// Add a new image
	Image* image = documentResources->images.Add(uniqueFileName);

	// Image is NOT an absolute path
	image->pathIsAbsolute = false;

	// Get image "alt" name
	ai::UnicodeString artName;
	AIBoolean isDefaultName = false;
	sAIArt->GetArtName(artHandle, artName, &isDefaultName);
	std::string cleanName = artName.as_Platform();
	CleanFunction(cleanName);
	CleanString(cleanName, false);
	image->name = cleanName;

	// Get the art bounding box (which includes transformations)
	AIRealRect bounds;
	sAIArt->GetArtBounds(artHandle, &bounds);

	// Transform the art bounding box
	TransformRect(bounds);

	// Draw image
	image->RenderDrawImage(contextName, bounds.left, bounds.top);
	image->DebugBounds(contextName, bounds);
}

// 10/11/2012: Added alpha support
void Canvas::RenderMidPointColor(const AIColor& color1, AIReal alpha1, const AIColor& color2, AIReal alpha2)
{
	// Convert colors to RGB color space
	AIColor rgbColor1;
	AIColor rgbColor2;
	ConvertColorToRGB(color1, rgbColor1);
	ConvertColorToRGB(color2, rgbColor2);

	// Calculate mid-point
	AIReal percentage = 0.5;

    // Output color values
	if (alpha1 != 1.0f ||
        alpha2 != 1.0f)
	{
		// Include alpha
        outFile << "rgba(" <<
		(int)((rgbColor1.c.rgb.red + (percentage *(rgbColor2.c.rgb.red - rgbColor1.c.rgb.red)))*(float)255) << ", " <<
		(int)((rgbColor1.c.rgb.green + (percentage *(rgbColor2.c.rgb.green - rgbColor1.c.rgb.green)))*(float)255) << ", " <<
		(int)((rgbColor1.c.rgb.blue + (percentage *(rgbColor2.c.rgb.blue - rgbColor1.c.rgb.blue)))*(float)255) << ", " <<
        setiosflags(ios::fixed) << setprecision(2) << (alpha1 + (percentage * (alpha2 - alpha1))) << ")";
	}
	else
	{
        outFile << "rgb(" <<
		(int)((rgbColor1.c.rgb.red + (percentage *(rgbColor2.c.rgb.red - rgbColor1.c.rgb.red)))*(float)255) << ", " <<
		(int)((rgbColor1.c.rgb.green + (percentage *(rgbColor2.c.rgb.green - rgbColor1.c.rgb.green)))*(float)255) << ", " <<
		(int)((rgbColor1.c.rgb.blue + (percentage *(rgbColor2.c.rgb.blue - rgbColor1.c.rgb.blue)))*(float)255) << ")";
	}
}

void Canvas::RenderGradient(const AIGradientStyle& gradientStyle, unsigned int depth)
{
	// What kind of gradient is it?
	short type;
	sAIGradient->GetGradientType(gradientStyle.gradient, &type);

	// Grab the transformation matrix
	AIRealMatrix matrix = gradientStyle.matrix;

	// Apply current internal transform
	if (currentState->isProcessingSymbol)
	{
		// If this is a symbol, we need to harden the matrix
		sAIHardSoft->AIRealMatrixHarden(&matrix);
	}
	else
	{
		sAIRealMath->AIRealMatrixConcat(&matrix, &currentState->internalTransform, &matrix);
	}

	// Is there any transformation other than translation?
	AIBoolean isTransformed = false;
	if (fabsf((float)matrix.a) != 1.0f || matrix.b != 0.0f || matrix.c != 0.0f || fabsf((float)matrix.d) != 1.0f)
	{
		// Will need to apply transformation
		isTransformed = true;
	}

	// Do we have a transform to apply?
	if (isTransformed)
	{
		// We need to fill *without* the current transform, so push an extra state (it will be automatically popped later);
		depth++;
		SetContextDrawingState(depth);

		// Set gradient transform
		outFile << "\n" << Indent(depth) << contextName << ".transform(";
		RenderTransform(matrix);
		outFile << ");";
	}

	// Grab the origin
	AIRealPoint p1 = gradientStyle.gradientOrigin;

	switch (type)
	{
		case (kLinearGradient): 
		{
			AIRealPoint p2;
			sAIRealMath->AIRealPointLengthAngle(gradientStyle.gradientLength, sAIRealMath->DegreeToRadian(gradientStyle.gradientAngle), &p2);
			sAIRealMath->AIRealPointAdd(&p1, &p2, &p2);

			// If we aren't transforming with a matrix, simply transform the individual points
			if (!isTransformed)
			{
				TransformPointWithMatrix(p1, matrix);
				TransformPointWithMatrix(p2, matrix);
			}

			outFile << "\n" << Indent(depth) << "gradient = " << contextName << ".createLinearGradient(" <<
				setiosflags(ios::fixed) << setprecision(1) <<
				p1.h << ", " << p1.v << ", " << p2.h << ", " << p2.v << ");";

			RenderGradientStops(gradientStyle, depth);

			break;
		}
		case (kRadialGradient): 
		{
			AIRealPoint p2;
			sAIRealMath->AIRealPointLengthAngle((gradientStyle.hiliteLength * gradientStyle.gradientLength), sAIRealMath->DegreeToRadian(gradientStyle.hiliteAngle), &p2);
			sAIRealMath->AIRealPointAdd(&p1, &p2, &p2);

			// If we aren't transforming with a matrix, simply transform the individual points
			if (!isTransformed)
			{
				TransformPointWithMatrix(p1, matrix);
				TransformPointWithMatrix(p2, matrix);
			}

			// Don't pre-transform any points, because our world transformation will do it for us
			outFile << "\n" << Indent(depth) << "gradient = " << contextName << ".createRadialGradient(" <<
				setiosflags(ios::fixed) << setprecision(1) <<
				p2.h << ", " << p2.v << ", " << 0.0f << ", " << p1.h << ", " << p1.v << ", " << gradientStyle.gradientLength << ");";

			RenderGradientStops(gradientStyle, depth);

			break;
		}
	}
}

// NOTE: Gradient stop opacity was introduced after CS2, which is why we can't take advantage of it here
// 10/11/2012: Added gradient stop support for CS6
void Canvas::RenderGradientStops(const AIGradientStyle& gradientStyle, unsigned int depth)
{
	AIGradientStop gradientStop;
	AIGradientStop gradientStopNext;
	short count, index;
	AIReal stopPoint;

	sAIGradient->GetGradientStopCount(gradientStyle.gradient, &count);

	for (index = 0; index < count; index++)
	{
		sAIGradient->GetNthGradientStop(gradientStyle.gradient, index, &gradientStop);
		stopPoint = gradientStop.rampPoint / (float)100;
		outFile << "\n" << Indent(depth) << "gradient.addColorStop(" <<
			setiosflags(ios::fixed) << setprecision(2) <<
			stopPoint << ", " << GetColor(gradientStop.color, gradientStop.opacity) << ");";

		// Handle midpoints that aren't exacly at 50% (ignore midpoint for last stop)
		if (gradientStop.midPoint != 50.0f && index < (count - 1))
		{
			sAIGradient->GetNthGradientStop(gradientStyle.gradient, index + 1, &gradientStopNext);
			stopPoint = (gradientStop.rampPoint + ((gradientStop.midPoint / (float)100)*(gradientStopNext.rampPoint - gradientStop.rampPoint))) / (float)100;
			outFile << "\n" << Indent(depth) << "gradient.addColorStop(" <<
				setiosflags(ios::fixed) << setprecision(2) <<
				stopPoint << ", \"";
			RenderMidPointColor(gradientStop.color, gradientStop.opacity, gradientStopNext.color, gradientStopNext.opacity);
			outFile << "\");";
		}
	}
}

// Output fill information
void Canvas::RenderFillInfo(const AIColor& fillColor, unsigned int depth)
{
	// Allocate memory for fill style value string
	std::string fillStyle;

	// Get fill style value
	GetFillStyle(fillColor, 1.0f, fillStyle);

	// Render based on the kind of fill style
	switch (fillColor.kind)
	{
		case (kGrayColor):
        case (kFourColor):
        case (kCustomColor):
        case (kThreeColor):
		{
			// Is the fill color different?
			if (fillStyle != currentState->fillStyle)
			{
				// Change to new fill color
				currentState->fillStyle = fillStyle;

				// Change the fill style
				outFile << "\n" << Indent(depth) << contextName << ".fillStyle = " << currentState->fillStyle << ";";
			}
			break;
		}
		case (kPattern): 
		{
			// Find the pattern
			Pattern* pattern = documentResources->patterns.Find(fillColor.c.p.pattern);

			// Did we find the pattern?
			// NOTE: This should always succeed
			if (pattern)
			{
				// Change to "pattern" fill style
				currentState->fillStyle = fillStyle;

				// We need to fill *without* the current transform, so push an extra state (it will be automatically popped later);
				depth++;
				SetContextDrawingState(depth);

				// Create the pattern
				// Don't save context, since this is a different/sub canvas
				outFile << "\n" << Indent(depth) << "pattern = " << contextName << ".createPattern(" <<
					"document.getElementById(\"pattern" << pattern->canvasIndex << "\"), \"repeat\");";

				// Set pattern fill transform
				// TODO: Need to figure out how to determine proper X and Y offsets
				// TODO: We should be able to avoid this, if the transform is identity
				outFile << "\n" << Indent(depth) << contextName << ".transform(";
				RenderTransform(fillColor.c.p.transform);
				outFile << ");";

				// Change fill style to pattern
				outFile << "\n" << Indent(depth) << contextName << ".fillStyle = " << currentState->fillStyle << ";";
			}

			break;
		}
		case (kGradient): 
		{
			// Write gradient information
			RenderGradient(fillColor.c.b, depth);

			// Change to "gradient" fill style
			// NOTE: Gradients are very rarely identical, so no special state optimization here
			currentState->fillStyle = fillStyle;

			// We need to fill *without* the current transform, so push an extra state (it will be automatically popped later);
			//depth++;
			//SetContextDrawingState(depth);

			// Set gradient fill transform
			// TODO: Need to figure out how to determine proper X and Y offsets
			// TODO: We should be able to avoid this, if the transform is identity
			//outFile << "\n%s%s.transform(", Indent(depth), m_currentCanvas->contextName.c_str());
			//RenderTransform(fillColor.c.b.matrix);
			//outFile << ");");

			// Change the fill style
			outFile << "\n" << Indent(depth) << contextName << ".fillStyle = " << currentState->fillStyle << ";";
			break;
		}
        case kNoneColor:
        case kAdvanceColor:
        {
            break;
        }
	}
}

// Returns a fill style string
// NOTE: Should allocate enough memory for a worst-case result ("rgba(000, 000, 000, 1.00)" + '\0') = 28 bytes
void Canvas::GetFillStyle(const AIColor& color, AIReal alpha, std::string& fillStyle)
{
	// Based on kind of color
	switch (color.kind)
	{
		case kGrayColor:
        case kFourColor:
        case kCustomColor:
        case kThreeColor:
		{
			// Get the fill color value
			fillStyle = GetColor(color, alpha);
			break;
		}
		case kPattern:
		{
			// Return "pattern"
			fillStyle = "pattern";
			break;
		}
		case kGradient:
		{
			// Return "gradient"
			fillStyle = "gradient";
			break;
		}
        case kNoneColor:
        case kAdvanceColor:
        {
            break;
        }
	}
}

// Report on a pattern style
void Canvas::ReportPatternStyleInfo(const AIPatternStyle& patternStyle)
{
	outFile << "\n\n// Pattern Info";
	outFile << "\n//   shiftDist = " <<
		setiosflags(ios::fixed) << setprecision(1) <<
		patternStyle.shiftDist;
	outFile << "\n//   shiftAngle = %.2f" <<
		setiosflags(ios::fixed) << setprecision(2) <<
		patternStyle.shiftAngle;
	outFile << "\n//   scale = " <<
		setiosflags(ios::fixed) << setprecision(1) <<
		patternStyle.scale.h << ", " << patternStyle.scale.v;
	outFile << "\n//   rotate = " <<
		setiosflags(ios::fixed) << setprecision(2) <<
		patternStyle.rotate;
	outFile << "\n//   reflect = " << patternStyle.reflect;
	outFile << "\n//   reflectAngle = " <<
		setiosflags(ios::fixed) << setprecision(2) <<
		patternStyle.reflectAngle;
	outFile << "\n//   shearAngle = " <<
		setiosflags(ios::fixed) << setprecision(2) <<
		patternStyle.shearAngle;
	outFile << "\n//   shiftDist = " <<
		setiosflags(ios::fixed) << setprecision(1) <<
		patternStyle.shiftDist;
	outFile << "\n//   shiftAxis = " <<
		setiosflags(ios::fixed) << setprecision(1) <<
		patternStyle.shearAxis;
	outFile << "\n//   transform = ";
	RenderTransform(patternStyle.transform);
}

// Output stroke information
void Canvas::RenderStrokeInfo(const AIStrokeStyle& strokeStyle, unsigned int depth)
{
	// Does this stroke use features that we can't convert?
	// TODO: Check for some false positives here...seem to see it where there aren't custom dash styles on occasion
	if (strokeStyle.dash.length != 0)
	{
		// Stroke uses a dash style that has no canvas equivalent
		outFile << "\n" << Indent(depth) << "// This artwork uses an unsupported dash style";
	}

	// Stroke thickness
	if (strokeStyle.width != currentState->lineWidth)
	{
		// Assign new line width
		currentState->lineWidth = strokeStyle.width;

		// Output line width change
		outFile << "\n" << Indent(depth) << contextName << ".lineWidth = " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			currentState->lineWidth << ";";
	}

	// Stroke color
	switch (strokeStyle.color.kind)
	{
		case kGrayColor:
        case kFourColor:
        case kCustomColor:
        case kThreeColor:
		{
			// Allocate memory for stroke style value string
			std::string strokeStyleValue;

			// Get the stroke color value
			strokeStyleValue = GetColor(strokeStyle.color, 1.0f);

			// Is the stroke color different?
			if (strokeStyleValue != currentState->strokeStyle)
			{
				// Change to new stroke color
				currentState->strokeStyle = strokeStyleValue;

				// Change the stroke style
				outFile << "\n" << Indent(depth) << contextName << ".strokeStyle = " << currentState->strokeStyle << ";";
			}
			break;
		}
        case kPattern:
        case kGradient:
        case kNoneColor:
        case kAdvanceColor:
        {
            break;
        }
	}

	// Do we have to define both start and end cap styles?
	if (strokeStyle.cap != currentState->lineCap)
	{
		// Assign new cap style
		currentState->lineCap = strokeStyle.cap;

		// Output new stroke style
		switch (currentState->lineCap)
		{
			case (kAIButtCap): 
			{
				// Butt line caps
				outFile << "\n" << Indent(depth) << contextName << ".lineCap = \"butt\";";
				break;
			}
			case (kAIRoundCap): 
			{
				// Round line caps
				outFile << "\n" << Indent(depth) << contextName << ".lineCap = \"round\";";
				break;
			}
			case (kAIProjectingCap): 
			{
				// Projecting/square line caps 
				outFile << "\n" << Indent(depth) << contextName << ".lineCap = \"square\";";
				break;
			}
		}
	}


	// How are segments joined?
	if ((strokeStyle.join != currentState->lineJoin) ||
		((strokeStyle.join == kAIMiterJoin) && (strokeStyle.miterLimit != currentState->miterLimit)))
	{
		// Assign new join style
		currentState->lineJoin = strokeStyle.join;

		// Output new join style
		switch (currentState->lineJoin)
		{
			case (kAIMiterJoin): 
			{
				// Miter line joins (the default join type)
				outFile << "\n" << Indent(depth) << contextName << ".lineJoin = \"miter\";";

				// Accomodate the miter limit (see NOTES to understand why this won't work) - set to "1" for now, which is basically the same as "Bevel"
				// Although we don't include "Miter", since it's the default, we do need this hack ("10" is the canvas default)
				// TODO: Report miter bug to IE9 team (Safari, Chrome, and Firefox work fine)
				AIReal miterLimit = strokeStyle.miterLimit;
				outFile << "\n" << Indent(depth) << contextName << ".miterLimit = " <<
					setiosflags(ios::fixed) << setprecision(1) <<
					miterLimit << ";";

				// Assign new miter limit
				currentState->miterLimit = miterLimit;

				break;
			}
			case (kAIRoundJoin): 
			{
				// Round line joins
				outFile << "\n" << Indent(depth) << contextName << ".lineJoin = \"round\";";
				break;
			}
			case (kAIBevelJoin): 
			{
				// Bevel line joins
				outFile << "\n" << Indent(depth) << contextName << ".lineJoin = \"bevel\";";
				break;
			}
		}
	}
}

void Canvas::RenderTextFrameArt(AIArtHandle artHandle, unsigned int depth)
{
	// Render the glyph runs
	RenderGlyphRuns(artHandle, depth);
}

void Canvas::RenderGlyphRuns(AIArtHandle textFrameArt, unsigned int depth)
{
	// Create ITextFrame object.
	TextFrameRef textFrameRef = nullptr;
	sAITextFrame->GetATETextFrame(textFrameArt, &textFrameRef);
	ATE::ITextFrame frame(textFrameRef);

	// Get the text frame matrix
	AIRealMatrix textFrameMatrix;
	textFrameMatrix = frame.GetMatrix();

	// Get the text lines
	ATE::ITextLinesIterator lines = frame.GetTextLinesIterator();
	while (lines.IsNotDone())
	{
		ATE::ITextLine line = lines.Item();
		ATE::IGlyphRunsIterator glyphRuns = line.GetGlyphRunsIterator();

		// Text for a set of glyph runs
		char *text = (char *)malloc(1);
		*text = '\0';

		// Do we need to grab an origin?
		// TODO: Seems messy...can we clean this logic up?
		AIBoolean grabOrigin = true;

		// Last glyph state so we can track changes
		GlyphState lastGlyphState;

		// Loop through all glyph runs
		while (glyphRuns.IsNotDone())
		{
			// Get next glyph run
			ATE::IGlyphRun glyphRun = glyphRuns.Item();

			// Initialize state/style information for this run
			GlyphState glyphState;
			ASInt32 count = 0;
			char *contents = nullptr;

			// Any contents?
			count = glyphRun.GetCharacterCount();
			if (count > 0)
			{
				// Get text contents of glyph run
				contents = (char *)calloc(count + 1, sizeof(char));
				glyphRun.GetContents(contents, count);

				// Get the state/style information for this glyph run
				// TODO: Remember to clear the memory from this instance
				GetGlyphState(glyphRun, glyphState, textFrameMatrix, depth);

				// We don't want to output every glyph run individually, so see if anything has changed that will force us to render
				// TODO: We need a better way to handle this!
				if (!GlyphStatesMatch(lastGlyphState, glyphState) && !grabOrigin)
				{
					// Output
					RenderGlyphRun(text, lastGlyphState, depth);

					// Since we've rendered this text, clear it
					text = (char *)realloc(text, 1);
					*text = '\0';

					// Also need to capture a new origin
					grabOrigin = true;
				}

				// Add current contents
				size_t length = strlen(text) + strlen(contents) + 1;
				text = (char *)realloc(text, length);
#ifdef MAC_ENV
				strcat(text, contents);
#endif
#ifdef WIN_ENV
				strcat_s(text, length, contents);
#endif

				// Remember last state
				// TODO: Do strings copy okay here?
				AIReal oldTx = lastGlyphState.glyphMatrix.tx;
				AIReal oldTy = lastGlyphState.glyphMatrix.ty;
				lastGlyphState = glyphState;

				// Carry forward the initial origin, but only if we don't need to capture the origin (where we initially capture it)
				if (!grabOrigin)
				{
					lastGlyphState.glyphMatrix.tx = oldTx;
					lastGlyphState.glyphMatrix.ty = oldTy;
				}

				// No longer the first pass
				grabOrigin = false;

				// Release memory
				free(contents);
				contents = nullptr;
			}

			// Get the next glyph run
			glyphRuns.Next();
		}

		// Do we have any text yet to render?
		if (strlen(text) > 0)
		{
			// Render it
			RenderGlyphRun(text, lastGlyphState, depth);
		}

		// Release memory
		free(text);
		text = nullptr;

		// Get the next line
		lines.Next();
	}
}

// Output the actual glyph run
void Canvas::RenderGlyphRun(char *contents, const GlyphState& glyphState, unsigned int depth)
{
	// Have any font attributes changed?
	if (glyphState.fontSize != currentState->fontSize ||
		glyphState.fontName != currentState->fontName ||
		glyphState.fontStyleName != currentState->fontStyleName)
	{
		// Output font and style information
		outFile << "\n" << Indent(depth) << contextName << ".font = \"";
		if (glyphState.fontStyleName != "Regular")
		{
			outFile << glyphState.fontStyleName << " ";
		}
		outFile << setiosflags(ios::fixed) << setprecision(1) << glyphState.fontSize << "px '" << glyphState.fontName << "'\";";

		// Remember current font state
		currentState->fontSize = glyphState.fontSize;
		currentState->fontName = glyphState.fontName;
		currentState->fontStyleName = glyphState.fontStyleName;
	}

	// Has the text been transformed?
	AIBoolean isTransformed = (glyphState.glyphMatrix.a != 1.0f || glyphState.glyphMatrix.b != 0.0f || glyphState.glyphMatrix.c != 0.0f || glyphState.glyphMatrix.d != 1.0f);
	if (isTransformed)
	{
		// Save canvas state, so we can temporarily transform
		depth++;
		SetContextDrawingState(depth);

		// Render transform
		outFile << "\n" << Indent(depth) << contextName << ".transform(";
		RenderTransform(glyphState.glyphMatrix);
		outFile << ");";
	}

	// Fill the text?
	if (glyphState.textFilled)
	{
		// Fill color...
		RenderFillInfo(glyphState.fillColor, depth);

		// Output text
		if (isTransformed)
		{
			// Allow transformation to position text
			outFile << "\n" << Indent(depth) << contextName << ".fillText(\"" << contents << "\", " <<
				setiosflags(ios::fixed) << setprecision(1) <<
				0 << ", " << 0 << ");";
		}
		else
		{
			// Since there's no transformation, simply output text at correct point
			outFile << "\n" << Indent(depth) << contextName << ".fillText(\"" << contents << "\", " <<
				setiosflags(ios::fixed) << setprecision(1) <<
				glyphState.glyphMatrix.tx << ", " << glyphState.glyphMatrix.ty << ");";
		}
	}

	// Stroke the text?
	if (glyphState.textStroked)
	{
		// Render stroke information
		RenderStrokeInfo(glyphState.strokeStyleValue, depth);

		// Output text
		if (isTransformed)
		{
			// Allow transformation to position text
			outFile << "\n" << Indent(depth) << contextName << ".strokeText(\"" << contents << "\", " <<
				setiosflags(ios::fixed) << setprecision(1) <<
				0 << ", " << 0 << ");";
		}
		else
		{
			// Since there's no transformation, simply output text at correct point
			outFile << "\n" << Indent(depth) << contextName << ".strokeText(\"" << contents << "\", " <<
				setiosflags(ios::fixed) << setprecision(1) <<
				glyphState.glyphMatrix.tx << ", " << glyphState.glyphMatrix.ty << ");";
		}
	}

	// If we transformed...
	if (isTransformed)
	{
		// Restore the state (mostly to return to prior transformation)
		depth--;
		SetContextDrawingState(depth);
	}
}

// Returns true if the two glyph states match (for values that we care about)
AIBoolean Canvas::GlyphStatesMatch(const GlyphState& state1, const GlyphState& state2)
{
	// TODO: Need to modify fill and stroke color comparisons (string values that we already generate?)
	//       Should we also watch for a change in glyphMatrix.ty? Since vertical spacing would require a new output.
	return (
			(state1.fontSize == state2.fontSize) &&
			(state1.verticalScale == state2.verticalScale) &&
			(state1.horizontalScale == state2.horizontalScale) &&
			(state1.glyphMatrix.a == state2.glyphMatrix.a) &&
			(state1.glyphMatrix.b == state2.glyphMatrix.b) &&
			(state1.glyphMatrix.c == state2.glyphMatrix.c) &&
			(state1.glyphMatrix.d == state2.glyphMatrix.d) &&
			(state1.fontName== state2.fontName) &&
			(state1.fontStyleName == state2.fontStyleName) &&
			(state1.textFilled == state2.textFilled) &&
			(state1.fillStyle == state2.fillStyle) &&
			(state1.textStroked == state2.textStroked) &&
			(state1.strokeStyle == state2.strokeStyle) &&
			(state1.strokeStyleValue.width == state2.strokeStyleValue.width) &&
			(state1.strokeStyleValue.cap == state2.strokeStyleValue.cap) &&
			(state1.strokeStyleValue.join == state2.strokeStyleValue.join) &&
			(state1.strokeStyleValue.miterLimit == state2.strokeStyleValue.miterLimit)
			);
}

// Gets all of the important state information for a glyph run
void Canvas::GetGlyphState(const ATE::IGlyphRun& glyphRun, GlyphState& glyphState, const AIRealMatrix& textFrameMatrix, unsigned int depth)
{
	// Get character features
	ATE::ICharFeatures features = glyphRun.GetCharFeatures();

	// To test for local feature assignments
	bool isAssigned = false;

	// Get font size
	// TODO: Is there ever a case when the font size *isn't* assigned? What's the default in that situation?
	glyphState.fontSize = features.GetFontSize(&isAssigned);

	// Get font info
	ATE::IFont font = features.GetFont(&isAssigned);
	if (isAssigned)
	{
		// Allocate memory for font names and styles
		char *systemFontName = (char*)calloc(1024, sizeof(char));
		char *fontStyleName = (char*)calloc(1024, sizeof(char));

		// Local font is assigned
		FontRef fontRef = font.GetRef();
		AIFontKey fontKey = nullptr;
		sAIFont->FontKeyFromFont(fontRef, &fontKey);

		// Get system font name
		// TODO: Note that this may be Windows-specific...need to figure out the Apple equivalent
		sAIFont->GetSystemFontName(fontKey, systemFontName, 1024);
		if (debug)
		{
			outFile << "\n" << Indent(depth) << "// Font system name: " << systemFontName;
		}

		// Determine font variant
		sAIFont->GetFontStyleName(fontKey, fontStyleName, 1024);
		if (debug)
		{
			outFile << "\n" << Indent(depth) << "// Font style name: " << fontStyleName;
		}

		// Copy to glyph state
		glyphState.fontName = std::string(systemFontName);
		glyphState.fontStyleName = std::string(fontStyleName);

		// Release memory
		free(systemFontName);
		systemFontName = nullptr;
		free(fontStyleName);
		fontStyleName = nullptr;
	}

	// Is there a vertical scale?
	glyphState.verticalScale = features.GetVerticalScale(&isAssigned);
	if (!isAssigned)
	{
		// No vertical scaling
		glyphState.verticalScale = 1.0f;
	}

	// Is there a horizontal scale?
	glyphState.horizontalScale = features.GetHorizontalScale(&isAssigned);
	if (!isAssigned)
	{
		// No horizontal scaling
		glyphState.horizontalScale = 1.0f;
	}

	// Get the matrix for this glyph run
	glyphState.glyphMatrix = glyphRun.GetMatrix();

	// Get character origin points array
	// Since we only use the first origin, no need to check array size
	// NOTE: Only use the first origin, since canvas doesn't support advanced character spacing (like Illustrator),
	//       and we choose to be programmable over being pixel-accurate. This behavior could easily be modified, however.
	ATE::IArrayRealPoint glyphOrigins = glyphRun.GetOrigins();

	// Glyph origin
	AIRealPoint glyphOrigin;

	// Get first origin offset for this glyph (no need to pull for each character)
	glyphOrigin = glyphOrigins.Item(0);

	// Apply scaling
	sAIRealMath->AIRealMatrixConcatScale(&glyphState.glyphMatrix, glyphState.horizontalScale, glyphState.verticalScale);

	// Concat by [1 0 0 -1 0 0] as text coordinates are going positive the other direction.
	AIRealMatrix flipY = 
	{
		1, 0, 0,  - 1, 0, 0
	};
	sAIRealMath->AIRealMatrixConcat(&flipY, &glyphState.glyphMatrix, &glyphState.glyphMatrix);

	// Translate the origin
	sAIRealMath->AIRealMatrixConcatTranslate(&glyphState.glyphMatrix, glyphOrigin.h, glyphOrigin.v);

	// Concatenate glyph matrix with text frame matrix
	sAIRealMath->AIRealMatrixConcat(&glyphState.glyphMatrix, &textFrameMatrix, &glyphState.glyphMatrix);

	// ATE space is application independent, and doesn't know about this Illustrator soft/hard coordinate thingy, so take care of it here
	sAIHardSoft->AIRealMatrixRealSoft(&glyphState.glyphMatrix);

	// Modify with our internal transform
	sAIRealMath->AIRealMatrixConcat(&glyphState.glyphMatrix, &currentState->internalTransform, &glyphState.glyphMatrix);

	// Is the text filled?
	glyphState.fillStyle = "";		// In case we don't have a fill style
	glyphState.textFilled = false;
	AIBoolean hasFill = features.GetFill(&isAssigned);
	if (isAssigned && hasFill)
	{
		// What color?
		ATE::IApplicationPaint ATEfillColor = features.GetFillColor(&isAssigned);
		if (isAssigned)
		{
			// We have enough information to fill the text
			glyphState.textFilled = true;

			// Get as AIColor
			sATEPaint->GetAIColor(ATEfillColor.GetRef(), &glyphState.fillColor);

			// Allocate memory for fill style value string
			GetFillStyle(glyphState.fillColor, 1.0f, glyphState.fillStyle);
		}
	}

	// Is the text stroked?
	glyphState.strokeStyle = "";		// In case we don't have a stroke style
	glyphState.textStroked = false;
	AIBoolean hasStroke = features.GetStroke(&isAssigned);
	if (isAssigned && hasStroke)
	{
		// What color?
		ATE::IApplicationPaint ATEstrokeColor = features.GetStrokeColor(&isAssigned);
		if (isAssigned)
		{
			// We have enough information to stroke the text
			glyphState.textStroked = true;

			// Get as AIColor
			sATEPaint->GetAIColor(ATEstrokeColor.GetRef(), &glyphState.strokeStyleValue.color);

			// Get stroke style
			GetFillStyle(glyphState.strokeStyleValue.color, 1.0f, glyphState.strokeStyle);

			// Stroke width
			AIReal strokeWidth = features.GetLineWidth(&isAssigned);
			if (isAssigned)
			{
				// Assign stroke width
				glyphState.strokeStyleValue.width = strokeWidth;
			}

			// Line cap
			ATE::LineCapType lineCapType = features.GetLineCap(&isAssigned);
			if (isAssigned)
			{
				// Assign line cap (NOTE: LineCapType and AILineCap enumerations are identical)
				glyphState.strokeStyleValue.cap = (AILineCap)lineCapType;
			}

			// Line join
			ATE::LineJoinType lineJoinType = features.GetLineJoin(&isAssigned);
			if (isAssigned)
			{
				// Assign line join (NOTE: LineJoinType and AILineJoin enumerations are identical)
				glyphState.strokeStyleValue.join = (AILineJoin)lineJoinType;
			}
		}
	}
}

void Canvas::ReportGlyphRunInfo(const ATE::IGlyphRun& glyphRun)
{
	// Get distance to baseline
	AIReal distanceToBaseline = glyphRun.GetDistanceToBaseline();
	if (debug)
	{
		outFile << "\n\n// Distance to baseline: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			distanceToBaseline;
	}

	// Get ascent
	AIReal ascent = glyphRun.GetAscent();
	if (debug)
	{
		outFile << "\n// Ascent: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			ascent;
	}

	// Get descent
	AIReal descent = glyphRun.GetDescent();
	if (debug)
	{
		outFile << "\n// Descent: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			descent;
	}

	// Get max cap height
	AIReal maxCapHeight = glyphRun.GetMaxCapHeight();
	if (debug)
	{
		outFile << "\n// Max cap height: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			maxCapHeight;
	}

	// Get min cap height
	AIReal minCapHeight = glyphRun.GetMinCapHeight();
	if (debug)
	{
		outFile << "\n// Min cap height: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			minCapHeight;
	}

	// Get tracking
	AIReal tracking = glyphRun.GetTracking();
	if (debug)
	{
		outFile << "\n// Tracking: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			tracking;
	}
}

void Canvas::ReportCharacterFeatures(const ATE::ICharFeatures& features)
{
	// To test for local feature assignments
	bool isAssigned = false;

	// Get horizontal scale
	AIReal horizontalScale = features.GetHorizontalScale(&isAssigned);
	if (!isAssigned)
	{
		horizontalScale = 0;
	}
	if (debug)
	{
		outFile << "\n\n// Horizontal scale: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			horizontalScale;
	}

	// Get vertical scale
	AIReal verticalScale = features.GetVerticalScale(&isAssigned);
	if (!isAssigned)
	{
		verticalScale = 0;
	}
	if (debug)
	{
		outFile << "\n// Vertical scale: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			verticalScale;
	}

	// Get leading
	AIReal leading = features.GetLeading(&isAssigned);
	if (!isAssigned)
	{
		leading = 0;
	}
	if (debug)
	{
		outFile << "\n// Leading: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			leading;
	}

	// Get tracking
	ASInt32 tracking = features.GetTracking(&isAssigned);
	if (!isAssigned)
	{
		tracking = 0;
	}
	if (debug)
	{
		outFile << "\n// Tracking: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			tracking;
	}

	// Get baseline shift
	AIReal baselineShift = features.GetBaselineShift(&isAssigned);
	if (!isAssigned)
	{
		baselineShift = 0;
	}
	if (debug)
	{
		outFile << "\n// Baseline shift: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			baselineShift;
	}

	// Get character rotation
	AIReal characterRotation = features.GetCharacterRotation(&isAssigned);
	if (!isAssigned)
	{
		characterRotation = 0;
	}
	if (debug)
	{
		outFile << "\n// Character rotation: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			characterRotation;
	}

	// Get underline offset
	AIReal underlineOffset = features.GetUnderlineOffset(&isAssigned);
	if (!isAssigned)
	{
		underlineOffset = 0;
	}
	if (debug)
	{
		outFile << "\n// Underline offset: " <<
			setiosflags(ios::fixed) << setprecision(1) <<
			underlineOffset;
	}
}

// Returns a color value string
// NOTE: Should allocate enough memory for a worst-case result ("rgba(000, 000, 000, 1.00)" + '\0') = 28 bytes
std::string Canvas::GetColor(const AIColor& color, AIReal alpha)
{
	// Convert to RGB color space
	AIColor rgbColor;
	ConvertColorToRGB(color, rgbColor);

	// Stream for color string
	std::ostringstream colorValue;

	// Output color values
	if (alpha != 1.0f)
	{
		// Include alpha
		colorValue << "\"rgba(" << (int)(rgbColor.c.rgb.red * 255.0f) <<
						   ", " << (int)(rgbColor.c.rgb.green * 255.0f) <<
						   ", " << (int)(rgbColor.c.rgb.blue * 255.0f) <<
						   ", " << setiosflags(ios::fixed) << setprecision(2) << alpha <<
						   ")\"";
	}
	else
	{
		colorValue << "\"rgb(" << (int)(rgbColor.c.rgb.red * 255.0f) <<
						   ", " << (int)(rgbColor.c.rgb.green * 255.0f) <<
						   ", " << (int)(rgbColor.c.rgb.blue * 255.0f) <<
						   ")\"";
	}
	
	// Return the color string
	return colorValue.str();
}

void Canvas::ConvertColorToRGB(const AIColor& sourceColor, AIColor& rbgColor)
{
	ai::int32 srcSpace = 0;
	ai::int32 dstSpace = kAIRGBColorSpace;
	SampleComponent srcColor[5];
	SampleComponent dstColor[5];
	ASBoolean inGamut;
	AICustomColor customColor;

	switch (sourceColor.kind)
	{
		case kGrayColor:
		{
			srcSpace = kAIGrayColorSpace;
			srcColor[0] = (SampleComponent)(1.0f - sourceColor.c.g.gray); // !!!! Why do I have to invert? Seems wrong !!!!
			break;
		}
		case kFourColor:
		{
			srcSpace = kAICMYKColorSpace;
			srcColor[0] = (SampleComponent)sourceColor.c.f.cyan;
			srcColor[1] = (SampleComponent)sourceColor.c.f.magenta;
			srcColor[2] = (SampleComponent)sourceColor.c.f.yellow;
			srcColor[3] = (SampleComponent)sourceColor.c.f.black;
			break;
		}
		case (kCustomColor):
		{
			sAICustomColor->GetCustomColor(sourceColor.c.c.color, &customColor);

			// Convert custom color (why'd they make this different!?)
			switch (customColor.kind)
			{
				case kCustomFourColor:
				{
					srcSpace = kAICMYKColorSpace;
					srcColor[0] = (SampleComponent)customColor.c.f.cyan;
					srcColor[1] = (SampleComponent)customColor.c.f.magenta;
					srcColor[2] = (SampleComponent)customColor.c.f.yellow;
					srcColor[3] = (SampleComponent)customColor.c.f.black;
					break;
				}
				case kCustomThreeColor:
				{
					// Pretty pointless :)
					srcSpace = kAIRGBColorSpace;
					srcColor[0] = (SampleComponent)customColor.c.rgb.red;
					srcColor[1] = (SampleComponent)customColor.c.rgb.green;
					srcColor[2] = (SampleComponent)customColor.c.rgb.blue;
					break;
				}
                case kCustomLabColor:
                {
                    break;
                }
			}
			break;
		}
		case kThreeColor:
		{
			// Pretty pointless :)
			srcSpace = kAIRGBColorSpace;
			srcColor[0] = (SampleComponent)sourceColor.c.rgb.red;
			srcColor[1] = (SampleComponent)sourceColor.c.rgb.green;
			srcColor[2] = (SampleComponent)sourceColor.c.rgb.blue;
			break;
		}
        case kPattern:
        case kGradient:
        case kNoneColor:
        case kAdvanceColor:
        {
            break;
        }
	}

	// Perform the color conversion
	sAIColorConversion->ConvertSampleColor(srcSpace, srcColor, dstSpace, dstColor, AIColorConvertOptions::kForExport, &inGamut);

	rbgColor.kind = kThreeColor;
	rbgColor.c.rgb.red = dstColor[0];
	rbgColor.c.rgb.green = dstColor[1];
	rbgColor.c.rgb.blue = dstColor[2];
}

void Canvas::TransformRect(AIRealRect& rect)
{
	// Transform upper-left point
	AIRealPoint upperLeftPoint;
	upperLeftPoint.h = rect.left;
	upperLeftPoint.v = rect.top;
	TransformPoint(upperLeftPoint);
	rect.left = upperLeftPoint.h;
	rect.top = upperLeftPoint.v;

	// Transform lower-right point
	AIRealPoint lowerRightPoint;
	lowerRightPoint.h = rect.right;
	lowerRightPoint.v = rect.bottom;
	TransformPoint(lowerRightPoint);
	rect.right = lowerRightPoint.h;
	rect.bottom = lowerRightPoint.v;
}

void Canvas::TransformPoint(AIRealPoint& point)
{
	// Transform point using current context and internal transform
	TransformPointWithMatrix(point, currentState->internalTransform);
}

void Canvas::TransformPointWithMatrix(AIRealPoint& point, const AIRealMatrix& matrix)
{
	// Are we processing a symbol?
	// If we're processing a symbol, we don't need to transform anything, since symbols are defined in their own coordinate space
	if (currentState->isProcessingSymbol)
	{
		// Simply harden the matrix
		sAIHardSoft->AIRealPointHarden(&point, &point);
	}
	else
	{
		sAIRealMath->AIRealMatrixXformPoint(&matrix, &point, &point);
	}
}

void Canvas::AddBreadcrumb(const std::string& artName, unsigned int depth)
{
	// Are we under the maximum breadcrumb count?
	if (breadcrumbs.size() < MAX_BREADCRUMB_DEPTH)
	{
		// Copy the string
		std::string cleanArtName = artName;

		// If this is at depth = 1, then make sure we clean any custom function names
		if (depth == 1)
		{
			// Remove parenthesis and parameters (if they exist)
			CleanFunction(cleanArtName);

			// Convert to camel-case
			CleanString(cleanArtName, true);
		}

		// Strip invalid characters from art name
		CleanString(cleanArtName, false);

		// Add clean name to breadcrumb
		breadcrumbs.push_back(cleanArtName);

		// Output path and name
		if (depth > 1)
		{
			outFile << "\n\n" << Indent(depth) << "// ";

			// Loop through breadcrumbs
			for (unsigned int i = 0; i < breadcrumbs.size(); i++)
			{
				if (i > 0)
				{
					outFile << "/";
				}
				outFile << breadcrumbs[i];
			}
		}
	}
}

void Canvas::RemoveBreadcrumb()
{
	// Remove breadcrumb
	breadcrumbs.pop_back();
}
