#pragma once
#ifndef __INCLUDED_SOURCE_TRIGGERSYSTEMIMPL_H
#define __INCLUDED_SOURCE_TRIGGERSYSTEMIMPL_H
//========================================================================
// EventMangerImpl.cpp : implementation side of the event system
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
//  class EventSnooper			- Chapter 10, page 280
//  class EventManager			- Chapter 10, page 287
//



#include "EventManager.h"

#include <vector>
#include <list>
#include <map>
#include <set>
#include "..\Scripting\ScriptEventListener.h"


// EventManager Description
//
// One implementation of the listener registry
//

typedef std::vector<EventListenerPtr> EventListenerList;

typedef std::vector<EventType>      EventTypeList;

class EventManager : public IEventManager
{
public:

	explicit EventManager( char const * const pName,
									   bool setAsGlobal );

	virtual ~EventManager();

	// --- primary use functions --- make it work ---
	
	// Register a listener for a specific event type, implicitly
	// the event type will be added to the known event types if
	// not already known.
	//
	// The function will return false on failure for any
	// reason. The only really anticipated failure reason is if
	// the input event type is bad ( e.g.: known-ident number
	// with different signature text, or signature text is empty
	// )
	
	virtual bool VAddListener ( EventListenerPtr const & inListener,
					   EventType const & inType );

	// Remove a listener/type pairing from the internal tables
	//
	// Returns false if the pairing was not found.
	
	virtual bool VDelListener ( EventListenerPtr const & inListener,
					   EventType const & inType );

	// Fire off event - synchronous - do it NOW kind of thing -
	// analogous to Win32 SendMessage() API.
	//
	// returns true if the event was consumed, false if not. Note
	// that it is acceptable for all event listeners to act on an
	// event and not consume it, this return signature exists to
	// allow complete propogation of that shred of information
	// from the internals of this system to outside uesrs.

	virtual bool VTrigger ( IEventData const & inEvent ) const;

	// Fire off event - asynchronous - do it WHEN the event
	// system tick() method is called, normally at a judicious
	// time during game-loop processing.
	//
	// returns true if the message was added to the processing
	// queue, false otherwise.

	virtual bool VQueueEvent ( IEventDataPtr const & inEvent );

	// This was added to allow real time events to be sent into the 
	// event manager.
	virtual bool VThreadSafeQueueEvent ( IEventDataPtr const & inEvent );

	// Find the next-available instance of the named event type
	// and remove it from the processing queue.
	//
	// This may be done up to the point that it is actively being
	// processed ...  e.g.: is safe to happen during event
	// processing itself.
	//
	// if 'allOfType' is input true, then all events of that type
	// are cleared from the input queue.
	//
	// returns true if the event was found and removed, false
	// otherwise

	virtual bool VAbortEvent ( EventType const & inType,
					  bool allOfType );

	// Allow for processing of any queued messages, optionally
	// specify a processing time limit so that the event
	// processing does not take too long. Note the danger of
	// using this artificial limiter is that all messages may not
	// in fact get processed.
	//
	// returns true if all messages ready for processing were
	// completed, false otherwise (e.g. timeout )

	virtual bool VTick ( unsigned long maxMillis );

	// --- information lookup functions ---
	
	// Validate an event type, this does NOT add it to the
	// internal registry, only verifies that it is legal (
	// e.g. either the ident number is not yet assigned, or it is
	// assigned to matching signature text, and the signature
	// text is not empty ).

	virtual bool VValidateType( EventType const & inType ) const;

	// Get the list of listeners associated with a specific event
	// type

	EventListenerList GetListenerList ( EventType const & eventType ) const;

	// Get the list of known event types

	EventTypeList GetTypeList ( void ) const;

	// Registers an event type for the particular usage desired.
	// ...for an event defined in script:
	void RegisterScriptEvent( const EventType & eventType );
	// ...for an event defined by code, *NOT* callable by script.
	void RegisterCodeOnlyEvent( const EventType & eventType );
	// ...for an event defined by code, but callable by script.  REQUIRES the event type to have a constructor taking a LuaObject.
	template< class T> void RegisterEvent( const EventType & eventType );

private:
	
