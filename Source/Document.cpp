// Document.cpp
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
#include "Document.h"

// Current plug-in version
#define PLUGIN_VERSION "1.5"

using namespace CanvasExport;

Document::Document(const std::string& pathName)
{
	// Initialize Document
	this->canvas = NULL;
	this->fileName = "";
	this->hasAnimation = false;

	// Parse the folder path
	ParseFolderPath(pathName);

	// Add a canvas for the primary document
	this->canvas = canvases.Add("canvas", "ctx", &resources);
}

Document::~Document()
{
	// Clear layers
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		// Remove instance
		delete layers[i];
	}
}

void Document::Render()
{
	// Document type
	outFile << "<!DOCTYPE html>";

	// Header, version information, and contact details
	#ifdef MAC_ENV
		outFile << "\n\n<!-- Created with Ai->Canvas Export Plug-In Version " << PLUGIN_VERSION << " (Mac)   -->";
	#endif 
	#ifdef WIN_ENV
	#ifdef _WIN64
		outFile << "\n\n<!-- Created with Ai->Canvas Export Plug-In Version " << PLUGIN_VERSION << " (PC/64) -->";
	#else
		outFile << "\n\n<!-- Created with Ai->Canvas Export Plug-In Version " << PLUGIN_VERSION << " (PC/32) -->";
	#endif
	#endif 
	size_t length = std::string(PLUGIN_VERSION).length();
	std::string padding = std::string(length, ' ');
	outFile <<   "\n<!-- By Mike Swanson (http://blog.mikeswanson.com/)    " << padding << "      -->\n";

	// Output header information
	outFile << "\n<html lang=\"en\">";
	outFile << "\n <head>";
	outFile << "\n  <meta charset=\"UTF-8\" />";

	// NOTE: The following meta tag is required when browsing HTML files using IE9 over an intranet
	//outFile << "\n  <meta http-equiv=\"X-UA-Compatible\" content=\"IE=9\" />";

	outFile << "\n  <title>" << fileName << "</title>";

	if (debug)
	{
		outFile << "\n\n<!--";
	}

	if (debug)
	{
		WriteArtTree();
	}

	// Scan the document for layers and layer attributes
	ScanDocument();

	// Parse the layers
	ParseLayers();

	if (debug)
	{
		outFile << "\n-->\n";
	}

	// If we have animation, link to animation JavaScript support file
	if (hasAnimation)
	{
		// Create the animation support file (if it doesn't already exist)
		CreateAnimationFile();

		// Output a reference
		outFile << "\n  <script src=\"Ai2CanvasAnimation.js\"></script>";
	}

	// Note that "type='text/javascript'" is no longer required as of HTML5, unless the language isn't javascript
	outFile << "\n  <script>";
	
	// Render the document
	RenderDocument();

	// Close script tag
	outFile << "\n  </script>";

	// Debug style information
	if (debug)
	{
		outFile << "\n  <style type=\"text/css\">";
		outFile << "\n    body {";
		outFile << "\n      font-family: Verdana, Geneva, sans-serif;";
		outFile << "\n      font-size: 12px;";
		outFile << "\n    }";
		outFile << "\n    canvas {";
		outFile << "\n      border: 1px solid grey;";
		outFile << "\n    }";
		outFile << "\n  </style>";
	}

	// End of header
	outFile << "\n </head>";

	// Body header with onLoad init function to execute
	outFile << "\n <body onload=\"init()\">";

	// Render canvases
	canvases.Render();

	// Render images
	resources.images.Render();

	// Include basic debug info
	if (debug)
	{
		DebugInfo();
	}

	// Body end
	outFile << "\n </body>";

	// End of document
	outFile << "\n</html>";
}

// Set the bounds for the primary document
void Document::SetDocumentBounds()
{
	// Set default bounds
	// Start with maximums and minimums, so any value will cause them to be set
	documentBounds.left = FLT_MAX;
	documentBounds.right = -FLT_MAX;
	documentBounds.top = -FLT_MAX;
	documentBounds.bottom = FLT_MAX;

	// Loop through all layers
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		// Does this layer crop the entire canvas?
		if (layers[i]->crop)
		{
			// This layer's bounds crop the entire canvas
			documentBounds = layers[i]->bounds;

			// No need to look through any more layers
			break;
		}
		else
		{
			// Update with layer bounds
			UpdateBounds(layers[i]->bounds, documentBounds);
		}
	}

	// Set canvas size
	canvas->width = documentBounds.right - documentBounds.left;
	canvas->height = documentBounds.top - documentBounds.bottom;
}

// Find the base folder path and filename
void Document::ParseFolderPath(const std::string& pathName)
{
	// Construct a FilePath
	ai::UnicodeString usPathName(pathName);
	ai::FilePath aiFilePath(usPathName);

	// Extract folder and file names
	resources.folderPath = aiFilePath.GetDirectory(false).as_Platform();
	fileName = aiFilePath.GetFileNameNoExt().as_Platform();
}

// Parse the layers
void Document::ParseLayers()
{
	// Loop through all layers
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		// Parse the layer name (and grab options)
		std::string name;
		std::string optionValue;
		ParseLayerName(*layers[i], name, optionValue);
		
		// Tokenize the options
		std::vector<std::string> options = Tokenize(optionValue, ";");

		Function* function = NULL;

		// Is this an animation?
		if (HasAnimationOption(options))
		{
			// Create a new animation function (will automatically make the function name unique, if necessary)
			// TODO: Fix weird cast.
			AnimationFunction* animationFunction = functions.AddAnimationFunction(name);

			// Set values
			animationFunction->artHandle = layers[i]->artHandle;
			animationFunction->canvas = canvas;

			// Set pointer
			function = animationFunction;

			// Note that this document has animations
			hasAnimation = true;
		}
		else
		{
			// This is a draw function
			// TODO: Fix weird cast.
			DrawFunction* drawFunction = functions.AddDrawFunction(name);

			// Add this layer to the function
			drawFunction->layers.push_back(layers[i]);

			// Track whether this function will be drawing with alpha, a gradient, or a pattern
			drawFunction->hasAlpha |= layers[i]->hasAlpha;
			drawFunction->hasGradients |= layers[i]->hasGradients;
			drawFunction->hasPatterns |= layers[i]->hasPatterns;

			// Set canvas
			drawFunction->canvas = canvas;

			// Set pointer
			function = drawFunction;
		}

		// Update to include new layer bounds
		UpdateBounds(layers[i]->bounds, function->bounds);

		// Set function options
		SetFunctionOptions(options, *function);
	}

	// Bind string animation funtion names to actual animation function objects
	functions.BindAnimationFunctions();

	// Bind triggers
	functions.BindTriggers();
}

bool Document::HasAnimationOption(const std::vector<std::string>& options)
{
	bool result = false;

	// Loop through all options
	for (unsigned int i = 0; i < options.size(); i++)
	{
		// Split the parameter and value
		std::vector<std::string> split = Tokenize(options[i], ":");

		// Did we get two values?
		if (split.size() == 2)
		{
			// Clean the parameter name
			std::string parameter = split[0];
			CleanParameter(parameter);
			ToLower(parameter);

			// Clean the parameter value
			std::string value = split[1];
			CleanParameter(value);
			ToLower(value);

			// Type?
			if (parameter == "type" ||
				parameter == "t")
			{
				// Is this an animation type?
				if (value == "animation" ||
					value == "a")
				{
					// Is an animation
					result = true;
					break;
				}
				else if (value == "drawing" ||
						 value == "d")
				{
					// Not an animation
					result = false;
					break;
				}
			}
		}
	}

	return result;
}

