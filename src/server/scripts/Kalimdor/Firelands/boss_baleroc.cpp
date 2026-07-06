/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Baleroc, the Gatekeeper - Firelands (4.3.4)
 *
 * Timer sources: 4.3.4 DBM (Baleroc.lua r7607) cross-checked against legacy
 * Firelands sniffs. Blaze of Glory every 8.5s, Shards of Torment every 34s
 * (first ~5s), blades first 30.5s then every 47s (15s active, random type -
 * sniffs show back-to-back Inferno Blades, blades do NOT alternate),
 * Countdown (heroic) every 45s, Berserk 6min.
 *
 * Strike delivery: Inferno Blade relies on the core-native
 * SPELL_AURA_OVERRIDE_AUTOATTACK_WITH_MELEE_SPELL (aura 361) - every melee
 * swing becomes Inferno Strike (sniff: 7 strikes ~2s apart per 15s blade).
 * Decimation Blade suppresses white swings entirely; Decimating Strike is
 * cast on a fixed schedule instead (DBM: 6s on 10-man, 3s on 25-man).
 *
 * Difficulty scaling: scripts always cast the 10N base spell IDs; the core
 * remaps them at cast time via SpellDifficulty.dbc (Tormented 3750,
 * Torment 3790, Inferno Strike 3858, Wave of Torment 3914, Decimation
 * Blade 3751).
 *
 * Vital Flame (99263) uses the unimplemented aura 359 (mod healing done
 * versus targets with Blaze of Glory); it is remapped to
 * SPELL_AURA_MOD_HEALING_DONE_PERCENT in SpellMgr corrections, so the bonus
 * applies to ALL healing done by the healer for its 15s duration. Accepted
 * simplification - healers are tank-healing during that window.
 *
 * Share the Pain (5830): "...without allowing any member of your raid to
 * suffer Torment more than three times" - counted on each fresh Torment
 * beam application (99255), not on Tormented debuff applications.
 */

#include "ScriptMgr.h"
#include "Containers.h"
#include "firelands.h"
#include "GameTime.h"
#include "GridNotifiers.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"

namespace Firelands::Baleroc
{
enum Spells
{
    SPELL_INFERNO_BLADE             = 99350,
    SPELL_INFERNO_STRIKE            = 99351,
    SPELL_DECIMATION_BLADE          = 99352,
    SPELL_DECIMATION_BLADE_25       = 99405,
    SPELL_DECIMATING_STRIKE         = 99353,

    SPELL_BLAZE_OF_GLORY            = 99252,
    SPELL_INCENDIARY_SOUL           = 99369,

    SPELL_SHARDS_OF_TORMENT         = 99259,
    SPELL_SHARDS_OF_TORMENT_SUMMON  = 99260,
    SPELL_TORMENT_COSMETIC          = 99258,
    SPELL_TORMENT_ENGINE            = 99254,    // shard self-aura, pulses the 99253 targeting dummy every 500ms
    SPELL_TORMENT_BEAM              = 99255,    // permanent-duration carrier on the soaker, ticks 99256 every 1s
    SPELL_TORMENT_DAMAGE            = 99256,    // damage + 2s rolling stack marker (heroic/25 clones 100230-100232)
    SPELL_WAVE_OF_TORMENT           = 99261,
    SPELL_TORMENTED                 = 99257,    // 20s base; remaps to 99402 (30s) / 99403 (40s) / 99404 (60s)

    SPELL_COUNTDOWN                 = 99515,    // targeting dummy
    SPELL_COUNTDOWN_MARKER          = 99516,    // 8s aura, natively pulses the 99517 proximity check every 200ms
    SPELL_COUNTDOWN_CHECK           = 99517,
    SPELL_COUNTDOWN_EXPLOSION       = 99518,
    SPELL_COUNTDOWN_LINK            = 99519,

