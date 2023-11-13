#include "cbase.h"
#include "CTriggerChargerAOEBase.h"
#include <string>
#include <hl2/hl2_player.h>

BEGIN_DATADESC(CTriggerChargerAOEBase)

// Declare this function as being the touch function
DEFINE_ENTITYFUNC(StartTouch),
DEFINE_ENTITYFUNC(EndTouch),

DEFINE_KEYFIELD(m_sChargerName, FIELD_STRING, "charger"),
DEFINE_KEYFIELD(m_bAllowNonStalkerUse, FIELD_BOOLEAN, "AllowNonStalkerUse"),
DEFINE_FIELD(m_pCharger, FIELD_EHANDLE),
DEFINE_FIELD(m_bIsTouchingPlayer, FIELD_BOOLEAN),
DEFINE_FIELD(m_bWasCharging, FIELD_BOOLEAN),

DEFINE_OUTPUT(m_OnStartCharging, "OnStartCharging"),
DEFINE_OUTPUT(m_OnStopCharging, "OnStopCharging"),

END_DATADESC()

void CTriggerChargerAOEBase::Spawn()
{
	BaseClass::Spawn();
	InitTrigger();
}

//------------------------------------------------------------------------------
// Find the suit charger we will act as an AOE for.
//------------------------------------------------------------------------------
void CTriggerChargerAOEBase::Activate() {
	BaseClass::Activate();

	CBaseEntity* pResult = gEntList.FindEntityByName(NULL, m_sChargerName);

	while (pResult)
	{
		if (IsCharger(pResult)) {
			//Entity is a charger and we have it's handle
			break;
		}
		else {
			//entity matched but was NOT a charger, keep searching for the charger.
			pResult = gEntList.FindEntityByName(pResult, m_sChargerName);
		}
	}

	if (!IsCharger(pResult)) {
		Warning(this->GetClassname());
		Warning(" couldn't find an ");
		Warning(this->GetChargerClassname());
		Warning(" with the name '");
		Warning(STRING(m_sChargerName));
		Warning("'! This will not work properly!\n");
	}

	//save result of trying to find charger in pCharger
	m_pCharger = pResult;
	m_bWasCharging = false;

	SetThink(&CTriggerChargerAOEBase::Think);
	SetNextThink(gpGlobals->curtime + 0.01f);
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerChargerAOEBase::StartTouch(CBaseEntity* pOther)
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
void CTriggerChargerAOEBase::EndTouch(CBaseEntity* pOther)
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
void CTriggerChargerAOEBase::Think() {
	CBasePlayer* pPlayer = ToBasePlayer(UTIL_GetLocalPlayer());
	CHL2_Player* pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (!IsPlayerFull(pHL2Player, m_pCharger)) {
		//if player is not full (of armor, health, etc.)
		if (pHL2Player && pPlayer->IsSuitEquipped() && (m_bAllowNonStalkerUse || pHL2Player->m_bIsStalker)) {
			//If player is hl2 player, has suit equipped, and is either a stalker or we allow non stalkers to use us...
			if (m_bIsTouchingPlayer) {
				//if touching a player...
				if (m_pCharger && GetChargerJuice(m_pCharger) > 0) {
					//if charger is valid and it's juice is greater then 0...
					//DevMsg("Player SHOULD be charging now.\n");
					FakeChargeEvent(pHL2Player);
					if (!m_bWasCharging) {
						//If wasn't charging last think...
						m_OnStartCharging.FireOutput(pHL2Player, this);	//fire on start charging output
					}
					m_bWasCharging = true;
				}
				else if (m_pCharger && m_bWasCharging) {
					//If we were charging, and charger is valid, but charger is out of juice...
					m_bWasCharging = false;
					PlayEmptySound(m_pCharger);//make charger play deny sound once.
					m_OnStopCharging.FireOutput(pHL2Player, this);	//fire on stop charging output
				}
			}
			else {
				//Since player did not touch trigger, we are not charging anymore.
				if (m_bWasCharging) {
					m_OnStopCharging.FireOutput(pHL2Player, this);	//fire on stop charging output
				}
				m_bWasCharging = false;
			}
		}
	}

	SetThink(&CTriggerChargerAOEBase::Think);
	SetNextThink(gpGlobals->curtime + 0.01f);
}

void CTriggerChargerAOEBase::FakeChargeEvent(CHL2_Player* player) {
	if (m_pCharger) {
		inputdata_t inputDat = inputdata_t();
		inputDat.pActivator = player;
		inputDat.pCaller = this;
		m_pCharger->AcceptInput("Use", player, player, variant_t(), 0);
	}
}