// Parses an individual layer name/options
void Document::ParseLayerName(const Layer& layer, std::string& name, std::string& optionValue)
{
	// We may need to modify the functionName
	AIBoolean hasFunctionName = false;
	name = layer.name;

	// Does the layer name end with a function syntax?
	// "a();"

	// Is the name long enough to contain function syntax?
	size_t length = name.length();
	if (length > 3)
	{
		// Does the end of the layer name contain the function syntax?
		if (name.substr(length - 2) == ");")
		{
			// Find the opening parenthesis
			size_t index = name.find_last_of('(');

			// Did we find the parenthesis?
			if (index != string::npos)
			{
				// Copy options
				optionValue = name.substr((index + 1), (length - index - 3));

				if (debug)
				{
					outFile << "\n//   Found options = " << optionValue;
				}

				// Terminate the layer name starting at the opening parenthesis
				name.erase(index);

				// Convert to camel-case
				CleanString(name, true);

				// Note that we found a function name
				hasFunctionName = true;
			}
		}
	}

	// Do we have a specified function name?
	if (!hasFunctionName)
	{
		// Assign a default function name
		name = "draw";
	}
}

// Render the document
void Document::RenderDocument()
{
	// Set document bounds
	SetDocumentBounds();

	// For animation, create global canvas and context references
	if (hasAnimation)
	{
		outFile << "\n\n    // Main canvas and context references";
		outFile <<   "\n    var " << canvas->id << ";";
		outFile <<   "\n    var " << canvas->contextName << ";";
	}

	// Render animations
	RenderAnimations();

	// Begin init function block
	outFile << "\n\n    function init() {";

	// For animation, set main canvas and context references
	if (hasAnimation)
	{
		outFile << "\n\n      // Set main canvas and context references";
		outFile <<   "\n      " << canvas->id << " = document.getElementById(\"" << canvas->id << "\");";
		outFile <<   "\n      " << canvas->contextName << " = " << canvas->id << ".getContext(\"2d\");";
	}

	// Do we need a pattern function?
	if (canvas->documentResources->patterns.HasPatterns())
	{
		outFile << "\n\n      drawPatterns();";
	}

	// Any animations to set-up?
	if (hasAnimation)
	{
		if (debug)
		{
			// Use mouse movement to debug animation
			outFile << "\n\n      // Capture mouse events for debug clock";
		    outFile <<   "\n      " << canvas->id << ".addEventListener(\"click\", setDebugClock, false);";
		    outFile <<   "\n      " << canvas->id << ".addEventListener(\"mousemove\", getMouseLocation, false);";
		}

		// Initialize animation clocks
		functions.RenderClockInit();

		// Start animation clocks
		functions.RenderClockStart();

		// Set animation timer
		outFile << "\n\n      // Set animation timer";
		outFile <<   "\n      setInterval(drawFrame, (1000 / fps));";
		outFile <<   "\n    }";

		// Include "update animations"
		outFile << "\n\n    function updateAnimations() {";

		// Tick animation clocks
		functions.RenderClockTick();

		outFile <<   "\n    }";

		// Render drawFrame function
		outFile << "\n\n    function drawFrame() {";
		outFile << "\n\n      // Update animations";
		outFile <<   "\n      updateAnimations();";
		outFile << "\n\n      // Clear canvas";
		// The following method should clear the canvas, but it causes visual glitching with Safari and Chrome
		//outFile <<   "\n      " << canvas->id << ".width = " << canvas->id << ".width;";
		outFile <<   "\n      " << canvas->contextName << ".clearRect(0, 0, " << canvas->id << ".width, " << canvas->id << ".height);";

		// Draw function calls
		functions.RenderDrawFunctionCalls(documentBounds);

		// Debug Bezier curves
		if (debug && functions.HasAnimationFunctions())
		{
			// Debug linear animation
			outFile << "\n\n      plotLinearPoints(" << canvas->contextName  << ");";
			outFile <<   "\n      plotAnchorPoints(" << canvas->contextName  << ");";
		}

		// Debug animation fps
		if (debug)
		{
			outFile << "\n\n      // Count actual fps";
			outFile <<   "\n      ++frameCount;";
			outFile <<   "\n      var now = new Date().getTime();";
			outFile <<   "\n      if (now > frameTime) {";
			outFile << "\n\n        frameTime = now + 1000;";
			outFile <<   "\n        frameReport = frameCount;";
			outFile <<   "\n        frameCount = 0;";
			outFile <<   "\n      }";
			outFile << "\n\n      // Report debug information";
			outFile <<   "\n      " << canvas->contextName << ".save();";
			outFile <<   "\n      " << canvas->contextName << ".fillStyle = \"rgb(0, 0, 255)\";";
			outFile <<   "\n      " << canvas->contextName << ".fillText(frameReport + \" fps\", 5, 10);";
			outFile <<   "\n      " << canvas->contextName << ".fillText((debug.ticks() / 1000).toFixed(1) + \" / \" + debug.timeRange.toFixed(1) + \" s\", 5, 20);";
			outFile <<   "\n      " << canvas->contextName << ".restore();";
		}

		// End function block
		outFile <<   "\n    }";
	}
	else
	{
		// No animations
		outFile << "\n\n      var " << canvas->id << " = document.getElementById(\"" << canvas->id << "\");";
		outFile <<   "\n      var " << canvas->contextName << " = " << canvas->id << ".getContext(\"2d\");";

		// Draw function calls
		functions.RenderDrawFunctionCalls(documentBounds);

		// End function block
		outFile << "\n    }";
	}

	// Render the functions/layers
	functions.RenderDrawFunctions(documentBounds);

	// Render the symbol functions
	RenderSymbolFunctions();

	// Render the pattern function
	RenderPatternFunction();
}

void Document::RenderAnimations()
{
	// Is there any animation in this document (paths or rotation?)
	if (hasAnimation)
	{
		// Output frames per second
		outFile << "\n\n    // Frames per second";
		outFile <<   "\n    var fps = 60.0;";

		if (debug)
		{
			outFile <<   "\n    var frameTime = 0;";
			outFile <<   "\n    var frameCount = 0;";
			outFile <<   "\n    var frameReport = 0;";
			outFile <<   "\n    var debug = new debugClock();";
		}
	}

	// Initialize animations
	functions.RenderAnimationFunctionInits(documentBounds);

	if (debug)
	{
		if (functions.HasAnimationFunctions())
		{
			// Add linear animation debug JavaScript
			DebugAnimationPathJS();
		}

		if (hasAnimation)
		{
			// Add debug animation clock 
			DebugClockJS();
		}
	}
}

// Set the options for a draw or animation function
void Document::SetFunctionOptions(const std::vector<std::string>& options, Function& function)
{
	// Loop through options
	for (unsigned int i = 0; i < options.size(); i++)
	{
		// Split the parameter and value
		std::vector<std::string> split = Tokenize(options[i], ":");

		// Did we get two values?
		if (split.size() == 2)
		{
			// Clean the parameter name
			std::string parameter = split[0];
			CleanParameter(parameter);
			ToLower(parameter);

			// Clean the parameter value
			std::string value = split[1];
			CleanParameter(value);
		
			// Process options based on type
			// TODO: Can we make this more OO?
			switch (function.type)
			{
				case Function::kDrawFunction:
				{
					DrawFunction& drawFunction = (DrawFunction&)function;

					// Set draw function parameter
					drawFunction.SetParameter(parameter, value);

					// Any animation?
					if (!drawFunction.animationFunctionName.empty() ||
					   (drawFunction.rotateClock.direction != AnimationClock::kNone) ||
					   (drawFunction.scaleClock.direction != AnimationClock::kNone) ||
					   (drawFunction.alphaClock.direction != AnimationClock::kNone))
					{
						// Document has animation
						hasAnimation = true;
					}

					break;
				}
				case Function::kAnimationFunction:
				{
					// Set animation function parameter
					((AnimationFunction&)function).SetParameter(parameter, value);

					// Animation
					hasAnimation = true;

					break;
				}
                case Function::kAnyFunction:
                {
                    break;
                }
			}
		}
	}
}

