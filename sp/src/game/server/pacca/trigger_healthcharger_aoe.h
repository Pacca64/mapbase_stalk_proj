#pragma once
#include "cbase.h"
#include "CTriggerChargerAOEBase.h"
#include "CWallHealth.h"
#include <hl2/hl2_player.h>

class CTriggerHealthChargerAOE: public CTriggerChargerAOEBase
{
public:
	DECLARE_CLASS(CTriggerHealthChargerAOE, CTriggerChargerAOEBase);
	DECLARE_DATADESC();

	const char* GetChargerClassname();
	bool IsCharger(CBaseEntity* ent);
	int GetChargerJuice(CBaseEntity* ent);
	void PlayEmptySound(CBaseEntity* ent);
	bool IsPlayerFull(CBasePlayer* player, CBaseEntity* charger);
};
