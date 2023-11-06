//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_StalkerWep_H
#define WEAPON_StalkerWep_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_StalkerWep.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	StalkerWep_RANGE	75.0f
#define	StalkerWep_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponStalkerWep
//-----------------------------------------------------------------------------

class CWeaponStalkerWep : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponStalkerWep, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	DECLARE_DATADESC();

	CWeaponStalkerWep();

	float		GetRange( void )		{	return	StalkerWep_RANGE;	}
	float		GetFireRate( void )		{	return	StalkerWep_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		PrimaryAttack(void);
	void		SecondaryAttack(void);

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

#ifdef MAPBASE
	// Don't use backup activities
	acttable_t		*GetBackupActivityList() { return NULL; }
	int				GetBackupActivityListCount() { return 0; }
#endif

	void   	DrawBeam(const Vector& startPos, const Vector& endPos, float width);
	void	DoImpactEffect(trace_t& tr, int nDamageType);

	void DrawAttackBeam(void);

	int	m_nBulletType;	//probably will go unused

	//Taken from stalker code
	float				m_fNextDamageTime;

	float				m_bPlayingHitWall;
	float				m_bPlayingHitFlesh;

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};

#endif // WEAPON_StalkerWep_H
