
#pragma once

#include "CardTemplate.h"
#include "Maps.h"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt) {
                j = *opt; // this will call adl_serializer<T>::to_json which will
                // find the free function to_json in T's namespace!
            }
            else {
                j = nullptr;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = {};
            }
            else {
                opt = j.template get<T>(); // same as above, but with
                // adl_serializer<T>::from_json
            }
        }
    };
}

namespace api {
    struct InvalidVariant { };

    const uint64_t VERSION = 22;
    enum Upgrade {
        Upgrade_U0 = 0,
        Upgrade_U1 = 1000000,
        Upgrade_U2 = 2000000,
        Upgrade_U3 = 3000000,
    };

    //  ID of the card resource
    typedef uint32_t CardId;

    //  ID of squad resource
    typedef uint32_t SquadId;

    //  ID of building resource
    typedef uint32_t BuildingId;

    //  ID of spell resource
    typedef uint32_t SpellId;

    //  ID of ability resource
    typedef uint32_t AbilityId;

    //  ID of mode resource
    typedef uint32_t ModeId;

    //  ID of an entity present in the match unique to that match
    //  First entity have ID 1, next 2, ...
    //  Ids are never reused
    typedef uint32_t EntityId;

    //  Time information 1 tick = 0.1s = 100 ms
    typedef uint32_t Tick;

    //  Difference between two `Tick` (points in times, remaining time, ...)
    typedef uint32_t TickCount;

    //  `x` and `z` are coordinates on the 2D map.
    struct Position {
        float x;
        //  Also known as height.
        float y;
        float z;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position, x, y, z);

    struct Position2D {
        float x;
        float y;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position2D, x, y);

    struct Position2DWithOrientation {
        float x;
        float y;
        //  in default camera orientation
        //  0 = down, π/2 = right, π = up, π3/2 = left
        float orientation;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Position2DWithOrientation, x, y, orientation);

    //  Color of an orb.
    enum OrbColor {
        OrbColor_White = 0,
        OrbColor_Shadow = 1,
        OrbColor_Nature = 2,
        OrbColor_Frost = 3,
        OrbColor_Fire = 4,
        OrbColor_Starting = 5,
        OrbColor_All = 7,
    };

    //  Subset of `OrbColor`, because creating the other colors does not make sense.
    enum CreateOrbColor {
        CreateOrbColor_Shadow = 1,
        CreateOrbColor_Nature = 2,
        CreateOrbColor_Frost = 3,
        CreateOrbColor_Fire = 4,
    };

    //  Target entity
    struct SingleTargetSingleEntity {
        EntityId id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SingleTargetSingleEntity, id);
    //  Target location on the ground
    struct SingleTargetLocation {
        Position2D xy;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SingleTargetLocation, xy);

