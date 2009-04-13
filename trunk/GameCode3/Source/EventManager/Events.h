#pragma once
//========================================================================
// Events.h : defines common game events
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
//  NOTE: This code is different than in the presentation in Chapter 11.
//        Additional methods to construct events and event data from streams have been
//        added to support sending/recieving or marshalling events across a network.
//        The network code is presented in Chapter 16.
//
//  class EvtData_Fire_Weapon		- Chapter 19, page 696
//  struct EvtData_Thrust			- Chapter 19, page 698
//  struct EvtData_Steer			- Chapter 19, page 699			

//
// TODO: 
//   - EvtData_Fire_Weapon should probably be a struct....old code from 2nd edition maybe
//

#include "EventManager.h"
#include "../GameCode.h"
#include "../Actors.h"

// Auxillary data decls ...
//
// data that is passed per-event in the userData parameter
// 
// ( for some, but not all, events )

//-- new object notification

// This event is sent out when an actor is
// *actually* created.
struct EvtData_New_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_New_Actor( ActorId id,
							  ActorParams *pCreateParams)
	{
		m_id = id;
		m_pActorParams = reinterpret_cast<ActorParams *>(GCC_NEW char[pCreateParams->m_Size]);
		memcpy(m_pActorParams, pCreateParams, pCreateParams->m_Size);
		m_pActorParams->m_Id = id;
	}

	explicit EvtData_New_Actor( std::istrstream & in )
	{
		in >> m_id;
		m_pActorParams = ActorParams::CreateFromStream( in );
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_New_Actor( m_id, m_pActorParams ) );
	}

	virtual ~EvtData_New_Actor() 
	{ 
		SAFE_DELETE(m_pActorParams); 
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "ActorId", m_id );
		
		//Create a new sub-table to hold actor params.
		LuaObject actorParamsObj = m_LuaEventData.CreateTable( "ActorParams" );
		if ( NULL != m_pActorParams )
		{
			//TODO JWC!   James what the hell is supposed to go here???
		}

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id << " ";
		m_pActorParams->VSerialize(out);
	}

	ActorId m_id;	//ID of actor created
	ActorParams *m_pActorParams;	//Parameters for actor

private:
	LuaObject	m_LuaEventData;
};

// struct EvtData_Destroy_Actor				- Chapter 10, 279

struct EvtData_Destroy_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Destroy_Actor( ActorId id )
		: m_id( id )
	{
	}

	explicit EvtData_Destroy_Actor( std::istrstream & in )
	{
		in >> m_id;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Destroy_Actor ( m_id ) );
	}

	virtual ~EvtData_Destroy_Actor() {}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "ActorId", m_id );
		
		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

	ActorId m_id;

private:
	LuaObject	m_LuaEventData;
};

struct EvtData_Move_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Move_Actor( ActorId id, const Mat4x4 & mat)
	{
		m_Id = id;
		m_Mat = mat;
	}

	explicit EvtData_Move_Actor( std::istrstream & in )
	{
		in >> m_Id;
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				in >> m_Mat.m[i][j];
			}
		}
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Now assign the data.
		m_LuaEventData.SetInteger( "ActorId", m_Id );

		//We don't want a whole 4x4 matrix, so just give us the position.
		const Vec3 srcPos = m_Mat.GetPosition(); 
		LuaObject posTable = m_LuaEventData.CreateTable( "Pos", 3 );
		posTable.SetNumber( 1, srcPos.x );
		posTable.SetNumber( 2, srcPos.y );
		posTable.SetNumber( 3, srcPos.z );

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_Id << " ";
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				out << m_Mat.m[i][j] << " ";
			}
		}
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(GCC_NEW EvtData_Move_Actor(m_Id, m_Mat));
	}

	ActorId m_Id;
	Mat4x4 m_Mat;

private:
	LuaObject	m_LuaEventData;
};

class EvtData_New_Game : public EmptyEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_New_Game()
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_New_Game( ) );
	}

	EvtData_New_Game( std::istrstream &in )
	{
	}

	EvtData_New_Game( LuaObject srcData )
	{
	}
};

class EvtData_Request_Start_Game : public EmptyEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	EvtData_Request_Start_Game()
	{
	}


	EvtData_Request_Start_Game( std::istrstream &in )
	{
	}

	EvtData_Request_Start_Game( LuaObject srcData )
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Request_Start_Game( ) );
	}
};

struct EvtData_Game_State : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Game_State( const BaseGameState gameState )
		: m_gameState( gameState )
	{
	}

	EvtData_Game_State( std::istrstream &in )
	{
		int tempVal;
		in >> tempVal;
		m_gameState = static_cast<BaseGameState>( tempVal );
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Game_State( m_gameState ) );
	}

	virtual ~EvtData_Game_State() {}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		char const * const pGSVarName = "GameState";
		switch( m_gameState )
		{
		case BGS_Initializing:
			m_LuaEventData.SetString( pGSVarName, "Initializing" );
			break;
		case BGS_LoadingGameEnvironment:
			m_LuaEventData.SetString( pGSVarName, "LoadingGameEnvironment" );
			break;
		case BGS_WaitingForPlayers:
			m_LuaEventData.SetString( pGSVarName, "WaitingForPlayers" );
			break;
		case BGS_Running:
			m_LuaEventData.SetString( pGSVarName, "Running" );
			break;
		default:
			assert( 0 && "Unknown/unsupported game state encountered!" );
			break;
		}

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		const int tempVal = static_cast< int >( m_gameState );
		out << tempVal;
	}

	BaseGameState m_gameState;

