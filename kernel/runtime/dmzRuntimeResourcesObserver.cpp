#include "dmzRuntimeContext.h"
#include "dmzRuntimeContextResources.h"
#include <dmzRuntimeHandle.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzRuntimeResourcesObserver.h>

/*!

\file dmzRuntimeResourcesObserver.h
\ingroup Runtime
\brief Contains ResourcesObserver class.

\enum dmz::ResourcesModeEnum
\ingroup Runtime
\brief Resource update callback mode.

\class dmz::ResourcesObserver
\ingroup Runtime
\brief Observes changes in runtime Resources.

*/

struct dmz::ResourcesObserver::State {

   RuntimeHandle *handlePtr;
   const Handle ObsHandle;
   RuntimeContextResources *rc;
   Boolean active;

   State (const Handle TheObsHandle, const String &ObsName, RuntimeContext *context) :
         handlePtr (
            TheObsHandle ?
               new RuntimeHandle (ObsName + ".ResourcesObserver", context) :
               0),
         ObsHandle (handlePtr ? handlePtr->get_runtime_handle () : TheObsHandle),
         rc (context ? context->get_resources_context () : 0),
         active (False) {

      if (rc) {

         rc->ref ();
      }
   }

   ~State () {

      release_observer ();

      if (rc) {

         rc->unref ();
         rc = 0;
      }

      if (handlePtr) { delete handlePtr; handlePtr = 0; }
   }

   void register_observer (ResourcesObserver *self) {

      if (rc && ObsHandle && !active) { active = rc->obsTable.store (ObsHandle, self); }
   }

   void release_observer () {

      if (rc && ObsHandle && active) { active = (rc->obsTable.remove (ObsHandle) != 0); }
   }
};


/*!

\brief Constructor.
\param[in] context Pointer to the RuntimeContext.

*/
dmz::ResourcesObserver::ResourcesObserver (RuntimeContext *context) :
      __state (*(new State (0, "<Anonymous Resource Observer>", context))) {;}


/*!

\brief Constructor.
\param[in] Info Reference to the PluginInfo.

*/
dmz::ResourcesObserver::ResourcesObserver (const PluginInfo &Info) :
      __state (*(new State (Info.get_handle (), Info.get_name (), Info.get_context ()))) {
}


//! Destructor.
dmz::ResourcesObserver::~ResourcesObserver () { delete &__state; }


//! Activates dmz::ResourcesObserver::update_resource() callback.
void
dmz::ResourcesObserver::activate_resources_calllback () {

   if (!__state.active) {

      __state.register_observer (this);

      if (__state.active && __state.rc) {

         HashTableStringIterator it;
         Config *ptr (0);

         while (__state.rc->rcTable.get_next (it, ptr)) {

            update_resource (it.get_hash_key (), ResourceCreated);
         }
      }
   }
}


//! deactivates dmz::ResourcesObserver::update_resource() callback.
void
dmz::ResourcesObserver::deactivate_resources_calllback () {

   __state.release_observer ();
}


/*!

\brief Dumps the current Resources to the observer.
\details The dmz::ResourcesObserver::update_resource() Mode parameter will be set to dmz::ResourceDumped.

*/
void
dmz::ResourcesObserver::dump_current_resources () {

   if (__state.rc) {

      HashTableStringIterator it;
      Config *ptr (0);

      while (__state.rc->rcTable.get_next (it, ptr)) {

         update_resource (it.get_hash_key (), ResourceDumped);
      }
   }
}


/*!

\fn void dmz::ResourcesObserver::update_resource (const String &Name, const ResourcesModeEnum Mode)
\brief Function invoked when runtime Resources are create, updated, or removed.

*/
