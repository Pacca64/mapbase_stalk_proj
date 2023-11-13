#include "cbase.h"
#include "trigger_suitcharger_aoe.h"
#include <string>
#include <hl2/hl2_player.h>

LINK_ENTITY_TO_CLASS(trigger_suitcharger_aoe, CTriggerSuitChargerAOE);

BEGIN_DATADESC(CTriggerSuitChargerAOE)

// Declare this function as being the touch function
DEFINE_ENTITYFUNC(StartTouch),
DEFINE_ENTITYFUNC(EndTouch),

DEFINE_KEYFIELD(m_sChargerName, FIELD_STRING, "charger"),
DEFINE_FIELD(m_pCharger, FIELD_EHANDLE),
DEFINE_FIELD(m_bIsTouchingPlayer, FIELD_BOOLEAN),
DEFINE_FIELD(m_bWasCharging, FIELD_BOOLEAN),

END_DATADESC()

void CTriggerSuitChargerAOE::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

//------------------------------------------------------------------------------
// Find the suit charger we will act as an AOE for.
//------------------------------------------------------------------------------
void CTriggerSuitChargerAOE::Activate() {
	BaseClass::Activate();

	CBaseEntity* pResult = gEntList.FindEntityByName(NULL, m_sChargerName);
	CNewRecharge* pCharge = NULL;

	while (pResult)
	{
		pCharge = dynamic_cast<CNewRecharge*>(pResult);
		if (pCharge) {
			//Entity is a charger and we have it's handle
			break;
		} else {
			//entity matched but was NOT a charger, keep searching for the charger.
			pResult = gEntList.FindEntityByName(pResult, m_sChargerName);
		}
	}

	if (!pCharge) {
		Warning("trigger_suitcharger_aoe couldn't find an item_suitcharger with the name '");
		Warning(STRING(m_sChargerName));
		Warning("'! This will not work properly!\n");
	}

	//save result of trying to find charger in pCharger
	m_pCharger = pCharge;
	m_bWasCharging = false;

	SetThink(&CTriggerSuitChargerAOE::Think);
	SetNextThink(gpGlobals->curtime + 0.01f);
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerSuitChargerAOE::StartTouch(CBaseEntity* pOther)
{
	//DevMsg("start touch called\n");
	BaseClass::StartTouch(pOther);

	CBasePlayer* pPlayer = ToBasePlayer(pOther);
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (pHL2Player) {
		//DevMsg("Player touched charger aoe!\n");
		m_bIsTouchingPlayer = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerSuitChargerAOE::EndTouch(CBaseEntity* pOther)
{
	//DevMsg("end touch called\n");
	BaseClass::EndTouch(pOther);

	CBasePlayer* pPlayer = ToBasePlayer(pOther);
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (pHL2Player) {
		//DevMsg("Player stopped touching charger aoe\n");
		m_bIsTouchingPlayer = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Make player continuously use charger until player leaves volume.
//-----------------------------------------------------------------------------
void CTriggerSuitChargerAOE::Think() {
	CBasePlayer* pPlayer = ToBasePlayer(UTIL_GetLocalPlayer());
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (pHL2Player && pPlayer->IsSuitEquipped()) {
		if (m_bIsTouchingPlayer) {
			//if touching a player...
			if (m_pCharger && m_pCharger->m_iJuice > 0) {
				//if charger is valid and it's juice is greater then 0...
				//DevMsg("Player SHOULD be charging now.\n");
				FakeChargeEvent(0, pHL2Player);
				m_bWasCharging = true;
			}
			else if (m_pCharger && m_bWasCharging) {
				//If we were charging, and charger is valid, but charger is out of juice...
				m_bWasCharging = false;	
				m_pCharger->EmitSound("SuitRecharge.Deny");	//make charger play deny sound once.
			}
		}
		else {
			//Since player did not touch trigger, we are not charging anymore.
			m_bWasCharging = false;
		}
	}

	SetThink(&CTriggerSuitChargerAOE::Think);
	SetNextThink(gpGlobals->curtime + 0.01f);
}



void CTriggerSuitChargerAOE::FakeChargeEvent(float juiceToRemove, CHL2_Player* player) {
	if (m_pCharger) {
		inputdata_t inputDat = inputdata_t();
		inputDat.pActivator = player;
		inputDat.pCaller = this;
		m_pCharger->AcceptInput("Use", player, player, variant_t(), 0);
	}
	/*
	if (m_pCharger) {
		m_pCharger->m_flJuice = m_pCharger->m_flJuice;

		m_pCharger->UpdateJuice(m_pCharger->m_flJuice);

		m_pCharger->ResetSequence(m_pCharger->m_iJuice > 0 ? m_pCharger->LookupSequence("idle") : m_pCharger->LookupSequence("empty"));
		m_pCharger->StudioFrameAdvance();
	}
	*/
}