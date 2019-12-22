// Utility.cpp
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
#include "Utility.h"

using namespace CanvasExport;

// TODO: Consider something more like: fprintf(m_fp, "\n%*sIndented by 2", 2, "");
// Or std::string(indent, ' ')
std::string CanvasExport::Indent(size_t depth)
{
	if (debug)
	{
		return std::string((depth * 2), ' ');
	}
	else
	{
		return "      ";
	}
}

bool CanvasExport::OpenFile(const std::string& filePath)
{
	// Open the file
	outFile.open(filePath.c_str(), ios::out);

	// Return result
	return outFile.is_open();
}

void CanvasExport::CloseFile()
{
	// Close the file
	outFile.close();
}

void CanvasExport::RenderTransform(const AIRealMatrix& matrix)
{
	// Transform
	outFile << setiosflags(ios::fixed) << setprecision(3) <<
		matrix.a << ", " << matrix.b << ", " << matrix.c << ", " << matrix.d << ", " << 
		setprecision(1) <<
		matrix.tx << ", " << matrix.ty;
}

// In-place replacement of one character for another
void CanvasExport::Replace(std::string& s, char find, char replace)
{
	// Loop through string
    for (unsigned int i = 0; i < s.length(); i++)
	{
		// Does this character match?
		if (s[i] == find)
		{
			// Replace the character
			s[i] = replace;
		}
    }
}

// Removes spaces and other invalid characters from the string
// camelCase == true, converts string to camel case
// TODO: Should we allow dashes and periods here? Or use a separate function to clean those?
void CanvasExport::CleanString(std::string& s, AIBoolean camelCase)
{
	int destIndex = 0;
	AIBoolean newWord = true;		// Are we processing a new word?
	AIBoolean firstLetter = true;	// Are we looking for the first letter (so we can make lower-case)?

	// Loop through the whole string
	for (unsigned int i = 0; i < s.length(); i++)
	{
		// Is this a valid character?
		if ((s[i] >= '0' && s[i] <= '9') ||
			(s[i] >= 'A' && s[i] <= 'Z') ||
			(s[i] >= 'a' && s[i] <= 'z') ||
			(s[i] == ' '))
		{
			// If we're doing camel casing
			if (camelCase)
			{
				// Do we need to watch for a new word?
				if (s[i] == ' ')
				{
					// Watching for new word
					newWord = true;
				}
				else
				{
					// Are we watching for a new word?
					if (newWord)
					{
						// Have we processed the first letter yet?
						if (firstLetter)
						{
							// Is the first letter upper-case?
							if (s[i] >= 'A' && s[i] <= 'Z')
							{
								// Make lower-case
								s[i] += 32;
							}

							// No longer watching for the first letter
							firstLetter = false;
						}
						else
						{
							// Do we have a lower-case letter we need to make upper-case?
							if (s[i] >= 'a' && s[i] <= 'z')
							{
								// Make upper-case
								s[i] -= 32;
							}
						}

						// No longer watching for a new word
						newWord = false;
					}
				}
			}

			// If we don't have a space or we're not doing camel casing
			if (s[i] != ' ' || !camelCase)
			{
				// Copy the character
				s[destIndex] = s[i];

				// Move destination index to next character
				destIndex++;
			}
		}
	}

	// Remove any remaining characters
	s.erase(destIndex);
}

// If they exist, removes parenthesis and parameters from a function name
void CanvasExport::CleanFunction(std::string& s)
{
	size_t length = s.length();
	if (length > 3)
	{
		// Does the end of the string contain function syntax?
		if (s.substr(length - 2) == ");")
		{
			// Find the opening parenthesis
			size_t index = s.find_last_of('(');

			// Did we find the parenthesis?
			if (index != string::npos)
			{
				// Terminate the name starting at the opening parenthesis
				s.erase(index);
			}
		}
	}
}

// Cleans a function parameter value
void CanvasExport::CleanParameter(std::string& s)
{
	int destIndex = 0;

	// Loop through the whole string
	for (unsigned int i = 0; i < s.length(); i++)
	{
		// Is this a valid character?
		if ((s[i] >= '0' && s[i] <= '9') ||
			(s[i] >= 'A' && s[i] <= 'Z') ||
			(s[i] >= 'a' && s[i] <= 'z') ||
			(s[i] == '-') ||
			(s[i] == '.') ||
			(s[i] == ','))
		{
			// Copy the character
			s[destIndex] = s[i];

			// Move destination index to next character
			destIndex++;
		}
	}

	// Remove any remaining characters
	s.erase(destIndex);
}

