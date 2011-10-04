//========================================================================
// EventManagerImpl.cpp : implementation side of the event system
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
#pragma once

#include "GameCodeStd.h"

#include "../GameCode.h"
#include "EventManagerImpl.h"

// EventManager

//
// EventManager::EventManager		 - Chapter 10, page 290
//
EventManager::EventManager(
	char const * const pName,
	bool setAsGlobal )
	: IEventManager( pName, setAsGlobal ),
	  m_activeQueue(0)
{
	//Open up access to script.
	{
		// Note that this is slightly different than in the book, since the
		// Lua chapter was AFTER the event manager chapter.

		//Create our metatable...
		m_MetaTable = g_pApp->m_pLuaStateManager->GetGlobalState()->GetGlobals().CreateTable("EventManager");
		m_MetaTable.SetObject("__index", m_MetaTable);
		
		m_MetaTable.RegisterObjectDirect( "TriggerEvent", (EventManager *)0, &EventManager::TriggerEventFromScript );
		m_MetaTable.RegisterObjectDirect( "RegisterEventType", (EventManager *)0, &EventManager::RegisterScriptEventType );
		m_MetaTable.RegisterObjectDirect( "AddScriptListener", (EventManager *)0, &EventManager::AddScriptListener );
		m_MetaTable.RegisterObjectDirect( "RemoveScriptListener", (EventManager *)0, &EventManager::RemoveScriptListener );
		m_MetaTable.RegisterObjectDirect( "AddScriptActorListener", (EventManager *)0, &EventManager::AddScriptActorListener );
		m_MetaTable.RegisterObjectDirect( "RemoveScriptActorListener", (EventManager *)0, &EventManager::RemoveScriptActorListener );
		
		LuaPlus::LuaObject luaStateManObj = g_pApp->m_pLuaStateManager->GetGlobalState()->BoxPointer(this);
		luaStateManObj.SetMetaTable(m_MetaTable);
		g_pApp->m_pLuaStateManager->GetGlobalState()->GetGlobals().SetObject("EventManager", luaStateManObj);
	}
}


//
// EventManager::~EventManager		 - Chapter 10, page 290
//
EventManager::~EventManager()
{
	m_activeQueue = 0;
}

//
// EventManager::VAddListener		 - Chapter 10, page 290
//
// Register a listener for a specific event type, implicitly the
// event type will be added to the known event types if not
// already known.
//
// The function will return false on failure for any reason. The
// only really anticipated failure reason is if the input event
// type is bad ( e.g.: known-ident number with different
// signature text, or signature text is empty )
	
bool EventManager::VAddListener (
	EventListenerPtr const & inListener,
	EventType const & inType )
{
	if ( ! VValidateType( inType ) )
		return false;

	// check / update type list
	
	EventTypeSet::iterator evIt = m_typeList.find( inType );

	// find listener map entry, create one if no table already
	// exists for this entry ...

	EventListenerMap::iterator elmIt =
		m_registry.find( inType.getHashValue() );

	if ( elmIt == m_registry.end() )
	{
		EventListenerMapIRes elmIRes = m_registry.insert(
			EventListenerMapEnt( inType.getHashValue(),
								  EventListenerTable() ) );

		// whoops, could not insert into map!?!?
		if ( elmIRes.second == false )
			return false;

		// should not be possible, how did we insert and create
		// an empty table!?!?!
		if ( elmIRes.first == m_registry.end() )
			return false;

		// store it so we can update the mapped list next ...
		elmIt = elmIRes.first;
	}

	// update the mapped list of listeners, walk the existing
	// list (if any entries) to prevent duplicate addition of
	// listeners. This is a bit more costly at registration time
	// but will prevent the hard-to-notice duplicate event
	// propogation sequences that would happen if double-entries
	// were allowed.

	// note: use reference to make following code more simple
	EventListenerTable & evlTable = (*elmIt).second;
	
	for ( EventListenerTable::iterator it = evlTable.begin(),
			  itEnd = evlTable.end(); it != itEnd ; it++ )
	{
		bool bListenerMatch = ( *it == inListener );

		if ( bListenerMatch )
			return false;
	}

	// okay, event type validated, event listener validated,
	// event listener not already in map, add it

	evlTable.push_back( inListener );

	return true;
}