    //  When targeting you can target either entity, or ground coordinates.
    struct SingleTarget {
        std::variant<InvalidVariant, SingleTargetSingleEntity, SingleTargetLocation> v;
    };
    inline void to_json(nlohmann::json& j, const SingleTarget& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "SingleEntity", std::get<SingleTargetSingleEntity>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "Location", std::get<SingleTargetLocation>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, SingleTarget& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "SingleEntity") { v.v = value.template get<SingleTargetSingleEntity>(); }
            if (key == "Location") { v.v = value.template get<SingleTargetLocation>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    struct TargetSingle {
        SingleTarget single;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetSingle, single);
    struct TargetMulti {
        Position2D xy_begin;
        Position2D xy_end;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetMulti, xy_begin, xy_end);

    struct Target {
        std::variant<InvalidVariant, TargetSingle, TargetMulti> v;
    };
    inline void to_json(nlohmann::json& j, const Target& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "Single", std::get<TargetSingle>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "Multi", std::get<TargetMulti>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Target& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Single") { v.v = value.template get<TargetSingle>(); }
            if (key == "Multi") { v.v = value.template get<TargetMulti>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    enum WalkMode {
        WalkMode_PartialForce = 1,
        WalkMode_Force = 2,
        //  Also called by players "Attack move", or "Q move"
        WalkMode_Normal = 4,
        WalkMode_Crusade = 5,
        WalkMode_Scout = 6,
        WalkMode_Patrol = 7,
    };

    struct CommunityMapInfo {
        //  Name of the map.
        std::string name;
        //  Checksum of them map.
        uint64_t crc;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommunityMapInfo, name, crc);

    //  Official spectator maps are normal maps (have unique id) so only `map` field is needed.
    struct MapInfo {
        //  Represents the map, unfortunately EA decided, it will be harder for community maps.
        maps::Maps map;
        //  Is relevant only for community maps.
        std::optional<CommunityMapInfo> community_map_details;
    };
    inline void to_json(nlohmann::json& j, const MapInfo& v) {
        j["map"] = v.map;
        if (v.community_map_details.has_value()) {
            j["community_map_details"] = v.community_map_details;
        }
    }
    inline void from_json(const nlohmann::json& j, MapInfo& v) {
        j.at("map").get_to(v.map);
        if (j.count("community_map_details") != 0) {
            j.at("community_map_details").get_to(v.community_map_details);
        }
    }

    struct Deck {
        //  Name of the deck, must be unique across decks used by bot, but different bots can have same deck names.
        //  Must not contain spaces, to be addable in game.
        std::string name;
        //  Index of a card that will be deck icon 0 to 19 inclusive
        uint8_t cover_card_index;
        //  List of 20 cards in deck.
        //  Fill empty spaces with `NotACard`.
        CardId cards[20];
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Deck, name, cover_card_index, cards);

    enum AbilityLine {
        AbilityLine__EAsBug_betterSafeThanSorry = 0,
        AbilityLine_ModifyWalkSpeed = 1,
        AbilityLine_UnControllable = 4,
        AbilityLine_UnKillable = 5,
        AbilityLine_UnAttackable = 9,
        AbilityLine_HitMultiple = 10,
        AbilityLine__ACModifier = 14,
        AbilityLine_DamageOverTime = 15,
        AbilityLine_DamageBuff = 21,
        AbilityLine_PowerOutputModifier = 23,
        AbilityLine_MoveSpeedOverwrite = 24,
        AbilityLine_HitMultipleRanged = 25,
        AbilityLine_HPModifier = 26,
        AbilityLine_Aura = 27,
        AbilityLine_PreventCardPlay = 29,
        AbilityLine__SpreadFire = 31,
        AbilityLine__SquadSpawnZone = 32,
        AbilityLine_HitMultipleProjectile = 33,
        AbilityLine__MarkedTargetDamageMultiplier = 36,
        AbilityLine__MarkedTargetDamage = 37,
        AbilityLine__AttackPauseDelay = 39,
        AbilityLine__MarkedForTeleport = 40,
        AbilityLine__RangeModifier = 41,
        AbilityLine_ForceAttack = 42,
        AbilityLine_OnEntityDie = 44,
        AbilityLine__FireLanceAbility = 47,
        AbilityLine_Collector = 50,
        AbilityLine__ChangeTargetAggro = 51,
        AbilityLine__FireLanceBurstCollector = 53,
        AbilityLine_RegenerationOld = 54,
        AbilityLine__TimedSpell = 57,
        AbilityLine_Scatter = 58,
        AbilityLine__DamageOverTimeNoCombat = 59,
        AbilityLine_LifeStealer = 60,
        AbilityLine_BarrierGate = 61,
        AbilityLine_GlobalRevive = 62,
        AbilityLine_TrampleResistance = 64,
        AbilityLine_TrampleOverwrite = 65,
        AbilityLine_PushbackResistance = 66,
        AbilityLine_MeleePushbackOverride = 67,
        AbilityLine__FanCollector = 69,
        AbilityLine_MeleeFightSpeedModifier = 71,
        AbilityLine__SpellRangeModifierIncoming = 72,
        AbilityLine_SpellRangeModifierOutgoing = 73,
        AbilityLine__FanCollectorBurst = 74,
        AbilityLine_RangedFightSpeedModifier = 75,
        AbilityLine__SquadRestore = 76,
        AbilityLine_DamagePowerTransfer = 79,
        AbilityLine_TimedSpell = 80,
        AbilityLine_TrampleRevengeDamage = 81,
        AbilityLine_LinkedFire = 83,
        AbilityLine_DamageBuffAgainst = 84,
        AbilityLine_IncomingDamageModifier = 85,
        AbilityLine_GeneratorPower = 86,
        AbilityLine_IceShield = 87,
        AbilityLine_DoTRefresh = 88,
        AbilityLine_EnrageThreshold = 89,
        AbilityLine_Immunity = 90,
        AbilityLine__UnitSpawnZone = 91,
        AbilityLine_Rage = 92,
        AbilityLine__PassiveCharge = 93,
        AbilityLine_MeleeHitSpell = 95,
        AbilityLine__FireDebuff = 97,
        AbilityLine_FrostDebuff = 98,
        AbilityLine_SpellBlocker = 100,
        AbilityLine_ShadowDebuff = 102,
        AbilityLine_SuicidalBomb = 103,
        AbilityLine_GrantToken = 110,
        AbilityLine_TurretCannon = 112,
        AbilityLine_SpellOnSelfCast = 113,
        AbilityLine_AbilityOnSelfResolve = 114,
        AbilityLine_SuppressUserCommand = 118,
        AbilityLine_LineCast = 120,
        AbilityLine_NoCheer = 132,
        AbilityLine_UnitShredderJobCondition = 133,
        AbilityLine_DamageRadialArea = 134,
        AbilityLine__DamageConeArea = 137,
        AbilityLine_DamageConeCutArea = 138,
        AbilityLine_ConstructionRepairModifier = 139,
        AbilityLine_Portal = 140,
        AbilityLine_Tunnel = 141,
        AbilityLine_ModeConditionDelay = 142,
        AbilityLine_HealAreaRadial = 144,
        AbilityLine__145LeftoverDoesNotReallyExistButIsUsed = 145,
        AbilityLine__146LeftoverDoesNotReallyExistButIsUsed = 146,
        AbilityLine_OverrideWeaponType = 151,
        AbilityLine_DamageRadialAreaUsingCorpse = 153,
        AbilityLine_HealAreaRadialInstantContinues = 154,
        AbilityLine_ChargeableBombController = 155,
        AbilityLine_ChargeAttack = 156,
        AbilityLine_ChargeableBomb = 157,
        AbilityLine_ModifyRotationSpeed = 159,
        AbilityLine_ModifyAcceleration = 160,
        AbilityLine_FormationOverwrite = 161,
        AbilityLine_EffectHolder = 162,
        AbilityLine_WhiteRangersHomeDefenseTrigger = 163,
        AbilityLine__167LeftoverDoesNotReallyExistButIsUsed = 167,
        AbilityLine__168LeftoverDoesNotReallyExistButIsUsed = 168,
        AbilityLine_HealReservoirUsingCorpse = 170,
        AbilityLine_ModeChangeBlocker = 171,
        AbilityLine_BarrierModuleEnterBlock = 172,
        AbilityLine_ProduceAmmoUsingCorpseInjurity = 173,
        AbilityLine_IncomingDamageSpreadOnTargetAlignmentArea1 = 174,
        AbilityLine_DamageSelfOnMeleeHit = 175,
        AbilityLine_HealthCapCurrent = 176,
        AbilityLine_ConstructionUnCrushable = 179,
        AbilityLine_ProduceAmmoOverTime = 180,
        AbilityLine_BarrierSetBuildDelay = 181,
        AbilityLine_ChannelTimedSpell = 183,
        AbilityLine_AuraOnEnter = 184,
        AbilityLine_ParalyzeAbility = 185,
        AbilityLine_IgnoreSummoningSickness = 186,
        AbilityLine_BlockRepair = 187,
        AbilityLine_Corruption = 188,
        AbilityLine_UnHealable = 189,
        AbilityLine_Immobile = 190,
        AbilityLine_ModifyHealing = 191,
        AbilityLine_IgnoreInCardCondition = 192,
        AbilityLine_MovementMode = 193,
        AbilityLine_ConsumeAmmoHealSelf = 195,
        AbilityLine_ConsumeAmmoHealAreaRadial = 196,
        AbilityLine_CorpseGather = 197,
        AbilityLine_AbilityNearEntity = 198,
        AbilityLine_ModifyIceShieldDecayRate = 200,
        AbilityLine_ModifyDamageIncomingAuraContingentSelfDamage = 201,
        AbilityLine_ModifyDamageIncomingAuraContingentSelfDamageTargetAbility = 202,
        AbilityLine_ConvertCorpseToPower = 203,
        AbilityLine_EraseOverTime = 204,
        AbilityLine_FireStreamChannel = 205,
        AbilityLine_DisableMeleeAttack = 206,
        AbilityLine_AbilityOnPlayer = 207,
        AbilityLine_GlobalAbilityOnEntity = 208,
        AbilityLine_AuraModifyCardCost = 209,
        AbilityLine_AuraModifyBuildTime = 210,
        AbilityLine_GlobalRotTimeModifier = 211,
        AbilityLine__212LeftoverDoesNotReallyExistButIsUsed = 212,
        AbilityLine_MindControl = 213,
        AbilityLine_SpellOnEntityNearby = 214,
        AbilityLine_AmmoConsumeModifyIncomingDamage = 216,
        AbilityLine_AmmoConsumeModifyOutgoingDamage = 217,
        AbilityLine_GlobalSuppressRefund = 219,
        AbilityLine_DirectRefundOnDie = 220,
        AbilityLine_OutgoingDamageDependendSpell = 221,
        AbilityLine_DeathCounter = 222,
        AbilityLine_DeathCounterController = 223,
        AbilityLine_DamageRadialAreaUsingGraveyard = 224,
        AbilityLine_MovingIntervalCast = 225,
        AbilityLine_BarrierGateDelay = 226,
        AbilityLine_EffectHolderAmmo = 227,
        AbilityLine_FightDependentAbility = 228,
        AbilityLine_GlobalIgnoreCardPlayConditions = 229,
        AbilityLine_WormMovement = 230,
        AbilityLine_DamageRectAreaAligned = 231,
        AbilityLine_GlobalRefundOnEntityDie = 232,
        AbilityLine_GlobalDamageAbsorption = 233,
        AbilityLine_GlobalPowerRecovermentModifier = 234,
        AbilityLine_GlobalDamageAbsorptionTargetAbility = 235,
        AbilityLine_OverwriteVisRange = 236,
        AbilityLine_DamageOverTimeCastDepending = 237,
        AbilityLine_ModifyDamageIncomingAuraContingentSelfRadialAreaDamage = 238,
        AbilityLine_SuperWeaponShadow = 239,
        AbilityLine_NoMeleeAgainstAir = 240,
        AbilityLine__SuperWeaponShadowDamage = 242,
        AbilityLine_NoCardPlay = 243,
        AbilityLine_NoClaim = 244,
        AbilityLine_DamageRadialAreaAmmo = 246,
        AbilityLine_PathLayerOverride = 247,
        AbilityLine_ChannelBlock = 248,
        AbilityLine_Polymorph = 249,
        AbilityLine_Delay = 250,
        AbilityLine_ModifyDamageIncomingOnFigure = 251,
        AbilityLine_ImmobileRoot = 252,
        AbilityLine_GlobalModifyCorpseGather = 253,
        AbilityLine_AbilityDependentAbility = 254,
        AbilityLine_CorpseManager = 255,
        AbilityLine_DisableToken = 256,
        AbilityLine_Piercing = 258,
        AbilityLine_ReceiveMeleeAttacks = 259,
        AbilityLine_BuildBlock = 260,
        AbilityLine_PreventCardPlayAuraBuilding = 262,
        AbilityLine_GraveyardDependentRecast = 263,
        AbilityLine_ClaimBlock = 264,
        AbilityLine_AmmoStartup = 265,
        AbilityLine_DamageDistribution = 266,
        AbilityLine_SwapSquadNightGuard = 267,
        AbilityLine_Revive = 268,
        AbilityLine_Amok = 269,
        AbilityLine_NoCombat = 270,
        AbilityLine_SlowDownDisabled = 271,
        AbilityLine_CrowdControlTimeModifier = 272,
        AbilityLine_DamageOnMeleeHit = 273,
        AbilityLine_IgnoreIncomingDamageModifier = 275,
        AbilityLine_BlockRevive = 278,
        AbilityLine_GlobalMorphState = 279,
        AbilityLine_SpecialOnTarget = 280,
        AbilityLine_FleshBenderBugSwitch = 281,
        AbilityLine_TimedMorph = 282,
        AbilityLine_GlobalBuildTimeModifier = 283,
        AbilityLine_CardBlock = 285,
        AbilityLine_IceShieldRegeneration = 286,
        AbilityLine_HealOverTime = 287,
        AbilityLine_IceShieldTimerOffset = 288,
        AbilityLine_SpellOnVanish = 289,
        AbilityLine_GlobalVoidAbsorption = 290,
        AbilityLine_VoidContainer = 291,
        AbilityLine_ConvertCorpseToHealing = 292,
        AbilityLine_OnEntitySpawn = 293,
        AbilityLine_OnMorph = 294,
        AbilityLine_Sprint = 295,
    };

    struct AreaShapeCircle {
        Position2D center;
        float radius;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AreaShapeCircle, center, radius);
    struct AreaShapeCone {
        Position2D base;
        float radius;
        float angle;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AreaShapeCone, base, radius, angle);
    struct AreaShapeConeCut {
        Position2D start;
        //  or maybe direction (normalized to length 1), I did not quickly find example to check this on
        Position2D end;
        float radius;
        float width_near;
        float width_far;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AreaShapeConeCut, start, end, radius, width_near, width_far);
    struct AreaShapeWideLine {
        Position2D start;
        Position2D end;
        float width;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AreaShapeWideLine, start, end, width);

    struct AreaShape {
        std::variant<InvalidVariant, AreaShapeCircle, AreaShapeCone, AreaShapeConeCut, AreaShapeWideLine> v;
    };
    inline void to_json(nlohmann::json& j, const AreaShape& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "Circle", std::get<AreaShapeCircle>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "Cone", std::get<AreaShapeCone>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "ConeCut", std::get<AreaShapeConeCut>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "WideLine", std::get<AreaShapeWideLine>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, AreaShape& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Circle") { v.v = value.template get<AreaShapeCircle>(); }
            if (key == "Cone") { v.v = value.template get<AreaShapeCone>(); }
            if (key == "ConeCut") { v.v = value.template get<AreaShapeConeCut>(); }
            if (key == "WideLine") { v.v = value.template get<AreaShapeWideLine>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    struct AbilityEffectSpecificDamageArea {
        float progress_current;
        float progress_delta;
        float damage_remaining;
        AreaShape shape;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificDamageArea, progress_current, progress_delta, damage_remaining, shape);
    struct AbilityEffectSpecificDamageOverTime {
        TickCount tick_wait_duration;
        TickCount ticks_left;
        float tick_damage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificDamageOverTime, tick_wait_duration, ticks_left, tick_damage);
    struct AbilityEffectSpecificLinkedFire {
        bool linked;
        bool fighting;
        uint32_t fast_cast;
        uint16_t support_cap;
        uint8_t support_production;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificLinkedFire, linked, fighting, fast_cast, support_cap, support_production);
    struct AbilityEffectSpecificSpellOnEntityNearby {
        std::vector<SpellId> spell_on_owner;
        std::vector<SpellId> spell_on_source;
        float radius;
        uint32_t remaining_targets;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificSpellOnEntityNearby, spell_on_owner, spell_on_source, radius, remaining_targets);
    struct AbilityEffectSpecificTimedSpell {
        std::vector<SpellId> spells_to_cast;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificTimedSpell, spells_to_cast);
    struct AbilityEffectSpecificCollector {
        SpellId spell_to_cast;
        float radius;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificCollector, spell_to_cast, radius);
    struct AbilityEffectSpecificAura {
        std::vector<SpellId> spells_to_apply;
        std::vector<AbilityId> abilities_to_apply;
        float radius;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificAura, spells_to_apply, abilities_to_apply, radius);
    struct AbilityEffectSpecificMovingIntervalCast {
        std::vector<SpellId> spell_to_cast;
        Position2D direction_step;
        TickCount cast_every_nth_tick;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityEffectSpecificMovingIntervalCast, spell_to_cast, direction_step, cast_every_nth_tick);
    //  If you think something interesting got hidden by Other report it
    struct AbilityEffectSpecificOther {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AbilityEffectSpecificOther& v) {
        j = nlohmann::json{ { "AbilityEffectSpecificOther", {} } };
    }
    inline void from_json(const nlohmann::json& j, AbilityEffectSpecificOther& v) { v._just_to_make_cpp_compiler_happy = true; }

    struct AbilityEffectSpecific {
        std::variant<InvalidVariant, AbilityEffectSpecificDamageArea, AbilityEffectSpecificDamageOverTime, AbilityEffectSpecificLinkedFire, AbilityEffectSpecificSpellOnEntityNearby, AbilityEffectSpecificTimedSpell, AbilityEffectSpecificCollector, AbilityEffectSpecificAura, AbilityEffectSpecificMovingIntervalCast, AbilityEffectSpecificOther> v;
    };
    inline void to_json(nlohmann::json& j, const AbilityEffectSpecific& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "DamageArea", std::get<AbilityEffectSpecificDamageArea>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "DamageOverTime", std::get<AbilityEffectSpecificDamageOverTime>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "LinkedFire", std::get<AbilityEffectSpecificLinkedFire>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "SpellOnEntityNearby", std::get<AbilityEffectSpecificSpellOnEntityNearby>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "TimedSpell", std::get<AbilityEffectSpecificTimedSpell>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "Collector", std::get<AbilityEffectSpecificCollector>(v.v) } }; break;
        case 7: j = nlohmann::json{ { "Aura", std::get<AbilityEffectSpecificAura>(v.v) } }; break;
        case 8: j = nlohmann::json{ { "MovingIntervalCast", std::get<AbilityEffectSpecificMovingIntervalCast>(v.v) } }; break;
        case 9: j = nlohmann::json{ { "Other", std::get<AbilityEffectSpecificOther>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, AbilityEffectSpecific& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "DamageArea") { v.v = value.template get<AbilityEffectSpecificDamageArea>(); }
            if (key == "DamageOverTime") { v.v = value.template get<AbilityEffectSpecificDamageOverTime>(); }
            if (key == "LinkedFire") { v.v = value.template get<AbilityEffectSpecificLinkedFire>(); }
            if (key == "SpellOnEntityNearby") { v.v = value.template get<AbilityEffectSpecificSpellOnEntityNearby>(); }
            if (key == "TimedSpell") { v.v = value.template get<AbilityEffectSpecificTimedSpell>(); }
            if (key == "Collector") { v.v = value.template get<AbilityEffectSpecificCollector>(); }
            if (key == "Aura") { v.v = value.template get<AbilityEffectSpecificAura>(); }
            if (key == "MovingIntervalCast") { v.v = value.template get<AbilityEffectSpecificMovingIntervalCast>(); }
            if (key == "Other") { v.v = value.template get<AbilityEffectSpecificOther>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    struct AbilityEffect {
        AbilityId id;
        AbilityLine line;
        EntityId source;
        uint8_t source_team;
        std::optional<Tick> start_tick;
        std::optional<Tick> end_tick;
        AbilityEffectSpecific specific;
    };
    inline void to_json(nlohmann::json& j, const AbilityEffect& v) {
        j["id"] = v.id;
        j["line"] = v.line;
        j["source"] = v.source;
        j["source_team"] = v.source_team;
        if (v.start_tick.has_value()) {
            j["start_tick"] = v.start_tick;
        }
        if (v.end_tick.has_value()) {
            j["end_tick"] = v.end_tick;
        }
        j["specific"] = v.specific;
    }
    inline void from_json(const nlohmann::json& j, AbilityEffect& v) {
        j.at("id").get_to(v.id);
        j.at("line").get_to(v.line);
        j.at("source").get_to(v.source);
        j.at("source_team").get_to(v.source_team);
        if (j.count("start_tick") != 0) {
            j.at("start_tick").get_to(v.start_tick);
        }
        if (j.count("end_tick") != 0) {
            j.at("end_tick").get_to(v.end_tick);
        }
        j.at("specific").get_to(v.specific);
    }

    //  not mounted on any barrier (EA's 0)
    struct MountStateUnmounted {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const MountStateUnmounted& v) {
        j = nlohmann::json{ { "MountStateUnmounted", {} } };
    }
    inline void from_json(const nlohmann::json& j, MountStateUnmounted& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  squad in process of mounting to barrier (EA's 1, 2, 3)
    struct MountStateMountingSquad {
        EntityId barrier;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MountStateMountingSquad, barrier);
    //  figure in process of mounting to barrier (EA's 1, 2, 3)
    struct MountStateMountingFigure {
        EntityId barrier;
        uint8_t slot;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MountStateMountingFigure, barrier, slot);
    //  squad mounted to barrier (EA's 4)
    struct MountStateMountedSquad {
        EntityId barrier;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MountStateMountedSquad, barrier);
    //  figure mounted to barrier (EA's 4)
    struct MountStateMountedFigure {
        EntityId barrier;
        uint8_t slot;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MountStateMountedFigure, barrier, slot);
    //  Unknown (EA's 5, 6) please report a bug (ideally with steps to reproduce)
    struct MountStateUnknown {
        uint8_t mount_state;
        uint32_t enter_exit_barrier_module;
        uint32_t target_barrier_module;
        uint32_t current_barrier_module;
        uint32_t slot;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MountStateUnknown, mount_state, enter_exit_barrier_module, target_barrier_module, current_barrier_module, slot);

    //  State of entity being mounted (or not) on barrier
    struct MountState {
        std::variant<InvalidVariant, MountStateUnmounted, MountStateMountingSquad, MountStateMountingFigure, MountStateMountedSquad, MountStateMountedFigure, MountStateUnknown> v;
    };
    inline void to_json(nlohmann::json& j, const MountState& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "Unmounted", std::get<MountStateUnmounted>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "MountingSquad", std::get<MountStateMountingSquad>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "MountingFigure", std::get<MountStateMountingFigure>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "MountedSquad", std::get<MountStateMountedSquad>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "MountedFigure", std::get<MountStateMountedFigure>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "Unknown", std::get<MountStateUnknown>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, MountState& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Unmounted") { v.v = value.template get<MountStateUnmounted>(); }
            if (key == "MountingSquad") { v.v = value.template get<MountStateMountingSquad>(); }
            if (key == "MountingFigure") { v.v = value.template get<MountStateMountingFigure>(); }
            if (key == "MountedSquad") { v.v = value.template get<MountStateMountedSquad>(); }
            if (key == "MountedFigure") { v.v = value.template get<MountStateMountedFigure>(); }
            if (key == "Unknown") { v.v = value.template get<MountStateUnknown>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    //  Used by *mostly* power wells
    struct AspectPowerProduction {
        //  How much more power it will produce
        float current_power;
        //  Same as `current_power`, before it is build for the first time.
        float power_capacity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectPowerProduction, current_power, power_capacity);
    //  Health of an entity.
    struct AspectHealth {
        //  Actual HP that it can lose before dying.
        float current_hp;
        //  Current maximum including bufs and debufs.
        float cap_current_max;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectHealth, current_hp, cap_current_max);
    struct AspectCombat {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectCombat& v) {
        j = nlohmann::json{ { "AspectCombat", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectCombat& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectModeChange {
        ModeId current_mode;
        std::vector<ModeId> all_modes;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectModeChange, current_mode, all_modes);
    struct AspectAmmunition {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectAmmunition& v) {
        j = nlohmann::json{ { "AspectAmmunition", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectAmmunition& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectSuperWeaponShadow {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectSuperWeaponShadow& v) {
        j = nlohmann::json{ { "AspectSuperWeaponShadow", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectSuperWeaponShadow& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectWormMovement {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectWormMovement& v) {
        j = nlohmann::json{ { "AspectWormMovement", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectWormMovement& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectNPCTag {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectNPCTag& v) {
        j = nlohmann::json{ { "AspectNPCTag", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectNPCTag& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectPlayerKit {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectPlayerKit& v) {
        j = nlohmann::json{ { "AspectPlayerKit", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectPlayerKit& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectLoot {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectLoot& v) {
        j = nlohmann::json{ { "AspectLoot", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectLoot& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectImmunity {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectImmunity& v) {
        j = nlohmann::json{ { "AspectImmunity", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectImmunity& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectTurret {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectTurret& v) {
        j = nlohmann::json{ { "AspectTurret", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectTurret& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectTunnel {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectTunnel& v) {
        j = nlohmann::json{ { "AspectTunnel", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectTunnel& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectMountBarrier {
        MountState state;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectMountBarrier, state);
    struct AspectSpellMemory {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectSpellMemory& v) {
        j = nlohmann::json{ { "AspectSpellMemory", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectSpellMemory& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectPortal {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectPortal& v) {
        j = nlohmann::json{ { "AspectPortal", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectPortal& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectHate {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectHate& v) {
        j = nlohmann::json{ { "AspectHate", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectHate& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectBarrierGate {
        bool open;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectBarrierGate, open);
    struct AspectAttackable {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectAttackable& v) {
        j = nlohmann::json{ { "AspectAttackable", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectAttackable& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectSquadRefill {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectSquadRefill& v) {
        j = nlohmann::json{ { "AspectSquadRefill", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectSquadRefill& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectPortalExit {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectPortalExit& v) {
        j = nlohmann::json{ { "AspectPortalExit", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectPortalExit& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  When building / barrier is under construction it has this aspect.
    struct AspectConstructionData {
        //  Build ticks until finished.
        TickCount refresh_count_remaining;
        //  Build ticks needed from start of construction to finish it.
        TickCount refresh_count_total;
        //  How much health is added on build tick.
        float health_per_build_update_trigger;
        //  How much health is still missing.
        float remaining_health_to_add;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AspectConstructionData, refresh_count_remaining, refresh_count_total, health_per_build_update_trigger, remaining_health_to_add);
    struct AspectSuperWeaponShadowBomb {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectSuperWeaponShadowBomb& v) {
        j = nlohmann::json{ { "AspectSuperWeaponShadowBomb", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectSuperWeaponShadowBomb& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectRepairBarrierSet {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectRepairBarrierSet& v) {
        j = nlohmann::json{ { "AspectRepairBarrierSet", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectRepairBarrierSet& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectConstructionRepair {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectConstructionRepair& v) {
        j = nlohmann::json{ { "AspectConstructionRepair", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectConstructionRepair& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectFollower {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectFollower& v) {
        j = nlohmann::json{ { "AspectFollower", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectFollower& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectCollisionBase {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectCollisionBase& v) {
        j = nlohmann::json{ { "AspectCollisionBase", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectCollisionBase& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectEditorUniqueID {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectEditorUniqueID& v) {
        j = nlohmann::json{ { "AspectEditorUniqueID", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectEditorUniqueID& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct AspectRoam {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const AspectRoam& v) {
        j = nlohmann::json{ { "AspectRoam", {} } };
    }
    inline void from_json(const nlohmann::json& j, AspectRoam& v) { v._just_to_make_cpp_compiler_happy = true; }

    //  Most of the aspects do not contain data, if you think any of them would contain something,
    //  and you would want to use it, let me know, and I will add it
    struct Aspect {
        std::variant<InvalidVariant, AspectPowerProduction, AspectHealth, AspectCombat, AspectModeChange, AspectAmmunition, AspectSuperWeaponShadow, AspectWormMovement, AspectNPCTag, AspectPlayerKit, AspectLoot, AspectImmunity, AspectTurret, AspectTunnel, AspectMountBarrier, AspectSpellMemory, AspectPortal, AspectHate, AspectBarrierGate, AspectAttackable, AspectSquadRefill, AspectPortalExit, AspectConstructionData, AspectSuperWeaponShadowBomb, AspectRepairBarrierSet, AspectConstructionRepair, AspectFollower, AspectCollisionBase, AspectEditorUniqueID, AspectRoam> v;
    };
    inline void to_json(nlohmann::json& j, const Aspect& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "PowerProduction", std::get<AspectPowerProduction>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "Health", std::get<AspectHealth>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "Combat", std::get<AspectCombat>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "ModeChange", std::get<AspectModeChange>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "Ammunition", std::get<AspectAmmunition>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "SuperWeaponShadow", std::get<AspectSuperWeaponShadow>(v.v) } }; break;
        case 7: j = nlohmann::json{ { "WormMovement", std::get<AspectWormMovement>(v.v) } }; break;
        case 8: j = nlohmann::json{ { "NPCTag", std::get<AspectNPCTag>(v.v) } }; break;
        case 9: j = nlohmann::json{ { "PlayerKit", std::get<AspectPlayerKit>(v.v) } }; break;
        case 10: j = nlohmann::json{ { "Loot", std::get<AspectLoot>(v.v) } }; break;
        case 11: j = nlohmann::json{ { "Immunity", std::get<AspectImmunity>(v.v) } }; break;
        case 12: j = nlohmann::json{ { "Turret", std::get<AspectTurret>(v.v) } }; break;
        case 13: j = nlohmann::json{ { "Tunnel", std::get<AspectTunnel>(v.v) } }; break;
        case 14: j = nlohmann::json{ { "MountBarrier", std::get<AspectMountBarrier>(v.v) } }; break;
        case 15: j = nlohmann::json{ { "SpellMemory", std::get<AspectSpellMemory>(v.v) } }; break;
        case 16: j = nlohmann::json{ { "Portal", std::get<AspectPortal>(v.v) } }; break;
        case 17: j = nlohmann::json{ { "Hate", std::get<AspectHate>(v.v) } }; break;
        case 18: j = nlohmann::json{ { "BarrierGate", std::get<AspectBarrierGate>(v.v) } }; break;
        case 19: j = nlohmann::json{ { "Attackable", std::get<AspectAttackable>(v.v) } }; break;
        case 20: j = nlohmann::json{ { "SquadRefill", std::get<AspectSquadRefill>(v.v) } }; break;
        case 21: j = nlohmann::json{ { "PortalExit", std::get<AspectPortalExit>(v.v) } }; break;
        case 22: j = nlohmann::json{ { "ConstructionData", std::get<AspectConstructionData>(v.v) } }; break;
        case 23: j = nlohmann::json{ { "SuperWeaponShadowBomb", std::get<AspectSuperWeaponShadowBomb>(v.v) } }; break;
        case 24: j = nlohmann::json{ { "RepairBarrierSet", std::get<AspectRepairBarrierSet>(v.v) } }; break;
        case 25: j = nlohmann::json{ { "ConstructionRepair", std::get<AspectConstructionRepair>(v.v) } }; break;
        case 26: j = nlohmann::json{ { "Follower", std::get<AspectFollower>(v.v) } }; break;
        case 27: j = nlohmann::json{ { "CollisionBase", std::get<AspectCollisionBase>(v.v) } }; break;
        case 28: j = nlohmann::json{ { "EditorUniqueID", std::get<AspectEditorUniqueID>(v.v) } }; break;
        case 29: j = nlohmann::json{ { "Roam", std::get<AspectRoam>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Aspect& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "PowerProduction") { v.v = value.template get<AspectPowerProduction>(); }
            if (key == "Health") { v.v = value.template get<AspectHealth>(); }
            if (key == "Combat") { v.v = value.template get<AspectCombat>(); }
            if (key == "ModeChange") { v.v = value.template get<AspectModeChange>(); }
            if (key == "Ammunition") { v.v = value.template get<AspectAmmunition>(); }
            if (key == "SuperWeaponShadow") { v.v = value.template get<AspectSuperWeaponShadow>(); }
            if (key == "WormMovement") { v.v = value.template get<AspectWormMovement>(); }
            if (key == "NPCTag") { v.v = value.template get<AspectNPCTag>(); }
            if (key == "PlayerKit") { v.v = value.template get<AspectPlayerKit>(); }
            if (key == "Loot") { v.v = value.template get<AspectLoot>(); }
            if (key == "Immunity") { v.v = value.template get<AspectImmunity>(); }
            if (key == "Turret") { v.v = value.template get<AspectTurret>(); }
            if (key == "Tunnel") { v.v = value.template get<AspectTunnel>(); }
            if (key == "MountBarrier") { v.v = value.template get<AspectMountBarrier>(); }
            if (key == "SpellMemory") { v.v = value.template get<AspectSpellMemory>(); }
            if (key == "Portal") { v.v = value.template get<AspectPortal>(); }
            if (key == "Hate") { v.v = value.template get<AspectHate>(); }
            if (key == "BarrierGate") { v.v = value.template get<AspectBarrierGate>(); }
            if (key == "Attackable") { v.v = value.template get<AspectAttackable>(); }
            if (key == "SquadRefill") { v.v = value.template get<AspectSquadRefill>(); }
            if (key == "PortalExit") { v.v = value.template get<AspectPortalExit>(); }
            if (key == "ConstructionData") { v.v = value.template get<AspectConstructionData>(); }
            if (key == "SuperWeaponShadowBomb") { v.v = value.template get<AspectSuperWeaponShadowBomb>(); }
            if (key == "RepairBarrierSet") { v.v = value.template get<AspectRepairBarrierSet>(); }
            if (key == "ConstructionRepair") { v.v = value.template get<AspectConstructionRepair>(); }
            if (key == "Follower") { v.v = value.template get<AspectFollower>(); }
            if (key == "CollisionBase") { v.v = value.template get<AspectCollisionBase>(); }
            if (key == "EditorUniqueID") { v.v = value.template get<AspectEditorUniqueID>(); }
            if (key == "Roam") { v.v = value.template get<AspectRoam>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    //  Simplified version of how many monuments of each color player have
    struct Orbs {
        uint8_t shadow;
        uint8_t nature;
        uint8_t frost;
        uint8_t fire;
        //  Can be used instead of any color, and then changes to color of first token on the used card.
        uint8_t starting;
        //  Can be used only for colorless tokens on the card. (Curse Orb changes colored orb to white one)
        uint8_t white;
        //  Can be used as any color. Only provided by map scripts.
        uint8_t all;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Orbs, shadow, nature, frost, fire, starting, white, all);

    //  Technically it is specific case of `Entity`, but we decided to move players out,
    //  and move few fields up like position and owning player id
    struct PlayerEntity {
        //  Unique id of the entity
        EntityId id;
        //  List of effects the entity have.
        std::vector<AbilityEffect> effects;
        //  List of aspects entity have.
        std::vector<Aspect> aspects;
        uint8_t team;
        float power;
        float void_power;
        uint16_t population_count;
        std::string name;
        Orbs orbs;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerEntity, id, effects, aspects, team, power, void_power, population_count, name, orbs);

    struct MatchPlayer {
        //  Name of player.
        std::string name;
        //  Deck used by that player.
        //  TODO Due to technical difficulties might be empty.
        Deck deck;
        //  entity controlled by this player
        PlayerEntity entity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MatchPlayer, name, deck, entity);

    struct JobNoJob {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobNoJob& v) {
        j = nlohmann::json{ { "JobNoJob", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobNoJob& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobIdle {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobIdle& v) {
        j = nlohmann::json{ { "JobIdle", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobIdle& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobGoto {
        std::vector<Position2DWithOrientation> waypoints;
        std::optional<EntityId> target_entity_id;
        WalkMode walk_mode;
    };
    inline void to_json(nlohmann::json& j, const JobGoto& v) {
        j["waypoints"] = v.waypoints;
        if (v.target_entity_id.has_value()) {
            j["target_entity_id"] = v.target_entity_id;
        }
        j["walk_mode"] = v.walk_mode;
    }
    inline void from_json(const nlohmann::json& j, JobGoto& v) {
        j.at("waypoints").get_to(v.waypoints);
        if (j.count("target_entity_id") != 0) {
            j.at("target_entity_id").get_to(v.target_entity_id);
        }
        j.at("walk_mode").get_to(v.walk_mode);
    }
    struct JobAttackMelee {
        Target target;
        bool use_force_goto;
        bool no_move;
        float too_close_range;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobAttackMelee, target, use_force_goto, no_move, too_close_range);
    struct JobCastSpell {
        Target target;
        SpellId spell_id;
        bool use_force_goto;
        bool no_move;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobCastSpell, target, spell_id, use_force_goto, no_move);
    struct JobDie {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobDie& v) {
        j = nlohmann::json{ { "JobDie", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobDie& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobTalk {
        EntityId target;
        bool walk_to_target;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobTalk, target, walk_to_target);
    struct JobScriptTalk {
        bool hide_weapon;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobScriptTalk, hide_weapon);
    struct JobFreeze {
        Tick end_step;
        EntityId source;
        SpellId spell_id;
        TickCount duration;
        TickCount delay_ability;
        std::vector<AbilityId> ability_id_while_frozen;
        std::vector<AbilityId> ability_id_delayed;
        AbilityLine ability_line_id_cancel_on_start;
        bool pushback_immunity;
        uint32_t mode;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobFreeze, end_step, source, spell_id, duration, delay_ability, ability_id_while_frozen, ability_id_delayed, ability_line_id_cancel_on_start, pushback_immunity, mode);
    struct JobSpawn {
        TickCount duration;
        Tick end_step;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobSpawn, duration, end_step);
    struct JobCheer {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobCheer& v) {
        j = nlohmann::json{ { "JobCheer", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobCheer& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobAttackSquad {
        Target target;
        uint8_t weapon_type;
        float damage;
        float range_min;
        float range_max;
        std::optional<SpellId> attack_spell;
        bool use_force_goto;
        float operation_range;
        bool no_move;
        bool was_in_attack;
        bool melee_attack;
    };
    inline void to_json(nlohmann::json& j, const JobAttackSquad& v) {
        j["target"] = v.target;
        j["weapon_type"] = v.weapon_type;
        j["damage"] = v.damage;
        j["range_min"] = v.range_min;
        j["range_max"] = v.range_max;
        if (v.attack_spell.has_value()) {
            j["attack_spell"] = v.attack_spell;
        }
        j["use_force_goto"] = v.use_force_goto;
        j["operation_range"] = v.operation_range;
        j["no_move"] = v.no_move;
        j["was_in_attack"] = v.was_in_attack;
        j["melee_attack"] = v.melee_attack;
    }
    inline void from_json(const nlohmann::json& j, JobAttackSquad& v) {
        j.at("target").get_to(v.target);
        j.at("weapon_type").get_to(v.weapon_type);
        j.at("damage").get_to(v.damage);
        j.at("range_min").get_to(v.range_min);
        j.at("range_max").get_to(v.range_max);
        if (j.count("attack_spell") != 0) {
            j.at("attack_spell").get_to(v.attack_spell);
        }
        j.at("use_force_goto").get_to(v.use_force_goto);
        j.at("operation_range").get_to(v.operation_range);
        j.at("no_move").get_to(v.no_move);
        j.at("was_in_attack").get_to(v.was_in_attack);
        j.at("melee_attack").get_to(v.melee_attack);
    }
    struct JobCastSpellSquad {
        Target target;
        SpellId spell_id;
        bool use_force_goto;
        bool spell_fired;
        bool spell_per_source_entity;
        bool was_in_attack;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobCastSpellSquad, target, spell_id, use_force_goto, spell_fired, spell_per_source_entity, was_in_attack);
    struct JobPushBack {
        Position2D start_coord;
        Position2D target_coord;
        float speed;
        float rotation_speed;
        std::optional<float> damage;
        std::optional<EntityId> source;
    };
    inline void to_json(nlohmann::json& j, const JobPushBack& v) {
        j["start_coord"] = v.start_coord;
        j["target_coord"] = v.target_coord;
        j["speed"] = v.speed;
        j["rotation_speed"] = v.rotation_speed;
        if (v.damage.has_value()) {
            j["damage"] = v.damage;
        }
        if (v.source.has_value()) {
            j["source"] = v.source;
        }
    }
    inline void from_json(const nlohmann::json& j, JobPushBack& v) {
        j.at("start_coord").get_to(v.start_coord);
        j.at("target_coord").get_to(v.target_coord);
        j.at("speed").get_to(v.speed);
        j.at("rotation_speed").get_to(v.rotation_speed);
        if (j.count("damage") != 0) {
            j.at("damage").get_to(v.damage);
        }
        if (j.count("source") != 0) {
            j.at("source").get_to(v.source);
        }
    }
    struct JobStampede {
        SpellId spell;
        Target target;
        Position2D start_coord;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobStampede, spell, target, start_coord);
    struct JobBarrierCrush {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobBarrierCrush& v) {
        j = nlohmann::json{ { "JobBarrierCrush", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobBarrierCrush& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobBarrierGateToggle {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobBarrierGateToggle& v) {
        j = nlohmann::json{ { "JobBarrierGateToggle", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobBarrierGateToggle& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobFlameThrower {
        Target target;
        SpellId spell_id;
        TickCount duration_step_init;
        TickCount duration_step_shut_down;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobFlameThrower, target, spell_id, duration_step_init, duration_step_shut_down);
    struct JobConstruct {
        TickCount construction_update_steps;
        TickCount construction_update_count_remaining;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobConstruct, construction_update_steps, construction_update_count_remaining);
    struct JobCrush {
        TickCount crush_steps;
        TickCount entity_update_steps;
        TickCount remaining_crush_steps;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobCrush, crush_steps, entity_update_steps, remaining_crush_steps);
    struct JobMountBarrierSquad {
        EntityId barrier_module;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobMountBarrierSquad, barrier_module);
    struct JobMountBarrier {
        std::optional<EntityId> current_barrier_module;
        std::optional<EntityId> goal_barrier_module;
    };
    inline void to_json(nlohmann::json& j, const JobMountBarrier& v) {
        if (v.current_barrier_module.has_value()) {
            j["current_barrier_module"] = v.current_barrier_module;
        }
        if (v.goal_barrier_module.has_value()) {
            j["goal_barrier_module"] = v.goal_barrier_module;
        }
    }
    inline void from_json(const nlohmann::json& j, JobMountBarrier& v) {
        if (j.count("current_barrier_module") != 0) {
            j.at("current_barrier_module").get_to(v.current_barrier_module);
        }
        if (j.count("goal_barrier_module") != 0) {
            j.at("goal_barrier_module").get_to(v.goal_barrier_module);
        }
    }
    struct JobModeChangeSquad {
        ModeId new_mode;
        bool mode_change_done;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobModeChangeSquad, new_mode, mode_change_done);
    struct JobModeChange {
        ModeId new_mode;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobModeChange, new_mode);
    struct JobSacrificeSquad {
        EntityId target_entity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobSacrificeSquad, target_entity);
    struct JobUsePortalSquad {
        EntityId target_entity_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobUsePortalSquad, target_entity_id);
    struct JobChannel {
        std::optional<EntityId> target_squad_id;
        bool mode_target_world;
        std::optional<EntityId> entity_id;
        SpellId spell_id;
        std::optional<SpellId> spell_id_on_target_on_finish;
        std::optional<SpellId> spell_id_on_target_on_start;
        TickCount step_duration_until_finish;
        uint32_t timing_channel_start;
        uint32_t timing_channel_loop;
        uint32_t timing_channel_end;
        float abort_on_out_of_range_squared;
        bool abort_check_failed;
        bool orientate_to_target;
        TickCount orientate_to_target_max_step;
        bool abort_on_owner_get_damaged;
        bool abort_on_mode_change;
    };
    inline void to_json(nlohmann::json& j, const JobChannel& v) {
        if (v.target_squad_id.has_value()) {
            j["target_squad_id"] = v.target_squad_id;
        }
        j["mode_target_world"] = v.mode_target_world;
        if (v.entity_id.has_value()) {
            j["entity_id"] = v.entity_id;
        }
        j["spell_id"] = v.spell_id;
        if (v.spell_id_on_target_on_finish.has_value()) {
            j["spell_id_on_target_on_finish"] = v.spell_id_on_target_on_finish;
        }
        if (v.spell_id_on_target_on_start.has_value()) {
            j["spell_id_on_target_on_start"] = v.spell_id_on_target_on_start;
        }
        j["step_duration_until_finish"] = v.step_duration_until_finish;
        j["timing_channel_start"] = v.timing_channel_start;
        j["timing_channel_loop"] = v.timing_channel_loop;
        j["timing_channel_end"] = v.timing_channel_end;
        j["abort_on_out_of_range_squared"] = v.abort_on_out_of_range_squared;
        j["abort_check_failed"] = v.abort_check_failed;
        j["orientate_to_target"] = v.orientate_to_target;
        j["orientate_to_target_max_step"] = v.orientate_to_target_max_step;
        j["abort_on_owner_get_damaged"] = v.abort_on_owner_get_damaged;
        j["abort_on_mode_change"] = v.abort_on_mode_change;
    }
    inline void from_json(const nlohmann::json& j, JobChannel& v) {
        if (j.count("target_squad_id") != 0) {
            j.at("target_squad_id").get_to(v.target_squad_id);
        }
        j.at("mode_target_world").get_to(v.mode_target_world);
        if (j.count("entity_id") != 0) {
            j.at("entity_id").get_to(v.entity_id);
        }
        j.at("spell_id").get_to(v.spell_id);
        if (j.count("spell_id_on_target_on_finish") != 0) {
            j.at("spell_id_on_target_on_finish").get_to(v.spell_id_on_target_on_finish);
        }
        if (j.count("spell_id_on_target_on_start") != 0) {
            j.at("spell_id_on_target_on_start").get_to(v.spell_id_on_target_on_start);
        }
        j.at("step_duration_until_finish").get_to(v.step_duration_until_finish);
        j.at("timing_channel_start").get_to(v.timing_channel_start);
        j.at("timing_channel_loop").get_to(v.timing_channel_loop);
        j.at("timing_channel_end").get_to(v.timing_channel_end);
        j.at("abort_on_out_of_range_squared").get_to(v.abort_on_out_of_range_squared);
        j.at("abort_check_failed").get_to(v.abort_check_failed);
        j.at("orientate_to_target").get_to(v.orientate_to_target);
        j.at("orientate_to_target_max_step").get_to(v.orientate_to_target_max_step);
        j.at("abort_on_owner_get_damaged").get_to(v.abort_on_owner_get_damaged);
        j.at("abort_on_mode_change").get_to(v.abort_on_mode_change);
    }
    struct JobSpawnSquad {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const JobSpawnSquad& v) {
        j = nlohmann::json{ { "JobSpawnSquad", {} } };
    }
    inline void from_json(const nlohmann::json& j, JobSpawnSquad& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct JobLootTargetSquad {
        EntityId target_entity_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobLootTargetSquad, target_entity_id);
    struct JobMorph {
        Target target;
        SpellId spell;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobMorph, target, spell);
    //  if you see this it means we did not account for some EA's case, so please report it
    struct JobUnknown {
        uint32_t id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JobUnknown, id);

    //  With the way the game works, I would not be surprised, if this will cause more issues.
    //  If the game crashes send the log to `Kubik` it probably mean some field in
    //  one of the `Job`s needs to be `Option`.
    struct Job {
        std::variant<InvalidVariant, JobNoJob, JobIdle, JobGoto, JobAttackMelee, JobCastSpell, JobDie, JobTalk, JobScriptTalk, JobFreeze, JobSpawn, JobCheer, JobAttackSquad, JobCastSpellSquad, JobPushBack, JobStampede, JobBarrierCrush, JobBarrierGateToggle, JobFlameThrower, JobConstruct, JobCrush, JobMountBarrierSquad, JobMountBarrier, JobModeChangeSquad, JobModeChange, JobSacrificeSquad, JobUsePortalSquad, JobChannel, JobSpawnSquad, JobLootTargetSquad, JobMorph, JobUnknown> v;
    };
    inline void to_json(nlohmann::json& j, const Job& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "NoJob", std::get<JobNoJob>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "Idle", std::get<JobIdle>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "Goto", std::get<JobGoto>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "AttackMelee", std::get<JobAttackMelee>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "CastSpell", std::get<JobCastSpell>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "Die", std::get<JobDie>(v.v) } }; break;
        case 7: j = nlohmann::json{ { "Talk", std::get<JobTalk>(v.v) } }; break;
        case 8: j = nlohmann::json{ { "ScriptTalk", std::get<JobScriptTalk>(v.v) } }; break;
        case 9: j = nlohmann::json{ { "Freeze", std::get<JobFreeze>(v.v) } }; break;
        case 10: j = nlohmann::json{ { "Spawn", std::get<JobSpawn>(v.v) } }; break;
        case 11: j = nlohmann::json{ { "Cheer", std::get<JobCheer>(v.v) } }; break;
        case 12: j = nlohmann::json{ { "AttackSquad", std::get<JobAttackSquad>(v.v) } }; break;
        case 13: j = nlohmann::json{ { "CastSpellSquad", std::get<JobCastSpellSquad>(v.v) } }; break;
        case 14: j = nlohmann::json{ { "PushBack", std::get<JobPushBack>(v.v) } }; break;
        case 15: j = nlohmann::json{ { "Stampede", std::get<JobStampede>(v.v) } }; break;
        case 16: j = nlohmann::json{ { "BarrierCrush", std::get<JobBarrierCrush>(v.v) } }; break;
        case 17: j = nlohmann::json{ { "BarrierGateToggle", std::get<JobBarrierGateToggle>(v.v) } }; break;
        case 18: j = nlohmann::json{ { "FlameThrower", std::get<JobFlameThrower>(v.v) } }; break;
        case 19: j = nlohmann::json{ { "Construct", std::get<JobConstruct>(v.v) } }; break;
        case 20: j = nlohmann::json{ { "Crush", std::get<JobCrush>(v.v) } }; break;
        case 21: j = nlohmann::json{ { "MountBarrierSquad", std::get<JobMountBarrierSquad>(v.v) } }; break;
        case 22: j = nlohmann::json{ { "MountBarrier", std::get<JobMountBarrier>(v.v) } }; break;
        case 23: j = nlohmann::json{ { "ModeChangeSquad", std::get<JobModeChangeSquad>(v.v) } }; break;
        case 24: j = nlohmann::json{ { "ModeChange", std::get<JobModeChange>(v.v) } }; break;
        case 25: j = nlohmann::json{ { "SacrificeSquad", std::get<JobSacrificeSquad>(v.v) } }; break;
        case 26: j = nlohmann::json{ { "UsePortalSquad", std::get<JobUsePortalSquad>(v.v) } }; break;
        case 27: j = nlohmann::json{ { "Channel", std::get<JobChannel>(v.v) } }; break;
        case 28: j = nlohmann::json{ { "SpawnSquad", std::get<JobSpawnSquad>(v.v) } }; break;
        case 29: j = nlohmann::json{ { "LootTargetSquad", std::get<JobLootTargetSquad>(v.v) } }; break;
        case 30: j = nlohmann::json{ { "Morph", std::get<JobMorph>(v.v) } }; break;
        case 31: j = nlohmann::json{ { "Unknown", std::get<JobUnknown>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Job& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "NoJob") { v.v = value.template get<JobNoJob>(); }
            if (key == "Idle") { v.v = value.template get<JobIdle>(); }
            if (key == "Goto") { v.v = value.template get<JobGoto>(); }
            if (key == "AttackMelee") { v.v = value.template get<JobAttackMelee>(); }
            if (key == "CastSpell") { v.v = value.template get<JobCastSpell>(); }
            if (key == "Die") { v.v = value.template get<JobDie>(); }
            if (key == "Talk") { v.v = value.template get<JobTalk>(); }
            if (key == "ScriptTalk") { v.v = value.template get<JobScriptTalk>(); }
            if (key == "Freeze") { v.v = value.template get<JobFreeze>(); }
            if (key == "Spawn") { v.v = value.template get<JobSpawn>(); }
            if (key == "Cheer") { v.v = value.template get<JobCheer>(); }
            if (key == "AttackSquad") { v.v = value.template get<JobAttackSquad>(); }
            if (key == "CastSpellSquad") { v.v = value.template get<JobCastSpellSquad>(); }
            if (key == "PushBack") { v.v = value.template get<JobPushBack>(); }
            if (key == "Stampede") { v.v = value.template get<JobStampede>(); }
            if (key == "BarrierCrush") { v.v = value.template get<JobBarrierCrush>(); }
            if (key == "BarrierGateToggle") { v.v = value.template get<JobBarrierGateToggle>(); }
            if (key == "FlameThrower") { v.v = value.template get<JobFlameThrower>(); }
            if (key == "Construct") { v.v = value.template get<JobConstruct>(); }
            if (key == "Crush") { v.v = value.template get<JobCrush>(); }
            if (key == "MountBarrierSquad") { v.v = value.template get<JobMountBarrierSquad>(); }
            if (key == "MountBarrier") { v.v = value.template get<JobMountBarrier>(); }
            if (key == "ModeChangeSquad") { v.v = value.template get<JobModeChangeSquad>(); }
            if (key == "ModeChange") { v.v = value.template get<JobModeChange>(); }
            if (key == "SacrificeSquad") { v.v = value.template get<JobSacrificeSquad>(); }
            if (key == "UsePortalSquad") { v.v = value.template get<JobUsePortalSquad>(); }
            if (key == "Channel") { v.v = value.template get<JobChannel>(); }
            if (key == "SpawnSquad") { v.v = value.template get<JobSpawnSquad>(); }
            if (key == "LootTargetSquad") { v.v = value.template get<JobLootTargetSquad>(); }
            if (key == "Morph") { v.v = value.template get<JobMorph>(); }
            if (key == "Unknown") { v.v = value.template get<JobUnknown>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    enum Ping {
        Ping_Attention = 0,
        Ping_Attack = 1,
        Ping_Defend = 2,
        Ping_NeedHelp = 4,
        Ping_Meet = 5,
    };

    //  Entity on the map
    struct Entity {
        //  Unique id of the entity
        EntityId id;
        //  List of effects the entity have.
        std::vector<AbilityEffect> effects;
        //  List of aspects entity have.
        std::vector<Aspect> aspects;
        //  What is the entity doing right now
        Job job;
        //  position on the map
        Position position;
        //  id of player that owns this entity
        std::optional<EntityId> player_entity_id;
    };
    inline void to_json(nlohmann::json& j, const Entity& v) {
        j["id"] = v.id;
        j["effects"] = v.effects;
        j["aspects"] = v.aspects;
        j["job"] = v.job;
        j["position"] = v.position;
        if (v.player_entity_id.has_value()) {
            j["player_entity_id"] = v.player_entity_id;
        }
    }
    inline void from_json(const nlohmann::json& j, Entity& v) {
        j.at("id").get_to(v.id);
        j.at("effects").get_to(v.effects);
        j.at("aspects").get_to(v.aspects);
        j.at("job").get_to(v.job);
        j.at("position").get_to(v.position);
        if (j.count("player_entity_id") != 0) {
            j.at("player_entity_id").get_to(v.player_entity_id);
        }
    }

    struct Projectile {
        //  Unique id of the entity
        EntityId id;
        //  position on the map
        Position position;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Projectile, id, position);

    struct PowerSlot {
        Entity entity;
        uint32_t res_id;
        uint32_t state;
        uint8_t team;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PowerSlot, entity, res_id, state, team);

    struct TokenSlot {
        Entity entity;
        OrbColor color;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TokenSlot, entity, color);

    struct AbilityWorldObject {
        Entity entity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AbilityWorldObject, entity);

    struct Squad {
        Entity entity;
        CardId card_id;
        SquadId res_squad_id;
        float bound_power;
        uint8_t squad_size;
        //  IDs of the figures in the squad
        std::vector<EntityId> figures;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Squad, entity, card_id, res_squad_id, bound_power, squad_size, figures);

    struct Figure {
        Entity entity;
        EntityId squad_id;
        float current_speed;
        float rotation_speed;
        uint8_t unit_size;
        uint8_t move_mode;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Figure, entity, squad_id, current_speed, rotation_speed, unit_size, move_mode);

    struct Building {
        Entity entity;
        BuildingId building_id;
        CardId card_id;
        float power_cost;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Building, entity, building_id, card_id, power_cost);

    struct BarrierSet {
        Entity entity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BarrierSet, entity);

    struct BarrierModule {
        Entity entity;
        uint8_t team;
        EntityId set;
        uint32_t state;
        uint8_t slots;
        uint8_t free_slots;
        bool walkable;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BarrierModule, entity, team, set, state, slots, free_slots, walkable);

    //  Play card of building type.
    struct CommandBuildHouse {
        //  TODO will be 0 when received as command by another player
        uint8_t card_position;
        Position2D xy;
        float angle;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandBuildHouse, card_position, xy, angle);
    //  Play card of Spell type. (single target)
    struct CommandCastSpellGod {
        uint8_t card_position;
        SingleTarget target;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandCastSpellGod, card_position, target);
    //  Play card of Spell type. (line target)
    struct CommandCastSpellGodMulti {
        uint8_t card_position;
        Position2D xy1;
        Position2D xy2;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandCastSpellGodMulti, card_position, xy1, xy2);
    //  Play card of squad type (on ground)
    struct CommandProduceSquad {
        uint8_t card_position;
        Position2D xy;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandProduceSquad, card_position, xy);
    //  Play card of squad type (on barrier)
    struct CommandProduceSquadOnBarrier {
        uint8_t card_position;
        //  Squad will spawn based on this position and go to the barrier.
        Position2D xy;
        //  Squad will go to this barrier, after spawning.
        EntityId barrier_to_mount;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandProduceSquadOnBarrier, card_position, xy, barrier_to_mount);
    //  Activates spell or ability on entity.
    struct CommandCastSpellEntity {
        EntityId entity;
        SpellId spell;
        SingleTarget target;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandCastSpellEntity, entity, spell, target);
    //  Opens or closes gate.
    struct CommandBarrierGateToggle {
        EntityId barrier_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandBarrierGateToggle, barrier_id);
    //  Build barrier. (same as BarrierRepair if not inverted)
    struct CommandBarrierBuild {
        EntityId barrier_id;
        bool inverted_direction;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandBarrierBuild, barrier_id, inverted_direction);
    //  Repair barrier.
    struct CommandBarrierRepair {
        EntityId barrier_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandBarrierRepair, barrier_id);
    struct CommandBarrierCancelRepair {
        EntityId barrier_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandBarrierCancelRepair, barrier_id);
    struct CommandRepairBuilding {
        EntityId building_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandRepairBuilding, building_id);
    struct CommandCancelRepairBuilding {
        EntityId building_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandCancelRepairBuilding, building_id);
    struct CommandGroupAttack {
        std::vector<EntityId> squads;
        EntityId target_entity_id;
        bool force_attack;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupAttack, squads, target_entity_id, force_attack);
    struct CommandGroupEnterWall {
        std::vector<EntityId> squads;
        EntityId barrier_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupEnterWall, squads, barrier_id);
    struct CommandGroupExitWall {
        std::vector<EntityId> squads;
        EntityId barrier_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupExitWall, squads, barrier_id);
    struct CommandGroupGoto {
        std::vector<EntityId> squads;
        std::vector<Position2D> positions;
        WalkMode walk_mode;
        float orientation;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupGoto, squads, positions, walk_mode, orientation);
    struct CommandGroupHoldPosition {
        std::vector<EntityId> squads;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupHoldPosition, squads);
    struct CommandGroupStopJob {
        std::vector<EntityId> squads;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupStopJob, squads);
    struct CommandModeChange {
        EntityId entity_id;
        ModeId new_mode_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandModeChange, entity_id, new_mode_id);
    struct CommandPowerSlotBuild {
        EntityId slot_id;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandPowerSlotBuild, slot_id);
    struct CommandTokenSlotBuild {
        EntityId slot_id;
        CreateOrbColor color;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandTokenSlotBuild, slot_id, color);
    struct CommandGroupKillEntity {
        std::vector<EntityId> entities;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupKillEntity, entities);
    struct CommandGroupSacrifice {
        std::vector<EntityId> squads;
        EntityId target;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandGroupSacrifice, squads, target);
    struct CommandPortalDefineExitPoint {
        EntityId portal;
        Position2D xy;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandPortalDefineExitPoint, portal, xy);
    struct CommandPortalRemoveExitPoint {
        EntityId portal;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandPortalRemoveExitPoint, portal);
    struct CommandTunnelMakeExitPoint {
        EntityId portal;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandTunnelMakeExitPoint, portal);
    struct CommandPing {
        Position2D xy;
        Ping ping;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandPing, xy, ping);
    struct CommandSurrender {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandSurrender& v) {
        j = nlohmann::json{ { "CommandSurrender", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandSurrender& v) { v._just_to_make_cpp_compiler_happy = true; }
    struct CommandWhisperToMaster {
        std::string text;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandWhisperToMaster, text);

    //  All the different command bot can issue.
    //  For spectating bots all commands except Ping and WhisperToMaster are ignored
    struct Command {
        std::variant<InvalidVariant, CommandBuildHouse, CommandCastSpellGod, CommandCastSpellGodMulti, CommandProduceSquad, CommandProduceSquadOnBarrier, CommandCastSpellEntity, CommandBarrierGateToggle, CommandBarrierBuild, CommandBarrierRepair, CommandBarrierCancelRepair, CommandRepairBuilding, CommandCancelRepairBuilding, CommandGroupAttack, CommandGroupEnterWall, CommandGroupExitWall, CommandGroupGoto, CommandGroupHoldPosition, CommandGroupStopJob, CommandModeChange, CommandPowerSlotBuild, CommandTokenSlotBuild, CommandGroupKillEntity, CommandGroupSacrifice, CommandPortalDefineExitPoint, CommandPortalRemoveExitPoint, CommandTunnelMakeExitPoint, CommandPing, CommandSurrender, CommandWhisperToMaster> v;
    };
    inline void to_json(nlohmann::json& j, const Command& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "BuildHouse", std::get<CommandBuildHouse>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "CastSpellGod", std::get<CommandCastSpellGod>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "CastSpellGodMulti", std::get<CommandCastSpellGodMulti>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "ProduceSquad", std::get<CommandProduceSquad>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "ProduceSquadOnBarrier", std::get<CommandProduceSquadOnBarrier>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "CastSpellEntity", std::get<CommandCastSpellEntity>(v.v) } }; break;
        case 7: j = nlohmann::json{ { "BarrierGateToggle", std::get<CommandBarrierGateToggle>(v.v) } }; break;
        case 8: j = nlohmann::json{ { "BarrierBuild", std::get<CommandBarrierBuild>(v.v) } }; break;
        case 9: j = nlohmann::json{ { "BarrierRepair", std::get<CommandBarrierRepair>(v.v) } }; break;
        case 10: j = nlohmann::json{ { "BarrierCancelRepair", std::get<CommandBarrierCancelRepair>(v.v) } }; break;
        case 11: j = nlohmann::json{ { "RepairBuilding", std::get<CommandRepairBuilding>(v.v) } }; break;
        case 12: j = nlohmann::json{ { "CancelRepairBuilding", std::get<CommandCancelRepairBuilding>(v.v) } }; break;
        case 13: j = nlohmann::json{ { "GroupAttack", std::get<CommandGroupAttack>(v.v) } }; break;
        case 14: j = nlohmann::json{ { "GroupEnterWall", std::get<CommandGroupEnterWall>(v.v) } }; break;
        case 15: j = nlohmann::json{ { "GroupExitWall", std::get<CommandGroupExitWall>(v.v) } }; break;
        case 16: j = nlohmann::json{ { "GroupGoto", std::get<CommandGroupGoto>(v.v) } }; break;
        case 17: j = nlohmann::json{ { "GroupHoldPosition", std::get<CommandGroupHoldPosition>(v.v) } }; break;
        case 18: j = nlohmann::json{ { "GroupStopJob", std::get<CommandGroupStopJob>(v.v) } }; break;
        case 19: j = nlohmann::json{ { "ModeChange", std::get<CommandModeChange>(v.v) } }; break;
        case 20: j = nlohmann::json{ { "PowerSlotBuild", std::get<CommandPowerSlotBuild>(v.v) } }; break;
        case 21: j = nlohmann::json{ { "TokenSlotBuild", std::get<CommandTokenSlotBuild>(v.v) } }; break;
        case 22: j = nlohmann::json{ { "GroupKillEntity", std::get<CommandGroupKillEntity>(v.v) } }; break;
        case 23: j = nlohmann::json{ { "GroupSacrifice", std::get<CommandGroupSacrifice>(v.v) } }; break;
        case 24: j = nlohmann::json{ { "PortalDefineExitPoint", std::get<CommandPortalDefineExitPoint>(v.v) } }; break;
        case 25: j = nlohmann::json{ { "PortalRemoveExitPoint", std::get<CommandPortalRemoveExitPoint>(v.v) } }; break;
        case 26: j = nlohmann::json{ { "TunnelMakeExitPoint", std::get<CommandTunnelMakeExitPoint>(v.v) } }; break;
        case 27: j = nlohmann::json{ { "Ping", std::get<CommandPing>(v.v) } }; break;
        case 28: j = nlohmann::json{ { "Surrender", std::get<CommandSurrender>(v.v) } }; break;
        case 29: j = nlohmann::json{ { "WhisperToMaster", std::get<CommandWhisperToMaster>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Command& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "BuildHouse") { v.v = value.template get<CommandBuildHouse>(); }
            if (key == "CastSpellGod") { v.v = value.template get<CommandCastSpellGod>(); }
            if (key == "CastSpellGodMulti") { v.v = value.template get<CommandCastSpellGodMulti>(); }
            if (key == "ProduceSquad") { v.v = value.template get<CommandProduceSquad>(); }
            if (key == "ProduceSquadOnBarrier") { v.v = value.template get<CommandProduceSquadOnBarrier>(); }
            if (key == "CastSpellEntity") { v.v = value.template get<CommandCastSpellEntity>(); }
            if (key == "BarrierGateToggle") { v.v = value.template get<CommandBarrierGateToggle>(); }
            if (key == "BarrierBuild") { v.v = value.template get<CommandBarrierBuild>(); }
            if (key == "BarrierRepair") { v.v = value.template get<CommandBarrierRepair>(); }
            if (key == "BarrierCancelRepair") { v.v = value.template get<CommandBarrierCancelRepair>(); }
            if (key == "RepairBuilding") { v.v = value.template get<CommandRepairBuilding>(); }
            if (key == "CancelRepairBuilding") { v.v = value.template get<CommandCancelRepairBuilding>(); }
            if (key == "GroupAttack") { v.v = value.template get<CommandGroupAttack>(); }
            if (key == "GroupEnterWall") { v.v = value.template get<CommandGroupEnterWall>(); }
            if (key == "GroupExitWall") { v.v = value.template get<CommandGroupExitWall>(); }
            if (key == "GroupGoto") { v.v = value.template get<CommandGroupGoto>(); }
            if (key == "GroupHoldPosition") { v.v = value.template get<CommandGroupHoldPosition>(); }
            if (key == "GroupStopJob") { v.v = value.template get<CommandGroupStopJob>(); }
            if (key == "ModeChange") { v.v = value.template get<CommandModeChange>(); }
            if (key == "PowerSlotBuild") { v.v = value.template get<CommandPowerSlotBuild>(); }
            if (key == "TokenSlotBuild") { v.v = value.template get<CommandTokenSlotBuild>(); }
            if (key == "GroupKillEntity") { v.v = value.template get<CommandGroupKillEntity>(); }
            if (key == "GroupSacrifice") { v.v = value.template get<CommandGroupSacrifice>(); }
            if (key == "PortalDefineExitPoint") { v.v = value.template get<CommandPortalDefineExitPoint>(); }
            if (key == "PortalRemoveExitPoint") { v.v = value.template get<CommandPortalRemoveExitPoint>(); }
            if (key == "TunnelMakeExitPoint") { v.v = value.template get<CommandTunnelMakeExitPoint>(); }
            if (key == "Ping") { v.v = value.template get<CommandPing>(); }
            if (key == "Surrender") { v.v = value.template get<CommandSurrender>(); }
            if (key == "WhisperToMaster") { v.v = value.template get<CommandWhisperToMaster>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    //  Command that happen.
    struct PlayerCommand {
        EntityId player;
        Command command;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerCommand, player, command);

    enum WhyCanNotPlayCardThere {
        WhyCanNotPlayCardThere_DoesNotHaveEnoughPower = 0x10,
        //  too close to (0,y), or (x,0)
        WhyCanNotPlayCardThere_InvalidPosition = 0x20,
        WhyCanNotPlayCardThere_CardCondition = 0x80,
        WhyCanNotPlayCardThere_ConditionPreventCardPlay = 0x100,
        WhyCanNotPlayCardThere_DoesNotHaveThatCard = 0x200,
        WhyCanNotPlayCardThere_DoesNotHaveEnoughOrbs = 0x400,
        WhyCanNotPlayCardThere_CastingTooOften = 0x10000,
    };

    //  Rejection reason for `BuildHouse`, `ProduceSquad`, and `ProduceSquadOnBarrier`
    struct CommandRejectionReasonCardRejected {
        WhyCanNotPlayCardThere reason;
        std::vector<uint32_t> failed_card_conditions;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandRejectionReasonCardRejected, reason, failed_card_conditions);
    //  Player did not have enough power to play the card or activate the ability
    struct CommandRejectionReasonNotEnoughPower {
        float player_power;
        uint16_t required;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandRejectionReasonNotEnoughPower, player_power, required);
    //  Spell with given ID does not exist
    struct CommandRejectionReasonSpellDoesNotExist {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonSpellDoesNotExist& v) {
        j = nlohmann::json{ { "CommandRejectionReasonSpellDoesNotExist", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonSpellDoesNotExist& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  The entity is not on the map
    struct CommandRejectionReasonEntityDoesNotExist {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonEntityDoesNotExist& v) {
        j = nlohmann::json{ { "CommandRejectionReasonEntityDoesNotExist", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonEntityDoesNotExist& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Entity exist, but type is not correct
    struct CommandRejectionReasonInvalidEntityType {
        uint32_t entity_type;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandRejectionReasonInvalidEntityType, entity_type);
    //  Rejection reason for `CastSpellEntity`
    struct CommandRejectionReasonCanNotCast {
        std::vector<uint32_t> failed_spell_conditions;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandRejectionReasonCanNotCast, failed_spell_conditions);
    //  Bot issued command for entity that is not owned by anyone
    struct CommandRejectionReasonEntityNotOwned {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonEntityNotOwned& v) {
        j = nlohmann::json{ { "CommandRejectionReasonEntityNotOwned", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonEntityNotOwned& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Bot issued command for entity owned by someone else
    struct CommandRejectionReasonEntityOwnedBySomeoneElse {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonEntityOwnedBySomeoneElse& v) {
        j = nlohmann::json{ { "CommandRejectionReasonEntityOwnedBySomeoneElse", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonEntityOwnedBySomeoneElse& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Bot issued command for entity to change mode, but the entity does not have `ModeChange` aspect.
    struct CommandRejectionReasonNoModeChange {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonNoModeChange& v) {
        j = nlohmann::json{ { "CommandRejectionReasonNoModeChange", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonNoModeChange& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Trying to change to mode, in which the entity already is.
    struct CommandRejectionReasonEntityAlreadyInThisMode {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonEntityAlreadyInThisMode& v) {
        j = nlohmann::json{ { "CommandRejectionReasonEntityAlreadyInThisMode", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonEntityAlreadyInThisMode& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Trying to change to moe, that the entity does not have.
    struct CommandRejectionReasonModeNotExist {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonModeNotExist& v) {
        j = nlohmann::json{ { "CommandRejectionReasonModeNotExist", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonModeNotExist& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Card index must be 0-19
    struct CommandRejectionReasonInvalidCardIndex {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonInvalidCardIndex& v) {
        j = nlohmann::json{ { "CommandRejectionReasonInvalidCardIndex", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonInvalidCardIndex& v) { v._just_to_make_cpp_compiler_happy = true; }
    //  Card on the given index is invalid
    struct CommandRejectionReasonInvalidCard {
        bool _just_to_make_cpp_compiler_happy;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReasonInvalidCard& v) {
        j = nlohmann::json{ { "CommandRejectionReasonInvalidCard", {} } };
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReasonInvalidCard& v) { v._just_to_make_cpp_compiler_happy = true; }

    //  Reason why command was rejected
    struct CommandRejectionReason {
        std::variant<InvalidVariant, CommandRejectionReasonCardRejected, CommandRejectionReasonNotEnoughPower, CommandRejectionReasonSpellDoesNotExist, CommandRejectionReasonEntityDoesNotExist, CommandRejectionReasonInvalidEntityType, CommandRejectionReasonCanNotCast, CommandRejectionReasonEntityNotOwned, CommandRejectionReasonEntityOwnedBySomeoneElse, CommandRejectionReasonNoModeChange, CommandRejectionReasonEntityAlreadyInThisMode, CommandRejectionReasonModeNotExist, CommandRejectionReasonInvalidCardIndex, CommandRejectionReasonInvalidCard> v;
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReason& v) {
        switch (v.v.index()) {
        case 0:  throw std::runtime_error("Invalid default variant!");
        case 1: j = nlohmann::json{ { "CardRejected", std::get<CommandRejectionReasonCardRejected>(v.v) } }; break;
        case 2: j = nlohmann::json{ { "NotEnoughPower", std::get<CommandRejectionReasonNotEnoughPower>(v.v) } }; break;
        case 3: j = nlohmann::json{ { "SpellDoesNotExist", std::get<CommandRejectionReasonSpellDoesNotExist>(v.v) } }; break;
        case 4: j = nlohmann::json{ { "EntityDoesNotExist", std::get<CommandRejectionReasonEntityDoesNotExist>(v.v) } }; break;
        case 5: j = nlohmann::json{ { "InvalidEntityType", std::get<CommandRejectionReasonInvalidEntityType>(v.v) } }; break;
        case 6: j = nlohmann::json{ { "CanNotCast", std::get<CommandRejectionReasonCanNotCast>(v.v) } }; break;
        case 7: j = nlohmann::json{ { "EntityNotOwned", std::get<CommandRejectionReasonEntityNotOwned>(v.v) } }; break;
        case 8: j = nlohmann::json{ { "EntityOwnedBySomeoneElse", std::get<CommandRejectionReasonEntityOwnedBySomeoneElse>(v.v) } }; break;
        case 9: j = nlohmann::json{ { "NoModeChange", std::get<CommandRejectionReasonNoModeChange>(v.v) } }; break;
        case 10: j = nlohmann::json{ { "EntityAlreadyInThisMode", std::get<CommandRejectionReasonEntityAlreadyInThisMode>(v.v) } }; break;
        case 11: j = nlohmann::json{ { "ModeNotExist", std::get<CommandRejectionReasonModeNotExist>(v.v) } }; break;
        case 12: j = nlohmann::json{ { "InvalidCardIndex", std::get<CommandRejectionReasonInvalidCardIndex>(v.v) } }; break;
        case 13: j = nlohmann::json{ { "InvalidCard", std::get<CommandRejectionReasonInvalidCard>(v.v) } }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReason& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "CardRejected") { v.v = value.template get<CommandRejectionReasonCardRejected>(); }
            if (key == "NotEnoughPower") { v.v = value.template get<CommandRejectionReasonNotEnoughPower>(); }
            if (key == "SpellDoesNotExist") { v.v = value.template get<CommandRejectionReasonSpellDoesNotExist>(); }
            if (key == "EntityDoesNotExist") { v.v = value.template get<CommandRejectionReasonEntityDoesNotExist>(); }
            if (key == "InvalidEntityType") { v.v = value.template get<CommandRejectionReasonInvalidEntityType>(); }
            if (key == "CanNotCast") { v.v = value.template get<CommandRejectionReasonCanNotCast>(); }
            if (key == "EntityNotOwned") { v.v = value.template get<CommandRejectionReasonEntityNotOwned>(); }
            if (key == "EntityOwnedBySomeoneElse") { v.v = value.template get<CommandRejectionReasonEntityOwnedBySomeoneElse>(); }
            if (key == "NoModeChange") { v.v = value.template get<CommandRejectionReasonNoModeChange>(); }
            if (key == "EntityAlreadyInThisMode") { v.v = value.template get<CommandRejectionReasonEntityAlreadyInThisMode>(); }
            if (key == "ModeNotExist") { v.v = value.template get<CommandRejectionReasonModeNotExist>(); }
            if (key == "InvalidCardIndex") { v.v = value.template get<CommandRejectionReasonInvalidCardIndex>(); }
            if (key == "InvalidCard") { v.v = value.template get<CommandRejectionReasonInvalidCard>(); }
            return;
        } throw std::runtime_error("invalid json");
    }

    //  Command that was rejected.
    struct RejectedCommand {
        EntityId player;
        CommandRejectionReason reason;
        Command command;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RejectedCommand, player, reason, command);

    //  Response on the `/hello` endpoint.
    struct AiForMap {
        //  The unique name of the bot.
        std::string name;
        //  List of decks this bot can use on the map.
        //  Empty to signalize, that bot can not play on given map.
        std::vector<Deck> decks;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AiForMap, name, decks);

    struct MapEntities {
        std::vector<Projectile> projectiles;
        std::vector<PowerSlot> power_slots;
        std::vector<TokenSlot> token_slots;
        std::vector<AbilityWorldObject> ability_world_objects;
        std::vector<Squad> squads;
        std::vector<Figure> figures;
        std::vector<Building> buildings;
        std::vector<BarrierSet> barrier_sets;
        std::vector<BarrierModule> barrier_modules;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MapEntities, projectiles, power_slots, token_slots, ability_world_objects, squads, figures, buildings, barrier_sets, barrier_modules);

    //  Used in `/start` endpoint.
    struct GameStartState {
        //  Tells the bot which player it is supposed to control.
        //  If bot is only spectating, this is the ID of player that it is spectating for
        EntityId your_player_id;
        //  Players in the match.
        std::vector<MatchPlayer> players;
        MapEntities entities;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameStartState, your_player_id, players, entities);

    //  Used in `/tick` endpoint, on every tick from 2 forward.
    struct GameState {
        //  Time since start of the match measured in ticks.
        //  One tick is 0.1 second = 100 milliseconds = (10 ticks per second)
        //  Each tick is 100 ms. 1 second is 10 ticks. 1 minute is 600 ticks.
        Tick current_tick;
        //  Commands that will be executed this tick.
        std::vector<PlayerCommand> commands;
        //  Commands that was rejected.
        std::vector<RejectedCommand> rejected_commands;
        //  player entities in the match
        std::vector<PlayerEntity> players;
        MapEntities entities;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameState, current_tick, commands, rejected_commands, players, entities);

    //  Used in `/prepare` endpoint
    struct Prepare {
        //  Name of deck, selected from `AiForMap` returned by `/hello` endpoint.
        std::string deck;
        //  Repeating `map_info` in case bot want to prepare differently based on map.
        MapInfo map_info;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Prepare, deck, map_info);

    //  Used in `/hello` endpoint
    struct ApiHello {
        //  Must match the version in this file, to guarantee structures matching.
        uint64_t version;
        //  Map about which is the game asking.
        MapInfo map;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ApiHello, version, map);

}
