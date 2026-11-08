/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/engine.h>

#include <engine/shared/config.h>

#include <generated/client_data.h>

#include <game/client/components/particles.h>
#include <game/client/components/skins.h>
#include <game/client/components/flow.h>
#include <game/client/components/damageind.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>

#include "effects.h"

inline vec2 RandomDir() { return normalize(vec2(random_float()-0.5f, random_float()-0.5f)); }

CEffects::CEffects()
{
	m_Add50hz = false;
	m_Add100hz = false;
	mem_zero(m_aDamageTaken, sizeof(m_aDamageTaken));
	mem_zero(m_aDamageTakenTick, sizeof(m_aDamageTakenTick));
}

void CEffects::AirJump(vec2 Pos)
{
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_AIRJUMP;
	p.m_Pos = Pos + vec2(-6.0f, 16.0f);
	p.m_Vel = vec2(0, -200);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 48.0f;
	p.m_EndSize = 0;
	p.m_Rot = random_float()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.7f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	p.m_Pos = Pos + vec2(6.0f, 16.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_AIRJUMP, 1.0f, Pos);
}

void CEffects::DamageIndicator(vec2 Pos, int Amount, float Angle, int ClientID)
{
	// ignore if there is no damage
	if(Amount == 0)
		return;

	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
	{
		m_pClient->m_pDamageind->Create(vec2(Pos.x, Pos.y), direction(Angle));
		return;
	}

	m_aDamageTaken[ClientID]++;

	// create healthmod indicator
	if(Client()->LocalTime() < m_aDamageTakenTick[ClientID]+0.5f)
	{
		// make sure that the damage indicators don't group together
		Angle = m_aDamageTaken[ClientID]*0.25f;
	}
	else
	{
		m_aDamageTaken[ClientID] = 0;
		Angle = 0;
	}

	float a = 3*pi/2 + Angle;
	float s = a-pi/3;
	float e = a+pi/3;
	for(int i = 0; i < Amount; i++)
	{
		float f = mix(s, e, float(i+1)/float(Amount+2));
		m_pClient->m_pDamageind->Create(vec2(Pos.x, Pos.y), direction(f));
	}

	m_aDamageTakenTick[ClientID] = Client()->LocalTime();
}

void CEffects::PowerupShine(vec2 Pos, vec2 size)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SLICE;
	p.m_Pos = Pos + vec2((random_float()-0.5f)*size.x, (random_float()-0.5f)*size.y);
	p.m_Vel = vec2(0, 0);
	p.m_LifeSpan = 0.5f;
	p.m_StartSize = 16.0f;
	p.m_EndSize = 0;
	p.m_Rot = random_float()*pi*2;
	p.m_Rotspeed = pi*2;
	p.m_Gravity = 500;
	p.m_Friction = 0.9f;
	p.m_FlowAffected = 0.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
}

void CEffects::SmokeTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add50hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + random_float()*0.5f;
	p.m_StartSize = 12.0f + random_float()*8;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = random_float()*-500.0f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}


void CEffects::SkidTrail(vec2 Pos, vec2 Vel)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_SMOKE;
	p.m_Pos = Pos;
	p.m_Vel = Vel + RandomDir()*50.0f;
	p.m_LifeSpan = 0.5f + random_float()*0.5f;
	p.m_StartSize = 24.0f + random_float()*12;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	p.m_Gravity = random_float()*-500.0f;
	p.m_Color = vec4(0.75f,0.75f,0.75f,1.0f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	// immersion: small dust grains kicked up by the skid
	if(random_int()%3 == 0)
	{
		CParticle d;
		d.SetDefault();
		d.m_Spr = SPRITE_PART_BALL;
		d.m_Pos = Pos;
		d.m_Vel = Vel + RandomDir()*120.0f + vec2(0, -60.0f);
		d.m_LifeSpan = 0.2f + random_float()*0.2f;
		d.m_StartSize = 4.0f + random_float()*3.0f;
		d.m_EndSize = 0;
		d.m_Gravity = 600.0f;
		d.m_Friction = 0.8f;
		d.m_Color = vec4(0.65f, 0.6f, 0.5f, 0.8f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &d);
	}
}