//
// EventManager::VDelListener		- Chapter 10, page 292
//
// Remove a listener/type pairing from the internal tables
//
// Returns false if the pairing was not found.
//
bool EventManager::VDelListener (
	EventListenerPtr const & inListener, EventType const & inType )
{
	if ( ! VValidateType( inType ) )
		return false;

	bool rc = false;

	// brute force method, iterate through all existing mapping
	// entries looking for the matching listener and remove it.

	for ( EventListenerMap::iterator it = m_registry.begin(),
			  itEnd = m_registry.end(); it != itEnd; it++ )
	{
		unsigned int const    kEventId = it->first;
		EventListenerTable & table    = it->second;

		for ( EventListenerTable::iterator it2 = table.begin(),
				  it2End = table.end(); it2 != it2End; it2++ )
		{
			if ( *it2 == inListener )
			{
				// found match, remove from table,
				table.erase( it2 );

				// update return code
				rc = true;

				// and early-quit the inner loop as addListener()
				// code ensures that each listener can only
				// appear in one event's processing list once.
				break;
			}
		}
	}
	
	return rc;
}


//
// EventManager::VTrigger		- Chapter 10, page 293
//
// Fire off event - synchronous - do it NOW kind of thing -
// analogous to Win32 SendMessage() API.
//
// returns true if the event was consumed, false if not. Note
// that it is acceptable for all event listeners to act on an
// event and not consume it, this return signature exists to
// allow complete propogation of that shred of information from
// the internals of this system to outside uesrs.
//
bool EventManager::VTrigger (
	IEventData const & inEvent ) const
{
	if ( ! VValidateType( inEvent.VGetEventType() ) )
		return false;

	EventListenerMap::const_iterator itWC = m_registry.find( 0 );

	if ( itWC != m_registry.end() )
	{
		EventListenerTable const & table = itWC->second;

		bool processed = false;
	
		for ( EventListenerTable::const_iterator it2 = table.begin(),
				  it2End = table.end(); it2 != it2End; it2++ )
		{
			(*it2)->HandleEvent( inEvent );
		}
	}
	
	EventListenerMap::const_iterator it =
		m_registry.find( inEvent.VGetEventType().getHashValue() );

	if ( it == m_registry.end() )
		return false;

	EventListenerTable const & table = it->second;

	bool processed = false;
	
	for ( EventListenerTable::const_iterator it2 = table.begin(),
			  it2End = table.end(); it2 != it2End; it2++ )
	{
		EventListenerPtr listener = *it2;
		if ( listener->HandleEvent( inEvent ) )
		{
			// only set to true, if processing eats the messages
			processed = true;
		}
	}
	
	return processed;
}

//
// EventManager::VQueueEvent			- Chapter 10, page 294
//
// Fire off event - asynchronous - do it WHEN the event system
// tick() method is called, normally at a judicious time during
// game-loop processing.
//
// returns true if the message was added to the processing queue,
// false otherwise.
//
bool EventManager::VQueueEvent ( IEventDataPtr const & inEvent )
{
	assert ( m_activeQueue >= 0 );
	assert ( m_activeQueue < kNumQueues );

	if ( ! VValidateType( inEvent->VGetEventType() ) )
		return false;

	EventListenerMap::const_iterator it =
		m_registry.find( inEvent->VGetEventType().getHashValue() );

	if ( it == m_registry.end() )
	{		
		// if global listener is not active, then abort queue add
		EventListenerMap::const_iterator itWC = m_registry.find( 0 );

		if ( itWC == m_registry.end() )
		{
			// no listeners for this event, skipit
			return false;
		}
	}
	
	m_queues[m_activeQueue].push_back( inEvent );
	
	return true;
}