	// This class holds meta data for each event type, and allows 
	// (or disallows!) creation of code-defined events from script.
	
	// class IRegisteredEvent				- Chapter 11, page 324
	class IRegisteredEvent
	{
	public:
		//Meta data about the type and usage of this event.
		enum eRegisteredEventMetaData
		{
			// Event is defined in script.
			kREMD_ScriptDefined,
			
			//Event is defined by code, and is *NOT* callable from script.
			kREMD_CodeEventOnly,

			//Event is defined by code, but is callable from script.
			kREMD_CodeEventScriptCallable,
		};

		IRegisteredEvent( const eRegisteredEventMetaData metaData )
			: m_MetaData( metaData )
		{
		}

		virtual ~IRegisteredEvent()
		{
		}

		virtual bool VTriggerEventFromScript( LuaPlus::LuaObject & srcData ) const = 0;
		virtual bool VQueueEventFromScript( LuaPlus::LuaObject & srcData ) const = 0;
		eRegisteredEventMetaData GetEventMetaData( void ) const
		{
			return m_MetaData;
		}
	private:
		const eRegisteredEventMetaData m_MetaData;
	};

	// class ScriptDefinedEvent						- Chapter 11, page 327
	//   Script defined event type.
	class ScriptDefinedEvent : public IRegisteredEvent
	{
	public:
		ScriptDefinedEvent( const EventType & eventType )
			: IRegisteredEvent( IRegisteredEvent::kREMD_ScriptDefined )
			, m_EventType( eventType )
		{
		}

		virtual bool VTriggerEventFromScript( LuaPlus::LuaObject & srcData ) const;
		virtual bool VQueueEventFromScript( LuaPlus::LuaObject & srcData ) const;

	private:
		//We need to hold onto the event type for when it gets triggered.
		const EventType m_EventType;
	};

	// class CodeOnlyDefinedEvent					- Chapter 11, page 327
	//   Code defined, but script is NOT allowed to create this event type.
	class CodeOnlyDefinedEvent : public IRegisteredEvent
	{
	public:
		explicit CodeOnlyDefinedEvent( void )
			: IRegisteredEvent( IRegisteredEvent::kREMD_CodeEventOnly )
		{
		}

		virtual bool VTriggerEventFromScript( LuaPlus::LuaObject & srcData ) const
		{
			assert( 0 && "Attempted to trigger a code-ONLY triggerable event!" );
			return false;
		}

		virtual bool VQueueEventFromScript( LuaPlus::LuaObject & srcData ) const
		{
			assert( 0 && "Attempted to queue a code-ONLY queueable event!" );
			return false;
		}
	};

	// class ScriptCallableCodeEvent			- Chapter 11, page 330
	//   Code defined, but also creatable from script.
	template < class T >
	class ScriptCallableCodeEvent : public IRegisteredEvent
	{
	public:
		explicit ScriptCallableCodeEvent( void )
			: IRegisteredEvent( IRegisteredEvent::kREMD_CodeEventScriptCallable )
		{
		}

		virtual bool VTriggerEventFromScript( LuaPlus::LuaObject & srcData ) const
		{
			const T eventData( srcData );	//Construct directly.
			return safeTriggerEvent( eventData );
		}

		virtual bool VQueueEventFromScript( LuaPlus::LuaObject & srcData ) const
		{
			IEventDataPtr eventPtr( GCC_NEW T( srcData ) );
			return safeQueEvent( eventPtr );
		}
	};		

	typedef boost::shared_ptr< IRegisteredEvent > IRegisteredEventPtr;

	//Verifies that such an event does not already exist, then registers it.
	void AddRegisteredEventType( const EventType & eventType, IRegisteredEventPtr metaData );

	// one global instance
	typedef std::map< EventType, IRegisteredEventPtr >		EventTypeSet;

