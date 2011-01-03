/* Copyright (C) 2006 - 2011 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Herod
SD%Complete: 95
SDComment: Should in addition spawn Myrmidons in the hallway outside
SDCategory: Scarlet Monastery
EndScriptData */

#include "precompiled.h"
#include "escort_ai.h"

#define SAY_AGGRO                   -1189000
#define SAY_WHIRLWIND               -1189001
#define SAY_ENRAGE                  -1189002
#define SAY_KILL                    -1189003
#define EMOTE_ENRAGE                -1189004

#define SPELL_RUSHINGCHARGE         8260
#define SPELL_CLEAVE                15496
#define SPELL_WHIRLWIND             8989
#define SPELL_FRENZY                8269

#define ENTRY_SCARLET_TRAINEE       6575
#define ENTRY_SCARLET_MYRMIDON      4295

struct MANGOS_DLL_DECL boss_herodAI : public ScriptedAI
{
    boss_herodAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    bool Enrage;
    uint32 Cleave_Timer;
    uint32 Whirlwind_Timer;

    void Reset()
    {
        Enrage = false;
        Cleave_Timer = 12000;
        Whirlwind_Timer = 45000;
    }

    void Aggro(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);
        DoCastSpellIfCan(m_creature,SPELL_RUSHINGCHARGE);
    }

    void KilledUnit(Unit *victim)
    {
        DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* killer)
    {
        for(uint8 i = 0; i < 20; ++i)
            m_creature->SummonCreature(ENTRY_SCARLET_TRAINEE, 1939.18f, -431.58f, 17.09f, 6.22f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 600000);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //If we are <30% hp goes Enraged
        if (!Enrage && m_creature->GetHealthPercent() <= 30.0f && !m_creature->IsNonMeleeSpellCasted(false))
        {
            if (DoCastSpellIfCan(m_creature,SPELL_FRENZY) == CAST_OK)
            {
                DoScriptText(EMOTE_ENRAGE, m_creature);
                DoScriptText(SAY_ENRAGE, m_creature);
                Enrage = true;
            }
        }

        //Cleave_Timer
        if (Cleave_Timer < diff)
        {
            DoCastSpellIfCan(m_creature->getVictim(),SPELL_CLEAVE);
            Cleave_Timer = 12000;
        }else Cleave_Timer -= diff;

        if (Whirlwind_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_WHIRLWIND) == CAST_OK)
            {
                DoScriptText(SAY_WHIRLWIND, m_creature);
                Whirlwind_Timer = 30000;
            }
        }else Whirlwind_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_herod(Creature* pCreature)
{
    return new boss_herodAI(pCreature);
}

struct MANGOS_DLL_DECL mob_scarlet_traineeAI : public npc_escortAI
{
    mob_scarlet_traineeAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Start_Timer = urand(1000,6000);
        Reset();
    }

    uint32 Start_Timer;

    void Reset() { }
    void WaypointReached(uint32 uiPoint) { }

    void UpdateEscortAI(const uint32 diff)
    {
        if (Start_Timer)
        {
            if (Start_Timer <= diff)
            {
                Start(true,true);
                Start_Timer = 0;
            }else Start_Timer -= diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_scarlet_trainee(Creature* pCreature)
{
    return new mob_scarlet_traineeAI(pCreature);
}

void AddSC_boss_herod()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_herod";
    newscript->GetAI = &GetAI_boss_herod;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_scarlet_trainee";
    newscript->GetAI = &GetAI_mob_scarlet_trainee;
    newscript->RegisterSelf();
}
