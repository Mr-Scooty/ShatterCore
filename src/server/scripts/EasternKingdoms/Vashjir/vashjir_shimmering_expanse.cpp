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
 * Shimmering Expanse quest scripts (sniff-reconstructed, see scratchpad arc
 * reports S1-S7):
 *
 *  - Legion's Rest: Spirit Vision trance chain (25334), naga defense waves and
 *    the Fathom-Lord Zin'jatar showdown (25164). Retail drives the event with
 *    player auras: 74850 (phase 170) -> 74848 (phase 171 + wave engine) ->
 *    74849 (phase 172) -> 75324 (base). The aura-native wave engine
 *    (74848 -> 74845 -> 74843) summons at implicit dest (on the player), so
 *    its periodic is suppressed and the controller bunny 40163 owns the waves.
 *  - Vortex (25441): Toshe's Vortex whirlpool 40277 (vehicle 735) sucking in
 *    Swarming Serpents via 75104/75109/46598/75574.
 *  - Nespirah (25890/25922): tunnel areatrigger 5958 escort vignette and the
 *    Swiftfin escape seahorse ride out of the shell.
 *  - Visions of the Past (25760/25755/25626): the three Battlemaiden
 *    possession vehicles (SummonProperties 827 + ride 76546 are core-native;
 *    scripts add the blade -> transform bunny glue, RP, exits and phasing).
 *  - Quel'Dormir finale (25951): two-stage bridge defense controller
 *    (phase 183 hold -> 78323/phase 184 counter-push -> Hagrim -> 78324).
 *  - Full Circle (26219): static-submarine "fake voyage" to Darkbreak Cove
 *    (same pattern as the Gilneas static gunship).
 */

#include "ScriptMgr.h"
#include "vashjir.h"
#include "Containers.h"
#include "EventMap.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include <list>

namespace Vashjir::ShimmeringExpanse
{

enum ShimmeringCreatures
{
    // Legion's Rest
    NPC_FARSEER_GADRA               = 39226,
    NPC_SPIRIT_OF_GADRA             = 40398, // summoned by 75482
    NPC_CAVE_DEFENSE_CONTROLLER     = 40163,
    NPC_FATHOM_LORD_ZINJATAR_CAVE   = 40161, // phase-172 showdown copy
    NPC_TSUNAMI_BUNNY               = 40375,
    NPC_TOSHE_CHAOSRENDER           = 39877,
    NPC_TOSHE_PHASE_172             = 40366,
    NPC_EARTHEN_RING_SHAMAN         = 39411,
    NPC_GREATER_EARTH_ELEMENTAL     = 39389,
    NPC_GREATER_FIRE_ELEMENTAL      = 40831,
    NPC_DERTH_FIRESINGER            = 39874,
    NPC_FATHOM_STALKER_WAVE_1       = 40162,
    NPC_FATHOM_STALKER_WAVE_2       = 40372,
    NPC_FATHOM_STALKER_WAVE_3       = 39397,
    NPC_FEND_OFF_CREDIT             = 40163,

    // Vortex (25441)
    NPC_TOSHES_VORTEX               = 40277, // vehicle 735
    NPC_SWARMING_SERPENT            = 40280,
    NPC_ZINJATAR_RAVAGER            = 40275,

    // Nespirah (25890 / 25922)
    NPC_DUARN_ESCORT                = 41532, // summoned by 77632
    NPC_ERUNAK_ESCORT               = 41803, // summoned by 77963
    NPC_ESCAPE_SEAHORSE_A           = 41785, // vehicle 840
    NPC_ESCAPE_SEAHORSE_H           = 41778, // vehicle 840
    NPC_SWIFTFIN_SEAHORSE_CLICK     = 41776,

    // Battlemaiden visions
    NPC_TRANSFORM_BUNNY_VISION_1    = 41160,
    NPC_TRANSFORM_BUNNY_VISION_2    = 41436,
    NPC_TRANSFORM_BUNNY_VISION_3    = 41484,
    NPC_BATTLEMAIDEN_VISION_1       = 39584, // vehicle 694
    NPC_BATTLEMAIDEN_VISION_2       = 41225, // vehicle 812
    NPC_BATTLEMAIDEN_VISION_3       = 41986, // vehicle 848
    NPC_AZJENTUS_VISION_1           = 40978,
    NPC_KVALDIR_LIMBRIPPER          = 41105,
    NPC_KVALDIR_SANDTERROR          = 41451,
    NPC_KVALDIR_PILLAGER            = 41102,
    NPC_KVALDIR_WASTEROAMER         = 41106,
    NPC_KVALDIR_DEEPWALKER          = 41107,
    NPC_KVALDIR_PLUNDERER           = 41108,
    NPC_WAR_PARTY_HARPOONER         = 44421, // summoned by 82976
    NPC_WAR_PARTY_TIDE_PRIESTESS    = 44422, // summoned by 82977
    NPC_WAR_PARTY_FATHOM_STALKER    = 44423, // summoned by 82975

    // Quel'Dormir temple / bridge finale
    NPC_TEMPLE_CREDIT_BUNNY         = 41982, // 25626 objective 1 proximity credit
    NPC_BRIDGE_CONTROLLER           = 42135, // "Defend the Bridge Quad Credit" doubles as event controller
    NPC_KVALDIR_BONESNAPPER         = 42057,
    NPC_KVALDIR_SANDREAPER          = 42058,
    NPC_KVALDIR_SKINFLAYER          = 42060,
    NPC_LADY_NAZJAR_TEMPLE          = 42077,
    NPC_FATHOM_LORD_TEMPLE          = 42073,
    NPC_AZJENTUS_TEMPLE             = 42075,
    NPC_HAGRIM_HOPEBREAKER          = 42063,
    NPC_GENERIC_CONTROLLER_CSA      = 40789,

    // Honor and Privilege / Full Circle
    NPC_JORLAN_TRUEBLADE            = 40645,
    NPC_RESCUE_BALLOON              = 41572,
    NPC_CHIEF_ENGINEER_YOON         = 42488,
    NPC_CAPTAIN_GLOVAAL_SUB         = 48423,
    NPC_FIRST_LIEUTENANT_WILEY      = 48429,
    NPC_BOARDING_CREDIT             = 42486,
    NPC_CAVERN_CREDIT               = 42487
};

enum ShimmeringSpells
{
    // Spirit Vision (25334)
    SPELL_SPIRIT_TRANCE             = 74386, // E0 stun aura, BP 74385 (cast on expire - scripted)
    SPELL_SPIRIT_VISION_TELEPORT    = 74385, // teleport (dest-db) + linked 81812/75492 (native aura 284)
    SPELL_SPIRIT_VISION_2           = 75492, // E2 triggers 81811
    SPELL_SPIRIT_VISION_3           = 81812, // phase 194
    SPELL_SPIRIT_VISION_TIMER       = 81811, // E0 dummy aura BP 75482 (cast on expire - scripted)
    SPELL_SUMMON_SPIRIT_OF_GADRA    = 75482, // summon 40398 at dest-db
    SPELL_SPIRIT_VISION_KILL_CREDIT = 75479, // E90 KC 40307 at summoner
    SPELL_SPIRIT_VISION_RETURN      = 74391, // stun + teleport back (dest-db)

    // Legion's Rest defense (25164)
    SPELL_PHASE_2_INTRO_CAVE        = 74848, // phase 171 + wave-engine periodic (E1 suppressed)
    SPELL_PHASE_3_INTRO_CAVE        = 74849, // phase 172, removes 74848
    SPELL_SEE_QUEST_INVIS_3_CAVE    = 75334,
    SPELL_REMOVE_PHASE_3            = 75324, // removes 74849
    SPELL_TSUNAMI_KNOCKBACK         = 75312, // knockback + KC 40161 (conditions target players)
    SPELL_99_DAMAGE_REDUCTION       = 76187,
    SPELL_FROST_CAST                = 70452,

    // Vortex (25441)
    SPELL_TOSHES_VORTEX_AURA        = 75104, // periodic 1 s -> 75109
    SPELL_TOSHES_VORTEX_TRIGGER     = 75109, // dummy AoE - scripted target grab
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_SERPENT_TRAP_CREDIT       = 75574, // KC2 40277 at summoner

    // Nespirah (25890 / 25922)
    SPELL_SUMMON_DUARN_ESCORT       = 77632,
    SPELL_SUMMON_ERUNAK_ESCORT      = 77963, // E1 dummy self-aura used as once-only marker
    SPELL_SUMMON_ESCAPE_SEAHORSE    = 77927, // E0 dummy BP 77915 (H), E1 dummy BP 77920 (A), E2 KC2 41776
    SPELL_SUMMON_SEAHORSE_ALLIANCE  = 77920,
    SPELL_SUMMON_SEAHORSE_HORDE     = 77915,

    // Battlemaiden visions
    SPELL_BLADE_OF_THE_BATTLEMAIDEN = 77292, // E77 at nearby transform bunny (conditions)
    SPELL_FORCECAST_BATTLEMAIDEN_V1 = 77293,
    SPELL_FORCECAST_BATTLEMAIDEN_V2 = 77566,
    SPELL_FORCECAST_BATTLEMAIDEN_V3 = 78265,
    SPELL_NAZJAR_BATTLEMAIDEN_V1    = 73974, // summon 39584 + phase 170 + screen fx
    SPELL_NAZJAR_BATTLEMAIDEN_V2    = 77565, // summon 41225 + phase 171 + screen fx
    SPELL_NAZJAR_BATTLEMAIDEN_V3    = 78264, // summon 41986 + trigger 78332 + screen fx
    SPELL_RIDE_BATTLEMAIDEN         = 76546,
    SPELL_BLESSING_OF_AZSHARA       = 76580,
    SPELL_SUMMON_REINFORCEMENT_AURA = 76570, // periodic 6 s -> 76569
    SPELL_REINFORCEMENT_PING        = 76569, // dummy AoE - scripted (kvaldir cast 8297x)
    SPELL_SUMMON_FATHOM_STALKER     = 82975,
    SPELL_SUMMON_IDRAKESS_SLAVER    = 82976,
    SPELL_SUMMON_TIDE_PRIESTESS     = 82977,
    SPELL_VISION_1_CREDIT           = 77283, // E90 KC 41220 - exit for vision 1 (+ teleport back)
    SPELL_VISION_2_CREDIT           = 77284, // E90 KC 41221 - exit for vision 2 (in place)
    SPELL_VISION_3_CREDIT           = 77285, // E90 KC 41222 - exit for vision 3 (25951 reward spell)
    SPELL_BATTLEMAIDEN_BACKUP       = 80674, // E77 - grants missed vision-1 credit
    SPELL_BATTLEMAIDEN_FINAL_PHASE  = 78332, // E77 - applies 78263
    SPELL_PHASE_RUINS_179           = 77359,
    SPELL_PHASE_RUINS_180           = 77665,
    SPELL_PHASE_TEMPLE_172          = 78263,
    SPELL_PHASE_TEMPLE_183          = 78322, // 25860 source spell
    SPELL_PHASE_TEMPLE_184          = 78323,
    SPELL_PHASE_TEMPLE_185          = 78324,

