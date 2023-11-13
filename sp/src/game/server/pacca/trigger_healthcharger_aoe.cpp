#include "cbase.h"
#include "trigger_healthcharger_aoe.h"
#include <string>
#include <hl2/hl2_player.h>

LINK_ENTITY_TO_CLASS(trigger_healthcharger_aoe, CTriggerHealthChargerAOE);

BEGIN_DATADESC(CTriggerHealthChargerAOE)

END_DATADESC()

const char* CTriggerHealthChargerAOE::GetChargerClassname()
{
	return "item_healthcharger";
}

bool CTriggerHealthChargerAOE::IsCharger(CBaseEntity* ent)
{
	if (dynamic_cast<CNewWallHealth*>(ent)) {
		//If entity can be cast to a CNewRecharge...
		return true;
	}

	return false;
}

int CTriggerHealthChargerAOE::GetChargerJuice(CBaseEntity* ent)
{
	CNewWallHealth* charger = dynamic_cast<CNewWallHealth*>(ent);
	if (!charger) {
		//if this is not a CNewRecharge...
		return 0;
	}

	return charger->m_iJuice;
}

void CTriggerHealthChargerAOE::PlayEmptySound(CBaseEntity* ent)
{
	ent->EmitSound("WallHealth.Deny");
}

bool CTriggerHealthChargerAOE::IsPlayerFull(CBasePlayer* player, CBaseEntity* charger)
{
	return player->GetHealth() >= player->GetMaxHealth();	//if player health is GREATER THEN OR EQUAL TO max health, player is full
}