// Scan all visible elements in the art tree
//   Track maximum bounds of visible artwork per layer
//   Track pattern fills used by visible artwork
//   Track if gradient fills are used by visible artwork per layer
void Document::ScanDocument()
{
	AILayerHandle layerHandle = NULL;
	ai::int32 layerCount = 0;

	// How many layers in this document?
	sAILayer->CountLayers(&layerCount);

	// Loop through all layers backwards
	// We loop backwards, since the HTML5 canvas element uses a "painter model"
	for (ai::int32 i = (layerCount - 1); i > -1; i--)
	{
		// Get a reference to the layer
		sAILayer->GetNthLayer(i, &layerHandle);

		// Is the layer visible?
		AIBoolean isLayerVisible = false;
		sAILayer->GetLayerVisible(layerHandle, &isLayerVisible);
		if (debug)
		{
			outFile << "\n\n// Layer visible = " << isLayerVisible;
		}

		// Only process if the layer is visible
		if (isLayerVisible)
		{
			// Add this layer
			Layer* layer = AddLayer(layers, layerHandle);

			// Scan this layer
			ScanLayer(*layer);
		}
	}
}

void Document::ScanLayer(Layer& layer)
{
	// Get the first art in this layer
	AIArtHandle artHandle = NULL;
	sAIArt->GetFirstArtOfLayer(layer.layerHandle, &artHandle);

	// Remember artwork handle
	layer.artHandle = artHandle;

	// Scan the artwork tree and capture layer data
	ScanLayerArtwork(layer.artHandle, 1, layer);
}

// Scans a layer's artwork tree to capture important data
void Document::ScanLayerArtwork(AIArtHandle artHandle, unsigned int depth, Layer& layer)
{
	// Loop through all artwork at this depth
	do
	{
		// Is this art visible?
		AIBoolean isArtVisible = false;
		ai::int32 attr = 0;
		sAIArt->GetArtUserAttr(artHandle, kArtHidden, &attr);
		isArtVisible = !((attr &kArtHidden) == kArtHidden);

		// Only consider if art is visible
		if (isArtVisible)
		{
			// Get the art bounds
			AIRealRect artBounds;
			sAIArt->GetArtBounds(artHandle, &artBounds);

			// Update the bounds
			UpdateBounds(artBounds, layer.bounds);

			// Get type
			short type = 0;
			sAIArt->GetArtType(artHandle, &type);

			// Is this symbol art?
			if (type == kSymbolArt)
			{
				// Get the symbol pattern
				AIPatternHandle symbolPatternHandle = NULL;
				sAISymbol->GetSymbolPatternOfSymbolArt(artHandle, &symbolPatternHandle);

				// Add the symbol pattern
				bool added = canvas->documentResources->patterns.Add(symbolPatternHandle, true);

				// If we added a new pattern, scan its artwork
				if (added)
				{
					AIArtHandle patternArtHandle = NULL;
					sAIPattern->GetPatternArt(symbolPatternHandle, &patternArtHandle);

					// Look inside, but don't screw up bounds for our current layer
					Layer symbolLayer;
					ScanLayerArtwork(patternArtHandle, (depth + 1), symbolLayer);

					// Capture features for pattern
					Pattern* pattern = canvas->documentResources->patterns.Find(symbolPatternHandle);
					pattern->hasGradients = symbolLayer.hasGradients;
					pattern->hasPatterns = symbolLayer.hasPatterns;		// Can this ever happen?
					pattern->hasAlpha = symbolLayer.hasAlpha;
				}
			}
			else if (type == kPluginArt)
			{
				// Get the result art handle
				AIArtHandle resultArtHandle = NULL;
				sAIPluginGroup->GetPluginArtResultArt(artHandle, &resultArtHandle);

				// Get the first art element in the result group
				AIArtHandle childArtHandle = NULL;
				sAIArt->GetArtFirstChild(resultArtHandle, &childArtHandle);

				// Look inside the result group
				ScanLayerArtwork(childArtHandle, (depth + 1), layer);
			}

			// Get opacity
			AIReal opacity = sAIBlendStyle->GetOpacity(artHandle);
			if (opacity != 1.0f)
			{
				// Flag that this layer includes alpha/opacity changes
				layer.hasAlpha = true;
			}

			// Get the style for this artwork
			AIPathStyle style;
			AIBoolean outHasAdvFill = false;
			sAIPathStyle->GetPathStyle(artHandle, &style, &outHasAdvFill);

			// Does this artwork use a pattern fill or a gradient?
			if (style.fillPaint)
			{
				switch (style.fill.color.kind)
				{
					case kPattern:
					{
						// Add the pattern
						canvas->documentResources->patterns.Add(style.fill.color.c.p.pattern, false);

						// Flag that this layer includes patterns
						layer.hasPatterns = true;
						break;
					}
					case kGradient:
					{
						// Flag that this layer includes gradients
						layer.hasGradients = true;
						break;
					}
                    case kGrayColor:
                    case kFourColor:
                    case kCustomColor:
                    case kThreeColor:
                    case kNoneColor:
                    case kAdvanceColor:
                    {
                        break;
                    }
				}
			}

			// Does this artwork use a pattern stroke?
			if (style.strokePaint)
			{
				switch (style.stroke.color.kind)
				{
					case kPattern:
					{
						// Add the pattern
						canvas->documentResources->patterns.Add(style.stroke.color.c.p.pattern, false);

						// Flag that this layer includes patterns
						layer.hasPatterns = true;
						break;
					}
					case kGradient:
					{
						// Flag that this layer includes gradients
						layer.hasGradients = true;
						break;
					}
                    case kGrayColor:
                    case kFourColor:
                    case kCustomColor:
                    case kThreeColor:
                    case kNoneColor:
                    case kAdvanceColor:
                    {
                        break;
                    }
				}
			}

			// See if this artwork has any children
			AIArtHandle childArtHandle = NULL;
			sAIArt->GetArtFirstChild(artHandle, &childArtHandle);

			// Did we find anything?
			if (childArtHandle)
			{
				// Scan artwork at the next depth
				ScanLayerArtwork(childArtHandle, (depth + 1), layer);
			}
		}

		// Find the next sibling
		sAIArt->GetArtSibling(artHandle, &artHandle);
	}
	while (artHandle != NULL);
}

// Creates the JavaScript animation file (if it doesn't already exist)
void Document::CreateAnimationFile()
{
	// Full path to JavaScript animation support file
	std::string fullPath = resources.folderPath + "Ai2CanvasAnimation.js";

	// Ensure that the file doesn't already exist
	if (!FileExists(fullPath))
	{
		ofstream animFile;

		// Create the file
		animFile.open(fullPath.c_str(), ios::out);

		// Is the file open?
		if (animFile.is_open())
		{
			// Header
			OutputScriptHeader(animFile);

			// Clock functions
			OutputClockFunctions(animFile);

			// Animation functions
			OutputAnimationFunctions(animFile);

			// Easing functions
			OutputTimingFunctions(animFile);
		}

		// Close the file
		animFile.close();
	}
}

void Document::OutputScriptHeader(ofstream& file)
{
	file <<     "// Ai2CanvasAnimation.js Version " << PLUGIN_VERSION;
	file <<   "\n// Animation support for the Ai->Canvas Export Plug-In";
	file <<   "\n// By Mike Swanson (http://blog.mikeswanson.com/)";
}