    // Honor and Privilege (25898)
    SPELL_SEE_QUEST_INVIS_5         = 77861, // 25898 source spell
    SPELL_RESCUE_FLARE              = 77741, // dummy - scripted delayed KC 41572

    // Full Circle (26005 / 26219)
    SPELL_SEE_QUEST_INVIS_18        = 87236,
    SPELL_SUB_PHASE_GROUP           = 79230, // aura 326, phase group 543 {169, 228} - core-native
    SPELL_MOVE_OCCUPANTS_TO_LAND    = 79239  // teleport to Darkbreak Cove (dest-db) + phase updates
};

enum ShimmeringQuests
{
    QUEST_THE_LOOMING_THREAT        = 25334,
    QUEST_BACKED_INTO_A_CORNER      = 25164,
    QUEST_VORTEX                    = 25441,
    QUEST_NESPIRAH                  = 25890,
    QUEST_WAKING_THE_BEAST          = 25922,
    QUEST_VISIONS_INVASION          = 25760, // vision 1
    QUEST_VISIONS_SLAUGHTER         = 25755, // vision 2
    QUEST_VISIONS_RISE              = 25626, // vision 3
    QUEST_FATHOM_LORDS_CALL         = 25637,
    QUEST_AT_ALL_COSTS              = 25860,
    QUEST_FINAL_JUDGEMENT           = 25951,
    QUEST_HONOR_AND_PRIVILEGE       = 25898,
    QUEST_FULL_CIRCLE               = 26219
};

// creature_text group numbers this module drives (rows owned by the SQL agents)
enum ShimmeringTexts
{
    // 40398 Spirit of Farseer Gadra
    SAY_GADRA_BREACH                = 0, // "Dis breach leads to tha plane of water..."
    SAY_GADRA_WORKING_TOGETHER      = 1, // "Tha naga and de Twilight Cult be workin' together..."
    SAY_GADRA_DISTURBING            = 2, // "Disturbin' prospects..."
    SAY_GADRA_READY_TO_LEAVE        = 3, // "Let me know when ya be ready ta leave."

    // 40161 Fathom-Lord Zin'jatar (cave)
    SAY_ZINJATAR_AGGRO              = 0, // "Before the day is done, you will all be slaves and corpses!"
    EMOTE_ZINJATAR_ENTER            = 1, // "Fathom-Lord Zin'jatar has entered the battle!"
    SAY_ZINJATAR_CONCEDE            = 2, // "Enough! Have your cave, little shaman."

    // 41532 Earthmender Duarn (escort)
    SAY_DUARN_WHATS_THIS            = 0, // "What's this over here?"

    // 41803 Erunak Stonespeaker (escort)
    SAY_ERUNAK_WELL_SUITED          = 0, // "This job is well-suited for you, Duarn..."
    SAY_ERUNAK_GOOD_LUCK            = 1, // "Good luck to you too, $n."

    // 39584 Naz'jar Battlemaiden (vision 1)
    SAY_BATTLEMAIDEN_SINGLE_PRONG   = 0, // "A single prong. Hardly enough..."
    WHISPER_BATTLEMAIDEN_ABILITY    = 1, // "Further attuning yourself with the Battlemaiden..."

    // 40978 Fathom-Stalker Azjentus (vision 1)
    SAY_AZJENTUS_TRIDENT            = 0, // "I could hear the sound of your trident breaking..."
    SAY_AZJENTUS_DOUBT              = 1, // "Indeed. I doubt much would."

    // 42077 Lady Naz'jar (bridge)
    SAY_NAZJAR_MOVE_FORWARD         = 0, // "Move forward! Hold the bridge!"
    SAY_NAZJAR_CUT_THEM_DOWN        = 1, // "Cut them down, Battlemaiden!"
    SAY_NAZJAR_ALLIES_ARRIVED       = 2, // "Our allies have arrived! Push them back..."

    // 42073 Fathom-Lord Zin'jatar (bridge)
    SAY_ZINJATAR_BRIDGE_MARCH       = 0, // "Your march to your deaths!..."
    SAY_ZINJATAR_BRIDGE_SLAUGHTER   = 1, // "Slaughter them all! Show no mercy, brothers!"

    // 42075 Fathom-Stalker Azjentus (bridge)
    SAY_AZJENTUS_BRIDGE_SANDS       = 0, // "Back to the sands with you, wretch!"

    // 42060 Kvaldir Skinflayer
    SAY_SKINFLAYER_CURSE            = 0, // "A curse upon your kind, trespasser!"

    // 42063 Hagrim Hopebreaker
    SAY_HAGRIM_WAVES                = 0, // "You should have left this city to the waves."
    SAY_HAGRIM_DISEASE              = 1, // "Your race is a disease upon the sea."

    // 40789 Generic Controller Bunny (CSA)
    WHISPER_CRUCIBLE_ABANDONED      = 0, // "The crucible looks abandoned and powerless..."
    WHISPER_BRIDGE_DEFENDED         = 1, // "You've succeeded in defending the bridge..."

    // 42488 Chief Engineer Yoon
    SAY_YOON_ARRIVING_SHORTLY       = 0, // "The Pincer X2 will be arriving shortly!..."
    SAY_YOON_ALL_ABOARD             = 1, // "The Pincer X2 has docked. All aboard!"

    // 48429 First Lieutenant Wiley
    SAY_WILEY_APPROACHING           = 0, // "We are approaching the cavern, Captain."
    SAY_WILEY_REPORTS_ACCURATE      = 1, // "The reports were accurate! The beast is here..."
    SAY_WILEY_BEAST_ESCAPED         = 2, // "The beast escaped! By Mekkatorque's moustache..."

    // 48423 Captain Glovaal (submarine copy)
    SAY_GLOVAAL_TAKE_HER_IN         = 0, // "Take her in slowly, number two."
    SAY_GLOVAAL_STEADY              = 1, // "Steady, number two..."
    SAY_GLOVAAL_FIRE                = 2, // "Fire! Fire! Blow it out of the water!"
    SAY_GLOVAAL_DONT_WORRY          = 3, // "Don't worry, number two..."

    // 40645 Jorlan Trueblade (surface copy)
    SAY_JORLAN_FRESH_AIR            = 0, // "Fresh air! ... fire the flare in their direction."
    SAY_JORLAN_NO_WAY_MISS          = 1  // "Ha ha! There's no way they'll miss that..."
};

enum ShimmeringPhases
{
    PHASE_DEFAULT                   = 169,
    PHASE_CAVE_VISION               = 194,
    PHASE_CAVE_DEFENSE              = 171,
    PHASE_CAVE_SHOWDOWN             = 172,
    PHASE_VISION_1                  = 170,
    PHASE_VISION_2                  = 171,
    PHASE_TEMPLE_AMBIENT            = 172,
    PHASE_TEMPLE_HOLD               = 183,
    PHASE_TEMPLE_PUSH               = 184,
    PHASE_TEMPLE_DONE               = 185,
    PHASE_SUB_STAGING               = 228
};

enum ShimmeringPoints
{
    POINT_ZINJATAR_TUNNEL           = 1,
    POINT_ESCORT_CAMP               = 2,
    POINT_ESCAPE_END                = 3,
    POINT_COLUMN_ADVANCE            = 4,
    POINT_WAVE_TEMPLE               = 5
};

// -------------------------------------------------------------------------
// shared helpers

Creature* FindPlayerSummon(Player* player, uint32 entry, float range = 150.0f)
{
    std::list<Creature*> list;
    player->GetCreatureListWithEntryInGrid(list, entry, range);
    for (Creature* creature : list)
        if (TempSummon* summon = creature->ToTempSummon())
            if (summon->GetSummonerGUID() == player->GetGUID())
                return creature;
    return nullptr;
}

struct BattlemaidenVision
{
    uint32 QuestId;
    uint32 AuraId;
    uint32 VehicleEntry;
    uint32 BunnyEntry;
    uint32 ForcecastSpell;
    uint32 ExitCreditSpell;
};

BattlemaidenVision const BattlemaidenVisions[] =
{
    { QUEST_VISIONS_INVASION,  SPELL_NAZJAR_BATTLEMAIDEN_V1, NPC_BATTLEMAIDEN_VISION_1, NPC_TRANSFORM_BUNNY_VISION_1, SPELL_FORCECAST_BATTLEMAIDEN_V1, SPELL_VISION_1_CREDIT },
    { QUEST_VISIONS_SLAUGHTER, SPELL_NAZJAR_BATTLEMAIDEN_V2, NPC_BATTLEMAIDEN_VISION_2, NPC_TRANSFORM_BUNNY_VISION_2, SPELL_FORCECAST_BATTLEMAIDEN_V2, SPELL_VISION_2_CREDIT },
    { QUEST_VISIONS_RISE,      SPELL_NAZJAR_BATTLEMAIDEN_V3, NPC_BATTLEMAIDEN_VISION_3, NPC_TRANSFORM_BUNNY_VISION_3, SPELL_FORCECAST_BATTLEMAIDEN_V3, SPELL_VISION_3_CREDIT }
};

// Retail exit teleport for vision 1 (KS sniff SMSG_MOVE_TELEPORT)
Position const VisionOneExitPos = { -7188.67f, 4719.97f, -595.90f, 5.376f };

BattlemaidenVision const* GetVisionByAura(uint32 auraId)
{
    for (BattlemaidenVision const& vision : BattlemaidenVisions)
        if (vision.AuraId == auraId)
            return &vision;
    return nullptr;
}

// Tears a player out of a Battlemaiden vision: eject + despawn the possessed
// vehicle, strip the vision aura (removes its 261-phase natively) and the
// temple phase ladder, teleport back for vision 1.
void EndBattlemaidenVision(Player* player, uint32 visionAura)
{
    BattlemaidenVision const* vision = GetVisionByAura(visionAura);
    if (!vision)
        return;

    Creature* vehicle = nullptr;
    if (Unit* base = player->GetVehicleBase())
        if (base->GetEntry() == vision->VehicleEntry)
            vehicle = base->ToCreature();
    if (!vehicle)
        vehicle = FindPlayerSummon(player, vision->VehicleEntry);

    player->ExitVehicle();
    if (vehicle)
        vehicle->DespawnOrUnsummon(1s);

    player->RemoveAurasDueToSpell(visionAura);
    player->RemoveAurasDueToSpell(SPELL_PHASE_TEMPLE_172);
    player->RemoveAurasDueToSpell(SPELL_PHASE_TEMPLE_183);
    player->RemoveAurasDueToSpell(SPELL_PHASE_TEMPLE_184);
    player->RemoveAurasDueToSpell(SPELL_PHASE_TEMPLE_185);

    if (visionAura == SPELL_NAZJAR_BATTLEMAIDEN_V1)
        player->NearTeleportTo(VisionOneExitPos);
}

/*######
## Quest 25334 - The Looming Threat (Spirit Vision)
######*/

// 74386 - Spirit Trance: E0 stun with BP 74385; retail casts the BP spell when
// the 5 s trance runs out (teleport into the phase-194 vision tableau).
class spell_vashjir_spirit_trance_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SPIRIT_VISION_TELEPORT });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::Expired))
            return;

        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_SPIRIT_VISION_TELEPORT, true);
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_vashjir_spirit_trance_aura::HandleRemove, EFFECT_0, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

