# Ai2Canvas #

By [Mike Swanson](http://blog.mikeswanson.com/)

The Ai->Canvas plug-in enables Adobe Illustrator to export vector and bitmap artwork directly to an HTML5 canvas element using JavaScript drawing commands. Animation can be added to control rotation, scaling, opacity, and motion along a path. Then, events can be used to trigger other animations. Finally, the exported HTML and JavaScript can be extended and used in applications running on the latest versions of Internet Explorer, Firefox, Chrome, Safari, and Opera.

This brief video (while a bit dated) provides an overview of the plug-in’s functionality.

[![Ai2Canvas Overview](http://img.youtube.com/vi/L1W9AyK2MPc/0.jpg)](http://www.youtube.com/watch?v=L1W9AyK2MPc)

## Requirements ##

This repository includes both a Visual Studio 2019 solution for PC and a Xcode 12.5 project for Mac. It also requires the Adobe Illustrator CC 2021 SDK.

## Getting Started ##

Because the project depends on the Illustrator SDK and references many files using relative paths, it is important to place the project in the correct location.

1. Download and extract the Adobe Illustrator CC 2021 SDK for PC or Mac from the [Adobe Developer Console](https://console.adobe.io/).

2. From the _Adobe Illustrator CC 2021 SDK/samplecode_ folder:

			git clone https://github.com/mikeswanson/Ai2Canvas.git

3. You can now open and build the Visual Studio solution or the Xcode project. Output can be found in the _Adobe Illustrator CC 2021 SDK/samplecode/output_ folder.

If you decide to move the project, you will need to update the many relevant paths. As a historical note, Ai->Canvas started its life based on an older version of Adobe's _TextFileFormat_ sample, and it was easiest to create the new project in a parallel folder to keep the relative references intact.

## Documentation ##

For more detail about how the plug-in works along with a full tutorial and extended documentation, visit the [Ai->Canvas Plug-In for Adobe Illustrator](http://blog.mikeswanson.com/ai2canvas) project page on my blog.