    SPELL_VITAL_SPARK               = 99262,
    SPELL_VITAL_FLAME               = 99263,

    SPELL_SMOULDERING               = 101093,

    SPELL_BERSERK                   = 26662
};

enum Events
{
    EVENT_BLADE = 1,
    EVENT_RESTORE_WEAPONS,
    EVENT_BLAZE_OF_GLORY,
    EVENT_DECIMATING_STRIKE,
    EVENT_SHARDS_OF_TORMENT,
    EVENT_COUNTDOWN,
    EVENT_BERSERK,

    EVENT_SHARD_ACTIVATE,
    EVENT_SHARD_DEACTIVATE
};

enum Texts
{
    EMOTE_AGGRO                     = 0,
    EMOTE_SHARDS_OF_TORMENT         = 1,
    EMOTE_INFERNO_BLADE             = 2,
    EMOTE_DECIMATION_BLADE          = 3,
    EMOTE_KILL                      = 4,
    EMOTE_ENRAGE                    = 5,
    EMOTE_ENRAGE_2                  = 6,
    EMOTE_DEATH                     = 7,
    ABILITY_INFERNO_BLADE           = 8,
    ABILITY_DECIMATION_BLADE        = 9
};

enum Misc
{
    EQUIP_DECIMATION_BLADE          = 71082,
    EQUIP_INFERNO_BLADE             = 71138,

    GUID_TORMENTED                  = 1,
    DATA_SHARE_THE_PAIN             = 5830,

    ACTION_WAVE_OF_TORMENT          = 1,

    QUEST_ITEM_HEART_OF_FLAME       = 69848
};

uint32 const TormentedIds[] = { 99257, 99402, 99403, 99404 };

struct boss_baleroc : public BossAI
{
    boss_baleroc(Creature* creature) : BossAI(creature, DATA_BALEROC) { }

    void Reset() override
    {
        _Reset();
        me->SetMaxPower(POWER_RAGE, 0);
        SetEquipmentSlots(true);
        me->SetCanDualWield(true);
    }

    void SpellHit(WorldObject* /*caster*/, SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_INFERNO_BLADE:
                SetEquipmentSlots(false, EQUIP_INFERNO_BLADE, EQUIP_UNEQUIP);
                me->SetCanDualWield(false);
                events.ScheduleEvent(EVENT_RESTORE_WEAPONS, 15s);
                break;
            case SPELL_DECIMATION_BLADE:
            case SPELL_DECIMATION_BLADE_25:
                SetEquipmentSlots(false, EQUIP_DECIMATION_BLADE, EQUIP_UNEQUIP);
                me->SetCanDualWield(false);
                // White swings are replaced by scheduled strikes for the blade's duration (DBM: 6s / 3s cadence).
                events.ScheduleEvent(EVENT_DECIMATING_STRIKE, DecimatingStrikePeriod());
                events.ScheduleEvent(EVENT_RESTORE_WEAPONS, 15s);
                break;
            default:
                break;
        }
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        Talk(EMOTE_AGGRO);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

        events.ScheduleEvent(EVENT_SHARDS_OF_TORMENT, 5s);
        events.ScheduleEvent(EVENT_BLAZE_OF_GLORY, 8500ms);
        events.ScheduleEvent(EVENT_BLADE, 30500ms);
        events.ScheduleEvent(EVENT_BERSERK, 6min);
        if (IsHeroic())
            events.ScheduleEvent(EVENT_COUNTDOWN, 45s);

        // Reset here and not in Reset() - Tormented may still spread after the boss has reset.
        _tormentCounts.clear();
    }