void CEffects::BulletTrail(vec2 Pos)
{
	if(!m_Add100hz)
		return;

	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_BALL;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.25f + random_float()*0.25f;
	p.m_StartSize = 8.0f;
	p.m_EndSize = 0;
	p.m_Friction = 0.7f;
	m_pClient->m_pParticles->Add(CParticles::GROUP_PROJECTILE_TRAIL, &p);
}

void CEffects::PlayerSpawn(vec2 Pos)
{
	for(int i = 0; i < 32; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SHELL;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * (powf(random_float(), 3)*600.0f);
		p.m_LifeSpan = 0.3f + random_float()*0.3f;
		p.m_StartSize = 64.0f + random_float()*32;
		p.m_EndSize = 0;
		p.m_Rot = random_float()*pi*2;
		p.m_Rotspeed = random_float();
		p.m_Gravity = random_float()*-400.0f;
		p.m_Friction = 0.7f;
		p.m_Color = vec4(0xb5/255.0f, 0x50/255.0f, 0xcb/255.0f, 1.0f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);

	}
	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_PLAYER_SPAWN, 1.0f, Pos);
}

void CEffects::PlayerDeath(vec2 Pos, int ClientID)
{
	vec3 BloodColor(1.0f,1.0f,1.0f);

	if(ClientID >= 0)
	{
		if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		{
			int ColorVal = m_pClient->m_pSkins->GetTeamColor(
				m_pClient->m_aClients[ClientID].m_aUseCustomColors[SKINPART_BODY],
				m_pClient->m_aClients[ClientID].m_aSkinPartColors[SKINPART_BODY],
				m_pClient->m_aClients[ClientID].m_Team, SKINPART_BODY);
			BloodColor = m_pClient->m_pSkins->GetColorV3(ColorVal);
		}
		else
		{
			if(m_pClient->m_aClients[ClientID].m_aUseCustomColors[SKINPART_BODY])
				BloodColor = m_pClient->m_pSkins->GetColorV3(m_pClient->m_aClients[ClientID].m_aSkinPartColors[SKINPART_BODY]);
			else
			{
				const CSkins::CSkinPart *pSkinPart = m_pClient->m_pSkins->GetSkinPart(SKINPART_BODY, m_pClient->m_aClients[ClientID].m_SkinPartIDs[SKINPART_BODY]);
				if(pSkinPart)
					BloodColor = pSkinPart->m_BloodColor;
			}
		}
	}

	for(int i = 0; i < 64; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SPLAT01 + (random_int()%3);
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((random_float()+0.1f)*900.0f);
		p.m_LifeSpan = 0.3f + random_float()*0.3f;
		p.m_StartSize = 24.0f + random_float()*16;
		p.m_EndSize = 0;
		p.m_Rot = random_float()*pi*2;
		p.m_Rotspeed = (random_float()-0.5f) * pi;
		p.m_Gravity = 800.0f;
		p.m_Friction = 0.8f;
		vec3 c = BloodColor * (0.75f + random_float()*0.25f);
		p.m_Color = vec4(c.r, c.g, c.b, 0.75f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}

	// immersion: fine blood mist and heavier drops that fall longer
	for(int i = 0; i < 24; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SPLAT01 + (random_int()%3);
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((random_float()+0.2f)*500.0f) + vec2(0, -150.0f);
		p.m_LifeSpan = 0.6f + random_float()*0.5f;
		p.m_StartSize = 10.0f + random_float()*10.0f;
		p.m_EndSize = 0;
		p.m_Rot = random_float()*pi*2;
		p.m_Rotspeed = (random_float()-0.5f) * pi;
		p.m_Gravity = 1100.0f;
		p.m_Friction = 0.9f;
		vec3 c = BloodColor * (0.5f + random_float()*0.3f);
		p.m_Color = vec4(c.r, c.g, c.b, 0.85f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}
}


void CEffects::Explosion(vec2 Pos)
{
	// add to flow
	for(int y = -8; y <= 8; y++)
		for(int x = -8; x <= 8; x++)
		{
			if(x == 0 && y == 0)
				continue;

			float a = 1 - (length(vec2(x,y)) / length(vec2(8,8)));
			m_pClient->m_pFlow->Add(Pos+vec2(x,y)*16, normalize(vec2(x,y))*5000.0f*a, 10.0f);
		}

	// add the explosion
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_EXPL01;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.4f;
	p.m_StartSize = 150.0f;
	p.m_EndSize = 0;
	p.m_Rot = random_float()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);

	// immersion: short bright flash at the core of the explosion
	CParticle f;
	f.SetDefault();
	f.m_Spr = SPRITE_PART_BALL;
	f.m_Pos = Pos;
	f.m_LifeSpan = 0.12f;
	f.m_StartSize = 200.0f;
	f.m_EndSize = 0;
	f.m_Color = vec4(1.0f, 0.85f, 0.5f, 0.6f);
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &f);

	// add the smoke
	for(int i = 0; i < 24; i++)
	{
		CParticle p;
		p.SetDefault();
		p.m_Spr = SPRITE_PART_SMOKE;
		p.m_Pos = Pos;
		p.m_Vel = RandomDir() * ((1.0f + random_float()*0.2f) * 1000.0f);
		p.m_LifeSpan = 0.5f + random_float()*0.4f;
		p.m_StartSize = 32.0f + random_float()*8;
		p.m_EndSize = 0;
		p.m_Gravity = random_float()*-800.0f;
		p.m_Friction = 0.4f;
		p.m_Color = mix(vec4(0.75f,0.75f,0.75f,1.0f), vec4(0.5f,0.5f,0.5f,1.0f), random_float());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
	}

	// immersion: glowing debris/sparks thrown out and pulled down by gravity
	for(int i = 0; i < 16; i++)
	{
		CParticle d;
		d.SetDefault();
		d.m_Spr = SPRITE_PART_BALL;
		d.m_Pos = Pos;
		d.m_Vel = RandomDir() * ((0.4f + random_float()*0.8f) * 900.0f);
		d.m_LifeSpan = 0.4f + random_float()*0.4f;
		d.m_StartSize = 6.0f + random_float()*5.0f;
		d.m_EndSize = 0;
		d.m_Gravity = 900.0f;
		d.m_Friction = 0.85f;
		d.m_Color = mix(vec4(1.0f, 0.8f, 0.3f, 1.0f), vec4(1.0f, 0.4f, 0.1f, 1.0f), random_float());
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &d);
	}
}