void Document::OutputClockFunctions(ofstream& file)
{
	file << "\n\n// Create a shared standard clock";
	file <<   "\nvar timeProvider = new standardClock();";
	file << "\n\n// All animation clocks";
	file <<   "\nvar clocks = new Array();";
	file << "\n\n// Represents an animation clock";
	file <<   "\nfunction clock(duration, delay, direction, reverses, iterations, timingFunction, range, multiplier, offset) {";
	file << "\n\n  // Initialize";
	file <<   "\n  this.timeProvider = timeProvider;                 // Time provider";
	file <<   "\n  this.duration = duration;                         // Duration (in seconds)";
	file <<   "\n  this.delay = delay;                               // Initial delay (in seconds)";
	file <<   "\n  this.direction = direction;                       // Direction (-1 = backward, 1 = forward)";
	file <<   "\n  this.reverses = reverses;                         // Does this reverse? (true/false)";
	file <<   "\n  this.iterations = iterations;                     // Number of iterations (0 = infinite)";
	file <<   "\n  this.timingFunction = timingFunction;             // Timing function";
	file <<   "\n  this.multiplier = (range * multiplier);           // Value multiplier (after timing function)";
	file <<   "\n  this.offset = (range * offset);                   // Value offset (after multiplier)";
	file << "\n\n  // Reset the clock";
	file <<   "\n  this.reset = function () {";
	file << "\n\n    this.startTime = 0;                             // Start time reference";
	file <<   "\n    this.stopTime = 0;                              // Stop time reference";
	file <<   "\n    this.lastTime = 0;                              // Last time reference";
	file <<   "\n    this.baseDirection = this.direction;            // Base direction";
	file <<   "\n    this.d = this.baseDirection;                    // Current direction";
	file <<   "\n    this.t = (this.baseDirection == 1 ? 0.0 : 1.0); // Current clock time (0.0 - 1.0)";
	file <<   "\n    this.i = 0;                                     // Current iteration";
	file <<   "\n    this.isRunning = false;                         // Is this running?";
	file <<   "\n    this.isFinished = false;                        // Is the entire clock run finished?";
	file <<   "\n    this.value = 0.0;                               // Current computed clock value";
	file <<   "\n  }";
	file << "\n\n  // Reset to initial conditions";
	file <<   "\n  this.reset();";
	file << "\n\n  // Add events";
	file <<   "\n  this.started = new customEvent(\"started\");";
	file <<   "\n  this.stopped = new customEvent(\"stopped\");";
	file <<   "\n  this.iterated = new customEvent(\"iterated\");";
	file <<   "\n  this.finished = new customEvent(\"finished\");";
	file << "\n\n  // Start the clock";
	file <<   "\n  this.start = function () {";
	file << "\n\n    // Only start if the clock isn't running and it hasn't finished";
	file <<   "\n    if (!this.isRunning && !this.isFinished) {";
	file << "\n\n      // Capture start time";
	file <<   "\n      this.startTime = this.timeProvider.ticks() - (this.stopTime - this.startTime);";
	file << "\n\n      // Start the animation";
	file <<   "\n      this.isRunning = true;";
	file << "\n\n      // Started event";
	file <<   "\n      this.started.fire(null, { message: this.started.eventName });";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Re-start the clock (reset and start)";
	file <<   "\n  this.restart = function () {";
	file << "\n\n    this.reset();";
	file <<   "\n    this.start();";
	file <<   "\n  }";
	file << "\n\n  // Stop the clock";
	file <<   "\n  this.stop = function () {";
	file << "\n\n    // Only stop if the clock is running and it hasn't finished";
	file <<   "\n    if (this.isRunning && !this.isFinished) {";
	file << "\n\n      // Capture stop time";
	file <<   "\n      this.stopTime = this.timeProvider.ticks();";
	file << "\n\n      // Stop the animation";
	file <<   "\n      this.isRunning = false;";
	file << "\n\n      // Stopped event";
	file <<   "\n      this.stopped.fire(null, { message: this.stopped.eventName });";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Toggle the clock";
	file <<   "\n  this.toggle = function () {";
	file << "\n\n    // Only toggle the clock if it hasn't finished";
	file <<   "\n    if (!this.isFinished) {";
	file << "\n\n      // Is the clock running?";
	file <<   "\n      if (this.isRunning) {";
	file << "\n\n        // Stop the clock";
	file <<   "\n        this.stop();";
	file <<   "\n      }";
	file <<   "\n      else {";
	file << "\n\n        // Start the clock";
	file <<   "\n        this.start();";
	file <<   "\n      }";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Rewind the clock";
	file <<   "\n  this.rewind = function () {";
	file << "\n\n    // Only rewind if the clock is running and it hasn't finished";
	file <<   "\n    if (this.isRunning && !this.isFinished) {";
	file << "\n\n      // Rewind to the beginning of the current iteration";
	file <<   "\n      this.jumpTo(this.i);";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Fast-forward the clock";
	file <<   "\n  this.fastForward = function () {";
	file << "\n\n    // Only fast-forward if the clock is running and it hasn't finished";
	file <<   "\n    if (this.isRunning && !this.isFinished) {";
	file << "\n\n      // Fast-forward to the beginning of the next iteration";
	file <<   "\n      this.jumpTo(this.i + 1);";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Reverse the clock";
	file <<   "\n  this.reverse = function () {";
	file << "\n\n    // Only reverse if the clock is running and it hasn't finished";
	file <<   "\n    if (this.isRunning && !this.isFinished) {";
	file << "\n\n      // Reverse the clock direction";
	file <<   "\n      this.baseDirection = -this.baseDirection;";
	file << "\n\n      // Jump to the same position, but in reverse";
	file <<   "\n      var position = this.i + (this.d == -1.0 ? this.t : (1.0 - this.t));";
	file <<   "\n      this.jumpTo(position);";
	file <<   "\n    }";
	file <<   "\n  }";
	file << "\n\n  // Jump to iteration";
	file <<   "\n  this.jumpTo = function(iteration) {";
	file << "\n\n    // Determine iteration time";
	file <<   "\n    var now = this.timeProvider.ticks();";
	file <<   "\n    var ticksPerSecond = this.timeProvider.ticksPerSecond();";
	file <<   "\n    var iterationTime = (this.delay * ticksPerSecond) + ";
	file <<   "\n                        ((iteration * this.duration) * ticksPerSecond);";
	file <<   "\n    this.startTime = (now - iterationTime);";
	file <<   "\n  }";
	file << "\n\n  // Update function";
	file <<   "\n  this.update = updateClock;";
	file << "\n\n  // Set initial value";
	file <<   "\n  this.value = (this.timingFunction(this.t) * this.multiplier) + this.offset;";
	file << "\n\n  // Add to clocks array";
	file <<   "\n  clocks.push(this);";
	file <<   "\n}";
	file << "\n\n// Update clock state";
	file <<   "\nfunction updateClock() {";
	file << "\n\n  // Is clock running?";
	file <<   "\n  if (this.isRunning && !this.isFinished) {";
	file << "\n\n    // Capture the current time";
	file <<   "\n    var now = this.timeProvider.ticks();";
	file << "\n\n    // Has the time changed?";
	file <<   "\n    if (now != this.lastTime) {";
	file << "\n\n      // How many seconds have elapsed since the clock started?";
	file <<   "\n      var elapsed = (now - this.startTime) / this.timeProvider.ticksPerSecond();";
	file << "\n\n      // How many possible iterations?";
	file <<   "\n      var iterations = (elapsed - this.delay) / this.duration;";
	file << "\n\n      // Need to wait more?";
	file <<   "\n      if (iterations < 0.0) {";
	file << "\n\n        // Reset to 0";
	file <<   "\n        iterations = 0.0;";
	file <<   "\n      }";
	file << "\n\n      // Capture current iteration";
	file <<   "\n      var currentIteration = Math.floor(iterations);";
	file << "\n\n      // Iteration changed?";
	file <<   "\n      if (currentIteration != this.i) {";
	file << "\n\n        // Iterated event";
	file <<   "\n        this.iterated.fire(null, { message: this.iterated.eventName });";
	file <<   "\n      }";
	file << "\n\n      // How far \"into\" the iteration?";
	file <<   "\n      this.t = iterations - currentIteration;";
	file << "\n\n      // Is this finite?";
	file <<   "\n      if (this.iterations != 0) {";
	file << "\n\n        // Reached the limit?";
	file <<   "\n        if (currentIteration >= this.iterations) {";
	file << "\n\n          // Set to end of final iteration";
	file <<   "\n          currentIteration = this.iterations - 1;";
	file <<   "\n          this.t = 1.0;";
	file << "\n\n          // Stop clock";
	file <<   "\n          this.stop();";
	file << "\n\n          // This clock has finished";
	file <<   "\n          this.isFinished = true;";
	file << "\n\n          // Finished event";
	file <<   "\n          this.finished.fire(null, { message: this.finished.eventName });";
	file <<   "\n        }";
	file <<   "\n      }";
	file << "\n\n      // Track current iteration";
	file <<   "\n      this.i = currentIteration;";
	file << "\n\n      // Does direction ever change?";
	file <<   "\n      if (this.reverses) {";
	file << "\n\n        // Is this an even iteration? (0 is considered even)";
	file <<   "\n        if ((Math.floor(this.i) % 2) == 0) {";
	file << "\n\n          // Original direction";
	file <<   "\n          this.d = this.baseDirection;";
	file <<   "\n        }";
	file <<   "\n        else {";
	file << "\n\n          // Alternate direction";
	file <<   "\n          this.d = -this.baseDirection;";
	file <<   "\n        }";
	file <<   "\n      }";
	file <<   "\n      else {";
	file << "\n\n        // Direction doesn't change";
	file <<   "\n        this.d = this.baseDirection;";
	file <<   "\n      }";
	file << "\n\n      // Moving \"backwards\"?";
	file <<   "\n      if (this.d == -1) {";
	file << "\n\n        // Adjust \"t\"";
	file <<   "\n        this.t = (1.0 - this.t);";
	file <<   "\n      }";
	file << "\n\n      // Update current computed clock value";
	file <<   "\n      this.value = (this.timingFunction(this.t) * this.multiplier) + this.offset;";
	file << "\n\n      // Remember last time";
	file <<   "\n      this.lastTime = now;";
	file <<   "\n    }";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\n// Update all animation clocks";
	file <<   "\nfunction updateAllClocks() {";
	file << "\n\n  // Loop through clocks";
	file <<   "\n  var clockCount = clocks.length;";
	file <<   "\n  for (var i = 0; i < clockCount; i++) {";
	file << "\n\n    // Update clock";
	file <<   "\n    clocks[i].update();";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\n// Standard clock";
	file <<   "\nfunction standardClock() {";
	file << "\n\n  // Return current tick count";
	file <<   "\n  this.ticks = function() {";
	file << "\n\n    return new Date().getTime();";
	file <<   "\n  }";
	file << "\n\n  // Return number of ticks per second";
	file <<   "\n  this.ticksPerSecond = function() {";
	file << "\n\n    return 1000;";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\n// Custom event";
	file <<   "\nfunction customEvent() {";
	file << "\n\n  // Name of the event";
	file <<   "\n  this.eventName = arguments[0];";
	file << "\n\n  // Subscribers to notify on event fire";
	file <<   "\n  this.subscribers = new Array();";
	file << "\n\n  // Subscribe a function to the event";
	file <<   "\n  this.subscribe = function(fn) {";
	file << "\n\n    // Only add if the function doesn't already exist";
	file <<   "\n    if (this.subscribers.indexOf(fn) == -1) {";
	file << "\n\n      // Add the function";
	file <<   "\n      this.subscribers.push(fn);";
	file <<   "\n    }";
	file <<   "\n  };";
	file << "\n\n  // Fire the event";
	file <<   "\n  this.fire = function(sender, eventArgs) {";
	file << "\n\n    // Any subscribers?";
	file <<   "\n    if (this.subscribers.length > 0) {";
	file << "\n\n      // Loop through all subscribers";
	file <<   "\n      for (var i = 0; i < this.subscribers.length; i++) {";
	file << "\n\n        // Notify subscriber";
	file <<   "\n        this.subscribers[i](sender, eventArgs);";
	file <<   "\n      }";
	file <<   "\n    }";
	file <<   "\n  };";
	file <<   "\n};";
}