// EventManager::VThreadSafeQueueEvent			- Chapter 18, page 671

bool EventManager::VThreadSafeQueueEvent ( IEventDataPtr const & inEvent )
{
	m_RealtimeEventQueue.push(inEvent);
	return true;
}

//
// EventManager::VAbortEvent			- Chapter 10, page 295
//
// Find the next-available instance of the named event type and
// remove it from the processing queue.
//
// This may be done up to the point that it is actively being
// processed ...  e.g.: is safe to happen during event processing
// itself.
//
// returns true if the event was found and removed, false
// otherwise
//
bool EventManager::VAbortEvent ( EventType const & inType,
											bool allOfType )
{
	assert ( m_activeQueue >= 0 );
	assert ( m_activeQueue < kNumQueues );

	if ( ! VValidateType( inType ) )
		return false;

	EventListenerMap::iterator it = m_registry.find( inType.getHashValue() );

	if ( it == m_registry.end() )
		return false; // no listeners for this event, skipit

	bool rc = false;

	// See a good discussion on this code here:
	// http://www.mcshaffry.com/GameCode/thread.php?threadid=517&sid=

	EventQueue &evtQueue = m_queues[m_activeQueue];

	for ( EventQueue::iterator it = evtQueue.begin(),
			  itEnd = evtQueue.end(); it != itEnd; it++ )
	{
		if ( (*it)->VGetEventType() == inType )
		{
			it = evtQueue.erase(it);
			rc = true;
			if ( !allOfType )
				break;
		}
		else
		{
			++it;
		}
	}

	return rc;
}


//
// EventManager::VTick			- Chapter 10, page 296
// EventManager::VTick			- Chapter 18, page 671  (for the threadsafe stuff)								
//
// Allow for processing of any queued messages, optionally
// specify a processing time limit so that the event processing
// does not take too long. Note the danger of using this
// artificial limiter is that all messages may not in fact get
// processed.
//
// returns true if all messages ready for processing were
// completed, false otherwise (e.g. timeout )
//
bool EventManager::VTick ( unsigned long maxMillis )
{
	unsigned long curMs = GetTickCount();
	unsigned long maxMs =
		maxMillis == IEventManager::kINFINITE
		? IEventManager::kINFINITE
		: (curMs + maxMillis );

	EventListenerMap::const_iterator itWC = m_registry.find( 0 );

	// This section added to handle events from other threads
	// Check out Chapter 18
	// --------------------------------------------------------
	IEventDataPtr rte;

	while (m_RealtimeEventQueue.try_pop(rte))
	{
		VQueueEvent(rte);

		curMs = GetTickCount();
		if ( maxMillis != IEventManager::kINFINITE )
		{

			if ( curMs >= maxMs )
			{
				assert(0 && "A realtime process is spamming the event manager!");
			}
		}
	}

	// --------------------------------------------------------

	// swap active queues, make sure new queue is empty after the
	// swap ...
	
	int queueToProcess = m_activeQueue;

	m_activeQueue = ( m_activeQueue + 1 ) % kNumQueues;

	m_queues[m_activeQueue].clear();

	// now process as many events as we can ( possibly time
	// limited ) ... always do AT LEAST one event, if ANY are
	// available ...

	while ( m_queues[queueToProcess].size() > 0 )
	{
		IEventDataPtr event = m_queues[queueToProcess].front();

		m_queues[queueToProcess].pop_front();
		
		EventType const & eventType = event->VGetEventType();

		EventListenerMap::const_iterator itListeners =
			m_registry.find( eventType.getHashValue() );

		if ( itWC != m_registry.end() )
		{
			EventListenerTable const & table = itWC->second;

			bool processed = false;
	
			for ( EventListenerTable::const_iterator
					  it2 = table.begin(), it2End = table.end();
				  it2 != it2End; it2++ )
			{
				(*it2)->HandleEvent( *event );
			}
		}

		// no listerners currently for this event type, skipit
		if ( itListeners == m_registry.end() )
			continue;

		unsigned int const kEventId = itListeners->first;
		EventListenerTable const & table = itListeners->second;

		for ( EventListenerTable::const_iterator
				  it = table.begin(), end = table.end();
			  it != end ; it++ )
		{
			if ( (*it)->HandleEvent( *event ) )
			{
				break;
			}
		}

		curMs = GetTickCount();

		if ( maxMillis != IEventManager::kINFINITE )
		{

			if ( curMs >= maxMs )
			{
				// time ran about, abort processing loop
				break;
			}
		}
	}
	
	// if any events left to process, push them onto the active
	// queue.
	//
	// Note: to preserver sequencing, go bottom-up on the
	// raminder, inserting them at the head of the active
	// queue...

	bool queueFlushed = ( m_queues[queueToProcess].size() == 0 );

	if ( !queueFlushed )
	{
		while ( m_queues[queueToProcess].size() > 0 )
		{
			IEventDataPtr event = m_queues[queueToProcess].back();

			m_queues[queueToProcess].pop_back();

			m_queues[m_activeQueue].push_front( event );
		}
	}
	
	// all done, this pass
	
	return queueFlushed;
}

