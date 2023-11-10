//=============================================================================
//
// A logical entity that handles an HL1 styled Monster teleport effect.
// Can be done in hammer, but this will be easier to make and use, and use less entities.
//
// Inspired by the similarly named GoldSrc entity, but built from scratch, using logic_relay as a base.
// 
//=============================================================================

#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "soundent.h"
#include "env_warpball.h"
#ifdef MAPBASE
#include "mapbase/variant_tools.h"
#include "saverestore_utlvector.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <string>

const int SF_REMOVE_ON_FIRE = 0x001;	// Once effect is finished, destroy this entity.
const int SF_ALLOW_FAST_RETRIGGER = 0x002;	// If set, effect can be forcibly replayed. This will cutoff the previous effect though.

LINK_ENTITY_TO_CLASS(env_warpball, CEnvWarpBall);


BEGIN_DATADESC(CEnvWarpBall)

DEFINE_FIELD(m_bWaitForRefire, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_bDisabled, FIELD_BOOLEAN, "StartDisabled"),

DEFINE_KEYFIELD(m_iszSpriteExplodeName, FIELD_STRING, "ExplodeSpriteName"),
DEFINE_KEYFIELD(m_iszSpriteGreenName, FIELD_STRING, "FadeSpriteName"),
DEFINE_KEYFIELD(m_iszSpriteBeamName, FIELD_STRING, "BeamSpriteName"),

DEFINE_KEYFIELD(m_iszSoundAName, FIELD_SOUNDNAME, "StartSoundName"),
DEFINE_KEYFIELD(m_iszSoundBName, FIELD_SOUNDNAME, "EndSoundName"),

DEFINE_KEYFIELD(m_cSpriteExplodeColor, FIELD_COLOR32, "ExplodeSpriteColor"),
DEFINE_KEYFIELD(m_cSpriteGreenColor, FIELD_COLOR32, "FadeSpriteColor"),
DEFINE_KEYFIELD(m_cBeamColor, FIELD_COLOR32, "BeamColor"),

DEFINE_FIELD(m_pGlowExplode, FIELD_EHANDLE),
DEFINE_FIELD(m_pGlowGreen, FIELD_EHANDLE),
DEFINE_FIELD(m_pGlowExplode, FIELD_EHANDLE),

DEFINE_FIELD(m_fTimeSinceEffectStarted, FIELD_FLOAT),
DEFINE_FIELD(m_bSecondSoundPlayed, FIELD_BOOLEAN),
DEFINE_FIELD(m_fAlphaSpriteGreen, FIELD_FLOAT),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),
DEFINE_INPUTFUNC(FIELD_VOID, "Trigger", InputTrigger),

