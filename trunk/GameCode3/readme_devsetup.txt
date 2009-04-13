=================================================
GameCode2 v2.2 Source Code - Teapot Wars


The complete source code for the book by
Mike McShaffry: Game Coding Complete 2nd Edition.
=================================================

NOTE: YOU CAN'T SKIP ANY OF THIS STUFF! ITS ALL IMPORTANT.

Out of the Box Requirements:

1. Install Microsoft DevStudio .NET 2003
2. Install DirectX 9 Summer 2005 or better from msdn.microsoft.com

Due to licensing complications, NovodeX (recently renamed by AGEIA to PhysX) is not included,
but a build target, Debug-NoPhysics is provided to allow compilation and running
without NovodeX (PhysX).

5. Optional - Install AGEIA's PhysX SDK from http://www.ageia.com/developers/downloads.html


================================
GENERAL INSTALLATION OF ALL SDKS
================================

The GameCode2 source includes Ogg-Vorbis and Boost. 
These SDKs are referenced in the proj files.
Following is information about 3rd Party SDKs for those that choose
to install them.

======================
DIRECTX 9 October 2005 Release
======================

Download from msdn.microsoft.com.
It's pretty big - a few hundred megabytes - it takes a long time.

The DirectX SDK include and library directories must be added
to the Visual Studio Tools -> Options -> Projects ->
VC++Directories settings (which is usually done by the installer).

=====================
OggVorbis - Audio SDK
=====================

If you want the latest version find it at www.vorbis.org - but check the 
GameCode web site to see if there are any compatibility problems!

================================
BOOST - C++ Template Library kit
================================

If you want the latest version find it at www.boost.org - but check the 
GameCode web site to see if there are any compatibility problems!

No compilation of the link libraries is required for use with
GameCode2, although to make complete use of Boost C++ you will want to compile it.
Follow the installation instructions to do this if you wish.


==============================
PhysX 2.3.1 - Game Physics SDK
==============================

You can obtain the current NovodeX SDK from the following website:
http://www.ageia.com/developers/downloads.html

As of February 6th, 2006, this codebase is compatible with PhysX 2.3.1, but
you must download the SDK yourself.  If you choose not to download the SDK,
you can still compile and run the code by adding DISABLE_PHYSICS to the
preprocessor defines.  This has already been done for you in the build
configuration named "NoPhysics-Debug".

After you download and install the SDK - you MUST follow these steps to get things working:
1. Copy the "PhysX SDK 2.3.1\SDKs" folder as your "GameCode\Source\3rdParty\PhysX" folder
2. Copy "PhysX SDK 2.3.1\Bin\win32\PhysXLoader.dll" to the "GameCode\Bin" directory
3. Copy "PhysX SDK 2.3.1\Bin\win32\PhysXCore.dll" to the "GameCode\Bin" directory


==============================
VISUAL STUDIO .NET 2003 SETUP
==============================

You are almost there....you need to set up your debugging environment.

1. Double click on GameCode2.sln and launch VS.NET.
2. Notice that GameCode2 has 4 projects: DXUT, GameCode2, Lang, and TeapotWars
3. Right click on TeapotWars and select "Set as Startup Project"
4. Right click on TeapotWars again, and select "Properties"
5. Make sure the "Configuration" drop down is set to "All Configurations"
6. Under "Configuration Properties" select "Debugging"
7. Look for "Working Directory" and set it to "..\..\Bin"

Ok - you are ready to go!




==============================================
CHANGE LOG
==============================================

Changes from 2.2 to 2.3
------------------------
1. Added 2005 SLN files
2. Removed 2005 warnings using pragma warning and defining a _VS2005_ macro
3. Upgraded Boost to version 1.33.1 (from 1.31.0)

Changes from 2.1 to 2.2
------------------------
1. Fixed compile problems related to AGEIA's latest PhysX SDK
2. Fixed a physics debug renderer problem - verts were being calculated incorrectly.
3. Fixed a problem with the random number generator's method to create a FLOAT randomly in a range
4. Added debug memory checks to all the DXUTIL code to help find memory leaks (see DX_NEW)
5. Fixed some build target malfunctions
6. Moved all resources from GameCode2.lib project to TeapotWars project
7. Created a new Teapot icon and got it to work (search for LoadIcon to see how this works)
8. New member pure virtuals in GameCodeApp - separates your game application class from GameCode internals
    a. VGetGameTitle() - replaces non-virtual GetGameTitle
    b. VGetGameAppDirectory() - replaces non-virtual GetGameAppDirectory()
    c. VGetIcon() - new virtual 
    These new vituals are implemented in TeapotWarsApp - and if you have been using 2.1 you'll need to do the
    same thing.
9. Added _CRTDBG_LEAK_CHECK_DF flag to the _CrtSetDbgFlag call - this ensures a memory leak dump just before the 
   game exits. The call to _CrtDumpMemoryLeaks() was not catching all the globally constructed/destructed objects.
10. Added a destructor to TeapotWarsGame class to formally destroy actors - you'll need to reimplement this same
    method in your game or you'll have leaks too!









