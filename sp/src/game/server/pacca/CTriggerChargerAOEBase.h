#pragma once
#pragma once
#include "cbase.h"
#include "triggers.h"
#include <hl2/func_recharge.h>
#include <hl2/hl2_player.h>

class CTriggerChargerAOEBase : public CBaseTrigger
{
public:
	DECLARE_CLASS(CTriggerChargerAOEBase, CBaseTrigger);
	DECLARE_DATADESC();

	void Spawn();

	void Activate();

	void StartTouch(CBaseEntity* pOther);

	void EndTouch(CBaseEntity* pOther);

	void Think();

	void FakeChargeEvent(CHL2_Player* player);
	virtual const char* GetChargerClassname() = 0;
	virtual bool IsCharger(CBaseEntity* ent) = 0;
	virtual int GetChargerJuice(CBaseEntity* ent) = 0;
	virtual void PlayEmptySound(CBaseEntity* ent) = 0;

	string_t	m_sChargerName;
	CBaseEntity* m_pCharger;
	bool m_bIsTouchingPlayer;
	bool m_bWasCharging;

	bool m_bAllowNonStalkerUse;
};