void Document::OutputAnimationFunctions(ofstream& file)
{
	// Animation path support functions
	file << "\n\n// Updates animation path";
	file <<   "\nfunction updatePath() {";
	file << "\n\n  // Reference the animation path clock";
	file <<   "\n  var clock = this.pathClock;";
	file << "\n\n  // Where is T in the linear animation?";
	file <<   "\n  var t = clock.value;";
	file << "\n\n  // Has the clock value changed?";
	file <<   "\n  if (t != this.lastValue) {";
	file << "\n\n    // Limit t";
	file <<   "\n    if (t < 0.0 || t > (this.linear.length - 1)) {";
	file << "\n\n      t = (t < 0.0) ? 0.0 : (this.linear.length - 1);";
	file <<   "\n    }";
	file <<   "\n    var tIndex = Math.floor(t);";
	file << "\n\n    // Distance between index points";
	file <<   "\n    var d = (t - tIndex);";
	file << "\n\n    // Get segment indices";
	file <<   "\n    var segment1Index = this.linear[tIndex][0];";
	file <<   "\n    var segment2Index = segment1Index;";
	file << "\n\n    // U values to interpolate between";
	file <<   "\n    var u1 = this.linear[tIndex][1];";
	file <<   "\n    var u2 = u1;";
	file << "\n\n    // Get T values";
	file <<   "\n    var t1 = this.linear[tIndex][2];";
	file <<   "\n    var t2 = t1;";
	file << "\n\n    // If in bounds, grab second segment";
	file <<   "\n    if ((tIndex + 1) < (this.linear.length))";
	file <<   "\n    {";
	file <<   "\n      var segment2Index = this.linear[(tIndex + 1)][0];";
	file <<   "\n      var u2 = this.linear[(tIndex + 1)][1];";
	file <<   "\n      var t2 = this.linear[(tIndex + 1)][2];";
	file <<   "\n    }";
	file << "\n\n    // Segment index and U value";
	file <<   "\n    var segmentIndex = segment1Index;";
	file <<   "\n    var u = 0.0;";
	file << "\n\n    // Interpolate";
	file << "\n\n    // Same segment?";
	file <<   "\n    if (segment1Index == segment2Index)";
	file <<   "\n    {";
	file <<   "\n      // Interpolate U value";
	file <<   "\n      u = (d * (u2 - u1)) + u1;";
	file <<   "\n    }";
	file <<   "\n    else";
	file <<   "\n    {";
	file << "\n\n      // Difference in T";
	file <<   "\n      var deltaT = t2 - t1;";
	file << "\n\n      // Based on distance, how \"far\" are we along T?";
	file <<   "\n      var tDistance = d * deltaT;";
	file << "\n\n      // How much segment 1 T?";
	file <<   "\n      var segment1T = (this.segmentT[segment1Index] - t1);";
	file << "\n\n      // Part of the first segment (before the anchor point)?";
	file <<   "\n      if ((t1 + tDistance) < this.segmentT[segment1Index])";
	file <<   "\n      {";
	file << "\n\n        // How far along?";
	file <<   "\n        var p = (segment1T == 0 ? 0 : tDistance / segment1T);";
	file << "\n\n        // Compute U";
	file <<   "\n        u = ((1.0 - u1) * p) + u1;";
	file <<   "\n      }";
	file <<   "\n      else";
	file <<   "\n      {";
	file <<   "\n        // Beginning of second segment";
	file <<   "\n        segmentIndex = segment2Index;";
	file << "\n\n        // How much segment 2 T?";
	file <<   "\n        var segment2T = (t2 - this.segmentT[segment1Index]);";
	file << "\n\n        // How much T remains in this segment?";
	file <<   "\n        var tRemaining = tDistance - segment1T;";
	file << "\n\n        // How far along?";
	file <<   "\n        var p = (segment2T == 0 ? 0 : tRemaining / segment2T);";
	file << "\n\n        // Compute U";
	file <<   "\n        u = p * u2;";
	file <<   "\n      }";
	file <<   "\n    }";
	file << "\n\n    // Calculate bezier curve position";
	file <<   "\n    this.x = bezier(u,";
	file <<   "\n                    this.points[segmentIndex][0][0],";
	file <<   "\n                    this.points[segmentIndex][1][0],";
	file <<   "\n                    this.points[segmentIndex][2][0],";
	file <<   "\n                    this.points[segmentIndex][3][0]);";
	file << "\n\n    this.y = bezier(u,";
	file <<   "\n                    this.points[segmentIndex][0][1],";
	file <<   "\n                    this.points[segmentIndex][1][1],";
	file <<   "\n                    this.points[segmentIndex][2][1],";
	file <<   "\n                    this.points[segmentIndex][3][1]);";
	file << "\n\n    // Determine follow orientation";
	file <<   "\n    var qx = 0.0;";
	file <<   "\n    var qy = 0.0;";
	file << "\n\n    // At a 0.0 or 1.0 boundary?";
	file <<   "\n    if (u == 0.0) {";
	file << "\n\n      // Use control point";
	file <<   "\n      qx = this.points[segmentIndex][1][0];";
	file <<   "\n      qy = this.points[segmentIndex][1][1];";
	file << "\n\n      this.orientation = followOrientation(this.x, this.y, qx, qy, clock.d);";
	file <<   "\n    }";
	file <<   "\n    else if (u == 1.0) {";
	file << "\n\n      // Use control point";
	file <<   "\n      qx = this.points[segmentIndex][1][0];";
	file <<   "\n      qy = this.points[segmentIndex][1][1];";
	file << "\n\n      this.orientation = followOrientation(qx, qy, this.x, this.y, clock.d);";
	file <<   "\n    }";
	file <<   "\n    else {";
	file << "\n\n      // Calculate quadratic curve position";
	file <<   "\n      qx = quadratic(u,";
	file <<   "\n                     this.points[segmentIndex][0][0],";
	file <<   "\n                     this.points[segmentIndex][1][0],";
	file <<   "\n                     this.points[segmentIndex][2][0]);";
	file << "\n\n      qy = quadratic(u,";
	file <<   "\n                     this.points[segmentIndex][0][1],";
	file <<   "\n                     this.points[segmentIndex][1][1],";
	file <<   "\n                     this.points[segmentIndex][2][1]);";
	file << "\n\n      this.orientation = followOrientation(qx, qy, this.x, this.y, clock.d);";
	file <<   "\n    }";
	file << "\n\n    // Remember this clock value";
	file <<   "\n    this.lastValue = t;";
	file <<   "\n  }";
	file << "\n\n  // Update clock";
	file <<   "\n  clock.update();";
	file <<   "\n}";
	file << "\n\n// Returns follow orientation";
	file <<   "\nfunction followOrientation(x1, y1, x2, y2, direction) {";
	file << "\n\n  // Forward?";
	file <<   "\n  if (direction == 1) {";
	file << "\n\n    return slope(x1, y1, x2, y2);";
	file <<   "\n  }";
	file <<   "\n  else {";
	file << "\n\n    return slope(x2, y2, x1, y1);";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\n// Returns a position along a cubic Bezier curve";
	file <<   "\nfunction bezier(u, p0, p1, p2, p3) {";
	file << "\n\n  return Math.pow(u, 3) * (p3 + 3 * (p1 - p2) - p0)";
	file <<   "\n         + 3 * Math.pow(u, 2) * (p0 - 2 * p1 + p2)";
	file <<   "\n         + 3 * u * (p1 - p0) + p0;";
	file <<   "\n}";
	file << "\n\n// Returns a position along a quadratic curve";
	file <<   "\nfunction quadratic(u, p0, p1, p2) {";
	file << "\n\n  u = Math.max(Math.min(1.0, u), 0.0);";
	file << "\n\n  return Math.pow((1.0 - u), 2) * p0 +";
	file <<   "\n         2 * u * (1.0 - u) * p1 +";
	file <<   "\n         u * u * p2;";
	file <<   "\n}";
	file << "\n\n// Returns the slope between two points";
	file <<   "\nfunction slope(x1, y1, x2, y2) {";
	file << "\n\n  var dx = (x2 - x1);";
	file <<   "\n  var dy = (y2 - y1);";
	file << "\n\n  return Math.atan2(dy, dx);";
	file <<   "\n}";
}

