#pragma once
static ConVar	sk_suitcharger("sk_suitcharger", "0");
static ConVar	sk_suitcharger_citadel("sk_suitcharger_citadel", "0");
static ConVar	sk_suitcharger_citadel_maxarmor("sk_suitcharger_citadel_maxarmor", "0");

#define SF_CITADEL_RECHARGER	0x2000
#define SF_KLEINER_RECHARGER	0x4000 // Gives only 25 health

//NEW
class CNewRecharge : public CBaseAnimating
{
public:
	DECLARE_CLASS(CNewRecharge, CBaseAnimating);

	void Spawn();
	bool CreateVPhysics();
	int DrawDebugTextOverlays(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue(const char* szKeyName, const char* szValue);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return (BaseClass::ObjectCaps() | m_iCaps); }

	void SetInitialCharge(void);

	float MaxJuice() const;
	void UpdateJuice(int newJuice);

//Moved by pacca from private to public.
	float	m_flNextCharge;
	int		m_iReactivate; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	int		m_nState;
	int		m_iCaps;

	COutputFloat m_OutRemainingCharge;
	COutputEvent m_OnHalfEmpty;
	COutputEvent m_OnEmpty;
	COutputEvent m_OnFull;
	COutputEvent m_OnPlayerUse;

	int		m_iMaxJuice;
#ifdef MAPBASE
	int		m_iIncrementValue;
#endif

	virtual void StudioFrameAdvance(void);
	float m_flJuice;

private:
	void InputRecharge(inputdata_t& inputdata);
	void InputSetCharge(inputdata_t& inputdata);
#ifdef MAPBASE
	void InputSetChargeNoMax(inputdata_t& inputdata);
#endif
	void Precache(void);

	DECLARE_DATADESC();
};