// 81811 - Spirit Vision (timer): E0 dummy aura with BP 75482; on expiry the
// player summons the Spirit of Farseer Gadra at the overlook (dest-db).
class spell_vashjir_spirit_vision_timer : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_SPIRIT_OF_GADRA });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (!GetTargetApplication()->GetRemoveMode().HasFlag(AuraRemoveFlags::Expired))
            return;

        Unit* target = GetTarget();
        target->CastSpell(target, SPELL_SUMMON_SPIRIT_OF_GADRA, true);
    }

    void Register() override
    {
        AfterEffectRemove.Register(&spell_vashjir_spirit_vision_timer::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

/*######
## npc_vashjir_spirit_of_gadra - 40398, per-player vision narrator
## Timed RP (sniff offsets from the 74385 teleport, minus the 4 s timer aura),
## then 75479 (native KC 40307 at summoner). The 25334 turn-in glue lives in
## the zone PlayerScript (works for both the 40398 and 39226 enders).
######*/

enum GadraEvents
{
    EVENT_GADRA_TALK_BREACH = 1,
    EVENT_GADRA_TALK_TOGETHER,
    EVENT_GADRA_TALK_DISTURBING,
    EVENT_GADRA_CREDIT,
    EVENT_GADRA_VALIDATE
};

struct npc_vashjir_spirit_of_gadra : public PassiveAI
{
    npc_vashjir_spirit_of_gadra(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        _events.ScheduleEvent(EVENT_GADRA_TALK_BREACH, 5s);
        _events.ScheduleEvent(EVENT_GADRA_TALK_TOGETHER, 15s + 500ms);
        _events.ScheduleEvent(EVENT_GADRA_TALK_DISTURBING, 23s + 500ms);
        _events.ScheduleEvent(EVENT_GADRA_CREDIT, 31s + 500ms);
        _events.ScheduleEvent(EVENT_GADRA_VALIDATE, 10s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
            {
                me->DespawnOrUnsummon();
                return;
            }

            switch (eventId)
            {
                case EVENT_GADRA_TALK_BREACH:
                    Talk(SAY_GADRA_BREACH, player);
                    break;
                case EVENT_GADRA_TALK_TOGETHER:
                    Talk(SAY_GADRA_WORKING_TOGETHER, player);
                    break;
                case EVENT_GADRA_TALK_DISTURBING:
                    Talk(SAY_GADRA_DISTURBING, player);
                    break;
                case EVENT_GADRA_CREDIT:
                    me->CastSpell(nullptr, SPELL_SPIRIT_VISION_KILL_CREDIT, true); // TargA 92 -> summoner
                    Talk(SAY_GADRA_READY_TO_LEAVE, player);
                    break;
                case EVENT_GADRA_VALIDATE:
                    // Player left the vision phase without turning in - clean up.
                    if (!player->HasAura(SPELL_SPIRIT_VISION_3))
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    _events.ScheduleEvent(EVENT_GADRA_VALIDATE, 10s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## Quest 25164 - Backed Into a Corner
######*/

// 74848 - Phase Shift 2: Intro Cave. E1 (periodic 10 s -> 74845 -> 74843)
// summons wave naga at the player's implicit dest; suppressed - the defense
// controller owns the waves and spawns them at the sniffed cave inlets.
class spell_vashjir_naga_wave_engine : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectPeriodic.Register(&spell_vashjir_naga_wave_engine::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

/*######
## npc_vashjir_cave_defense_controller - 40163 "Intro Cave Defense Controller Bunny"
## Spawned (SQL) in phase 171; the AI joins 172 so it survives the flip.
## Runs the naga wave cycle against the Earthen Ring defenders, then flips
## every defense-phase player to the phase-172 showdown after ~3 minutes
## (retail: 74857 forcecast 3m06s after accept; applied per player here).
######*/

enum DefenseEvents
{
    EVENT_DEFENSE_SCAN = 1,
    EVENT_DEFENSE_WAVE,
    EVENT_DEFENSE_FLIP,
    EVENT_DEFENSE_PRESENCE
};

uint32 const WaveStalkerEntries[] = { NPC_FATHOM_STALKER_WAVE_1, NPC_FATHOM_STALKER_WAVE_2, NPC_FATHOM_STALKER_WAVE_3 };

// Wave inlets above the cave mouth (naga swim down to the floor at z ~ -14)
Position const WaveInletPositions[] =
{
    { -5151.2f, 4001.2f, -27.0f, 4.71f },
    { -5184.1f, 4000.2f, -27.0f, 4.71f },
    { -5208.0f, 4000.5f, -27.0f, 4.71f }
};

uint32 const DefenderEntries[] =
{
    NPC_TOSHE_CHAOSRENDER, NPC_TOSHE_PHASE_172, NPC_EARTHEN_RING_SHAMAN,
    NPC_GREATER_EARTH_ELEMENTAL, NPC_GREATER_FIRE_ELEMENTAL, NPC_DERTH_FIRESINGER
};

struct npc_vashjir_cave_defense_controller : public NullCreatureAI
{
    npc_vashjir_cave_defense_controller(Creature* creature) : NullCreatureAI(creature), _summons(creature), _active(false), _waveIndex(0) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        // SQL spawn carries phase 171 - join 172 so we persist across the flip
        PhasingHandler::AddPhase(me, PHASE_CAVE_SHOWDOWN, true);
        _events.ScheduleEvent(EVENT_DEFENSE_SCAN, 2s);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
        // Waves belong to the defense phase only
        PhasingHandler::RemovePhase(summon, PHASE_CAVE_SHOWDOWN, false);
        PhasingHandler::AddPhase(summon, PHASE_CAVE_DEFENSE, true);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_DEFENSE_SCAN:
                    if (!_active && FindDefensePlayer())
                    {
                        _active = true;
                        _events.ScheduleEvent(EVENT_DEFENSE_WAVE, 2s);
                        _events.ScheduleEvent(EVENT_DEFENSE_FLIP, 3min);
                        _events.ScheduleEvent(EVENT_DEFENSE_PRESENCE, 15s);
                    }
                    else
                        _events.ScheduleEvent(EVENT_DEFENSE_SCAN, 2s);
                    break;
                case EVENT_DEFENSE_WAVE:
                    if (!_active)
                        break;
                    SummonWave();
                    _events.ScheduleEvent(EVENT_DEFENSE_WAVE, 10s);
                    break;
                case EVENT_DEFENSE_PRESENCE:
                    if (!_active)
                        break;
                    if (!FindDefensePlayer())
                    {
                        StopEvent();
                        break;
                    }
                    _events.ScheduleEvent(EVENT_DEFENSE_PRESENCE, 15s);
                    break;
                case EVENT_DEFENSE_FLIP:
                {
                    if (!_active)
                        break;

                    std::list<Player*> players;
                    me->GetPlayerListInGrid(players, 90.0f);
                    for (Player* player : players)
                    {
                        if (!player->IsAlive() || !player->HasAura(SPELL_PHASE_2_INTRO_CAVE))
                            continue;
                        // Retail 74857: KC 40163 + See Quest Invis 3 + phase 172
                        player->KilledMonsterCredit(NPC_FEND_OFF_CREDIT);
                        player->CastSpell(player, SPELL_SEE_QUEST_INVIS_3_CAVE, true);
                        player->CastSpell(player, SPELL_PHASE_3_INTRO_CAVE, true);
                    }
                    StopEvent();
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    Player* FindDefensePlayer() const
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 90.0f);
        for (Player* player : players)
            if (player->IsAlive() && player->HasAura(SPELL_PHASE_2_INTRO_CAVE))
                return player;
        return nullptr;
    }

    Unit* PickWaveTarget() const
    {
        std::vector<Unit*> targets;
        for (uint32 entry : DefenderEntries)
            if (Creature* defender = me->FindNearestCreature(entry, 80.0f))
                if (defender->IsAlive())
                    targets.push_back(defender);

        if (targets.empty())
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 80.0f);
            for (Player* player : players)
                if (player->IsAlive() && player->HasAura(SPELL_PHASE_2_INTRO_CAVE))
                    targets.push_back(player);
        }

        if (targets.empty())
            return nullptr;
        return Trinity::Containers::SelectRandomContainerElement(targets);
    }

    void SummonWave()
    {
        uint8 count = urand(2, 3);
        for (uint8 i = 0; i < count; ++i)
        {
            Position const& inlet = WaveInletPositions[(_waveIndex + i) % std::size(WaveInletPositions)];
            uint32 entry = WaveStalkerEntries[urand(0, uint32(std::size(WaveStalkerEntries)) - 1)];
            if (Creature* stalker = me->SummonCreature(entry, inlet, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 2min))
                if (Unit* target = PickWaveTarget())
                    if (stalker->IsAIEnabled())
                        stalker->AI()->AttackStart(target);
        }
        ++_waveIndex;
    }

    void StopEvent()
    {
        _active = false;
        _summons.DespawnAll();
        _events.CancelEvent(EVENT_DEFENSE_WAVE);
        _events.CancelEvent(EVENT_DEFENSE_FLIP);
        _events.CancelEvent(EVENT_DEFENSE_PRESENCE);
        _events.ScheduleEvent(EVENT_DEFENSE_SCAN, 5s);
    }

    EventMap _events;
    SummonList _summons;
    bool _active;
    uint8 _waveIndex;
};

/*######
## boss_vashjir_fathom_lord_zinjatar - 40161 (phase-172 showdown, DB spawn)
## At 10% he concedes: 99% damage reduction, retreat to the tunnel, then the
## Tsunami Bunnies wash the players out of the event phase (75312 + 75324).
######*/

enum ZinjatarEvents
{
    EVENT_ZINJATAR_FROST_CAST = 1,
    EVENT_ZINJATAR_TSUNAMI,
    EVENT_ZINJATAR_DESPAWN
};

Position const ZinjatarTunnelPos = { -5159.27f, 3990.04f, -14.0f, 1.35f };

struct boss_vashjir_fathom_lord_zinjatar : public ScriptedAI
{
    boss_vashjir_fathom_lord_zinjatar(Creature* creature) : ScriptedAI(creature), _conceded(false) { }

    void Reset() override
    {
        _conceded = false;
        _events.Reset();
        me->RemoveAurasDueToSpell(SPELL_99_DAMAGE_REDUCTION);
        me->SetImmuneToPC(false);
        me->SetReactState(REACT_AGGRESSIVE);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_ZINJATAR_AGGRO);
        Talk(EMOTE_ZINJATAR_ENTER);
        _events.ScheduleEvent(EVENT_ZINJATAR_FROST_CAST, 6s);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // He never dies - concedes the cave at 10%
        if (damage >= me->GetHealth())
            damage = me->GetHealth() - 1;

        if (!_conceded && me->HealthBelowPctDamaged(10, damage))
        {
            _conceded = true;
            _events.Reset();
            me->CastSpell(me, SPELL_99_DAMAGE_REDUCTION, true);
            Talk(SAY_ZINJATAR_CONCEDE);
            me->AttackStop();
            me->SetReactState(REACT_PASSIVE);
            me->SetImmuneToPC(true);
            me->GetMotionMaster()->MovePoint(POINT_ZINJATAR_TUNNEL, ZinjatarTunnelPos);
            _events.ScheduleEvent(EVENT_ZINJATAR_TSUNAMI, 8s);
            _events.ScheduleEvent(EVENT_ZINJATAR_DESPAWN, 12s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_conceded && !UpdateVictim())
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ZINJATAR_FROST_CAST:
                    DoCastVictim(SPELL_FROST_CAST);
                    _events.ScheduleEvent(EVENT_ZINJATAR_FROST_CAST, Milliseconds(urand(9000, 14000)));
                    break;
                case EVENT_ZINJATAR_TSUNAMI:
                {
                    // Back-wall bunnies wash the cave (knockback + KC via conditions)...
                    std::list<Creature*> bunnies;
                    me->GetCreatureListWithEntryInGrid(bunnies, NPC_TSUNAMI_BUNNY, 90.0f);
                    for (Creature* bunny : bunnies)
                        bunny->CastSpell(nullptr, SPELL_TSUNAMI_KNOCKBACK, true);

                    // ...and every event player is credited + phased back manually
                    // (75312's scripted E2 dummy carries BP 75324 on retail).
                    std::list<Player*> players;
                    me->GetPlayerListInGrid(players, 90.0f);
                    for (Player* player : players)
                    {
                        if (!player->HasAura(SPELL_PHASE_3_INTRO_CAVE))
                            continue;
                        player->KilledMonsterCredit(me->GetEntry());
                        player->CastSpell(player, SPELL_REMOVE_PHASE_3, true);
                    }
                    break;
                }
                case EVENT_ZINJATAR_DESPAWN:
                    me->DespawnOrUnsummon(0s, 30s);
                    break;
                default:
                    break;
            }
        }

        if (!_conceded)
            DoMeleeAttackIfReady();
    }

private:
    EventMap _events;
    bool _conceded;
};

/*######
## Quest 25441 - Vortex
## npc_vashjir_toshes_vortex - 40277 (vehicle 735), summoned at the globe's
## impact point (75564 -> 75112, native). Pulses 75109 every second; trapped
## serpents ride a seat, credit the owner (75574) and are consumed.
######*/

struct npc_vashjir_toshes_vortex : public PassiveAI
{
    npc_vashjir_toshes_vortex(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->CastSpell(me, SPELL_TOSHES_VORTEX_AURA, true);
        me->GetMotionMaster()->MoveRandom(8.0f);
        me->DespawnOrUnsummon(25s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply || passenger->GetTypeId() != TYPEID_UNIT)
            return;

        // One trap credit per swallowed serpent (native KC2 40277 at summoner)
        me->CastSpell(nullptr, SPELL_SERPENT_TRAP_CREDIT, true);

        ObjectGuid riderGUID = passenger->GetGUID();
        Creature* vortex = me;
        me->m_Events.AddEventAtOffset([vortex, riderGUID]()
        {
            if (Unit* rider = ObjectAccessor::GetUnit(*vortex, riderGUID))
            {
                rider->ExitVehicle();
                if (Creature* creature = rider->ToCreature())
                    creature->DespawnOrUnsummon();
            }
        }, 2s + 500ms);
    }
};

// 75109 - Toshe's Vortex Trigger: dummy AoE from the whirlpool. Grabs up to
// three nearby unmounted serpents (and ravagers) and pulls them into seats.
class spell_vashjir_toshes_vortex_trigger : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_RIDE_VEHICLE_HARDCODED });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.clear();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Creature*> victims;
        caster->GetCreatureListWithEntryInGrid(victims, NPC_SWARMING_SERPENT, 10.0f);
        std::list<Creature*> ravagers;
        caster->GetCreatureListWithEntryInGrid(ravagers, NPC_ZINJATAR_RAVAGER, 10.0f);
        victims.splice(victims.end(), ravagers);

        victims.remove_if([](Creature* creature)
        {
            return !creature->IsAlive() || creature->GetVehicle();
        });

        if (victims.size() > 3)
            Trinity::Containers::RandomResize(victims, 3);

        for (Creature* victim : victims)
            targets.push_back(victim);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Creature* victim = GetHitCreature();
        if (!caster || !victim)
            return;

        victim->CastSpell(caster, SPELL_RIDE_VEHICLE_HARDCODED, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_vashjir_toshes_vortex_trigger::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        OnEffectHitTarget.Register(&spell_vashjir_toshes_vortex_trigger::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## Quest 25890 - Nespirah
## at_nespirah_tunnel - DBC areatrigger 5958 (plane across the throat tunnel).
## Summons the personal Duarn/Erunak escort pair once and completes the
## travel quest (retail uses a serverside polygon slightly further up).
######*/

class at_nespirah_tunnel : public AreaTriggerScript
{
public:
    at_nespirah_tunnel() : AreaTriggerScript("at_nespirah_tunnel") { }

    bool OnTrigger(Player* player, AreaTriggerEntry const* /*trigger*/) override
    {
        if (!player->IsAlive() || player->GetQuestStatus(QUEST_NESPIRAH) != QUEST_STATUS_INCOMPLETE)
            return false;

        // 77963's E1 dummy self-aura doubles as the retail once-only marker
        if (!player->HasAura(SPELL_SUMMON_ERUNAK_ESCORT) && !FindPlayerSummon(player, NPC_ERUNAK_ESCORT, 120.0f))
        {
            player->CastSpell(player, SPELL_SUMMON_DUARN_ESCORT, true);
            player->CastSpell(player, SPELL_SUMMON_ERUNAK_ESCORT, true);
        }

        player->AreaExploredOrEventHappens(QUEST_NESPIRAH);
        return true;
    }
};

/*######
## npc_vashjir_nespirah_escort - 41532 Duarn / 41803 Erunak escort pair
## They swim ahead of the player down to the entry chamber, chat, and leave.
######*/

enum EscortEvents
{
    EVENT_ESCORT_MOVE = 1,
    EVENT_ESCORT_TALK_MID,
    EVENT_ESCORT_TALK_ARRIVAL,
    EVENT_ESCORT_TALK_GOOD_LUCK,
    EVENT_ESCORT_DESPAWN
};

Position const DuarnEscortPath[] =
{
    { -6390.0f, 4130.5f, -434.0f },
    { -6414.5f, 4152.0f, -430.0f },
    { -6436.0f, 4170.0f, -426.5f },
    { -6446.5f, 4175.5f, -425.0f }
};

Position const ErunakEscortPath[] =
{
    { -6392.5f, 4134.0f, -433.5f },
    { -6417.0f, 4155.5f, -429.5f },
    { -6438.5f, 4173.0f, -426.0f },
    { -6449.0f, 4179.0f, -425.0f }
};

struct npc_vashjir_nespirah_escort : public PassiveAI
{
    npc_vashjir_nespirah_escort(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetDisableGravity(true);
        _events.ScheduleEvent(EVENT_ESCORT_MOVE, 1s);

        if (me->GetEntry() == NPC_DUARN_ESCORT)
            _events.ScheduleEvent(EVENT_ESCORT_TALK_MID, 9s);
        else
        {
            _events.ScheduleEvent(EVENT_ESCORT_TALK_ARRIVAL, 24s);
            _events.ScheduleEvent(EVENT_ESCORT_TALK_GOOD_LUCK, 30s);
        }
        _events.ScheduleEvent(EVENT_ESCORT_DESPAWN, 35s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ESCORT_MOVE:
                    if (me->GetEntry() == NPC_DUARN_ESCORT)
                        me->GetMotionMaster()->MoveSmoothPath(POINT_ESCORT_CAMP, DuarnEscortPath, std::size(DuarnEscortPath), false, true, 4.0f);
                    else
                        me->GetMotionMaster()->MoveSmoothPath(POINT_ESCORT_CAMP, ErunakEscortPath, std::size(ErunakEscortPath), false, true, 4.0f);
                    break;
                case EVENT_ESCORT_TALK_MID:
                    Talk(SAY_DUARN_WHATS_THIS);
                    break;
                case EVENT_ESCORT_TALK_ARRIVAL:
                    Talk(SAY_ERUNAK_WELL_SUITED);
                    break;
                case EVENT_ESCORT_TALK_GOOD_LUCK:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
                        Talk(SAY_ERUNAK_GOOD_LUCK, player);
                    break;
                case EVENT_ESCORT_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## Quest 25922 - Waking the Beast (escape ride)
######*/

// 77927 - Summon Escape Seahorse (Master): spellclick on 41776. E2 (KC2 41776)
// is native; the faction split lives in the two dummy BPs.
class spell_vashjir_summon_escape_seahorse : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_SEAHORSE_ALLIANCE, SPELL_SUMMON_SEAHORSE_HORDE });
    }

    void HandleDummy(SpellEffIndex effIndex)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        // EFFECT_0 carries the Horde summon, EFFECT_1 the Alliance one
        if ((effIndex == EFFECT_0) != (player->GetTeam() == HORDE))
            return;

        player->CastSpell(player, uint32(GetEffectValue()), true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_summon_escape_seahorse::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        OnEffectHitTarget.Register(&spell_vashjir_summon_escape_seahorse::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

/*######
## npc_vashjir_escape_seahorse - 41785 (A) / 41778 (H), vehicle 840
## Single ~30 s spline north through the valve, over the shell lip and down
## to the ledge camp (WPP path 884 endpoints, intermediate nodes synthesized).
######*/

enum SeahorseEvents
{
    EVENT_SEAHORSE_BOARD_CHECK = 1,
    EVENT_SEAHORSE_LAUNCH
};

Position const EscapeSeahorsePath[] =
{
    { -6551.6f, 4260.0f, -475.5f },
    { -6551.0f, 4310.0f, -475.5f },
    { -6552.5f, 4370.0f, -476.0f },
    { -6555.0f, 4430.0f, -478.0f },
    { -6560.0f, 4480.0f, -495.0f },
    { -6572.0f, 4520.0f, -508.0f },
    { -6586.7f, 4548.0f, -517.4f },
    { -6601.0f, 4520.0f, -528.0f },
    { -6611.0f, 4470.0f, -542.0f },
    { -6614.5f, 4420.0f, -553.0f },
    { -6612.0f, 4370.0f, -560.0f },
    { -6607.3f, 4310.5f, -564.5f }
};

struct npc_vashjir_escape_seahorse : public PassiveAI
{
    npc_vashjir_escape_seahorse(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();
        me->SetDisableGravity(true);
        _events.ScheduleEvent(EVENT_SEAHORSE_BOARD_CHECK, 1s);
        _events.ScheduleEvent(EVENT_SEAHORSE_LAUNCH, 3s);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || pointId != POINT_ESCAPE_END)
            return;

        // 50630 (Eject All Passengers) has no core handler - eject manually
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();
        me->DespawnOrUnsummon(2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
            {
                me->DespawnOrUnsummon();
                return;
            }

            switch (eventId)
            {
                case EVENT_SEAHORSE_BOARD_CHECK:
                    // SummonProperties 161 boards the summoner natively; force it if not
                    if (player->GetVehicleBase() != me)
                        player->CastSpell(me, SPELL_RIDE_VEHICLE_HARDCODED, true);
                    break;
                case EVENT_SEAHORSE_LAUNCH:
                    if (player->GetVehicleBase() != me)
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    me->GetMotionMaster()->MoveSmoothPath(POINT_ESCAPE_END, EscapeSeahorsePath, std::size(EscapeSeahorsePath), false, true, 15.0f);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
};

/*######
## Battlemaiden visions (25760 / 25755 / 25626)
######*/

// 77292 - Blade of the Naz'jar Battlemaiden: E77 at the nearby transform bunny
// (conditions bind entries 41160/41436/41484). The bunny answers with the
// vision's forcecast, which makes the player cast the possession spell.
class spell_vashjir_blade_of_the_battlemaiden : public SpellScript
{
    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Creature* bunny = GetHitCreature();
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!bunny || !player)
            return;

        for (BattlemaidenVision const& vision : BattlemaidenVisions)
        {
            if (vision.BunnyEntry != bunny->GetEntry())
                continue;
            if (player->GetQuestStatus(vision.QuestId) != QUEST_STATUS_INCOMPLETE || player->HasAura(vision.AuraId))
                return;
            bunny->CastSpell(player, vision.ForcecastSpell, true);
            return;
        }
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_blade_of_the_battlemaiden::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 78332 - Battlemaiden Final Phase Master: E77 -> apply the temple ambient phase
class spell_vashjir_battlemaiden_final_phase : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_PHASE_TEMPLE_172 });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(target, SPELL_PHASE_TEMPLE_172, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_battlemaiden_final_phase::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 80674 - Battlemaiden Backup Quest Credit: retail safety net cast on accept of
// the later vision quests; re-grants the vision-1 credit if it was missed.
// (The 77283 teardown script no-ops when the player is not in a vision.)
class spell_vashjir_battlemaiden_backup_credit : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_VISION_1_CREDIT });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        target->CastSpell(target, SPELL_VISION_1_CREDIT, true);
    }

    void Register() override
    {
        OnEffectHitTarget.Register(&spell_vashjir_battlemaiden_backup_credit::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 77283 / 77284 / 77285 - Battlemaiden Vision Kill Credits. The E90 credit is
// native; the script owns the delayed vision teardown (dismount, despawn,
// phase strip, vision-1 teleport back).
class spell_vashjir_battlemaiden_vision_exit : public SpellScript
{
    void HandleAfterHit()
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        uint32 visionAura = 0;
        switch (GetSpellInfo()->Id)
        {
            case SPELL_VISION_1_CREDIT: visionAura = SPELL_NAZJAR_BATTLEMAIDEN_V1; break;
            case SPELL_VISION_2_CREDIT: visionAura = SPELL_NAZJAR_BATTLEMAIDEN_V2; break;
            case SPELL_VISION_3_CREDIT: visionAura = SPELL_NAZJAR_BATTLEMAIDEN_V3; break;
            default: return;
        }

        // Backup-credit casts hit players outside the vision - nothing to tear down
        if (!player->HasAura(visionAura))
            return;

        player->m_Events.AddEventAtOffset([player, visionAura]()
        {
            if (player->HasAura(visionAura))
                EndBattlemaidenVision(player, visionAura);
        }, 1s + 200ms);
    }

    void Register() override
    {
        AfterHit.Register(&spell_vashjir_battlemaiden_vision_exit::HandleAfterHit);
    }
};

/*######
## npc_vashjir_battlemaiden - 39584 / 41225 / 41986 possession vehicles
## Possession (SummonProperties 827 + ride 76546) is core-native; the AI owns
## phase upkeep, ride safety, the vision-1 intro RP + reinforcement aura and
## the vision-2 west-edge exit trigger.
######*/

enum BattlemaidenEvents
{
    EVENT_BM_RIDE_CHECK = 1,
    EVENT_BM_VALIDATE,
    EVENT_BM_INTRO_AZJENTUS_1,
    EVENT_BM_INTRO_PRONG,
    EVENT_BM_INTRO_AZJENTUS_2,
    EVENT_BM_EXIT_CHECK
};

Position const VisionTwoExitPos = { -7280.67f, 4420.46f, -276.60f, 0.0f };

struct npc_vashjir_battlemaiden : public PassiveAI
{
    npc_vashjir_battlemaiden(Creature* creature) : PassiveAI(creature), _rideAttempts(0) { }

    uint32 GetVisionAura() const
    {
        switch (me->GetEntry())
        {
            case NPC_BATTLEMAIDEN_VISION_1: return SPELL_NAZJAR_BATTLEMAIDEN_V1;
            case NPC_BATTLEMAIDEN_VISION_2: return SPELL_NAZJAR_BATTLEMAIDEN_V2;
            case NPC_BATTLEMAIDEN_VISION_3: return SPELL_NAZJAR_BATTLEMAIDEN_V3;
            default: return 0;
        }
    }

    void IsSummonedBy(Unit* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = player->GetGUID();

        // The summon resolves before the phase aura lands on the player -
        // join the vision phase(s) explicitly.
        switch (me->GetEntry())
        {
            case NPC_BATTLEMAIDEN_VISION_1:
                PhasingHandler::AddPhase(me, PHASE_VISION_1, true);
                me->CastSpell(me, SPELL_BLESSING_OF_AZSHARA, true);
                me->CastSpell(me, SPELL_SUMMON_REINFORCEMENT_AURA, true);
                _events.ScheduleEvent(EVENT_BM_INTRO_AZJENTUS_1, 4s);
                _events.ScheduleEvent(EVENT_BM_INTRO_PRONG, 13s);
                _events.ScheduleEvent(EVENT_BM_INTRO_AZJENTUS_2, 20s);
                break;
            case NPC_BATTLEMAIDEN_VISION_2:
                PhasingHandler::AddPhase(me, PHASE_VISION_2, true);
                _events.ScheduleEvent(EVENT_BM_EXIT_CHECK, 2s);
                break;
            case NPC_BATTLEMAIDEN_VISION_3:
                PhasingHandler::AddPhase(me, PHASE_TEMPLE_AMBIENT, false);
                PhasingHandler::AddPhase(me, PHASE_TEMPLE_HOLD, false);
                PhasingHandler::AddPhase(me, PHASE_TEMPLE_PUSH, false);
                PhasingHandler::AddPhase(me, PHASE_TEMPLE_DONE, true);
                break;
            default:
                break;
        }

        // Retail 77010 strips the present-day ruins phases while possessed
        player->RemoveAurasDueToSpell(SPELL_PHASE_RUINS_179);
        player->RemoveAurasDueToSpell(SPELL_PHASE_RUINS_180);

        _events.ScheduleEvent(EVENT_BM_RIDE_CHECK, 1s);
        _events.ScheduleEvent(EVENT_BM_VALIDATE, 5s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // Player left the seat: if the vision aura survived (manual dismount),
        // finish the vision gracefully a moment later.
        uint32 visionAura = GetVisionAura();
        Creature* vehicle = me;
        ObjectGuid playerGUID = _playerGUID;
        me->m_Events.AddEventAtOffset([vehicle, playerGUID, visionAura]()
        {
            Player* player = ObjectAccessor::GetPlayer(*vehicle, playerGUID);
            if (player && player->HasAura(visionAura) && player->GetVehicleBase() != vehicle)
                EndBattlemaidenVision(player, visionAura);
        }, 2s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            if (player->HasAura(GetVisionAura()))
                EndBattlemaidenVision(player, GetVisionAura());
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
            if (!player)
            {
                me->DespawnOrUnsummon();
                return;
            }

            switch (eventId)
            {
                case EVENT_BM_RIDE_CHECK:
                    if (player->GetVehicleBase() != me)
                    {
                        if (!player->HasAura(GetVisionAura()) || ++_rideAttempts > 3)
                        {
                            me->DespawnOrUnsummon();
                            return;
                        }
                        player->CastSpell(me, SPELL_RIDE_BATTLEMAIDEN, true);
                        _events.ScheduleEvent(EVENT_BM_RIDE_CHECK, 2s);
                    }
                    break;
                case EVENT_BM_VALIDATE:
                    if (!player->HasAura(GetVisionAura()))
                    {
                        me->DespawnOrUnsummon();
                        return;
                    }
                    _events.ScheduleEvent(EVENT_BM_VALIDATE, 5s);
                    break;
                case EVENT_BM_INTRO_AZJENTUS_1:
                    if (Creature* azjentus = me->FindNearestCreature(NPC_AZJENTUS_VISION_1, 80.0f))
                        if (azjentus->IsAIEnabled())
                            azjentus->AI()->Talk(SAY_AZJENTUS_TRIDENT, player);
                    break;
                case EVENT_BM_INTRO_PRONG:
                    Talk(SAY_BATTLEMAIDEN_SINGLE_PRONG, player);
                    break;
                case EVENT_BM_INTRO_AZJENTUS_2:
                    if (Creature* azjentus = me->FindNearestCreature(NPC_AZJENTUS_VISION_1, 80.0f))
                        if (azjentus->IsAIEnabled())
                            azjentus->AI()->Talk(SAY_AZJENTUS_DOUBT, player);
                    break;
                case EVENT_BM_EXIT_CHECK:
                    // Vision 2 ends in place at the Biel'aran west edge
                    if (me->GetExactDist2d(VisionTwoExitPos.GetPositionX(), VisionTwoExitPos.GetPositionY()) < 15.0f)
                    {
                        if (player->HasAura(SPELL_NAZJAR_BATTLEMAIDEN_V2))
                            player->CastSpell(player, SPELL_VISION_2_CREDIT, true);
                        break;
                    }
                    _events.ScheduleEvent(EVENT_BM_EXIT_CHECK, 2s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGUID;
    uint8 _rideAttempts;
};

// 76569 - Summon Naga Reinforcement Ping: dummy AoE from the Battlemaiden.
// One nearby kvaldir is forced to summon a naga war-party attacker (8297x).
uint32 const ReinforcementKvaldirEntries[] =
{
    NPC_KVALDIR_LIMBRIPPER, NPC_KVALDIR_SANDTERROR, NPC_KVALDIR_PILLAGER,
    NPC_KVALDIR_WASTEROAMER, NPC_KVALDIR_DEEPWALKER, NPC_KVALDIR_PLUNDERER
};

class spell_vashjir_naga_reinforcement_ping : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_FATHOM_STALKER, SPELL_SUMMON_IDRAKESS_SLAVER, SPELL_SUMMON_TIDE_PRIESTESS });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.clear();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        std::list<Creature*> kvaldir;
        for (uint32 entry : ReinforcementKvaldirEntries)
        {
            std::list<Creature*> found;
            caster->GetCreatureListWithEntryInGrid(found, entry, 30.0f);
            kvaldir.splice(kvaldir.end(), found);
        }

        kvaldir.remove_if([](Creature* creature) { return !creature->IsAlive(); });
        if (kvaldir.empty())
            return;

        targets.push_back(Trinity::Containers::SelectRandomContainerElement(kvaldir));
    }

    void HandleHit(SpellEffIndex /*effIndex*/)
    {
        Creature* target = GetHitCreature();
        if (!target)
            return;

        uint32 const summonSpells[] = { SPELL_SUMMON_FATHOM_STALKER, SPELL_SUMMON_IDRAKESS_SLAVER, SPELL_SUMMON_TIDE_PRIESTESS };
        target->CastSpell(target, Trinity::Containers::SelectRandomContainerElement(summonSpells), true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect.Register(&spell_vashjir_naga_reinforcement_ping::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
        OnEffectHitTarget.Register(&spell_vashjir_naga_reinforcement_ping::HandleHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 44421 / 44422 / 44423 - naga war party: summoned onto a kvaldir, attacks it
struct npc_vashjir_war_party : public ScriptedAI
{
    npc_vashjir_war_party(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        me->DespawnOrUnsummon(1min);
        if (summoner->IsAlive())
            AttackStart(summoner);
    }
};

/*######
## npc_vashjir_temple_credit_bunny - 41982 (25626 objective 1)
## Proximity credit at the Quel'Dormir crucible + the scout's whisper.
## (SQL: spawn one 41982 at ~ -7270, 5075, -270.)
######*/

struct npc_vashjir_temple_credit_bunny : public NullCreatureAI
{
    npc_vashjir_temple_credit_bunny(Creature* creature) : NullCreatureAI(creature), _checkTimer(2000) { }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 2000;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 40.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || player->GetQuestStatus(QUEST_VISIONS_RISE) != QUEST_STATUS_INCOMPLETE)
                continue;
            if (player->GetReqKillOrCastCurrentCount(QUEST_VISIONS_RISE, int32(me->GetEntry())) > 0)
                continue;

            player->KilledMonsterCredit(me->GetEntry());
            if (Creature* scout = me->FindNearestCreature(NPC_GENERIC_CONTROLLER_CSA, 80.0f))
                if (scout->IsAIEnabled())
                    scout->AI()->Talk(WHISPER_CRUCIBLE_ABANDONED, player);
        }
    }

private:
    uint32 _checkTimer;
};

/*######
## Quest 25951 - Final Judgement
## npc_vashjir_bridge_controller - 42135 (spawned by SQL, phase 183; the AI
## joins 184). Stage 1 (phase 183): kvaldir stream over the bridge for ~93 s,
## then 78323 flips players to phase 184 with the first credit. Stage 2: the
## friendly column pushes south, Hagrim Hopebreaker spawns; his death grants
## the second credit + phase 185.
######*/

enum BridgeEvents
{
    EVENT_BRIDGE_SCAN = 1,
    EVENT_BRIDGE_WAVE,
    EVENT_BRIDGE_YELL_ZINJATAR,
    EVENT_BRIDGE_YELL_SKINFLAYER,
    EVENT_BRIDGE_FLIP,
    EVENT_BRIDGE_PRESENCE,
    EVENT_BRIDGE_PUSH_YELL_FORWARD,
    EVENT_BRIDGE_PUSH_YELL_CUT,
    EVENT_BRIDGE_PUSH_YELL_SANDS,
    EVENT_BRIDGE_PUSH_YELL_SLAUGHTER,
    EVENT_BRIDGE_HAGRIM,
    EVENT_BRIDGE_HAGRIM_YELL,
    EVENT_BRIDGE_PUSH_YELL_ALLIES,
    EVENT_BRIDGE_RESET
};

enum BridgeStages
{
    BRIDGE_STAGE_IDLE = 0,
    BRIDGE_STAGE_HOLD,
    BRIDGE_STAGE_PUSH,
    BRIDGE_STAGE_DONE
};

Position const BridgeWaveSpawns[] =
{
    { -7301.0f, 4761.0f, -284.9f, 1.57f },
    { -7292.0f, 4737.0f, -284.9f, 1.57f },
    { -7308.0f, 4712.0f, -284.9f, 1.57f }
};

Position const BridgeNorthEnd   = { -7286.0f, 5001.0f, -267.0f, 1.57f };
Position const ColumnLadyPos    = { -7300.4f, 4870.9f, -284.9f, 4.71f };
Position const ColumnZinjatarPos = { -7271.8f, 4859.8f, -284.9f, 4.71f };
Position const ColumnAzjentusPos = { -7313.3f, 4874.5f, -284.9f, 4.71f };
Position const HagrimSpawnPos   = { -7290.4f, 4513.7f, -261.0f, 1.57f };

Position const ColumnAdvancePath[] =
{
    { -7299.0f, 4820.0f, -285.0f },
    { -7297.0f, 4760.0f, -285.0f },
    { -7297.0f, 4700.0f, -285.0f },
    { -7298.0f, 4640.0f, -284.8f }
};

struct npc_vashjir_bridge_controller : public NullCreatureAI
{
    npc_vashjir_bridge_controller(Creature* creature) : NullCreatureAI(creature), _summons(creature), _stage(BRIDGE_STAGE_IDLE) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        // SQL spawn carries phase 183; join 184 for the counter-push stage
        PhasingHandler::AddPhase(me, PHASE_TEMPLE_PUSH, true);
        _events.ScheduleEvent(EVENT_BRIDGE_SCAN, 2s);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
        if (_stage == BRIDGE_STAGE_HOLD)
        {
            PhasingHandler::RemovePhase(summon, PHASE_TEMPLE_PUSH, false);
            PhasingHandler::AddPhase(summon, PHASE_TEMPLE_HOLD, true);
        }
        else
        {
            PhasingHandler::RemovePhase(summon, PHASE_TEMPLE_HOLD, false);
            PhasingHandler::AddPhase(summon, PHASE_TEMPLE_PUSH, true);
        }
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
    {
        if (summon->GetEntry() != NPC_HAGRIM_HOPEBREAKER || _stage != BRIDGE_STAGE_PUSH)
            return;

        _stage = BRIDGE_STAGE_DONE;
        _events.CancelEvent(EVENT_BRIDGE_WAVE);

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 250.0f);
        for (Player* player : players)
        {
            if (!player->HasAura(SPELL_PHASE_TEMPLE_184))
                continue;
            // Retail 78330: KC 42063 + forcecast phase 185
            player->KilledMonsterCredit(NPC_HAGRIM_HOPEBREAKER);
            player->CastSpell(player, SPELL_PHASE_TEMPLE_185, true);
            if (Creature* scout = me->FindNearestCreature(NPC_GENERIC_CONTROLLER_CSA, 200.0f))
                if (scout->IsAIEnabled())
                    scout->AI()->Talk(WHISPER_BRIDGE_DEFENDED, player);
        }

        _events.ScheduleEvent(EVENT_BRIDGE_RESET, 30s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BRIDGE_SCAN:
                    if (_stage == BRIDGE_STAGE_IDLE && FindEventPlayer(SPELL_PHASE_TEMPLE_183, true))
                        StartEvent();
                    else
                        _events.ScheduleEvent(EVENT_BRIDGE_SCAN, 2s);
                    break;
                case EVENT_BRIDGE_WAVE:
                    if (_stage != BRIDGE_STAGE_HOLD && _stage != BRIDGE_STAGE_PUSH)
                        break;
                    SummonWave();
                    _events.ScheduleEvent(EVENT_BRIDGE_WAVE, _stage == BRIDGE_STAGE_HOLD ? 15s : 20s);
                    break;
                case EVENT_BRIDGE_YELL_ZINJATAR:
                    ActorTalk(NPC_FATHOM_LORD_TEMPLE, SAY_ZINJATAR_BRIDGE_MARCH);
                    break;
                case EVENT_BRIDGE_YELL_SKINFLAYER:
                    if (Creature* skinflayer = me->SummonCreature(NPC_KVALDIR_SKINFLAYER, BridgeWaveSpawns[0], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 2min))
                    {
                        if (skinflayer->IsAIEnabled())
                            skinflayer->AI()->Talk(SAY_SKINFLAYER_CURSE);
                        WaveAttack(skinflayer);
                    }
                    break;
                case EVENT_BRIDGE_FLIP:
                {
                    if (_stage != BRIDGE_STAGE_HOLD)
                        break;
                    _stage = BRIDGE_STAGE_PUSH;

                    std::list<Player*> players;
                    me->GetPlayerListInGrid(players, 200.0f);
                    for (Player* player : players)
                    {
                        if (!player->HasAura(SPELL_PHASE_TEMPLE_183))
                            continue;
                        // Retail 78329: KC 42135 + forcecast phase 184
                        player->KilledMonsterCredit(NPC_BRIDGE_CONTROLLER);
                        player->CastSpell(player, SPELL_PHASE_TEMPLE_184, true);
                    }

                    // Old hold-stage waves are stranded in phase 183 - clear them
                    _summons.DespawnAll();
                    SummonColumn();

                    _events.ScheduleEvent(EVENT_BRIDGE_PUSH_YELL_FORWARD, 6s);
                    _events.ScheduleEvent(EVENT_BRIDGE_PUSH_YELL_CUT, 33s);
                    _events.ScheduleEvent(EVENT_BRIDGE_PUSH_YELL_SANDS, 54s);
                    _events.ScheduleEvent(EVENT_BRIDGE_PUSH_YELL_SLAUGHTER, 79s);
                    _events.ScheduleEvent(EVENT_BRIDGE_HAGRIM, 82s);
                    _events.ScheduleEvent(EVENT_BRIDGE_PUSH_YELL_ALLIES, 125s);
                    break;
                }
                case EVENT_BRIDGE_PRESENCE:
                    if (_stage == BRIDGE_STAGE_IDLE)
                        break;
                    if (!FindEventPlayer(SPELL_PHASE_TEMPLE_183, false) && !FindEventPlayer(SPELL_PHASE_TEMPLE_184, false))
                    {
                        ResetEvent();
                        break;
                    }
                    _events.ScheduleEvent(EVENT_BRIDGE_PRESENCE, 15s);
                    break;
                case EVENT_BRIDGE_PUSH_YELL_FORWARD:
                    SummonTalk(NPC_LADY_NAZJAR_TEMPLE, SAY_NAZJAR_MOVE_FORWARD);
                    break;
                case EVENT_BRIDGE_PUSH_YELL_CUT:
                    SummonTalk(NPC_LADY_NAZJAR_TEMPLE, SAY_NAZJAR_CUT_THEM_DOWN);
                    break;
                case EVENT_BRIDGE_PUSH_YELL_SANDS:
                    SummonTalk(NPC_AZJENTUS_TEMPLE, SAY_AZJENTUS_BRIDGE_SANDS);
                    break;
                case EVENT_BRIDGE_PUSH_YELL_SLAUGHTER:
                    SummonTalk(NPC_FATHOM_LORD_TEMPLE, SAY_ZINJATAR_BRIDGE_SLAUGHTER);
                    break;
                case EVENT_BRIDGE_HAGRIM:
                    if (Creature* hagrim = me->SummonCreature(NPC_HAGRIM_HOPEBREAKER, HagrimSpawnPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30s))
                    {
                        if (hagrim->IsAIEnabled())
                            hagrim->AI()->Talk(SAY_HAGRIM_WAVES);
                        _events.ScheduleEvent(EVENT_BRIDGE_HAGRIM_YELL, 29s);
                    }
                    break;
                case EVENT_BRIDGE_HAGRIM_YELL:
                    SummonTalk(NPC_HAGRIM_HOPEBREAKER, SAY_HAGRIM_DISEASE);
                    break;
                case EVENT_BRIDGE_PUSH_YELL_ALLIES:
                    SummonTalk(NPC_LADY_NAZJAR_TEMPLE, SAY_NAZJAR_ALLIES_ARRIVED);
                    break;
                case EVENT_BRIDGE_RESET:
                    ResetEvent();
                    break;
                default:
                    break;
            }
        }
    }

private:
    Player* FindEventPlayer(uint32 auraId, bool requireQuest) const
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 150.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || !player->HasAura(auraId))
                continue;
            if (requireQuest && player->GetQuestStatus(QUEST_FINAL_JUDGEMENT) != QUEST_STATUS_INCOMPLETE)
                continue;
            return player;
        }
        return nullptr;
    }

    void ActorTalk(uint32 entry, uint8 group) const
    {
        if (Creature* actor = me->FindNearestCreature(entry, 200.0f))
            if (actor->IsAIEnabled())
                actor->AI()->Talk(group);
    }

    void SummonTalk(uint32 entry, uint8 group)
    {
        for (ObjectGuid guid : _summons)
            if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                if (summon->GetEntry() == entry && summon->IsAlive())
                {
                    if (summon->IsAIEnabled())
                        summon->AI()->Talk(group);
                    return;
                }
        // fall back to a world spawn of the same entry
        ActorTalk(entry, group);
    }

    void StartEvent()
    {
        _stage = BRIDGE_STAGE_HOLD;
        _events.ScheduleEvent(EVENT_BRIDGE_WAVE, 5s);
        _events.ScheduleEvent(EVENT_BRIDGE_YELL_ZINJATAR, 28s);
        _events.ScheduleEvent(EVENT_BRIDGE_YELL_SKINFLAYER, 62s);
        _events.ScheduleEvent(EVENT_BRIDGE_FLIP, 93s);
        _events.ScheduleEvent(EVENT_BRIDGE_PRESENCE, 15s);
    }

    void WaveAttack(Creature* kvaldir)
    {
        Unit* target = nullptr;
        if (_stage == BRIDGE_STAGE_PUSH)
            for (ObjectGuid guid : _summons)
                if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                    if (summon->IsAlive() && summon->GetFaction() != kvaldir->GetFaction() && summon->GetEntry() != NPC_HAGRIM_HOPEBREAKER)
                    {
                        target = summon;
                        break;
                    }

        if (!target)
            target = FindEventPlayer(_stage == BRIDGE_STAGE_HOLD ? SPELL_PHASE_TEMPLE_183 : SPELL_PHASE_TEMPLE_184, false);

        if (target && kvaldir->IsAIEnabled())
            kvaldir->AI()->AttackStart(target);
        else
            kvaldir->GetMotionMaster()->MovePoint(POINT_WAVE_TEMPLE, BridgeNorthEnd);
    }

    void SummonWave()
    {
        uint8 count = _stage == BRIDGE_STAGE_HOLD ? 3 : 2;
        for (uint8 i = 0; i < count; ++i)
        {
            uint32 entry = (i == 2) ? NPC_KVALDIR_SANDREAPER : NPC_KVALDIR_BONESNAPPER;
            if (Creature* kvaldir = me->SummonCreature(entry, BridgeWaveSpawns[i % std::size(BridgeWaveSpawns)], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 2min))
                WaveAttack(kvaldir);
        }
    }

    void SummonColumn()
    {
        Creature* lady = me->SummonCreature(NPC_LADY_NAZJAR_TEMPLE, ColumnLadyPos, TEMPSUMMON_TIMED_DESPAWN, 10min);
        Creature* zinjatar = me->SummonCreature(NPC_FATHOM_LORD_TEMPLE, ColumnZinjatarPos, TEMPSUMMON_TIMED_DESPAWN, 10min);
        Creature* azjentus = me->SummonCreature(NPC_AZJENTUS_TEMPLE, ColumnAzjentusPos, TEMPSUMMON_TIMED_DESPAWN, 10min);
        for (Creature* member : { lady, zinjatar, azjentus })
            if (member)
                member->GetMotionMaster()->MoveSmoothPath(POINT_COLUMN_ADVANCE, ColumnAdvancePath, std::size(ColumnAdvancePath), true, false, 2.5f);
    }

    void ResetEvent()
    {
        _stage = BRIDGE_STAGE_IDLE;
        _summons.DespawnAll();
        _events.Reset();
        _events.ScheduleEvent(EVENT_BRIDGE_SCAN, 5s);
    }

    EventMap _events;
    SummonList _summons;
    uint8 _stage;
};

/*######
## Quest 25898 - Honor and Privilege
## 77741 - Rescue Flare: dummy; the flare arcs for ~5 s, then the balloon is
## credited and the surface Jorlan reacts.
######*/

class spell_vashjir_rescue_flare : public SpellScript
{
    void HandleAfterCast()
    {
        Unit* caster = GetCaster();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_HONOR_AND_PRIVILEGE) != QUEST_STATUS_INCOMPLETE)
            return;

        player->m_Events.AddEventAtOffset([player]()
        {
            player->KilledMonsterCredit(NPC_RESCUE_BALLOON);
            if (Creature* jorlan = player->FindNearestCreature(NPC_JORLAN_TRUEBLADE, 80.0f))
                if (jorlan->IsAIEnabled())
                    jorlan->AI()->Talk(SAY_JORLAN_NO_WAY_MISS, player);
        }, 5s);
    }

    void Register() override
    {
        AfterCast.Register(&spell_vashjir_rescue_flare::HandleAfterCast);
    }
};