void CEffects::HammerHit(vec2 Pos)
{
	// add the explosion
	CParticle p;
	p.SetDefault();
	p.m_Spr = SPRITE_PART_HIT01;
	p.m_Pos = Pos;
	p.m_LifeSpan = 0.3f;
	p.m_StartSize = 120.0f;
	p.m_EndSize = 0;
	p.m_Rot = random_float()*pi*2;
	m_pClient->m_pParticles->Add(CParticles::GROUP_EXPLOSIONS, &p);

	// immersion: sparks flying off the impact point
	for(int i = 0; i < 8; i++)
	{
		CParticle s;
		s.SetDefault();
		s.m_Spr = SPRITE_PART_BALL;
		s.m_Pos = Pos;
		s.m_Vel = RandomDir() * ((0.3f + random_float()*0.7f) * 600.0f);
		s.m_LifeSpan = 0.2f + random_float()*0.2f;
		s.m_StartSize = 5.0f + random_float()*3.0f;
		s.m_EndSize = 0;
		s.m_Gravity = 700.0f;
		s.m_Friction = 0.85f;
		s.m_Color = vec4(1.0f, 0.9f, 0.5f, 1.0f);
		m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &s);
	}

	m_pClient->m_pSounds->PlayAt(CSounds::CHN_WORLD, SOUND_HAMMER_HIT, 1.0f, Pos);
}

void CEffects::OnRender()
{
	const float Speed = m_pClient->GetAnimationPlaybackSpeed();
	if(Speed <= 0.0f)
		return;

	static int64 s_LastUpdate100hz = 0;
	static int64 s_LastUpdate50hz = 0;

	const int64 Now = time_get();
	const int64 Freq = time_freq();

	if(Now-s_LastUpdate100hz > Freq/(100*Speed))
	{
		m_Add100hz = true;
		s_LastUpdate100hz = Now;
	}
	else
		m_Add100hz = false;

	if(Now-s_LastUpdate50hz > Freq/(50*Speed))
	{
		m_Add50hz = true;
		s_LastUpdate50hz = Now;
	}
	else
		m_Add50hz = false;

	if(m_Add50hz)
		m_pClient->m_pFlow->Update();
}