// --- information lookup functions ---

//
// EventManager::VValidateType			- Chapter 10, page 299
//
// Validate an event type, this does NOT add it to the internal
// registry, only verifies that it is legal ( e.g. either the
// ident number is not yet assigned, or it is assigned to
// matching signature text, and the signature text is not empty
// ).
//
bool EventManager::VValidateType(
	EventType const & inType ) const
{
	if ( 0 == inType.getStr().length() )
		return false;

	if ( ( inType.getHashValue() == 0 ) &&
		(strcmp(inType.getStr().c_str(),kpWildcardEventType) != 0) )
		return false;

	EventTypeSet::const_iterator evIt =
		m_typeList.find( inType );

	if ( evIt == m_typeList.end() )
	{
		assert( 0 && "Failed validation of an event type; it was probably not registered with the EventManager!" );
		return false;
	}
	
	return true;
}

//
// EventManager::getListenerList		- Chapter 10, page 300
//
// Get the list of listeners associated with a specific event
// type
//
EventListenerList EventManager::GetListenerList (
	EventType const & eventType ) const
{
	// invalid event type, so sad
	if ( ! VValidateType( eventType ) )
		return EventListenerList();

	EventListenerMap::const_iterator itListeners =
		m_registry.find( eventType.getHashValue() );

	// no listerners currently for this event type, so sad
	if ( itListeners == m_registry.end() )
		return EventListenerList();

	EventListenerTable const & table = itListeners->second;

	// there was, but is not now, any listerners currently for
	// this event type, so sad
	if ( table.size() == 0 )
		return EventListenerList();

	EventListenerList result;

	result.reserve( table.size() );
	
	for ( EventListenerTable::const_iterator it = table.begin(),
			  end = table.end(); it != end ; it++ )
	{
		result.push_back( *it );
	}

	return result;
}


//
// EventManager::GetTypeList			- Chapter 11
//
// Get the list of known event types
//
EventTypeList EventManager::GetTypeList ( void ) const
{
	// no entries, so sad
	if ( m_typeList.size() == 0 )
		return EventTypeList();

	EventTypeList result;

	result.reserve( m_typeList.size() );

	for ( EventTypeSet::const_iterator it = m_typeList.begin(),
			  itEnd = m_typeList.end(); it != itEnd; it++ )
	{
		result.push_back( it->first );
	}
	
	return result;
}

