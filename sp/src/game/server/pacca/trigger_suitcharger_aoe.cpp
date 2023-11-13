#include "cbase.h"
#include "trigger_suitcharger_aoe.h"
#include <string>
#include <hl2/hl2_player.h>

LINK_ENTITY_TO_CLASS(trigger_suitcharger_aoe, CTriggerSuitChargerAOE);

BEGIN_DATADESC(CTriggerSuitChargerAOE)
	
END_DATADESC()

const char* CTriggerSuitChargerAOE::GetChargerClassname()
{
	return "item_suitcharger";
}

bool CTriggerSuitChargerAOE::IsCharger(CBaseEntity* ent)
{
	if (dynamic_cast<CNewRecharge*>(ent)) {
		//If entity can be cast to a CNewRecharge...
		return true;
	}

	return false;
}

int CTriggerSuitChargerAOE::GetChargerJuice(CBaseEntity* ent)
{
	CNewRecharge* charger = dynamic_cast<CNewRecharge*>(ent);
	if (!charger) {
		//if this is not a CNewRecharge...
		return 0;
	}

	return charger->m_iJuice;
}

void CTriggerSuitChargerAOE::PlayEmptySound(CBaseEntity* ent)
{
	ent->EmitSound("SuitRecharge.Deny");
}