// Outputs
DEFINE_OUTPUT(m_OnTrigger, "OnTrigger"),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CEnvWarpBall::CEnvWarpBall(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CEnvWarpBall::Precache(void)
{
	PrecacheModel(STRING(m_iszSpriteExplodeName));
	PrecacheModel(STRING(m_iszSpriteGreenName));
	PrecacheModel(STRING(m_iszSpriteBeamName));

	PrecacheScriptSound(STRING(m_iszSoundAName));
	PrecacheScriptSound(STRING(m_iszSoundBName));

	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Kickstarts a think if we have OnSpawn connections.
//------------------------------------------------------------------------------
void CEnvWarpBall::Activate()
{
	BaseClass::Activate();

	if (strcmp(GetDebugName(), STRING(m_iClassname)))
	{
		// strings are not equal
		// This means we have a name! continue as normal.
	}
	else
	{
		// strings are equal
		//This means we don't have a targetname!
		//If we don't have a targetname, the horrible I/O shenanigans with env_beam later on will fail!
		std::string newNameStr = std::string("env_warpball_default_name_") + std::to_string(random->RandomInt(0, 2147483647));
		const char* newName = (newNameStr).c_str();

		Warning("CEnvWarpBall has no name! Setting name to ");
		Warning(newName);
		Warning("!\n");
		this->SetName(AllocPooledString(newName));
	}
}


//-----------------------------------------------------------------------------
// Handles playing out effect over time.
//-----------------------------------------------------------------------------
void CEnvWarpBall::Think()
{
	if (gpGlobals->curtime > 0.5 + m_fTimeSinceEffectStarted && !m_bSecondSoundPlayed) {
		//if 0.5s have passed since start and second sound hasn't played yet...
		m_bSecondSoundPlayed = true;	//ensure this only runs once

		CPASAttenuationFilter filter(this->GetAbsOrigin(), STRING(m_iszSoundAName));
		filter.MakeReliable();
		EmitSound(filter, this->entindex(), STRING(m_iszSoundBName), &this->GetAbsOrigin());	//Play second sound
	} else if (gpGlobals->curtime > 1 + m_fTimeSinceEffectStarted) {
		//if 1s or more have passed since start... (runs every think frame after that point!)

		//Reduce alpha of green sprite to make it visibly fade away.
		m_fAlphaSpriteGreen -= 2.55;

		//update sprite with alpha value
		m_pGlowGreen->SetRenderColorA(clamp(m_fAlphaSpriteGreen, 0, 255));
	}

	if (m_fAlphaSpriteGreen <= 0) {
		//Has green sprite completely faded away?
		//If so, delete child entities and end the effect.
		UpdateOnRemove();

		m_bWaitForRefire = false;	//allow effect to be triggered again if need be.

		if (m_spawnflags & SF_REMOVE_ON_FIRE)
		{
			//If spawn flags say to destroy ourselves after firing, now is the time.
			UTIL_Remove(this);
		}
	}
	else {
		SetThink(&CEnvWarpBall::Think);			//Set Think to manage new effect
		SetNextThink(gpGlobals->curtime + 0.01f);	//^
	}
}


//------------------------------------------------------------------------------
// Purpose: Turns on the relay, allowing it to fire outputs.
//------------------------------------------------------------------------------
void CEnvWarpBall::InputEnable(inputdata_t& inputdata)
{
	m_bDisabled = false;
}


//------------------------------------------------------------------------------
// Purpose: Turns off the relay, preventing it from firing outputs.
//------------------------------------------------------------------------------
void CEnvWarpBall::InputDisable(inputdata_t& inputdata)
{
	m_bDisabled = true;
}


//------------------------------------------------------------------------------
// Purpose: Toggles the enabled/disabled state of the relay.
//------------------------------------------------------------------------------
void CEnvWarpBall::InputToggle(inputdata_t& inputdata)
{
	m_bDisabled = !m_bDisabled;
}


//-----------------------------------------------------------------------------
// Purpose: Input handler that triggers the relay.
//-----------------------------------------------------------------------------
void CEnvWarpBall::InputTrigger(inputdata_t& inputdata)
{
	if ((!m_bDisabled) && (!m_bWaitForRefire))	//if not disabled and not waiting for refire
	{
		m_OnTrigger.FireOutput(inputdata.pActivator, this);	//do effect after this I guess

		UpdateOnRemove();	//make sure we delete any leftovers just in case we were quickly retriggered

		SpawnEffectEntities();
		StartEffect();


		if (!(m_spawnflags & SF_ALLOW_FAST_RETRIGGER))	//if fast retrigger has not been set
		{
			//
			// Disable the relay so that it cannot be refired until after the last output
			// has been fired and post an input to re-enable ourselves.
			//
			m_bWaitForRefire = true;
		}
	}
}

//Start puppeteering the spawned objects to make the effect.
void CEnvWarpBall::StartEffect() {
	variant_t emptyVariant;
	m_pBeam->AcceptInput("TurnOn", this, this, emptyVariant, 10);	//make zap effect start right away
	m_pGlowExplode->TurnOn();	//show sprites
	m_pGlowGreen->TurnOn();	//show sprites

	CPASAttenuationFilter filter(this->GetAbsOrigin(), STRING(m_iszSoundAName));
	filter.MakeReliable();

	EmitSound(filter, this->entindex(), STRING(m_iszSoundAName), &this->GetAbsOrigin());	//Play first sound

	m_bSecondSoundPlayed = false;	//ensure next sound plays at right time

	m_fAlphaSpriteGreen = 255;	//init alpha counter value

	SetThink(&CEnvWarpBall::Think);			//Set Think to manage new effect
	SetNextThink(gpGlobals->curtime + 0.01f);	//^

	m_fTimeSinceEffectStarted = gpGlobals->curtime;	//keep track of time from this point
}

//Helper function
void CEnvWarpBall::SetEntKeyvalue(CBaseEntity* ent, const char* addoutputString, int outputID) {
	variant_t vString;
	vString.SetString(MAKE_STRING(addoutputString));
	ent->AcceptInput("AddOutput", this, this, vString, outputID);	//set keyvalue via addoutput
}

//Split to save my eyeballs from seeing this hogging the trigger section.
void CEnvWarpBall::SpawnEffectEntities() {
	//Spawn new entities
	Vector spritePos = this->GetAbsOrigin();

	//env_sprites
	m_pGlowExplode = CSprite::SpriteCreate(STRING(m_iszSpriteExplodeName), spritePos, FALSE);
	m_pGlowExplode->m_nRenderFX = kRenderFxNoDissipation;	//Constant Glow (14)
	m_pGlowExplode->m_nRenderMode = kRenderGlow;	//Glow (3)
	m_pGlowExplode->SetScale(1);
	m_pGlowExplode->AddSpawnFlags(2);	//Play Once (2)
	m_pGlowExplode->m_flSpriteFramerate = 10.0f;

	m_pGlowGreen = CSprite::SpriteCreate(STRING(m_iszSpriteGreenName), spritePos, FALSE);
	m_pGlowGreen->m_nRenderFX = kRenderFxNoDissipation;	//Constant Glow (14)
	m_pGlowGreen->m_nRenderMode = kRenderGlow;	//Glow (3)
	m_pGlowGreen->SetScale(1);
	//m_pGlowGreen->AddSpawnFlags(2);	//Play Once (2)
	m_pGlowGreen->m_flSpriteFramerate = 10.0f;


	//env_beam for zap effects. I'd rather hack env_beam to work without a .h file then copy it's code to make cbeam take its place.
	m_pBeam = (CBeam*)(CBaseEntity::Create("env_beam", spritePos, QAngle(0, 0, 0), this));	//This is weird and unwieldy sadly, but I prefer I/O based logic anyhow.

	SetEntKeyvalue(m_pBeam, "renderamt 150", 2);
	variant_t vColor;
	vColor.SetColor32(m_cBeamColor);
	m_pBeam->AcceptInput("Color", this, this, vColor, 1);	//set beam color
	SetEntKeyvalue(m_pBeam, "Radius 100", 2);
	SetEntKeyvalue(m_pBeam, "Life 0.5", 3);
	SetEntKeyvalue(m_pBeam, "BoltWidth 1.8", 4);
	SetEntKeyvalue(m_pBeam, "NoiseAmplitude 35", 5);
	std::string beamNameStr = std::string(STRING(m_iszSpriteBeamName));
	std::string textureStringSetter = (std::string("texture ") + beamNameStr);	//This better fucking work.
	DevMsg(textureStringSetter.c_str());
	SetEntKeyvalue(m_pBeam, textureStringSetter.c_str(), 6);
	SetEntKeyvalue(m_pBeam, "StrikeTime -.5", 7);
	variant_t vMyName;
	vMyName.SetString(MAKE_STRING(GetDebugName()));	//get our targetname (must actually have one for this to work right!)
	m_pBeam->AcceptInput("SetStartEntity", this, this, vMyName, 8);	//set start entity to me! In a horrible roundabout fashion X3
	variant_t vBeamSpawnFlags;
	vBeamSpawnFlags.SetInt(6);	//Toggle (2) + Random Strike (4)
	m_pBeam->AcceptInput("AddSpawnFlags", this, this, vBeamSpawnFlags, 9);	//set spawn flags
	SetEntKeyvalue(m_pBeam, "TextureScroll 35", 5);
	SetEntKeyvalue(m_pBeam, "HDRColorScale 1.0", 5);
	SetEntKeyvalue(m_pBeam, "decalname Bigshot", 5);

	m_pBeam->Spawn();
}

//------------------------------------------------------------------------------
// Purpose: Cleans up entities we've created.
//------------------------------------------------------------------------------
void CEnvWarpBall::UpdateOnRemove(void)
{
	if (m_pGlowExplode)
	{
		UTIL_Remove(m_pGlowExplode);
		m_pGlowExplode = NULL;
	}

	if (m_pGlowGreen)
	{
		UTIL_Remove(m_pGlowGreen);
		m_pGlowGreen = NULL;
	}

	if (m_pBeam)
	{
		UTIL_Remove(m_pBeam);
		m_pBeam = NULL;
	}

	SetThink(NULL);

	BaseClass::UpdateOnRemove();
}