/*######
## Quest 26219 - Full Circle (Pincer X2 voyage)
## npc_vashjir_chief_engineer_yoon - 42488 (DB spawn on Voldrin's Hold)
## Static-submarine simplification: Yoon announces the sub, and when a quest
## player reaches the hull door they are "boarded" (teleport + credit) and a
## personal crew-RP timeline plays out, ending with 79239 to Darkbreak Cove.
######*/

Position const SubInteriorPos = { -7234.0f, 3838.0f, -66.0f, 3.14f };

void SubCrewTalk(Player* player, uint32 entry, uint8 group)
{
    if (Creature* crew = player->FindNearestCreature(entry, 100.0f))
        if (crew->IsAIEnabled())
            crew->AI()->Talk(group, player);
}

void StartSubVoyage(Player* player)
{
    player->KilledMonsterCredit(NPC_BOARDING_CREDIT);
    player->NearTeleportTo(SubInteriorPos);

    // Crew RP offsets from the retail voyage (99299 at T0, arrival T+133 s)
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_FIRST_LIEUTENANT_WILEY, SAY_WILEY_APPROACHING); }, 67s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_CAPTAIN_GLOVAAL_SUB, SAY_GLOVAAL_TAKE_HER_IN); }, 75s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_FIRST_LIEUTENANT_WILEY, SAY_WILEY_REPORTS_ACCURATE); }, 84s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_CAPTAIN_GLOVAAL_SUB, SAY_GLOVAAL_STEADY); }, 91s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_CAPTAIN_GLOVAAL_SUB, SAY_GLOVAAL_FIRE); }, 96s);
    player->m_Events.AddEventAtOffset([player]() { player->KilledMonsterCredit(NPC_CAVERN_CREDIT); }, 101s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_FIRST_LIEUTENANT_WILEY, SAY_WILEY_BEAST_ESCAPED); }, 117s);
    player->m_Events.AddEventAtOffset([player]() { SubCrewTalk(player, NPC_CAPTAIN_GLOVAAL_SUB, SAY_GLOVAAL_DONT_WORRY); }, 125s);
    player->m_Events.AddEventAtOffset([player]()
    {
        player->RemoveAurasDueToSpell(SPELL_SUB_PHASE_GROUP);
        player->RemoveAurasDueToSpell(SPELL_SEE_QUEST_INVIS_18);
        // 79239: teleport to the Verne overlook (dest-db) + native phase updates
        player->CastSpell(player, SPELL_MOVE_OCCUPANTS_TO_LAND, true);
    }, 133s);
}

