//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "console/console.h"

#include "console/sim.h"
#include "console/simEvents.h"
#include "console/simObject.h"
#include "console/simSet.h"
#include "core/module.h"


MODULE_BEGIN( Sim )

   // Note: tho the SceneGraphs are created after the Manager, delete them after, rather
   //  than before to make sure that all the objects are removed from the graph.
   MODULE_INIT_AFTER( GFX )
   MODULE_SHUTDOWN_BEFORE( GFX )

   MODULE_INIT
   {
      Sim::init();
   }
   
   MODULE_SHUTDOWN
   {
      Sim::shutdown();
   }

MODULE_END;



namespace Sim
{
   // Don't forget to InstantiateNamed* in simManager.cc - DMM
   ImplementNamedSet(ActiveActionMapSet)
   ImplementNamedSet(GhostAlwaysSet)
   ImplementNamedSet(WayPointSet)
   ImplementNamedSet(fxReplicatorSet)
   ImplementNamedSet(fxFoliageSet)
   ImplementNamedSet(BehaviorSet)
   ImplementNamedSet(MaterialSet)
   ImplementNamedSet(SFXSourceSet)
   ImplementNamedSet(SFXDescriptionSet)
   ImplementNamedSet(SFXTrackSet)
   ImplementNamedSet(SFXEnvironmentSet)
   ImplementNamedSet(SFXStateSet)
   ImplementNamedSet(SFXAmbienceSet)
   ImplementNamedSet(TerrainMaterialSet)
   ImplementNamedSet(DataBlockSet);
   ImplementNamedGroup(ActionMapGroup)
   ImplementNamedGroup(ClientGroup)
   ImplementNamedGroup(GuiGroup)
   ImplementNamedGroup(GuiDataGroup)
   ImplementNamedGroup(TCPGroup)
   ImplementNamedGroup(SFXParameterGroup);

   //groups created on the client
   ImplementNamedGroup(ClientConnectionGroup)
   ImplementNamedSet(sgMissionLightingFilterSet)
}

//-----------------------------------------------------------------------------
// Console Functions
//-----------------------------------------------------------------------------

ConsoleFunctionGroupBegin ( SimFunctions, "Functions relating to Sim.");

ConsoleFunction(nameToID, S32, 2, 2, "nameToID(object)")
{
   TORQUE_UNUSED(argc);
   SimObject *obj = Sim::findObject(argv[1]);
   if(obj)
      return obj->getId();
   else
      return -1;
}

ConsoleFunction(isObject, bool, 2, 2, "isObject(object)")
{
   TORQUE_UNUSED(argc);
   if (!dStrcmp(argv[1], "0") || !dStrcmp(argv[1], ""))
      return false;
   else
      return (Sim::findObject(argv[1]) != NULL);
}

ConsoleDocFragment _spawnObject1(
   "@brief Global function used for spawning any type of object.\n\n"

   "Note: This is separate from SpawnSphere::spawnObject(). This function is not called off any "
   "other class and uses different parameters than the SpawnSphere's function. In the source, "
   "SpawnSphere::spawnObject() actually calls this function and passes its properties "
   "(spawnClass, spawnDatablock, etc).\n\n"

   "@param class Mandatory field specifying the object class, such as Player or TSStatic.\n\n"
   "@param datablock Field specifying the object's datablock, optional for objects such as TSStatic, mandatory for game objects like Player.\n\n"
   "@param name Optional field specifying a name for this instance of the object.\n\n"
   "@param properties Optional set of parameters applied to the spawn object during creation.\n\n"
   "@param script Optional command(s) to execute when spawning an object.\n\n"

   "@tsexample\n"
      "// Set the parameters for the spawn function\n"
      "%objectClass = \"Player\";\n"
      "%objectDatablock = \"DefaultPlayerData\";\n"
      "%objectName = \"PlayerName\";\n"
      "%additionalProperties = \"health = \\\"0\\\";\"; // Note the escape sequence \\ in front of quotes\n"
      "%spawnScript = \"echo(\\\"Player Spawned\\\");\" // Note the escape sequence \\ in front of quotes\n"
      "// Spawn with the engine's Sim::spawnObject() function\n"
      "%player = spawnObject(%objectClass, %objectDatablock, %objectName, %additionalProperties, %spawnScript);\n"
   "@endtsexample\n\n"

   "@ingroup Game",
   NULL,
   "bool spawnObject(class [, dataBlock, name, properties, script]);"
);

