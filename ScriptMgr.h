/* Copyright (C) 2006,2007 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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


#ifndef SCRIPTMGR_H
#define SCRIPTMGR_H

#include "../../game/Player.h"
#include "../../game/GameObject.h"
#include "../../game/SharedDefines.h"
#include "../../game/GossipDef.h"
#include "../../game/QuestDef.h"
#include "../../game/WorldSession.h"
#include "../../game/CreatureAI.h"
#include "../../game/Spell.h"
#include "../../game/TargetedMovementGenerator.h"
#include "../../shared/WorldPacket.h"
#include "../../shared/Database/DBCStores.h"

#define MAX_SCRIPTS 1000

struct Script
{
    Script() :
    pGossipHello(NULL), pQuestAccept(NULL), pGossipSelect(NULL), pGossipSelectWithCode(NULL),
        pQuestSelect(NULL), pQuestComplete(NULL), pNPCDialogStatus(NULL), pChooseReward(NULL),
        pItemHello(NULL), pGOHello(NULL), pAreaTrigger(NULL), pItemQuestAccept(NULL), pGOQuestAccept(NULL),
        pGOChooseReward(NULL),pReceiveEmote(NULL),pItemUse(NULL), GetAI(NULL)
        {}

    std::string Name;

    // -- Quest/gossip Methods to be scripted --
    bool (*pGossipHello         )(Player *player, Creature *_Creature);
    bool (*pQuestAccept         )(Player *player, Creature *_Creature, Quest *_Quest );
    bool (*pGossipSelect        )(Player *player, Creature *_Creature, uint32 sender, uint32 action );
    bool (*pGossipSelectWithCode)(Player *player, Creature *_Creature, uint32 sender, uint32 action, char* sCode );
    bool (*pQuestSelect         )(Player *player, Creature *_Creature, Quest *_Quest );
    bool (*pQuestComplete       )(Player *player, Creature *_Creature, Quest *_Quest );
    uint32 (*pNPCDialogStatus   )(Player *player, Creature *_Creature );
    bool (*pChooseReward        )(Player *player, Creature *_Creature, Quest *_Quest, uint32 opt );
    bool (*pItemHello           )(Player *player, Item *_Item, Quest *_Quest );
    bool (*pGOHello             )(Player *player, GameObject *_GO );
    bool (*pAreaTrigger         )(Player *player, Quest *_Quest, uint32 triggerID );
    bool (*pItemQuestAccept     )(Player *player, Item *_Item, Quest *_Quest );
    bool (*pGOQuestAccept       )(Player *player, GameObject *_GO, Quest *_Quest );
    bool (*pGOChooseReward      )(Player *player, GameObject *_GO, Quest *_Quest, uint32 opt );

    CreatureAI* (*GetAI)(Creature *_Creature);

    bool (*pReceiveEmote         )(Player *player, Creature *_Creature, uint32 emote);
    bool (*pItemUse             )(Player *player, Item* _Item);
    // -----------------------------------------

};

extern int nrscripts;
extern Script *m_scripts[MAX_SCRIPTS];

#define VISIBLE_RANGE (26.46f)

//Spell targets used by SelectSpell
enum SelectTarget
{
    SELECT_TARGET_DONTCARE = 0,         //All target types allowed
    
    SELECT_TARGET_SELF,                 //Only Self casting

    SELECT_TARGET_SINGLE_ENEMY,         //Only Single Enemy
    SELECT_TARGET_AOE_ENEMY,            //Only AoE Enemy
    SELECT_TARGET_ANY_ENEMY,            //AoE or Single Enemy

    SELECT_TARGET_SINGLE_FRIEND,        //Only Single Friend
    SELECT_TARGET_AOE_FRIEND,           //Only AoE Friend
    SELECT_TARGET_ANY_FRIEND,           //AoE or Single Friend
};

//Spell Effects used by SelectSpell
enum SelectEffect
{
    SELECT_EFFECT_DONTCARE = 0,         //All spell effects allowed
    SELECT_EFFECT_DAMAGE,               //Spell does damage
    SELECT_EFFECT_HEALING,              //Spell does healing
    SELECT_EFFECT_AURA,                 //Spell applies an aura
};

//Selection method used by SelectTarget
enum SelectAggroTarget
{
    SELECT_TARGET_RANDOM = 0,           //Just selects a random target
    SELECT_TARGET_TOPAGGRO,             //Selects targes from top aggro to bottom
    SELECT_TARGET_BOTTOMAGGRO,          //Selects targets from bottom aggro to top
};

//Chat defines
#define CHAT_MSG_MONSTER_SAY    0x0B
#define CHAT_MSG_MONSTER_YELL   0x0C
#define CHAT_MSG_MONSTER_EMOTE  0x0D

struct MANGOS_DLL_DECL ScriptedAI : public CreatureAI
{
    ScriptedAI(Creature* creature) : m_creature(creature) {}
    ~ScriptedAI() {}

    //*************
    //CreatureAI Functions to be Scripted
    //*************

    //Called if IsVisible(Unit *who) is true at each *who move
    void MoveInLineOfSight(Unit *) {}

    //Called at each attack of m_creature by any victim
    void AttackStart(Unit *);

    //Called at stoping attack by any attacker
    void EnterEvadeMode();

    //Called at any heal cast/item used (call non implemented in mangos)
    void HealBy(Unit *healer, uint32 amount_healed) {}

    // Called at any Damage to any victim (before damage apply)
    void DamageDeal(Unit *done_to, uint32 &damage) {}

    // Called at any Damage from any attacker (before damage apply)
    void DamageTaken(Unit *done_by, uint32 &damage) {}

    //Is unit visible for MoveInLineOfSight
    bool IsVisible(Unit *who) const
    {
        if (!who)
            return false;

        return m_creature->IsWithinDistInMap(who, VISIBLE_RANGE) && who->isVisibleForOrDetect(m_creature,true);
    }

    //Called at World update tick
    void UpdateAI(const uint32);

    //Called at creature death
    void JustDied(Unit*){}
    
    //Called at creature killing another unit
    void KilledUnit(Unit*){}

    //Pointer to creature we are manipulating
    Creature* m_creature;

    //Check condition for attack stop
    virtual bool needToStop() const;

    //*************
    //AI Helper Functions
    //*************

    //Start attack of victim and go to him
    void DoStartMeleeAttack(Unit* victim);

    //Start attack of victim but stay in position
    void DoStartRangedAttack(Unit* victim);

    //Stop attack of current victim
    void DoStopAttack();

    //Cast spell by Id
    void DoCast(Unit* victim, uint32 spelId);

    //Cast spell by spell info
    void DoCastSpell(Unit* who,SpellEntry const *spellInfo);

    //Creature say
    void DoSay(const char* text, uint32 language, Unit* target);

    //Creature Yell
    void DoYell(const char* text, uint32 language, Unit* target);

    //Creature Text emote
    void DoTextEmote(const char* text, Unit* target);

    //Go back to spawn point
    void DoGoHome();

    //Plays a sound to all nearby players
    void DoPlaySoundToSet(Unit* unit, uint32 sound);

    //Faces target
    void DoFaceTarget(Unit* unit);

    //Spawns a creature relative to m_creature
    Creature* DoSpawnCreature(uint32 id, float x, float y, float z, float angle, TempSummonType t, uint32 despawntime);
    
    //Selects a unit from the creature's current aggro list
    Unit* SelectUnit(SelectAggroTarget target, uint32 position);

    //Returns spells that meet the specified criteria from the creatures spell list
    SpellEntry const* SelectSpell(Unit* Target, int32 School, int32 Mechanic, SelectTarget Targets,  uint32 PowerCostMin, uint32 PowerCostMax, float RangeMin, float RangeMax, SelectEffect Effect);

    //Checks if you can cast the specified spell
    bool CanCast(Unit* Target, SpellEntry const *Spell);

    //Returns true if you are out of tether(spawnpoint) range
    bool CheckTether();
};
#endif