private:
	LuaObject	m_LuaEventData;
};

struct EvtData_Remote_Client : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Remote_Client( const int socketid, const int ipaddress )
		: m_socketId( socketid ), m_ipAddress(ipaddress)
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Remote_Client( m_socketId, m_ipAddress ) );
	}

	EvtData_Remote_Client( std::istrstream &in )
	{
		in >> m_socketId;
		in >> m_ipAddress;
	}

	virtual ~EvtData_Remote_Client() {}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "SocketId", m_socketId );
		m_LuaEventData.SetInteger( "IpAddress", m_ipAddress );

		
		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_socketId << " ";
		out << m_ipAddress;
	}

	int m_socketId;
	int m_ipAddress;

private:
	LuaObject	m_LuaEventData;
};

struct EvtData_Update_Tick : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Update_Tick( const int deltaMilliseconds )
		: m_DeltaMilliseconds( deltaMilliseconds )
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Update_Tick ( m_DeltaMilliseconds ) );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "DeltaMS", m_DeltaMilliseconds );
		m_LuaEventData.SetNumber( "Seconds", ( (float)m_DeltaMilliseconds / 1000.0f ) );

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		assert( 0 && "You should not be serializing update ticks!" );
	}

	int m_DeltaMilliseconds;
private:
	LuaObject m_LuaEventData;
};

struct EvtData_Debug_String : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	enum eDebugStringType
	{
		kDST_ScriptMsg,
		
		// ...Add additional types here.
	};

	explicit EvtData_Debug_String( const std::string & debugMessage, const eDebugStringType type )
		: m_DebugMessage( debugMessage )
		, m_Type( type )
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Debug_String (m_DebugMessage, m_Type) );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetString( "Text", m_DebugMessage.c_str() );
		switch( m_Type )
		{
		case kDST_ScriptMsg:
			m_LuaEventData.SetString( "Type", "Script" );
			break;
		default:
			assert( 0 && "Debug message of unknown/unsupported type encountered!" );
			break;
		}

		m_bHasLuaEventData = true;
	}

	virtual void serialize( std::ostrstream & out )
	{
		assert( 0 && "You should not be serializing debug strings!" );
	}

	std::string m_DebugMessage;
	eDebugStringType m_Type;
private:
	LuaObject m_LuaEventData;
};


struct EvtData_Network_Player_Actor_Assignment : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Network_Player_Actor_Assignment( const int actorId, const int remotePlayerId )
		: m_actorId( actorId ), m_remotePlayerId(remotePlayerId)
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		 return IEventDataPtr( GCC_NEW EvtData_Network_Player_Actor_Assignment( m_actorId, m_remotePlayerId ) ) ;
	}

	EvtData_Network_Player_Actor_Assignment( std::istrstream &in )
	{
		in >> m_actorId;
		in >> m_remotePlayerId;
	}


	virtual ~EvtData_Network_Player_Actor_Assignment() {}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "ActorId", m_actorId );
		m_LuaEventData.SetInteger( "RemotePlayerId", m_remotePlayerId );

		
		m_bHasLuaEventData = true;
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_actorId << " ";
		out << m_remotePlayerId;
	}

	int m_actorId;
	int m_remotePlayerId;

private:
	LuaObject	m_LuaEventData;
};



struct EvtData_Decompress_Request : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Decompress_Request( std::wstring zipFileName, std::string filename  )
		: m_zipFileName( zipFileName ),
		m_fileName ( filename )
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Decompress_Request ( m_zipFileName, m_fileName ) );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );
		m_bHasLuaEventData = false;
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		assert( 0 && "You should not be serializing decompression requests!" );
	}

public:
	std::wstring m_zipFileName;
	std::string m_fileName;

private:
	LuaObject m_LuaEventData;
};


struct EvtData_Decompression_Progress : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Decompression_Progress( int progress, std::wstring zipFileName, std::string filename, void *buffer )
		: m_progress(progress),
		m_zipFileName( zipFileName ),
		m_fileName ( filename ),
		m_buffer (buffer)
	{
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Decompression_Progress ( m_progress, m_zipFileName, m_fileName, m_buffer ) );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );
		m_bHasLuaEventData = false;
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		assert( 0 && "You should not be serializing decompression progress events!" );
	}

public:
	int m_progress;
	std::wstring m_zipFileName;
	std::string m_fileName;
	void *m_buffer;

private:
	LuaObject m_LuaEventData;
};