//--
// EventManager::AddScriptListener						- Chapter 11, page 336
//
// Creates a script-side event listener, given an appropriate Lua function.
bool EventManager::AddScriptListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction )
{
	//Ensure this event type exists.
	const EventType testEventType( pEventName );
	const EventTypeSet::const_iterator typeIter = m_typeList.find( testEventType );
	if ( m_typeList.end() == typeIter )
	{
		assert( 0 && "Attempted to listen to an event type that wasn't registered!" );
		return false;
	}

	const unsigned int eventID = testEventType.getHashValue();

	//OK, valid event type.  Make sure this isn't a duplicate.
	ScriptEventListenerMap::const_iterator mapIter = m_ScriptEventListenerMap.find( eventID );
	while ( m_ScriptEventListenerMap.end() != mapIter )
	{
		//Iterate through and ensure no duplicates.
		const ScriptEventListenerPtr evtListener = mapIter->second;
		const LuaPlus::LuaObject & evtObj = evtListener->GetHandlerFunction();
		if ( evtObj == callbackFunction )
		{
			assert( 0 && "Attempted to listen to the same event handler twice!" );
			return false;
		}
		++mapIter;
	}

	//Now let's rez up a new script listener.
	ScriptEventListenerPtr listener( GCC_NEW ScriptEventListener( callbackFunction ) );

	m_ScriptEventListenerMap.insert( std::make_pair( eventID, listener ) );

	const bool bSuccess = VAddListener( listener, testEventType );
	return bSuccess;
}

//--
// EventManager::RemoveScriptListener
// Removes a script-side listener.
bool EventManager::RemoveScriptListener( const char *const pEventName, LuaPlus::LuaObject callbackFunction )
{
	//Ensure this event type exists.
	const EventType testEventType( pEventName );
	const EventTypeSet::const_iterator typeIter = m_typeList.find( testEventType );
	if ( m_typeList.end() == typeIter )
	{
		assert( 0 && "Attempted to remove a listener for an event type that doesn't exist!" );
		return false;
	}

	const unsigned int eventID = testEventType.getHashValue();

	//OK, now ensure it exists in the multimap.
	bool bFound = false;
	ScriptEventListenerMap::iterator mapIter = m_ScriptEventListenerMap.find( eventID );
	while ( m_ScriptEventListenerMap.end() != mapIter )
	{
		const ScriptEventListenerPtr evtListener = mapIter->second;
		const LuaPlus::LuaObject & evtObj = evtListener->GetHandlerFunction();
		if ( evtObj == callbackFunction )
		{
			bFound = true;
			break;
		}
		++mapIter;
	}

	if ( false == bFound )
	{
		assert( 0 && "Attempted to remove a script listener for an event it was never listening for!" );
		return false;
	}

	//Remove this listener from the map.
	const ScriptEventListenerPtr listener = mapIter->second;
	m_ScriptEventListenerMap.erase( mapIter );

	//Now remove from the "main" listener set.
	const bool bSuccess = VDelListener( listener, testEventType );
	return bSuccess;
}

//--
// EventManager::AddScriptActorListener					- Chapter 11, page 341
// Creates a script-side *ACTOR* event listener, given an appropriate Lua function.
bool EventManager::AddScriptActorListener( char const * const pEventName, LuaPlus::LuaObject callbackFunction, const int actorID )
{
	//Ensure this event type exists.
	const EventType testEventType( pEventName );
	const EventTypeSet::const_iterator typeIter = m_typeList.find( testEventType );
	if ( m_typeList.end() == typeIter )
	{
		assert( 0 && "Attempted to listen to an event type that wasn't registered!" );
		return false;
	}

	const unsigned int eventID = testEventType.getHashValue();

	//OK, valid event type.  Make sure this isn't a duplicate.
	ScriptActorEventListenerMap::const_iterator mapIter = m_ScriptActorEventListenerMap.find( eventID );
	while ( m_ScriptActorEventListenerMap.end() != mapIter )
	{
		//Iterate through and ensure no duplicates.
		const ScriptActorEventListenerPtr evtListener = mapIter->second;
		const LuaPlus::LuaObject & evtObj = evtListener->GetHandlerFunction();
		if ( ( evtObj == callbackFunction ) && ( actorID == evtListener->GetActorID() ) )
		{
			assert( 0 && "Attempted to listen to the same event handler twice for a specific actor!" );
			return false;
		}
		++mapIter;
	}

	//Now let's rez up a new script listener.
	ScriptActorEventListenerPtr listener( GCC_NEW ScriptActorEventListener( callbackFunction, actorID ) );

	m_ScriptActorEventListenerMap.insert( std::make_pair( eventID, listener ) );

	const bool bSuccess = VAddListener( listener, testEventType );
	return bSuccess;
}