void Document::OutputTimingFunctions(ofstream& file)
{
	// Timing functions
	file << "\n\n// Penner timing functions";
	file <<   "\n// Based on Robert Penner's easing equations: http://www.robertpenner.com/easing/";
	file <<   "\nfunction linear(t) {";
	file <<   "\n  return t;";
	file <<   "\n}";
	file << "\n\nfunction sineEaseIn(t) {";
	file <<   "\n  return -Math.cos(t * (Math.PI/2)) + 1;";
	file <<   "\n}";
	file << "\n\nfunction sineEaseOut(t) {";
	file <<   "\n  return Math.sin(t * (Math.PI/2));";
	file <<   "\n}";
	file << "\n\nfunction sineEaseInOut(t) {";
	file <<   "\n  return -0.5 * (Math.cos(Math.PI * t) - 1);";
	file <<   "\n}";
	file << "\n\nfunction quintEaseIn(t) {";
	file <<   "\n  return t * t * t * t * t;";
	file <<   "\n}";
	file << "\n\nfunction quintEaseOut(t) {";
	file <<   "\n  t--;";
	file <<   "\n  return t * t * t * t * t + 1;";
	file <<   "\n}";
	file << "\n\nfunction quintEaseInOut(t) {";
	file <<   "\n  t /= 0.5;";
	file <<   "\n  if (t < 1) { return 0.5 * t * t * t * t * t; }";
	file <<   "\n  t -= 2;";
	file <<   "\n  return 0.5 * (t * t * t * t * t + 2);";
	file <<   "\n}";
	file << "\n\nfunction quartEaseIn(t) {";
	file <<   "\n  return t * t * t * t;";
	file <<   "\n}";
	file << "\n\nfunction quartEaseOut(t) {";
	file <<   "\n  t--;";
	file <<   "\n  return -(t * t * t * t - 1);";
	file <<   "\n}";
	file << "\n\nfunction quartEaseInOut(t) {";
	file <<   "\n  t /= 0.5;";
	file <<   "\n  if (t < 1) { return 0.5 * t * t * t * t; }";
	file <<   "\n  t -= 2;";
	file <<   "\n  return -0.5 * (t * t * t * t - 2);";
	file <<   "\n}";
	file << "\n\nfunction circEaseIn(t) {";
	file <<   "\n  return -(Math.sqrt(1 - (t * t)) - 1);";
	file <<   "\n}";
	file << "\n\nfunction circEaseOut(t) {";
	file <<   "\n  t--;";
	file <<   "\n  return Math.sqrt(1 - (t * t));";
	file <<   "\n}";
	file << "\n\nfunction circEaseInOut(t) {";
	file <<   "\n  t /= 0.5;";
	file <<   "\n  if (t < 1) { return -0.5 * (Math.sqrt(1 - t * t) - 1); }";
	file <<   "\n  t-= 2;";
	file <<   "\n  return 0.5 * (Math.sqrt(1 - t * t) + 1);";
	file <<   "\n}";
	file << "\n\nfunction quadEaseIn(t) {";
	file <<   "\n  return t * t;";
	file <<   "\n}";
	file << "\n\nfunction quadEaseOut(t) {";
	file <<   "\n  return -1.0 * t * (t - 2.0);";
	file <<   "\n}";
	file << "\n\nfunction quadEaseInOut(t) {";
	file <<   "\n  t /= 0.5;";
	file <<   "\n  if (t < 1.0) {";
	file <<   "\n    return 0.5 * t * t;";
	file <<   "\n  }";
	file <<   "\n  t--;";
	file <<   "\n  return -0.5 * (t * (t - 2.0) - 1);";
	file <<   "\n}";
	file << "\n\nfunction cubicEaseIn(t) {";
	file <<   "\n  return t * t * t;";
	file <<   "\n}";
	file << "\n\nfunction cubicEaseOut(t) {";
	file <<   "\n  t--;";
	file <<   "\n  return t * t * t + 1;";
	file <<   "\n}";
	file << "\n\nfunction cubicEaseInOut(t) {";
	file <<   "\n  t /= 0.5;";
	file <<   "\n  if (t < 1) { return 0.5 * t * t * t; }";
	file <<   "\n  t -= 2;";
	file <<   "\n  return 0.5 * (t * t * t + 2);";
	file <<   "\n}";
	file << "\n\nfunction bounceEaseOut(t) {";
	file <<   "\n  if (t < (1.0 / 2.75)) {";
	file <<   "\n    return (7.5625 * t * t);";
	file <<   "\n  } else if (t < (2 / 2.75)) {";
	file <<   "\n    t -= (1.5 / 2.75);";
	file <<   "\n    return (7.5625 * t * t + 0.75);";
	file <<   "\n  } else if (t < (2.5 / 2.75)) {";
	file <<   "\n    t -= (2.25 / 2.75);";
	file <<   "\n    return (7.5625 * t * t + 0.9375);";
	file <<   "\n  } else {";
	file <<   "\n    t -= (2.625 / 2.75);";
	file <<   "\n    return (7.5625 * t * t + 0.984375);";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\nfunction bounceEaseIn(t) {";
	file <<   "\n  return 1.0 - bounceEaseOut(1.0 - t);";
	file <<   "\n}";
	file << "\n\nfunction bounceEaseInOut(t) {";
	file <<   "\n  if (t < 0.5) {";
	file <<   "\n    return bounceEaseIn(t * 2.0) * 0.5;";
	file <<   "\n  } else {";
	file <<   "\n    return bounceEaseOut(t * 2.0 - 1.0) * 0.5 + 0.5;";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\nfunction expoEaseIn(t) {";
	file <<   "\n  return (t == 0.0) ? 0.0 : Math.pow(2.0, 10.0 * (t - 1));";
	file <<   "\n}";
	file << "\n\nfunction expoEaseOut(t) {";
	file <<   "\n  return (t == 1.0) ? 1.0 : -Math.pow(2.0, -10.0 * t) + 1.0;";
	file <<   "\n}";
	file << "\n\nfunction expoEaseInOut(t) {";
	file <<   "\n  if (t == 0) {";
	file <<   "\n    return 0.0;";
	file <<   "\n  } else if (t == 1.0) {";
	file <<   "\n    return 1.0;";
	file <<   "\n  } else if ((t / 0.5) < 1.0) {";
	file <<   "\n    t /= 0.5;";
	file <<   "\n    return 0.5 * Math.pow(2.0, 10.0 * (t - 1));";
	file <<   "\n  } else {";
	file <<   "\n    t /= 0.5;";
	file <<   "\n    return 0.5 * (-Math.pow(2.0, -10.0 * (t - 1)) + 2);";
	file <<   "\n  }";
	file <<   "\n}";
	file << "\n\n// Other timing functions";
	file << "\n\nfunction zeroStep(t) {";
	file <<   "\n  return (t <= 0.0 ? 0.0 : 1.0);";
	file << "\n\n}";
	file << "\n\nfunction halfStep(t) {";
	file <<   "\n  return (t < 0.5 ? 0.0 : 1.0);";
	file << "\n\n}";
	file << "\n\nfunction oneStep(t) {";
	file <<   "\n  return (t >= 1.0 ? 1.0 : 0.0);";
	file <<   "\n}";
	file << "\n\nfunction random(t) {";
	file <<   "\n  return Math.random();";
	file <<   "\n}";
	file << "\n\nfunction randomLimit(t) {";
	file <<   "\n  return Math.random() * t;";
	file <<   "\n}";
	file << "\n\nfunction clockTick(t) {";
	file <<   "\n  var steps = 60.0;";
	file <<   "\n  return Math.floor(t * steps) / steps;";
	file <<   "\n}";
}

void Document::RenderSymbolFunctions()
{
	// Do we have symbol functions to render?
	if (canvas->documentResources->patterns.HasSymbols())
	{
		// Loop through symbols
		for (unsigned int i = 0; i < canvas->documentResources->patterns.Patterns().size(); i++)
		{
			// Is this a symbol?
			if (canvas->documentResources->patterns.Patterns()[i]->isSymbol)
			{
				// Pointer to pattern (for convenience)
				Pattern* pattern = canvas->documentResources->patterns.Patterns()[i];

				// Begin symbol function block
				outFile << "\n\n    function " << pattern->name << "(ctx) {";

				// Need a blank line?
				if (pattern->hasAlpha || pattern->hasGradients || pattern->hasPatterns)
				{
					outFile << "\n";
				}

				// Does this draw function have alpha changes?
				if (pattern->hasAlpha)
				{
					// Grab the alpha value (so we can use it to compute new globalAlpha values during this draw function)
					outFile << "\n" << Indent(0) << "var alpha = ctx.globalAlpha;";
				}

				// Will we be encountering gradients?
				if (pattern->hasGradients)
				{
					outFile << "\n" << Indent(0) << "var gradient;";
				}

				// Will we be encountering patterns?
				// TODO: Is this even possible?
				if (pattern->hasPatterns)
				{
					outFile << "\n" << Indent(0) << "var pattern;";
				}

				// Get a handle to the pattern art
				AIArtHandle patternArtHandle = NULL;
				sAIPattern->GetPatternArt(pattern->patternHandle, &patternArtHandle);

				// While we're here, get the size of this canvas
				AIRealRect bounds;
				sAIArt->GetArtBounds(patternArtHandle, &bounds);
				if (debug)
				{
					outFile << "\n\n" << Indent(0) << "// Symbol art bounds = " <<
						"left:" << setiosflags(ios::fixed) << setprecision(1) << bounds.left <<
						", top:" << bounds.top <<
						", right:" << bounds.right <<
						", bottom:" << bounds.bottom;
				}

				// Create canvas and set size
				Canvas* canvas = new Canvas("canvas", &resources);			// No need to add it to the collection, since it doesn't represent a canvas element
				canvas->contextName = "ctx";
				canvas->width = bounds.right - bounds.left;
				canvas->height = bounds.top - bounds.bottom;
				canvas->currentState->isProcessingSymbol = true;

				// Get the first art element in the symbol
				AIArtHandle childArtHandle = NULL;
				sAIArt->GetArtFirstChild(patternArtHandle, &childArtHandle);

				// Render this sub-group
				canvas->RenderArt(childArtHandle, 1);

				// Restore remaining state
				canvas->SetContextDrawingState(1);

				// Free the canvas
				delete canvas;

				// End function block
				outFile << "\n    }";
			}
		}
	}
}

void Document::RenderPatternFunction()
{
	// Do we have pattern functions to render?
	if (canvas->documentResources->patterns.HasPatterns())
	{
		// Begin pattern function block
		outFile << "\n\n    function drawPatterns() {";

		// Loop through patterns
		for (unsigned int i = 0; i < canvas->documentResources->patterns.Patterns().size(); i++)
		{
			// Is this a pattern?
			if (!canvas->documentResources->patterns.Patterns()[i]->isSymbol)
			{
				// Allocate space for pattern name
				ai::UnicodeString patternName;

				// Pointer to pattern (for convenience)
				Pattern* pattern = canvas->documentResources->patterns.Patterns()[i];

				// Get pattern name
				sAIPattern->GetPatternName(pattern->patternHandle, patternName);
				if (debug)
				{
					outFile << "\n//   Pattern name = " << patternName.as_Platform() << " (" << pattern->patternHandle << ")";
				}

				// Create canvas ID
				std::ostringstream canvasID;
				canvasID << "pattern" << pattern->canvasIndex;

				// Create context name
				std::ostringstream contextName;
				contextName << "ctx" << pattern->canvasIndex;

				// Create canvas for this pattern
				Canvas* canvas = canvases.Add(canvasID.str(), contextName.str(), &resources);
				canvas->isHidden = true;
				canvas->currentState->isProcessingSymbol = false;

				// Render context commands
				outFile << "\n\n" << Indent(1) << "var " << canvas->id << " = document.getElementById(\"" << canvas->id << "\");";
				outFile << "\n" << Indent(1) << "var " << canvas->contextName << " = " << canvas->id << ".getContext(\"2d\");";

				// Get a handle to the pattern art
				AIArtHandle patternArtHandle = NULL;
				sAIPattern->GetPatternArt(pattern->patternHandle, &patternArtHandle);

				// While we're here, get the size of this canvas
				AIRealRect bounds;
				sAIArt->GetArtBounds(patternArtHandle, &bounds);
				if (debug)
				{
					outFile << "\n\n" << Indent(0) << "// Symbol art bounds = " <<
						"left:" << setiosflags(ios::fixed) << setprecision(1) << bounds.left <<
						", top:" << bounds.top <<
						", right:" << bounds.right <<
						", bottom:" << bounds.bottom;
				}

				// Set canvas size
				canvas->width = bounds.right - bounds.left;
				canvas->height = bounds.top - bounds.bottom;

				// If this isn't a symbol, modify the transformation
				if (!pattern->isSymbol)
				{
					// Set internal transform
					// TODO: While this works, it seems awfully convoluted
					sAIRealMath->AIRealMatrixSetIdentity(&canvas->currentState->internalTransform);
					sAIRealMath->AIRealMatrixConcatScale(&canvas->currentState->internalTransform, 1,  - 1);
					sAIRealMath->AIRealMatrixConcatTranslate(&canvas->currentState->internalTransform,  - 1 * bounds.left, bounds.top);
					sAIRealMath->AIRealMatrixConcatScale(&canvas->currentState->internalTransform, 1,  - 1);
					sAIRealMath->AIRealMatrixConcatTranslate(&canvas->currentState->internalTransform,  0, canvas->height);
				}

				// This canvas shound be hidden, since it's only used for the pattern artwork
				canvas->isHidden = true;

				// Get the first art element in the pattern
				AIArtHandle childArtHandle = NULL;
				sAIArt->GetArtFirstChild(patternArtHandle, &childArtHandle);

				// Render this sub-group
				canvas->RenderArt(childArtHandle, 1);

				// Restore remaining state
				canvas->SetContextDrawingState(1);
			}
		}

		// End function block
		outFile << "\n    }";
	}
}

void Document::DebugInfo()
{
	outFile << "\n\n<p>This document has been exported in debug mode.</p>";

	if (hasAnimation)
	{
		outFile <<   "\n<p>To scrub animations, click a Y location to set the time window, then move left/right to scrub.</p>";
	}

	resources.images.DebugInfo();

	functions.DebugInfo();
}

void Document::DebugClockJS()
{
	outFile << "\n\n    // Debug clock";
	outFile <<   "\n    function debugClock() {";
	outFile << "\n\n      // Mouse state";
	outFile <<   "\n      this.mouseX = 0;";
	outFile <<   "\n      this.mouseY = 0;";
	outFile <<   "\n      this.resetMouse = true;";
	outFile << "\n\n      // Y location on mouseDown";
	outFile <<   "\n      this.y = 0.0;";
	outFile << "\n\n      // Time range";
	outFile <<   "\n      this.timeRange = 0.0;";
	outFile << "\n\n      // Return current tick count";
	outFile <<   "\n      this.ticks = function() {";
	outFile << "\n\n        // Reset Y?    ";
	outFile <<   "\n        if (this.resetMouse) {";
	outFile << "\n\n          // Capture Y";
	outFile <<   "\n          this.y = this.mouseY;";
	outFile << "\n\n          // Update time range";
	outFile <<   "\n          this.timeRange = (this.y / " << canvas->id << ".height) * 120;";
	outFile <<   "\n          this.resetMouse = false;";
	outFile <<   "\n        }";
	outFile << "\n\n        return ((this.mouseX / " << canvas->id << ".width) * this.timeRange * 1000);";
	outFile <<   "\n      }";
	outFile << "\n\n      // Return number of ticks per second";
	outFile <<   "\n      this.ticksPerSecond = function() {";
	outFile << "\n\n        return 1000;";
	outFile <<   "\n      }";
	outFile <<   "\n    }";

	outFile << "\n\n    function setDebugClock() {";
	outFile << "\n\n      debug.resetMouse = true;";
    outFile <<   "\n    }";

	outFile << "\n\n    function getMouseLocation(e) {";
    outFile << "\n\n      debug.mouseX = e.clientX + document.body.scrollLeft +";
	outFile <<   "\n                     document.documentElement.scrollLeft - canvas.offsetLeft;";
    outFile <<   "\n      debug.mouseY = e.clientY + document.body.scrollTop +";
	outFile <<   "\n                     document.documentElement.scrollTop - canvas.offsetTop;";
    outFile <<   "\n    }";
}

void Document::DebugAnimationPathJS()
{
	outFile << "\n\n    function plotAnchorPoints(ctx) {";

	outFile << "\n\n      ctx.save();";
    outFile <<   "\n      ctx.fillStyle = \"rgb(255, 0, 0)\";";
    outFile << "\n\n      var animation;";
	outFile <<   "\n      var animationCount = animations.length;";
	outFile <<   "\n      for (var a = 0; a < animationCount; a++) {";
	outFile << "\n\n        animation = animations[a];";
	outFile << "\n\n        var pointCount = animation.points.length;";
	outFile <<   "\n        for (var i = 0; i < pointCount; i++) {";
	outFile << "\n\n          ctx.fillRect(animation.points[i][0][0] - 2, animation.points[i][0][1] - 2, 5, 5);";
    outFile <<   "\n        }";
    outFile <<   "\n      }";
	outFile << "\n\n      // Final anchor point";
	outFile <<   "\n      ctx.fillRect(animation.points[(animation.points.length - 1)][3][0] - 2,";
	outFile <<   "\n                   animation.points[(animation.points.length - 1)][3][1] - 2, 5, 5);";
	outFile << "\n\n      ctx.restore();";
	outFile <<   "\n    }";

	outFile << "\n\n    function plotLinearPoints(ctx) {";
	outFile << "\n\n      ctx.save();";
    outFile <<   "\n      ctx.fillStyle = \"rgb(0, 0, 255)\";";
	outFile << "\n\n      var animationCount = animations.length;";
	outFile <<   "\n      for (var a = 0; a < animationCount; a++) {";
	outFile << "\n\n        var animation = animations[a];";
	outFile << "\n\n        var linearCount = animation.linear.length;";
	outFile <<   "\n        for (var i = 0; i < linearCount; i++) {";
	outFile << "\n\n          var segmentIndex = animation.linear[i][0];";
    outFile <<   "\n          var u = animation.linear[i][1];";
	outFile << "\n\n          var x = bezier(u,";
	outFile <<   "\n                         animation.points[segmentIndex][0][0],";
    outFile <<   "\n                         animation.points[segmentIndex][1][0],";
	outFile <<   "\n                         animation.points[segmentIndex][2][0],";
	outFile <<   "\n                         animation.points[segmentIndex][3][0]);";
	outFile << "\n\n          var y = bezier(u,";
	outFile <<   "\n                         animation.points[segmentIndex][0][1],";
	outFile <<   "\n                         animation.points[segmentIndex][1][1],";
	outFile <<   "\n                         animation.points[segmentIndex][2][1],";
	outFile <<   "\n                         animation.points[segmentIndex][3][1]);";
	outFile << "\n\n          ctx.fillRect(x - 1, y - 1, 3, 3);";
    outFile <<   "\n        }";
    outFile <<   "\n      }";
	outFile << "\n\n      ctx.restore();";
	outFile <<   "\n    }";
}
