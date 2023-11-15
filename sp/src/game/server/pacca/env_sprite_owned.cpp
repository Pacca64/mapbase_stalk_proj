#include "cbase.h"
#include <Sprite.h>
#include "pacca/env_sprite_owned.h"
#include "tier0/memdbgon.h"

#define	ENV_SPRITE_OWNED_THINKCONTEXT		"SpriteOwnedContext"

LINK_ENTITY_TO_CLASS(env_sprite_owned, CSpriteOwned);


//#if !defined( CLIENT_DLL )
BEGIN_DATADESC(CSpriteOwned)

DEFINE_THINKFUNC(ThinkKillIfOwnerLost),

END_DATADESC()
//#endif

void CSpriteOwned::Spawn() {
	RegisterThinkContext(ENV_SPRITE_OWNED_THINKCONTEXT);
	SetContextThink(&CSpriteOwned::ThinkKillIfOwnerLost, gpGlobals->curtime + .5, ENV_SPRITE_OWNED_THINKCONTEXT);
	BaseClass::Spawn();
}


void CSpriteOwned::ThinkKillIfOwnerLost() {
	if (!GetOwnerEntity()) {
		//if owner is not valid...
		UTIL_Remove(this);	//destroy ourselves. Feels like a hacky workaround, but the rpg seems to do exactly this.
	}
	SetNextThink(gpGlobals->curtime + .5, ENV_SPRITE_OWNED_THINKCONTEXT);	//think every .5 seconds; we are in no rush.
}