//--
// EventManager::RemoveScriptActorListener
// Removes a script-side listener for a given actor.
bool EventManager::RemoveScriptActorListener( const char *const pEventName, LuaPlus::LuaObject callbackFunction, const int actorID )
{
	//Ensure this event type exists.
	const EventType testEventType( pEventName );
	const EventTypeSet::const_iterator typeIter = m_typeList.find( testEventType );
	if ( m_typeList.end() == typeIter )
	{
		assert( 0 && "Attempted to remove a listener for an event type that doesn't exist!" );
		return false;
	}

	const unsigned int eventID = testEventType.getHashValue();

	//OK, now ensure it exists in the multimap.
	bool bFound = false;
	ScriptActorEventListenerMap::iterator mapIter = m_ScriptActorEventListenerMap.find( eventID );
	while ( m_ScriptActorEventListenerMap.end() != mapIter )
	{
		const ScriptActorEventListenerPtr evtListener = mapIter->second;
		const LuaPlus::LuaObject & evtObj = evtListener->GetHandlerFunction();
		if ( ( evtObj == callbackFunction ) && ( actorID == evtListener->GetActorID() ) )
		{
			bFound = true;
			break;
		}
		++mapIter;
	}

	if ( false == bFound )
	{
		assert( 0 && "Attempted to remove a script listener for an actor event it was never listening for!" );
		return false;
	}

	//Remove this listener from the map.
	const ScriptActorEventListenerPtr listener = mapIter->second;
	m_ScriptActorEventListenerMap.erase( mapIter );

	//Now remove from the "main" listener set.
	const bool bSuccess = VDelListener( listener, testEventType );
	return bSuccess;
}

//--
// EventManager::TriggerEventFromScript					- Chapter 11, page 3338

bool EventManager::TriggerEventFromScript( char const * const pEventName, LuaPlus::LuaObject luaEventData )
{
	const EventType eventType( pEventName );

	//Look this event type up.
	const EventTypeSet::const_iterator iter = m_typeList.find( eventType );
	if ( iter == m_typeList.end() )
	{
		assert( 0 && "Attempted to trigger an event type that doesn't exist!" );
		return false;
	}

	//This level of indirection lets us create code-side events or script-side events.
	IRegisteredEventPtr regEvent = iter->second;
	const bool bResult = regEvent->VTriggerEventFromScript( luaEventData );

	return bResult;
}


// EventManager::RegisterScriptEventType			- Chapter 11, page 329
void EventManager::RegisterScriptEventType( char const * const pEventName )
{
	//Create a new script-defined event object.
	const EventType eventType( pEventName );
	RegisterScriptEvent( eventType );
}