// Turn a string into a valid HTML ID
// HTML5 ID draft spec: http://dev.w3.org/html5/spec/elements.html#the-id-attribute
// Note that most references recommend using A-Z, a-z, 0-9, and should begin with an alpha character
// Technically, hyphens ("-"), underscores ("_"), colons (":"), and periods (".") are allowed, but they're known to cause issues with things like jQuery
void CanvasExport::MakeValidID(std::string& s)
{
	// First, clean the string so that it only contains camel-cased alpha-numeric data
	CleanString(s, true);

	// Check to see if the first character is numeric
	if (s[0] >= '0' && s[0] <= '9')
	{
		// String starts with a number, so arbitrarily prepend an 'a'
		s.insert(0, "a");
	}
}

void CanvasExport::ToLower(std::string& s)
{
	// Loop through the whole string
	for (unsigned int i = 0; i < s.length(); i++)
	{
		// Is the letter upper-case?
		if (s[i] >= 'A' && s[i] <= 'Z')
		{
			// Make lower-case
			s[i] += 32;
		}
	}
}

// Handy tokenize function
vector<string> CanvasExport::Tokenize(const std::string& str, const std::string& delimiters)
{
	std::vector<std::string> tokens;
    	
	// skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    	
	// find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    	while (std::string::npos != pos || std::string::npos != lastPos)
    	{
        	// found a token, add it to the vector.
        	tokens.push_back(str.substr(lastPos, pos - lastPos));
		
        	// skip delimiters.  Note the "not_of"
        	lastPos = str.find_first_not_of(delimiters, pos);
		
        	// find next "non-delimiter"
        	pos = str.find_first_of(delimiters, lastPos);
    	}

	return tokens;
}

bool CanvasExport::FileExists(const std::string& fileName)
{
	ai::UnicodeString aiFileName(fileName);
	ai::FilePath filePath(aiFileName);

	return filePath.Exists(true);
}

// Update bounds to include newBounds
// TODO: Is there an Illustrator function to do this?
void CanvasExport::UpdateBounds(const AIRealRect& newBounds, AIRealRect& bounds)
{
	// Update minimums and maximums
	if (newBounds.top > bounds.top)
	{
		bounds.top = newBounds.top;
	}
	if (newBounds.left < bounds.left)
	{
		bounds.left = newBounds.left;
	}
	if (newBounds.bottom < bounds.bottom)
	{
		bounds.bottom = newBounds.bottom;
	}
	if (newBounds.right > bounds.right)
	{
		bounds.right = newBounds.right;
	}
}

// Find a unique filename
// Path should have a trailing backslash ("c:\output\"), and extension should include a period (".png")
std::string CanvasExport::GetUniqueFileName(const std::string& path, const std::string& fileName, const std::string& extension)
{
	// Find a unique filename
	std::ostringstream uniqueFileName;

	// Find a unique file name
	int unique = 0;
	do
	{
		// Increment to make unique name
		unique++;

		// Generate a unique file name
		uniqueFileName.str("");
		uniqueFileName << fileName << unique << extension;
	} while (FileExists((path + uniqueFileName.str())));

	// Return unique file name
	return uniqueFileName.str();
}

void CanvasExport::WriteArtTree()
{
	AILayerHandle layerHandle = nullptr;
	ai::int32 layerCount = 0;

	// How many layers in this document?
	sAILayer->CountLayers(&layerCount);

	// Loop through all layers
	for (ai::int32 i = 0; i < layerCount; i++)
	{
		// Get a reference to the layer
		sAILayer->GetNthLayer(i, &layerHandle);

		// Get the first art in this layer
		AIArtHandle artHandle = nullptr;
		sAIArt->GetFirstArtOfLayer(layerHandle, &artHandle);

		// Dig in
		WriteArtTree(artHandle, 0);
	}
}

void CanvasExport::WriteArtTree(AIArtHandle artHandle, int depth)
{
	// Simple way to describe art types for debugging purposes
	static const char *artTypes[] = 
	{
		"kUnknownArt", "kGroupArt", "kPathArt", "kCompoundPathArt", "kTextArtUnsupported", "kTextPathArtUnsupported", "kTextRunArtUnsupported", "kPlacedArt", "kMysteryPathArt", "kRasterArt", "kPluginArt", "kMeshArt", "kTextFrameArt", "kSymbolArt", "kForeignArt", "kLegacyTextArt"
	};

	// Loop through art and its siblings
	do
	{
		// Art type
		short type = 0;
		sAIArt->GetArtType(artHandle, &type);
		outFile << "\n//" << Indent(depth) << std::string(artTypes[type]) << " (" << type << ")";

		// Get art name
		ai::UnicodeString artName;
		AIBoolean isDefaultName = false;
		sAIArt->GetArtName(artHandle, artName, &isDefaultName);
		outFile << ": " << artName.as_Platform();

		// Any children?
		AIArtHandle childArtHandle;
		sAIArt->GetArtFirstChild(artHandle, &childArtHandle);
		if (childArtHandle != nullptr)
		{
			WriteArtTree(childArtHandle, depth + 1);
		}

		// Find the next sibling
		sAIArt->GetArtSibling(artHandle, &artHandle);
	}
	while (artHandle != nullptr);
}