    void KilledUnit(Unit* who) override
    {
        if (who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (!(rand32() % 5))
            Talk(EMOTE_KILL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(EMOTE_DEATH);
        SetEquipmentSlots(true);
        me->SetCanDualWield(true);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        Map::PlayerList const& playerList = me->GetMap()->GetPlayers();
        for (auto const& playerRef : playerList)
        {
            Player* player = playerRef.GetSource();
            if (player && player->HasQuestForItem(QUEST_ITEM_HEART_OF_FLAME))
            {
                DoCastAOE(SPELL_SMOULDERING);
                break;
            }
        }
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanupPlayerAuras();
        me->GetMotionMaster()->MoveTargetedHome();
        summons.DespawnAll();
        _DespawnAtEvade();
    }

    void CleanupPlayerAuras()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_BLAZE_OF_GLORY);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TORMENT_BEAM);
        for (uint8 i = 0; i < 4; ++i)
        {
            instance->DoRemoveAurasDueToSpellOnPlayers(sSpellMgr->GetSpellIdForDifficulty(SPELL_TORMENT_DAMAGE, me));
            instance->DoRemoveAurasDueToSpellOnPlayers(TormentedIds[i]);
        }
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VITAL_SPARK);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VITAL_FLAME);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_COUNTDOWN_MARKER);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_COUNTDOWN_LINK);
    }

    Milliseconds DecimatingStrikePeriod() const
    {
        return me->GetMap()->Is25ManRaid() ? 3s : 6s;
    }

    bool IsDecimationBladeActive() const
    {
        return me->HasAura(SPELL_DECIMATION_BLADE) || me->HasAura(SPELL_DECIMATION_BLADE_25);
    }

    // Share the Pain bookkeeping - fed by spell_baleroc_torment_beam on every fresh Torment application.
    void SetGUID(ObjectGuid const& guid, int32 id = 0) override
    {
        if (id == GUID_TORMENTED)
            ++_tormentCounts[guid];
    }

    uint32 GetData(uint32 type) const override
    {
        if (type != DATA_SHARE_THE_PAIN)
            return 0;

        for (auto const& pair : _tormentCounts)
            if (pair.second > 3)
                return 0;

        return 1;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BLADE:
                    if (urand(0, 1))
                    {
                        DoCastSelf(SPELL_INFERNO_BLADE);
                        Talk(EMOTE_INFERNO_BLADE);
                        Talk(ABILITY_INFERNO_BLADE);
                    }
                    else
                    {
                        DoCastSelf(SPELL_DECIMATION_BLADE);
                        Talk(EMOTE_DECIMATION_BLADE);
                        Talk(ABILITY_DECIMATION_BLADE);
                    }
                    events.Repeat(47s);
                    break;
                case EVENT_RESTORE_WEAPONS:
                    SetEquipmentSlots(true);
                    me->SetCanDualWield(true);
                    me->resetAttackTimer();
                    events.CancelEvent(EVENT_DECIMATING_STRIKE);
                    break;
                case EVENT_BLAZE_OF_GLORY:
                    if (Unit* victim = me->GetVictim())
                    {
                        DoCast(victim, SPELL_BLAZE_OF_GLORY);
                        DoCastSelf(SPELL_INCENDIARY_SOUL);
                    }
                    events.Repeat(8500ms);
                    break;
                case EVENT_DECIMATING_STRIKE:
                    if (Unit* victim = me->GetVictim())
                        if (me->IsWithinMeleeRange(victim))
                            DoCast(victim, SPELL_DECIMATING_STRIKE);
                    events.Repeat(DecimatingStrikePeriod());
                    break;
                case EVENT_SHARDS_OF_TORMENT:
                    Talk(EMOTE_SHARDS_OF_TORMENT);
                    DoCastAOE(SPELL_SHARDS_OF_TORMENT);
                    events.Repeat(34s);
                    break;
                case EVENT_COUNTDOWN:
                    DoCastAOE(SPELL_COUNTDOWN);
                    events.Repeat(45s);
                    break;
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK);
                    Talk(EMOTE_ENRAGE);
                    Talk(EMOTE_ENRAGE_2);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        // Decimation Blade replaces white swings entirely with the scheduled strikes above.
        // Outside of it, the native aura 361 override on Inferno Blade turns swings into Inferno Strikes.
        if (IsDecimationBladeActive())
            return;

        DoMeleeAttackIfReady();
    }

