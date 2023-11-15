#pragma once
#include "cbase.h"
#include <Sprite.h>

class CSpriteOwned : public CSprite
{
public:
	DECLARE_CLASS(CSpriteOwned, CSprite);

	void ThinkKillIfOwnerLost();

	DECLARE_DATADESC();

	void Spawn();
};