// This event is sent by a client ASKING to
// create an actor.  It can be sent from script
// or via code.
struct EvtData_Request_New_Actor : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Request_New_Actor( LuaObject srcData )
		: m_ActorDef( srcData )
		, m_pActorParams( NULL )
	{
		m_bHasLuaEventData = true;	//Our Lua event data got passed into us!
	}

	explicit EvtData_Request_New_Actor( const ActorParams *pCreateParams )
		: m_pActorParams( NULL )
	{
		m_bHasLuaEventData = false;	//We *don't* have Lua event data.
		m_pActorParams = reinterpret_cast<ActorParams *>(GCC_NEW char[pCreateParams->m_Size]);
		memcpy(m_pActorParams, pCreateParams, pCreateParams->m_Size);
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Request_New_Actor ( m_ActorDef ) );
	}

	virtual ~EvtData_Request_New_Actor()
	{
		SAFE_DELETE( m_pActorParams );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_ActorDef;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( true == m_bHasLuaEventData ) && "Already built lua event data!" );
		return;	//Already "built" when the event got created.
	}

	ActorParams * m_pActorParams;

private:
	LuaObject m_ActorDef;
};

// Nasty game dependant events here. We'll move this back into game specific TeapotEvents.h when
// Real streaming code gets written so Network.cpp doesn't have to know about all events.

class EvtData_Fire_Weapon : public BaseEventData
{
public:
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType() const
	{
		return sk_EventType;
	}

	explicit EvtData_Fire_Weapon( ActorId id )
		: m_id( id )
	{
	}

	explicit EvtData_Fire_Weapon( std::istrstream & in )
	{
		in >> m_id;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Fire_Weapon (m_id) );
	}

	virtual ~EvtData_Fire_Weapon() {}

	virtual LuaObject VGetLuaEventData() const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData()
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Serialize the data necessary.
		m_LuaEventData.SetInteger( "ActorId", m_id );
		
		m_bHasLuaEventData = true;
	}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
	}

	ActorId m_id;

private:
	LuaObject	m_LuaEventData;
};


///////////////////////////////////////////////////////////////////////////////
struct EvtData_Thrust : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Thrust( ActorId id, float throttle )
		: m_id(id), 
		  m_throttle(throttle)
	{}

	explicit EvtData_Thrust( std::istrstream & in )
	{
		in >> m_id;
		in >> m_throttle;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Thrust (m_id, m_throttle) );
	}

	virtual ~EvtData_Thrust()
	{
	}

	ActorId m_id;
	float m_throttle;

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Set the appropriate data.
		m_LuaEventData.SetInteger( "ActorId", m_id );
		m_LuaEventData.SetNumber( "Throttle", m_throttle );

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
		out << m_throttle << " ";
	}

private:
	LuaObject	m_LuaEventData;
};

///////////////////////////////////////////////////////////////////////////////
struct EvtData_Steer : public BaseEventData
{
	static const EventType sk_EventType;
	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_Steer( ActorId id, float dir)
		: m_id(id), 
		  m_dir(dir)
	{}

	explicit EvtData_Steer( std::istrstream & in )
	{
		in >> m_id;
		in >> m_dir;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Steer (m_id, m_dir) );
	}

	virtual ~EvtData_Steer()
	{
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( false == m_bHasLuaEventData ) && "Already built lua event data!" );

		//Get the global state.
		LuaState * pState = g_pApp->m_pLuaStateManager->GetGlobalState().Get();
		m_LuaEventData.AssignNewTable( pState );

		//Set appropriate data.
		m_LuaEventData.SetInteger( "ActorId", m_id );
		m_LuaEventData.SetNumber( "Dir", m_dir );

		m_bHasLuaEventData = true;
	}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_id << " ";
		out << m_dir << " ";
	}

	ActorId m_id;
	float m_dir;		// -1.0 is all the way left, 0 is straight, 1.0 is right

private:
	LuaObject	m_LuaEventData;
};



///////////////////////////////////////////////////////////////////////////////
// This event updates an actor's properties.  It is very general purpose,
// and only updates those stats requested to be altered.
// It's kind of gross, but it's useful as a generic method of altering
// parameters via script.

struct EvtData_UpdateActorParams : public BaseEventData
{
private:
	LuaObject m_LuaEventData;

public:
	static const EventType sk_EventType;
	ActorId m_ActorID;

	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	explicit EvtData_UpdateActorParams( LuaObject eventData )
		: m_LuaEventData( eventData )
		, m_ActorID( 0 )
	{
		assert( eventData.IsTable() && "UpdateActorParams event requires a valid table!" );
		LuaObject actorID = eventData[ "ActorID" ];
		assert( actorID.IsInteger() && "UpdateActorParams event requires member 'ActorID', and for it to be an integer!" );
		m_ActorID = actorID.GetInteger();
		m_bHasLuaEventData = true;	//We're just using what was passed into us.
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_UpdateActorParams(m_LuaEventData) );
	}

	virtual LuaObject VGetLuaEventData(void) const
	{
		assert( ( true == m_bHasLuaEventData ) && "Can't get lua event data because it hasn't been built yet!  Call BulidLuaEventData() first!" );
		return m_LuaEventData;
	}

	virtual void VBuildLuaEventData(void)
	{
		assert( ( true == m_bHasLuaEventData ) && "Already built lua event data!" );
		return;	//Already "built" when the event got created.
	}
};