private:
    std::unordered_map<ObjectGuid, uint32> _tormentCounts;
};

struct npc_shard_of_torment : public ScriptedAI
{
    npc_shard_of_torment(Creature* creature) : ScriptedAI(creature), _lastWaveTime(0)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_REMOVE_CLIENT_CONTROL | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
        me->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
        me->SetDisplayFromModel(1);
        _instance = creature->GetInstanceScript();
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (summoner->GetEntry() != BOSS_BALEROC || _instance->GetBossState(DATA_BALEROC) != IN_PROGRESS)
        {
            me->DespawnOrUnsummon();
            return;
        }

        DoCast(SPELL_TORMENT_COSMETIC);
        // Sniff: torment starts ~5s after spawn, pulses stop ~30s after spawn, object destroyed ~48s after spawn.
        _events.ScheduleEvent(EVENT_SHARD_ACTIVATE, 5s);
        _events.ScheduleEvent(EVENT_SHARD_DEACTIVATE, 30s);
        me->DespawnOrUnsummon(48s);
        DoZoneInCombat();
    }

    void KilledUnit(Unit* who) override
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
            if (Creature* baleroc = _instance->GetCreature(DATA_BALEROC))
                baleroc->AI()->KilledUnit(who);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_WAVE_OF_TORMENT)
            return;

        // The 99253 targeting dummy pulses every 500ms - throttle the punishment wave to its intended 1s cadence.
        uint32 now = GameTime::GetGameTimeMS();
        if (_lastWaveTime && now - _lastWaveTime < 900)
            return;

        _lastWaveTime = now;
        DoCastAOE(SPELL_WAVE_OF_TORMENT, true);
    }

    // Drops Torment (beam + stack marker) applied by this shard from every player except keep.
    // The 99256 removal fires its AuraScript and applies Tormented to the outgoing soaker.
    void RemoveTormentFromOthers(Unit* keep)
    {
        uint32 damageSpellId = sSpellMgr->GetSpellIdForDifficulty(SPELL_TORMENT_DAMAGE, me);
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        for (auto const& ref : players)
        {
            Player* player = ref.GetSource();
            if (!player || player == keep)
                continue;

            player->RemoveAurasDueToSpell(SPELL_TORMENT_BEAM, me->GetGUID());
            player->RemoveAurasDueToSpell(damageSpellId, me->GetGUID());
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHARD_ACTIVATE:
                    me->RemoveAurasDueToSpell(SPELL_TORMENT_COSMETIC);
                    DoCast(SPELL_TORMENT_ENGINE);
                    break;
                case EVENT_SHARD_DEACTIVATE:
                    me->RemoveAurasDueToSpell(SPELL_TORMENT_ENGINE);
                    RemoveTormentFromOthers(nullptr);
                    break;
                default:
                    break;
            }
        }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    uint32 _lastWaveTime;
};

