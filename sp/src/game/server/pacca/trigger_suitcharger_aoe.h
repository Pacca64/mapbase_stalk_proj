#pragma once
#include "cbase.h"
#include "CTriggerChargerAOEBase.h"
#include <hl2/func_recharge.h>
#include <hl2/hl2_player.h>

class CTriggerSuitChargerAOE : public CTriggerChargerAOEBase
{
public:
	DECLARE_CLASS(CTriggerSuitChargerAOE, CTriggerChargerAOEBase);
	DECLARE_DATADESC();

	const char* GetChargerClassname();
	bool IsCharger(CBaseEntity* ent);
	int GetChargerJuice(CBaseEntity* ent);
	void PlayEmptySound(CBaseEntity* ent);
};
