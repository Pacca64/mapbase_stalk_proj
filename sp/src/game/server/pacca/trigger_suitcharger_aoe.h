#pragma once
#include "cbase.h"
#include "triggers.h"
#include <hl2/func_recharge.h>
#include <hl2/hl2_player.h>

class CTriggerSuitChargerAOE : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerSuitChargerAOE, CBaseTrigger);
	DECLARE_DATADESC();

	void Spawn();

	void Activate();

	void StartTouch(CBaseEntity* pOther);

	void EndTouch(CBaseEntity* pOther);

	void Think();

	void FakeChargeEvent(float juiceToRemove, CHL2_Player* player);

	string_t	m_sChargerName;
	CNewRecharge*	m_pCharger;
	bool m_bIsTouchingPlayer;
	bool m_bWasCharging;
};