enum YoonEvents
{
    EVENT_YOON_SCAN = 1,
    EVENT_YOON_ALL_ABOARD
};

struct npc_vashjir_chief_engineer_yoon : public PassiveAI
{
    npc_vashjir_chief_engineer_yoon(Creature* creature) : PassiveAI(creature), _yellCooldown(0) { }

    void InitializeAI() override
    {
        _events.ScheduleEvent(EVENT_YOON_SCAN, 3s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        if (_yellCooldown > diff)
            _yellCooldown -= diff;
        else
            _yellCooldown = 0;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_YOON_SCAN:
                {
                    std::list<Player*> players;
                    me->GetPlayerListInGrid(players, 120.0f);
                    for (Player* player : players)
                    {
                        if (!player->IsAlive() || player->GetQuestStatus(QUEST_FULL_CIRCLE) != QUEST_STATUS_INCOMPLETE)
                            continue;
                        if (player->GetReqKillOrCastCurrentCount(QUEST_FULL_CIRCLE, int32(NPC_BOARDING_CREDIT)) > 0)
                            continue;

                        // Dock announcement for fresh arrivals near the deck
                        if (!_yellCooldown && me->GetExactDist2d(player) < 40.0f)
                        {
                            _yellCooldown = 120 * IN_MILLISECONDS;
                            Talk(SAY_YOON_ARRIVING_SHORTLY);
                            _events.ScheduleEvent(EVENT_YOON_ALL_ABOARD, 17s);
                        }

                        // Player reached the submarine door - board them
                        if (player->GetExactDist(&SubInteriorPos) < 12.0f)
                            StartSubVoyage(player);
                    }
                    _events.ScheduleEvent(EVENT_YOON_SCAN, 3s);
                    break;
                }
                case EVENT_YOON_ALL_ABOARD:
                    Talk(SAY_YOON_ALL_ABOARD);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    uint32 _yellCooldown;
};

/*######
## PlayerScript - Shimmering Expanse quest glue
######*/

class player_vashjir_shimmering_expanse : public PlayerScript
{
public:
    player_vashjir_shimmering_expanse() : PlayerScript("player_vashjir_shimmering_expanse") { }

    void OnPlayerQuestStatusChange(Player* player, uint32 questId) override
    {
        QuestStatus status = player->GetQuestStatus(questId);
        switch (questId)
        {
            case QUEST_THE_LOOMING_THREAT:
                if (status == QUEST_STATUS_REWARDED)
                {
                    // Turn-in (at the spirit inside the vision, or at Gadra):
                    // start the defense phase, and pull the player out of the
                    // phase-194 tableau if they are still in it.
                    player->CastSpell(player, SPELL_PHASE_2_INTRO_CAVE, true);
                    if (player->HasAura(SPELL_SPIRIT_VISION_3) || player->HasAura(SPELL_SPIRIT_VISION_2))
                    {
                        if (Creature* spirit = FindPlayerSummon(player, NPC_SPIRIT_OF_GADRA, 120.0f))
                            spirit->DespawnOrUnsummon(3s);
                        player->RemoveAurasDueToSpell(SPELL_SPIRIT_VISION_2);
                        player->RemoveAurasDueToSpell(SPELL_SPIRIT_VISION_3);
                        player->CastSpell(player, SPELL_SPIRIT_VISION_RETURN, true);
                    }
                }
                else if (status == QUEST_STATUS_NONE)
                {
                    player->RemoveAurasDueToSpell(SPELL_SPIRIT_TRANCE);
                    if (player->HasAura(SPELL_SPIRIT_VISION_3) || player->HasAura(SPELL_SPIRIT_VISION_2))
                    {
                        if (Creature* spirit = FindPlayerSummon(player, NPC_SPIRIT_OF_GADRA, 120.0f))
                            spirit->DespawnOrUnsummon();
                        player->RemoveAurasDueToSpell(SPELL_SPIRIT_VISION_2);
                        player->RemoveAurasDueToSpell(SPELL_SPIRIT_VISION_3);
                        player->CastSpell(player, SPELL_SPIRIT_VISION_RETURN, true);
                    }
                }
                break;
            case QUEST_BACKED_INTO_A_CORNER:
                if (status == QUEST_STATUS_NONE)
                {
                    // Abandon mid-event: unstick from the defense/showdown phases
                    player->RemoveAurasDueToSpell(SPELL_PHASE_2_INTRO_CAVE);
                    player->RemoveAurasDueToSpell(SPELL_PHASE_3_INTRO_CAVE);
                }
                break;
            case QUEST_FATHOM_LORDS_CALL:
                if (status == QUEST_STATUS_INCOMPLETE)
                    if (Creature* battlemaiden = FindPlayerSummon(player, NPC_BATTLEMAIDEN_VISION_1))
                        if (battlemaiden->IsAIEnabled())
                            battlemaiden->AI()->Talk(WHISPER_BATTLEMAIDEN_ABILITY, player);
                break;
            case QUEST_VISIONS_SLAUGHTER:
            case QUEST_VISIONS_RISE:
                if (status == QUEST_STATUS_INCOMPLETE)
                    player->CastSpell(player, SPELL_BATTLEMAIDEN_BACKUP, true); // retail accept-cast
                else if (status == QUEST_STATUS_NONE)
                    AbandonVision(player, questId);
                break;
            case QUEST_VISIONS_INVASION:
                if (status == QUEST_STATUS_NONE)
                    AbandonVision(player, questId);
                break;
            case QUEST_AT_ALL_COSTS:
                // quest_template_addon SourceSpellID backstop (phase 183)
                if (status == QUEST_STATUS_INCOMPLETE)
                    player->CastSpell(player, SPELL_PHASE_TEMPLE_183, true);
                break;
            case QUEST_HONOR_AND_PRIVILEGE:
                // quest_template_addon SourceSpellID backstop (surface reveal)
                if (status == QUEST_STATUS_INCOMPLETE)
                    player->CastSpell(player, SPELL_SEE_QUEST_INVIS_5, true);
                break;
            default:
                break;
        }
    }

    void OnPlayerLogin(Player* player) override
    {
        // Logged out mid-vision: the infinite possession aura is restored but
        // the vehicle chain is gone - finish the vision gracefully.
        for (BattlemaidenVision const& vision : BattlemaidenVisions)
        {
            if (!player->HasAura(vision.AuraId))
                continue;
            if (!player->GetVehicle())
                EndBattlemaidenVision(player, vision.AuraId);
            break;
        }
    }

private:
    static void AbandonVision(Player* player, uint32 questId)
    {
        for (BattlemaidenVision const& vision : BattlemaidenVisions)
            if (vision.QuestId == questId && player->HasAura(vision.AuraId))
            {
                EndBattlemaidenVision(player, vision.AuraId);
                break;
            }
    }
};

} // namespace Vashjir::ShimmeringExpanse

void AddSC_vashjir_shimmering_expanse()
{
    using namespace Vashjir::ShimmeringExpanse;

    // Legion's Rest
    RegisterSpellScript(spell_vashjir_spirit_trance_aura);
    RegisterSpellScript(spell_vashjir_spirit_vision_timer);
    RegisterSpellScript(spell_vashjir_naga_wave_engine);
    RegisterCreatureAI(npc_vashjir_spirit_of_gadra);
    RegisterCreatureAI(npc_vashjir_cave_defense_controller);
    RegisterCreatureAI(boss_vashjir_fathom_lord_zinjatar);

    // Vortex
    RegisterCreatureAI(npc_vashjir_toshes_vortex);
    RegisterSpellScript(spell_vashjir_toshes_vortex_trigger);

    // Nespirah
    new at_nespirah_tunnel();
    RegisterCreatureAI(npc_vashjir_nespirah_escort);
    RegisterSpellScript(spell_vashjir_summon_escape_seahorse);
    RegisterCreatureAI(npc_vashjir_escape_seahorse);

    // Battlemaiden visions
    RegisterSpellScript(spell_vashjir_blade_of_the_battlemaiden);
    RegisterSpellScript(spell_vashjir_battlemaiden_final_phase);
    RegisterSpellScript(spell_vashjir_battlemaiden_backup_credit);
    RegisterSpellScript(spell_vashjir_battlemaiden_vision_exit);
    RegisterCreatureAI(npc_vashjir_battlemaiden);
    RegisterSpellScript(spell_vashjir_naga_reinforcement_ping);
    RegisterCreatureAI(npc_vashjir_war_party);
    RegisterCreatureAI(npc_vashjir_temple_credit_bunny);

    // Quel'Dormir finale
    RegisterCreatureAI(npc_vashjir_bridge_controller);

    // Honor and Privilege / Full Circle
    RegisterSpellScript(spell_vashjir_rescue_flare);
    RegisterCreatureAI(npc_vashjir_chief_engineer_yoon);

    // Zone glue
    new player_vashjir_shimmering_expanse();
}