// 99515 - Countdown (heroic targeting dummy)
class spell_countdown_p1 : public SpellScript
{
    bool Load() override
    {
        target1 = nullptr;
        target2 = nullptr;
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void CastSpellLink()
    {
        if (!target1 || !target2)
            return;

        Player* player1 = target1->ToPlayer();
        Player* player2 = target2->ToPlayer();
        if (player1 && player2)
            player1->CastSpell(player2, SPELL_COUNTDOWN_LINK, true);
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_COUNTDOWN_MARKER, false);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj)
        {
            Player* player = obj->ToPlayer();
            return !player || !player->IsAlive();
        });

        // Never the active tank.
        if (Unit* victim = GetCaster()->GetVictim())
            targets.remove(victim);

        if (targets.size() < 2)
        {
            targets.clear();
            FinishCast(SPELL_FAILED_NO_VALID_TARGETS);
            return;
        }

        Trinity::Containers::RandomResize(targets, 2);
        target1 = targets.front();
        target2 = targets.back();
    }

    void Register() override
    {
        AfterCast.Register(&spell_countdown_p1::CastSpellLink);
        OnEffectHitTarget.Register(&spell_countdown_p1::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnObjectAreaTargetSelect.Register(&spell_countdown_p1::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }

    WorldObject* target1 = nullptr;
    WorldObject* target2 = nullptr;
};

// 99516 - Countdown (8s marker aura)
class spell_countdown_p2 : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();

        if (GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::Expired))
            target->CastSpell(nullptr, SPELL_COUNTDOWN_EXPLOSION, true);
        else if (GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::ByDeath))
        {
            // A linked player died - defuse the partner without detonation.
            Map::PlayerList const& players = target->GetMap()->GetPlayers();
            for (auto const& ref : players)
            {
                Player* player = ref.GetSource();
                if (player && player != target && player->HasAura(SPELL_COUNTDOWN_MARKER))
                {
                    player->RemoveAurasDueToSpell(SPELL_COUNTDOWN_MARKER);
                    player->RemoveAurasDueToSpell(SPELL_COUNTDOWN_LINK);
                }
            }
        }

        target->RemoveAurasDueToSpell(SPELL_COUNTDOWN_LINK);
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_countdown_p2::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 99517 - Countdown (proximity check, pulsed by the marker every 200ms, cast by the marked player)
class spell_countdown_p3 : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove(GetCaster());
        targets.remove_if([](WorldObject* obj)
        {
            Player* player = obj->ToPlayer();
            return !player || !player->HasAura(SPELL_COUNTDOWN_MARKER);
        });

        if (targets.empty())
            return;

        // The linked partner is within defuse range - clear both markers without detonation.
        for (WorldObject* obj : targets)
        {
            Player* player = obj->ToPlayer();
            player->RemoveAurasDueToSpell(SPELL_COUNTDOWN_MARKER);
            player->RemoveAurasDueToSpell(SPELL_COUNTDOWN_LINK);
        }

        GetCaster()->RemoveAurasDueToSpell(SPELL_COUNTDOWN_MARKER);
        GetCaster()->RemoveAurasDueToSpell(SPELL_COUNTDOWN_LINK);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_countdown_p3::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
    }
};

// 99353 - Decimating Strike
class spell_decimating_strike : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void ChangeDamage()
    {
        if (Unit* target = GetHitUnit())
            SetHitDamage(std::max<int32>(CalculatePct(static_cast<int32>(target->GetMaxHealth()), 90), 250000));
    }

    void Register() override
    {
        OnHit.Register(&spell_decimating_strike::ChangeDamage);
    }
};