//--
// EventSnooper::EventSnooper		- Chapter 11, page 351
//
EventSnooper::EventSnooper( char const * const kpLogFileName )
	: m_OutFile(0)
{
	if ( kpLogFileName )
	{
		// compute the path to our current exe and use it as the
		// basis for the log file ...

		char fullPathName[MAX_PATH];

		memset( fullPathName, 0, sizeof(fullPathName) );

		GetModuleFileNameA( NULL, fullPathName, MAX_PATH );

		// normalize path separators, take note of the last
		// separator found as we go ... if any

		char * pSep = NULL;

		for ( size_t i = 0, j = strlen(fullPathName); i < j ; i++ )
		{
			if ( fullPathName[i] == '\\' )
			{
				fullPathName[i] = '/';

				pSep = & fullPathName[i];
			}
		}

		#ifdef _VS2005
			if ( pSep != NULL )
			{
				size_t sz = MAX_PATH - static_cast<int>(pSep - fullPathName);
				strcpy_s( pSep + 1, sz, kpLogFileName );
			}
			else
			{
				strcpy_s( fullPathName, MAX_PATH, kpLogFileName );
			}
			fopen_s(&m_OutFile, fullPathName, "w+");
		#else
			if ( pSep != NULL )
			{
				strcpy( pSep + 1, kpLogFileName );
			}
			else
			{
				strcpy( fullPathName, kpLogFileName );
			}
			m_OutFile = fopen(fullPathName, "w+");
		#endif
	}
}

//
// EventSnooper::EventSnooper		- Chapter 11, page 352
//
EventSnooper::~EventSnooper()
{
	if ( !m_OutFile)
		fclose( m_OutFile );
}


//
// EventSnooper::EventSnooper		- Chapter 11, page 352
//
bool EventSnooper::HandleEvent( IEventData const & event )
{
#ifdef _DEBUG
	memset( m_msgBuf, 0, sizeof(m_msgBuf));
	
#pragma warning( push )
#pragma warning( disable : 4313 ) 

#ifdef _VS2005_
	_snprintf_s( m_msgBuf, sizeof(m_msgBuf)-1,
		"Event Snoop : event %08lx time %g : type %08lx [%s] : \n",
		& event,
		event.VGetTimeStamp(),
		event.VGetEventType().getHashValue(),
		event.VGetEventType().getStr(),
		0 );
#else
	_snprintf( m_msgBuf, sizeof(m_msgBuf)-1,
		"Event Snoop : event %08lx time %g : type %08lx [%s] : \n",
		& event,
		event.getTime(),
		event.getType().getIdent(),
		event.getType().getStr(),
		0 );
#endif

#pragma warning( pop )

	OutputDebugStringA( m_msgBuf );

	if ( m_OutFile != NULL )
	{
		fwrite( m_msgBuf, (DWORD) strlen(m_msgBuf), 1, m_OutFile );
	}
#endif

	return false;
}


// EventManager::RegisterScriptEvent				- Chapter 11, page 329
void EventManager::RegisterScriptEvent( const EventType & eventType )
{
	IRegisteredEventPtr metaData( GCC_NEW EventManager::ScriptDefinedEvent( eventType ) );
	AddRegisteredEventType( eventType, metaData );
}

void EventManager::RegisterCodeOnlyEvent( const EventType & eventType )
{
	IRegisteredEventPtr metaData( GCC_NEW CodeOnlyDefinedEvent() );
	AddRegisteredEventType( eventType, metaData );
}


// EventManager::ScriptDefinedEvent::VTriggerEventFromScript		- Chapter 11, page 329
bool EventManager::ScriptDefinedEvent::VTriggerEventFromScript( LuaPlus::LuaObject & srcData ) const
{
	const EvtData_ScriptEvtData scriptEvent( m_EventType, srcData );
	return safeTriggerEvent( scriptEvent );
}

bool EventManager::ScriptDefinedEvent::VQueueEventFromScript( LuaPlus::LuaObject & srcData ) const
{
	//TODO JWC
	return true;
}

// EventManager::AddRegisteredEventType				- Chapter 11, page 325
void EventManager::AddRegisteredEventType( const EventType & eventType, IRegisteredEventPtr metaData )
{
	const EventTypeSet::const_iterator iter = m_typeList.find( eventType );
	if ( iter != m_typeList.end() )
	{
		assert( 0 && "Attempted to register an event type that has already been registered!" );
	}
	else
	{
		// We're good...
		m_typeList.insert( std::make_pair( eventType, metaData ) );
	}
}