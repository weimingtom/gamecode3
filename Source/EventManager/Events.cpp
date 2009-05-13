//========================================================================
// Events.cpp : defines common game events
//
// Part of the GameCode3 Application
//
// GameCode3 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 3rd Edition" by Mike McShaffry, published by
// Charles River Media. ISBN-10: 1-58450-680-6   ISBN-13: 978-1-58450-680-5
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1584506806?ie=UTF8&tag=gamecodecompl-20&linkCode=as2&camp=1789&creative=390957&creativeASIN=1584506806
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: http://gamecode3.googlecode.com/svn/trunk/
//
// (c) Copyright 2009 Michael L. McShaffry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License v2
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

//========================================================================
//  Content References in Game Coding Complete 3rd Edition
// 
#include "GameCodeStd.h"

#include "Events.h"

const EventType EvtData_New_Game::sk_EventType( "new_game" );
const EventType EvtData_New_Actor::sk_EventType( "new_actor" );
const EventType EvtData_Move_Actor::sk_EventType( "move_actor" );
const EventType EvtData_Destroy_Actor::sk_EventType( "destroy_actor" );
const EventType EvtData_Game_State::sk_EventType( "game_state" );
const EventType EvtData_Request_Start_Game::sk_EventType( "game_request_start" );
const EventType EvtData_Remote_Client::sk_EventType( "remote_client" );
const EventType EvtData_Network_Player_Actor_Assignment::sk_EventType ( "netplayer_actor_assign" );
const EventType EvtData_Update_Tick::sk_EventType( "update_tick" );
const EventType EvtData_Debug_String::sk_EventType( "debug_string" );
const EventType EvtData_Decompress_Request::sk_EventType( "decompress_request" );
const EventType EvtData_Decompression_Progress::sk_EventType( "decompression_progress" );
const EventType EvtData_Request_New_Actor::sk_EventType( "request_new_actor" );
const EventType EvtData_UpdateActorParams::sk_EventType( "update_actor_params" );

// Note: these are game specific events, which really should be in TeapotEvents.cpp.
// They aren't because of an annoying dependancy in Network.cpp, which creates events
// from a bitstream. Once we get "real" streaming code in we can move these back to TeapotEvents.cpp.
const EventType EvtData_Fire_Weapon::sk_EventType( "fire_weapon" );
const EventType EvtData_Thrust::sk_EventType( "thrust" );
const EventType EvtData_Steer::sk_EventType( "steer" );


//
// ActorParams::CreateFromStream					- Chapter 19, page 687
//
ActorParams *ActorParams::CreateFromStream(std::istrstream &in)
{
	int actorType;
	in >> actorType;

	ActorParams *actor = NULL;
	switch (actorType)
	{
		case AT_Sphere:
			actor = GCC_NEW SphereParams;
			break;

		case AT_Teapot:
			actor = GCC_NEW TeapotParams;
			break;

		case AT_TestObject:
			actor = GCC_NEW TestObjectParams;
			break;

		case AT_Grid:
			actor = GCC_NEW GridParams;
			break;

		case AT_GenericMeshObject:
			actor = GCC_NEW GenericMeshObjectParams;
			break;
		default:
			assert(0 && _T("Unimplemented actor type in stream"));
			return 0;
	}

	if (! actor->VInit(in))
	{
		// something went wrong with the serialization...
		assert(0 && _T("Error in Actor stream initialization"));
		SAFE_DELETE(actor);
	}

	return actor;

}

//
// ActorParams::CreateFromLuaObj				- Chapter 19, page 688 
//
ActorParams * ActorParams::CreateFromLuaObj( LuaPlus::LuaObject srcData )
{
	//Make sure this is legit.
	if ( false == srcData.IsTable() )
	{
		assert( 0 && "Requested to create an actor, but no table was passed with actor params!" );
		return NULL;
	}

	//Find out the actor type.
	LuaPlus::LuaObject actorTypeObj = srcData[ "ActorType" ];
	if ( false == actorTypeObj.IsString() )
	{
		assert( 0 && "Member 'ActorType' wasn't found!" );
		return NULL;
	}

	//OK, we've got a string.  Match it up with the appropriate constructor to build the data.
	const char * pActorType = actorTypeObj.GetString();
	ActorParams * pActorParams = NULL;
	if ( 0 == stricmp( pActorType, "sphere" ) )
	{
		pActorParams = GCC_NEW SphereParams();
	}
	else if ( 0 == stricmp( pActorType, "teapot" ) )
	{
		pActorParams = GCC_NEW TeapotParams();
	}
	/******
	else if ( 0 == stricmp( pActorType, "aiteapot" ) )
	{
		pActorParams = GCC_NEW AiTeapotParams();
	}
	******/
	else if ( 0 == stricmp( pActorType, "grid" ) )
	{
		pActorParams = GCC_NEW GridParams();
	}
	else if ( 0 == stricmp( pActorType, "testObject" ) )
	{
		pActorParams = GCC_NEW TestObjectParams();
	}
	else if ( 0 == stricmp( pActorType, "genericMeshObject" ) )
	{
		pActorParams = GCC_NEW GenericMeshObjectParams();
	}
	else
	{
		assert( 0 && "Unknown/unsupported member in 'ActorType' encountered!" );
		return NULL;
	}

	if ( NULL != pActorParams )
	{
		TErrorMessageList errorMessages;
		if ( false == pActorParams->VInit( srcData, errorMessages ) )
		{
			//Spit out all the error messages.
			OutputDebugStringA( "ACTOR PARAMETER ERRORS:" );
			for ( TErrorMessageList::const_iterator iter = errorMessages.begin(), end = errorMessages.end(); iter != end; ++iter )
			{
				OutputDebugStringA( "\n" );
				const std::string & error = *iter;
				OutputDebugStringA( error.c_str() );
			}
			assert( 0 && "Error in actor parameter creation from script!" );
			SAFE_DELETE( pActorParams );
		}
	}

	return pActorParams;
}