// 99259 - Shards of Torment (targeting)
class spell_shards_of_torment : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        GetCaster()->CastSpell(GetHitUnit(), SPELL_SHARDS_OF_TORMENT_SUMMON, true);
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* obj)
        {
            Player* player = obj->ToPlayer();
            return !player || !player->IsAlive();
        });

        uint8 count = GetCaster()->GetMap()->Is25ManRaid() ? 2 : 1;

        // Avoid the tank when there are enough other targets.
        if (targets.size() > count)
            if (Unit* victim = GetCaster()->GetVictim())
                targets.remove(victim);

        if (targets.size() > count)
            Trinity::Containers::RandomResize(targets, count);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_shards_of_torment::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnObjectAreaTargetSelect.Register(&spell_shards_of_torment::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 99253 - Torment (shard targeting dummy, pulsed every 500ms by 99254)
class spell_baleroc_torment : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_UNIT;
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* caster = GetCaster();
        Creature* shard = caster->ToCreature();

        targets.remove_if([](WorldObject* obj)
        {
            Player* player = obj->ToPlayer();
            return !player || !player->IsAlive() || player->IsGameMaster();
        });

        Player* closest = nullptr;
        if (!targets.empty())
        {
            targets.sort(Trinity::ObjectDistanceOrderPred(caster, true));
            closest = targets.front()->ToPlayer();
        }

        if (!closest || closest->GetDistance2d(caster) > 15.0f)
        {
            // Nobody is soaking - drop the beam and punish the raid.
            targets.clear();
            if (shard && shard->IsAIEnabled())
            {
                shard->AI()->DoAction(ACTION_WAVE_OF_TORMENT);
                if (npc_shard_of_torment* shardAI = dynamic_cast<npc_shard_of_torment*>(shard->AI()))
                    shardAI->RemoveTormentFromOthers(nullptr);
            }
            return;
        }

        targets.clear();
        targets.push_back(closest);

        // Beam locks onto the closest player; the outgoing soaker loses Torment
        // instantly (and gains Tormented via the 99256 remove hook), so each new
        // soaker starts stacking from 1.
        if (shard)
            if (npc_shard_of_torment* shardAI = dynamic_cast<npc_shard_of_torment*>(shard->AI()))
                shardAI->RemoveTormentFromOthers(closest);

        if (!closest->HasAura(SPELL_TORMENT_BEAM, caster->GetGUID()))
            caster->CastSpell(closest, SPELL_TORMENT_BEAM, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_baleroc_torment::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 99255 - Torment (beam carrier) - Share the Pain accounting
class spell_baleroc_torment_beam : public AuraScript
{
    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        if (InstanceScript* instance = target->GetInstanceScript())
            if (Creature* baleroc = instance->GetCreature(DATA_BALEROC))
                baleroc->AI()->SetGUID(target->GetGUID(), GUID_TORMENTED);
    }

    void Register() override
    {
        AfterEffectApply.Register(&spell_baleroc_torment_beam::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 99256 / 100230 / 100231 / 100232 - Torment (damage + stack marker)
class spell_baleroc_tormented : public SpellScript
{
    void ChangeDamage()
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        // Per-difficulty base damage comes from the spell (clone); total tick scales with the stack count.
        uint32 stacks = std::max<uint32>(1, target->GetAuraCount(GetSpellInfo()->Id));
        SetHitDamage(GetHitDamage() * stacks);
    }

    void Register() override
    {
        OnHit.Register(&spell_baleroc_tormented::ChangeDamage);
    }
};

class spell_baleroc_tormented_AuraScript : public AuraScript
{
    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::ByDeath))
            return;

        // Cast the base spell - the core remaps to the 30/40/60s variants per difficulty.
        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_TORMENTED, true);
    }

    // Vital Spark: healing the Torment soaker infuses the healer, scaling with the soak's stack count.
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        return healer && healer->GetTypeId() == TYPEID_PLAYER && healer != GetUnitOwner()
            && eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetHeal();
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        uint32 total = std::max<uint32>(1, GetStackAmount() / 3);
        if (Aura* spark = healer->GetAura(SPELL_VITAL_SPARK))
            total += spark->GetStackAmount();

        healer->CastSpell(healer, SPELL_VITAL_SPARK, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, total));
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_baleroc_tormented_AuraScript::OnRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoCheckProc.Register(&spell_baleroc_tormented_AuraScript::CheckProc);
        OnProc.Register(&spell_baleroc_tormented_AuraScript::HandleProc);
    }
};