ConsoleFunction(spawnObject, S32, 3, 6, "spawnObject(class [, dataBlock, name, properties, script])"
				"@hide")
{
   String spawnClass((const char*)argv[1]);
   String spawnDataBlock;
   String spawnName;
   String spawnProperties;
   String spawnScript;

   if (argc >= 3)
      spawnDataBlock = (const char*)argv[2];
   if (argc >= 4)
      spawnName = (const char*)argv[3];
   if (argc >= 5)
      spawnProperties = (const char*)argv[4];
   if (argc >= 6)
      spawnScript = (const char*)argv[5];

   SimObject* spawnObject = Sim::spawnObject(spawnClass, spawnDataBlock, spawnName, spawnProperties, spawnScript);

   if (spawnObject)
      return spawnObject->getId();
   else
      return -1;
}

ConsoleFunction(cancel,void,2,2,"cancel(eventId)")
{
   Sim::cancelEvent(dAtoi(argv[1]));
}

ConsoleFunction(cancelAll,void,2,2,"cancelAll(objectId): cancel pending events on the specified object.  Events will be automatically cancelled if object is deleted.")
{
   Sim::cancelPendingEvents(Sim::findObject(argv[1]));
}

ConsoleFunction(isEventPending, bool, 2, 2, "isEventPending(%scheduleId);")
{
   return Sim::isEventPending(dAtoi(argv[1]));
}

ConsoleFunction(getEventTimeLeft, S32, 2, 2, "getEventTimeLeft(scheduleId) Get the time left in ms until this event will trigger.")
{
   return Sim::getEventTimeLeft(dAtoi(argv[1]));
}

ConsoleFunction(getScheduleDuration, S32, 2, 2, "getScheduleDuration(%scheduleId);")
{
   TORQUE_UNUSED(argc);   S32 ret = Sim::getScheduleDuration(dAtoi(argv[1]));
   return ret;
}

ConsoleFunction(getTimeSinceStart, S32, 2, 2, "getTimeSinceStart(%scheduleId);")
{
   TORQUE_UNUSED(argc);   S32 ret = Sim::getTimeSinceStart(dAtoi(argv[1]));
   return ret;
}

ConsoleFunction(schedule, S32, 4, 0, "schedule(time, refobject|0, command, <arg1...argN>)")
{
   U32 timeDelta = U32(dAtof(argv[1]));
   SimObject *refObject = Sim::findObject(argv[2]);
   if(!refObject)
   {
      if(!argv[2].isNull())
         return 0;

      refObject = Sim::getRootGroup();
   }
   SimConsoleEvent *evt = new SimConsoleEvent(argc - 3, argv + 3, false);

   S32 ret = Sim::postEvent(refObject, evt, Sim::getCurrentTime() + timeDelta);
   // #ifdef DEBUG
   //    Con::printf("ref %s schedule(%s) = %d", argv[2], argv[3], ret);
   //    Con::executef( "backtrace");
   // #endif
   return ret;
}

ConsoleFunction(getUniqueName, const char*, 2, 2, 
	"( String baseName )\n"
	"@brief Returns a unique unused SimObject name based on a given base name.\n\n"
	"@baseName Name to conver to a unique string if another instance exists\n"
	"@note Currently only used by editors\n"
	"@ingroup Editors\n"
	"@internal")
{
   String outName = Sim::getUniqueName( argv[1] );
   
   if ( outName.isEmpty() )
      return NULL;

   char *buffer = Con::getReturnBuffer( outName.size() );
   dStrcpy( buffer, outName );

   return buffer;
}

ConsoleFunction(getUniqueInternalName, const char*, 4, 4,
   "( String baseName, SimSet set, bool searchChildren )\n"
   "@brief Returns a unique unused internal name within the SimSet/Group based on a given base name.\n\n"
   "@note Currently only used by editors\n"
   "@ingroup Editors\n"
   "@internal")
{
   SimSet *set;
   if ( !Sim::findObject( argv[2], set ) )
   {
      Con::errorf( "getUniqueInternalName() - invalid parameter for SimSet." );
      return NULL;
   }

   String outName = Sim::getUniqueInternalName( argv[1], set, dAtob(argv[3]) );

   if ( outName.isEmpty() )
      return NULL;

   char *buffer = Con::getReturnBuffer( outName.size() );
   dStrcpy( buffer, outName );

   return buffer;
}

ConsoleFunction( isValidObjectName, bool, 2, 2, "( string name )"
				"@brief Return true if the given name makes for a valid object name.\n\n"
				"@param name Name of object\n"
				"@return True if name is allowed, false if denied (usually because it starts with a number, _, or invalid character"
				"@ingroup Console")
{
   const char* name = argv[ 1 ];
   return Sim::isValidObjectName( name );
}

ConsoleFunctionGroupEnd( SimFunctions );