	// insert result into event type set
	typedef std::pair< EventTypeSet::iterator, bool >		EventTypeSetIRes;

	// one list per event type ( stored in the map )
	typedef std::list< EventListenerPtr >					EventListenerTable;

	// mapping of event ident to listener list
	typedef std::map< unsigned int, EventListenerTable >	EventListenerMap;

	// entry in the event listener map
	typedef std::pair< unsigned int, EventListenerTable >	EventListenerMapEnt;

	// insert result into listener map
	typedef std::pair< EventListenerMap::iterator, bool >	EventListenerMapIRes;

	// queue of pending- or processing-events
	typedef std::list< IEventDataPtr >						EventQueue;

	enum eConstants
	{
		kNumQueues = 2
	};
	
	EventTypeSet     m_typeList;           // list of registered
											// event types

	EventListenerMap m_registry;           // mapping of event types
											// to listeners

	EventQueue       m_queues[kNumQueues]; // event processing queue,
											// double buffered to
											// prevent infinite cycles

	int               m_activeQueue;        // valid denoting which
											// queue is actively
											// processing, en-queing
											// events goes to the
											// opposing queue

	ThreadSafeEventQueue m_RealtimeEventQueue;

	// ALL SCRIPT-RELATED FUNCTIONS
private:
	// Registers a script-based event.
	void RegisterScriptEventType( char const * const pEventName );

	// Add/remove a script listener.  Note that we pass a Lua *function* as the
	// second parameter of each of these, and tie that function to a specific
	// event name.  We'll call the Lua function when the event type comes in.
	bool AddScriptListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction );
	bool RemoveScriptListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction );

	// Triggers an event from script.  The event data will be serialized for any
	// code listeners.  Script listeners will receive the data table passed in.
	bool TriggerEventFromScript( char const * const pEventName, LuaPlus::LuaObject luaEventData );

	// Actor-specific event listener controls.  These will pass in the actor context
	// upon calling.
	bool AddScriptActorListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction, const int actorID );
	bool RemoveScriptActorListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction, const int actorID );

	// Allows access for script calls.
	LuaPlus::LuaObject m_MetaTable;

	// Holds all allocated script listeners.
	// It maps an event ID to a set of listeners.
	typedef boost::shared_ptr< ScriptEventListener > ScriptEventListenerPtr;
	typedef std::multimap< unsigned int, ScriptEventListenerPtr > ScriptEventListenerMap;
	ScriptEventListenerMap m_ScriptEventListenerMap;

	// Holds all allocated script actor listeners.
	typedef boost::shared_ptr< ScriptActorEventListener > ScriptActorEventListenerPtr;
	typedef std::multimap< unsigned int, ScriptActorEventListenerPtr > ScriptActorEventListenerMap;
	ScriptActorEventListenerMap m_ScriptActorEventListenerMap;

	//Holds all registered script event types.
	typedef std::set< EventType > ScriptDefinedEventTypeSet;
	ScriptDefinedEventTypeSet m_ScriptDefinedEventTypeSet;
};

// EventManager::RegisterEvent						- Chapter 11, page 330
// Our templated registration function.
template<class T> void EventManager::RegisterEvent( const EventType & eventType )
{
	IRegisteredEventPtr metaData( GCC_NEW ScriptCallableCodeEvent< T >() );
	AddRegisteredEventType( eventType, metaData );
}

// Event listener used for snoooping ... simply emits event stats
// for all processed & known events to runtime log ( actually, in
// this case, debugger output )

class EventSnooper : public IEventListener
{
public:
	
	explicit EventSnooper( char const * const kpLogFileName = NULL);
	virtual ~EventSnooper();
	char const * GetName(void) { return "Snoop"; }
	bool HandleEvent( IEventData const & event );

private:

	FILE *m_OutFile;
	char m_msgBuf[4090];

};

#endif // __INCLUDED_SOURCE_TRIGGERSYSTEMIMPL_H