// 99252 - Blaze of Glory: healing the marked tank converts the healer's Vital Sparks into Vital Flame.
class spell_baleroc_blaze_of_glory : public AuraScript
{
    bool CheckProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        return healer && healer->GetTypeId() == TYPEID_PLAYER && healer->HasAura(SPELL_VITAL_SPARK)
            && eventInfo.GetHealInfo() && eventInfo.GetHealInfo()->GetHeal();
    }

    void HandleProc(ProcEventInfo& eventInfo)
    {
        Unit* healer = eventInfo.GetActor();
        Aura* spark = healer->GetAura(SPELL_VITAL_SPARK);
        if (!spark)
            return;

        // +5% healing per consumed spark; stacks consumed while a Flame is already
        // burning add to the existing bonus. Sparks are restored when Flame expires.
        int32 bonus = 5 * spark->GetStackAmount();
        if (Aura* flame = healer->GetAura(SPELL_VITAL_FLAME))
            if (AuraEffect const* eff = flame->GetEffect(EFFECT_0))
                bonus += eff->GetAmount();

        healer->RemoveAurasDueToSpell(SPELL_VITAL_SPARK);
        healer->CastSpell(healer, SPELL_VITAL_FLAME, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellBP0(bonus));
    }

    void Register() override
    {
        DoCheckProc.Register(&spell_baleroc_blaze_of_glory::CheckProc);
        OnProc.Register(&spell_baleroc_blaze_of_glory::HandleProc);
    }
};

// 99263 - Vital Flame: consumed Vital Sparks return when the flame expires.
class spell_baleroc_vital_flame : public AuraScript
{
    void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (!GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::Expired))
            return;

        uint32 stacks = std::max<int32>(aurEff->GetAmount(), 0) / 5;
        if (!stacks)
            return;

        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_VITAL_SPARK, CastSpellExtraArgs(TRIGGERED_FULL_MASK).AddSpellMod(SPELLVALUE_AURA_STACK, stacks));
    }

    void Register() override
    {
        // EFFECT_0 is remapped from the unimplemented aura 359 to MOD_HEALING_DONE_PERCENT in SpellMgr corrections.
        AfterEffectRemove.Register(&spell_baleroc_vital_flame::OnRemove, EFFECT_0, SPELL_AURA_MOD_HEALING_DONE_PERCENT, AURA_EFFECT_HANDLE_REAL);
    }
};

// 99489 - Tormented (heroic contagion, script effect cast by the Tormented player)
class spell_baleroc_tormented_heroic : public SpellScript
{
    bool Load() override
    {
        return GetCaster()->GetTypeId() == TYPEID_PLAYER;
    }

    void HandleScript(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
        if (!GetCaster()->GetMap()->IsHeroic())
            return;

        // Fresh full-duration application; the core remaps 99257 to the heroic variants.
        if (Unit* target = GetHitUnit())
            target->CastSpell(target, SPELL_TORMENTED, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_baleroc_tormented_heroic::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class achievement_share_the_pain : public AchievementCriteriaScript
{
    public:
        achievement_share_the_pain() : AchievementCriteriaScript("achievement_share_the_pain") { }

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            if (!target || !target->IsAIEnabled())
                return false;

            return target->GetAI()->GetData(DATA_SHARE_THE_PAIN) != 0;
        }
};
}

void AddSC_boss_baleroc()
{
    using namespace Firelands;
    using namespace Firelands::Baleroc;

    RegisterFirelandsCreatureAI(boss_baleroc);
    RegisterFirelandsCreatureAI(npc_shard_of_torment);

    RegisterSpellScript(spell_countdown_p1);
    RegisterSpellScript(spell_countdown_p2);
    RegisterSpellScript(spell_countdown_p3);
    RegisterSpellScript(spell_decimating_strike);
    RegisterSpellScript(spell_shards_of_torment);
    RegisterSpellScript(spell_baleroc_torment);
    RegisterSpellScript(spell_baleroc_torment_beam);
    RegisterSpellAndAuraScriptPair(spell_baleroc_tormented, spell_baleroc_tormented_AuraScript);
    RegisterSpellScript(spell_baleroc_blaze_of_glory);
    RegisterSpellScript(spell_baleroc_vital_flame);
    RegisterSpellScript(spell_baleroc_tormented_heroic);

    new achievement_share_the_pain();
}
