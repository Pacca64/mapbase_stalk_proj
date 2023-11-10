#pragma once
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef LOGICRELAY_H
#define LOGICRELAY_H

#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include <Sprite.h>
#include <beam_shared.h>

class CEnvWarpBall : public CLogicalEntity
{
public:
	DECLARE_CLASS(CEnvWarpBall, CLogicalEntity);

	CEnvWarpBall();

	void Precache(void);

	void Activate();
	void Think();

	// Input handlers
	void InputEnable(inputdata_t& inputdata);
	void InputDisable(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);
	void InputTrigger(inputdata_t& inputdata);

	void StartEffect();

	void SetEntKeyvalue(CBaseEntity* ent, const char* addoutputString, int outputID);

	void SpawnEffectEntities();

	void UpdateOnRemove(void);

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;

	bool IsDisabled(void) { return m_bDisabled; }

	string_t m_iszSpriteExplodeName;
	string_t m_iszSpriteGreenName;
	string_t m_iszSpriteBeamName;

	string_t m_iszSoundAName;
	string_t m_iszSoundBName;

	color32 m_cSpriteExplodeColor;
	color32 m_cSpriteGreenColor;
	color32 m_cBeamColor;

	int m_iRenderFX;
	int m_iRenderMode;
	//int m_iRenderAmt;	//this seems to not exist in engine?
	float m_fSpriteScale;
	int m_iViewHideFlags;
	float m_fGlowProxySize;
	//float m_fHDRColorScale;	//can't be set on the fly. Inits to 1.0 (default sane value) anyways.
	float m_fFramerate;
	float m_fZapRadius;

private:

	bool m_bDisabled;
	bool m_bWaitForRefire;			// Set to disallow a refire while we are waiting for our outputs to finish firing.

	CSprite* m_pGlowExplode;
	CSprite* m_pGlowGreen;
	CBeam* m_pBeam;

	float m_fTimeSinceEffectStarted;
	bool m_bSecondSoundPlayed;

	float m_fAlphaSpriteGreen;
};

#endif //LOGICRELAY_H
