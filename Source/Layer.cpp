// Layer.cpp
//
// Copyright (c) 2010-2021 Mike Swanson (http://blog.mikeswanson.com)
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
#include "Layer.h"

using namespace CanvasExport;

Layer::Layer()
{
	// Initialize Layer
	this->name = "";
	this->layerHandle = nullptr;
	this->artHandle = nullptr;
	this->hasGradients = false;
	this->hasPatterns = false;
	this->hasAlpha = false;
	this->crop = false;

	// Initialize bounds
	// Start with absolute maximums and minimums (these will be "trimmed")
	this->bounds.left = FLT_MAX;
	this->bounds.right = -FLT_MAX;
	this->bounds.top = -FLT_MAX;
	this->bounds.bottom = FLT_MAX;
}

Layer::~Layer()
{
}

// ******************** GLOBAL FUNCTIONS ********************

// Add a new layer
Layer* CanvasExport::AddLayer(std::vector<Layer*>& layers, const AILayerHandle& layerHandle)
{
	// Get layer name
	ai::UnicodeString layerName;
	sAILayer->GetLayerTitle(layerHandle, layerName);
	if (debug)
	{
		outFile << "\n//   Layer name = " << layerName.as_Platform();
	}

	// Create a new layer
	Layer* layer = new Layer();
	
	// Set values
	layer->layerHandle = layerHandle;
	layer->name = layerName.as_Platform();

	// Add to document
	layers.push_back(layer);

	// Return result
	return layer;
}
