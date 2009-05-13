=================================================
GameCode3 v3.0 Source Code - Teapot Wars


The complete source code for the book by
Mike McShaffry: Game Coding Complete 3rd Edition.
=================================================

NOTE: YOU CAN'T SKIP ANY OF THIS STUFF! ITS ALL IMPORTANT.

Out of the Box Requirements:

1. Install Microsoft DevStudio .NET 2005
2. Install DirectX SDK (November 2008) or later from msdn.microsoft.com
3. Download the 3rdParty.zip file from http://code.google.com/p/gamecode3/downloads/list
4. Extract it to GameCode3\Source\3rdParty



================================
GENERAL INSTALLATION OF ALL SDKS
================================

The GameCode3 source uses 3rd Party open source libraries. The 3rdParty.zip file includes just 
enough for you to compile and link, but not the entire SDK. Here's where to get each one:



----------------------------------
DIRECTX SDK November 2008 Release
----------------------------------

Download from msdn.microsoft.com.
It's pretty big - a few hundred megabytes - it takes a long time.

The DirectX SDK include and library directories must be added
to the Visual Studio Tools -> Options -> Projects ->
VC++Directories settings (which is usually done by the installer).

----------------------
OggVorbis - Audio SDK
----------------------

If you want the latest version find it at www.vorbis.org - but check the 
GameCode web site to see if there are any compatibility problems!

---------------------------------
BOOST - C++ Template Library kit
---------------------------------

If you want the latest version find it at www.boost.org - but check the 
GameCode web site to see if there are any compatibility problems!

Some of the link libraries is required for use with
GameCode3. Since compiling it takes forever - the libraries you need have already
been built with VS.NET 2005 and are incldued in 3rdPary.zip.

To make complete use of Boost C++ you will want to download the entire thing and compile it.
Follow the installation instructions to do this if you wish.

-------------------
Bullet Physics SDK
-------------------

Download it from here: http://code.google.com/p/bullet/


--------
LuaPlus 
--------

Download it from http://www.luaplus.org


==============================
VISUAL STUDIO .NET 2005 SETUP
==============================

You are almost there....you need to set up your debugging environment.

1. Double click on GameCode3.sln and launch VS.NET.
2. Notice that GameCode3 has 5 projects: DXUT, GameCode3, Editor, Lang, and TeapotWars
3. Right click on TeapotWars and select "Set as Startup Project"
4. Right click on TeapotWars again, and select "Properties"
5. Under "Configuration Properties" select "Debug"
6. Look for "Working Directory" and set it to "..\..\Bin"

Ok - you are ready to go!

If you want to compile and launch the simple game editor

1. Right click on Editor and select "Set as Startup Project"
2. Hit F5 and you'll soon see the editor windows!


==============================================
CHANGE LOG
==============================================

Changes from 2.3 to 3.0
------------------------
1. Ditched PhysX for Bullet
2. Added LuaPlus scripting
3. Added a simple C# editor
4. Added a multithreaded class, CRealtimeProcess, for managing multithreaded code
5. Added some AI systems: A*, and simple AI state machines


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









