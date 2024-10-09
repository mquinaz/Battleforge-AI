
#pragma once

#include "CardTemplate.h"
#include "Maps.h"
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

#define C_LIKE_API

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

namespace capi {
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
    enum class SingleTargetCase {
        Invalid,
        SingleEntity,
        Location
    };
    union SingleTargetUnion {
        InvalidVariant invalid_enum_variant;
        SingleTargetSingleEntity single_entity;
        SingleTargetLocation location;
        SingleTargetUnion() { invalid_enum_variant = {}; }
        ~SingleTargetUnion() { invalid_enum_variant = {}; }
        SingleTargetUnion(const SingleTargetUnion& other) = default;
        SingleTargetUnion(SingleTargetUnion&& other) = default;
        SingleTargetUnion& operator=(const SingleTargetUnion& other) = default;
        SingleTargetUnion& operator=(SingleTargetUnion&& other) = default;
    };
    struct SingleTarget {
        SingleTargetCase variant_case;
        SingleTargetUnion variant_union;
        SingleTarget() {
            memset(this, 0, sizeof(SingleTarget));
            variant_case = SingleTargetCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        SingleTarget(const SingleTarget& other) {
            memset(this, 0, sizeof(SingleTarget));
            variant_case = other.variant_case;
            switch (variant_case) {
            case SingleTargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case SingleTargetCase::SingleEntity: variant_union.single_entity = other.variant_union.single_entity; break;
            case SingleTargetCase::Location: variant_union.location = other.variant_union.location; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        SingleTarget& operator=(const SingleTarget& other) {
            memset(this, 0, sizeof(SingleTarget));
            variant_case = other.variant_case;
            switch (variant_case) {
            case SingleTargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case SingleTargetCase::SingleEntity: variant_union.single_entity = other.variant_union.single_entity; break;
            case SingleTargetCase::Location: variant_union.location = other.variant_union.location; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        SingleTarget(SingleTargetSingleEntity v) {
            memset(this, 0, sizeof(SingleTarget));
            variant_case = SingleTargetCase::SingleEntity;
            variant_union.single_entity = v;
        }
        SingleTarget(SingleTargetLocation v) {
            memset(this, 0, sizeof(SingleTarget));
            variant_case = SingleTargetCase::Location;
            variant_union.location = v;
        }
    };
    inline void to_json(nlohmann::json& j, const SingleTarget& v) {
        switch (v.variant_case) {
        case SingleTargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case SingleTargetCase::SingleEntity: j = nlohmann::json{ { "SingleEntity", v.variant_union.single_entity} }; break;
        case SingleTargetCase::Location: j = nlohmann::json{ { "Location", v.variant_union.location} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, SingleTarget& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "SingleEntity") { v.variant_union.single_entity = value.template get<SingleTargetSingleEntity>(); v.variant_case = SingleTargetCase::SingleEntity; }
            if (key == "Location") { v.variant_union.location = value.template get<SingleTargetLocation>(); v.variant_case = SingleTargetCase::Location; }
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

    enum class TargetCase {
        Invalid,
        Single,
        Multi
    };
    union TargetUnion {
        InvalidVariant invalid_enum_variant;
        TargetSingle single;
        TargetMulti multi;
        TargetUnion() { invalid_enum_variant = {}; }
        ~TargetUnion() { invalid_enum_variant = {}; }
        TargetUnion(const TargetUnion& other) = default;
        TargetUnion(TargetUnion&& other) = default;
        TargetUnion& operator=(const TargetUnion& other) = default;
        TargetUnion& operator=(TargetUnion&& other) = default;
    };
    struct Target {
        TargetCase variant_case;
        TargetUnion variant_union;
        Target() {
            memset(this, 0, sizeof(Target));
            variant_case = TargetCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        Target(const Target& other) {
            memset(this, 0, sizeof(Target));
            variant_case = other.variant_case;
            switch (variant_case) {
            case TargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case TargetCase::Single: variant_union.single = other.variant_union.single; break;
            case TargetCase::Multi: variant_union.multi = other.variant_union.multi; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Target& operator=(const Target& other) {
            memset(this, 0, sizeof(Target));
            variant_case = other.variant_case;
            switch (variant_case) {
            case TargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case TargetCase::Single: variant_union.single = other.variant_union.single; break;
            case TargetCase::Multi: variant_union.multi = other.variant_union.multi; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Target(TargetSingle v) {
            memset(this, 0, sizeof(Target));
            variant_case = TargetCase::Single;
            variant_union.single = v;
        }
        Target(TargetMulti v) {
            memset(this, 0, sizeof(Target));
            variant_case = TargetCase::Multi;
            variant_union.multi = v;
        }
    };
    inline void to_json(nlohmann::json& j, const Target& v) {
        switch (v.variant_case) {
        case TargetCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case TargetCase::Single: j = nlohmann::json{ { "Single", v.variant_union.single} }; break;
        case TargetCase::Multi: j = nlohmann::json{ { "Multi", v.variant_union.multi} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Target& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Single") { v.variant_union.single = value.template get<TargetSingle>(); v.variant_case = TargetCase::Single; }
            if (key == "Multi") { v.variant_union.multi = value.template get<TargetMulti>(); v.variant_case = TargetCase::Multi; }
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

    enum class AreaShapeCase {
        Invalid,
        Circle,
        Cone,
        ConeCut,
        WideLine
    };
    union AreaShapeUnion {
        InvalidVariant invalid_enum_variant;
        AreaShapeCircle circle;
        AreaShapeCone cone;
        AreaShapeConeCut cone_cut;
        AreaShapeWideLine wide_line;
        AreaShapeUnion() { invalid_enum_variant = {}; }
        ~AreaShapeUnion() { invalid_enum_variant = {}; }
        AreaShapeUnion(const AreaShapeUnion& other) = default;
        AreaShapeUnion(AreaShapeUnion&& other) = default;
        AreaShapeUnion& operator=(const AreaShapeUnion& other) = default;
        AreaShapeUnion& operator=(AreaShapeUnion&& other) = default;
    };
    struct AreaShape {
        AreaShapeCase variant_case;
        AreaShapeUnion variant_union;
        AreaShape() {
            memset(this, 0, sizeof(AreaShape));
            variant_case = AreaShapeCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        AreaShape(const AreaShape& other) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AreaShapeCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AreaShapeCase::Circle: variant_union.circle = other.variant_union.circle; break;
            case AreaShapeCase::Cone: variant_union.cone = other.variant_union.cone; break;
            case AreaShapeCase::ConeCut: variant_union.cone_cut = other.variant_union.cone_cut; break;
            case AreaShapeCase::WideLine: variant_union.wide_line = other.variant_union.wide_line; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        AreaShape& operator=(const AreaShape& other) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AreaShapeCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AreaShapeCase::Circle: variant_union.circle = other.variant_union.circle; break;
            case AreaShapeCase::Cone: variant_union.cone = other.variant_union.cone; break;
            case AreaShapeCase::ConeCut: variant_union.cone_cut = other.variant_union.cone_cut; break;
            case AreaShapeCase::WideLine: variant_union.wide_line = other.variant_union.wide_line; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        AreaShape(AreaShapeCircle v) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = AreaShapeCase::Circle;
            variant_union.circle = v;
        }
        AreaShape(AreaShapeCone v) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = AreaShapeCase::Cone;
            variant_union.cone = v;
        }
        AreaShape(AreaShapeConeCut v) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = AreaShapeCase::ConeCut;
            variant_union.cone_cut = v;
        }
        AreaShape(AreaShapeWideLine v) {
            memset(this, 0, sizeof(AreaShape));
            variant_case = AreaShapeCase::WideLine;
            variant_union.wide_line = v;
        }
    };
    inline void to_json(nlohmann::json& j, const AreaShape& v) {
        switch (v.variant_case) {
        case AreaShapeCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case AreaShapeCase::Circle: j = nlohmann::json{ { "Circle", v.variant_union.circle} }; break;
        case AreaShapeCase::Cone: j = nlohmann::json{ { "Cone", v.variant_union.cone} }; break;
        case AreaShapeCase::ConeCut: j = nlohmann::json{ { "ConeCut", v.variant_union.cone_cut} }; break;
        case AreaShapeCase::WideLine: j = nlohmann::json{ { "WideLine", v.variant_union.wide_line} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, AreaShape& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Circle") { v.variant_union.circle = value.template get<AreaShapeCircle>(); v.variant_case = AreaShapeCase::Circle; }
            if (key == "Cone") { v.variant_union.cone = value.template get<AreaShapeCone>(); v.variant_case = AreaShapeCase::Cone; }
            if (key == "ConeCut") { v.variant_union.cone_cut = value.template get<AreaShapeConeCut>(); v.variant_case = AreaShapeCase::ConeCut; }
            if (key == "WideLine") { v.variant_union.wide_line = value.template get<AreaShapeWideLine>(); v.variant_case = AreaShapeCase::WideLine; }
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

    enum class AbilityEffectSpecificCase {
        Invalid,
        DamageArea,
        DamageOverTime,
        LinkedFire,
        SpellOnEntityNearby,
        TimedSpell,
        Collector,
        Aura,
        MovingIntervalCast,
        Other
    };
    union AbilityEffectSpecificUnion {
        InvalidVariant invalid_enum_variant;
        AbilityEffectSpecificDamageArea damage_area;
        AbilityEffectSpecificDamageOverTime damage_over_time;
        AbilityEffectSpecificLinkedFire linked_fire;
        AbilityEffectSpecificSpellOnEntityNearby spell_on_entity_nearby;
        AbilityEffectSpecificTimedSpell timed_spell;
        AbilityEffectSpecificCollector collector;
        AbilityEffectSpecificAura aura;
        AbilityEffectSpecificMovingIntervalCast moving_interval_cast;
        AbilityEffectSpecificOther other;
        AbilityEffectSpecificUnion() { invalid_enum_variant = {}; }
        ~AbilityEffectSpecificUnion() { invalid_enum_variant = {}; }
        AbilityEffectSpecificUnion(const AbilityEffectSpecificUnion& other) = default;
        AbilityEffectSpecificUnion(AbilityEffectSpecificUnion&& other) = default;
        AbilityEffectSpecificUnion& operator=(const AbilityEffectSpecificUnion& other) = default;
        AbilityEffectSpecificUnion& operator=(AbilityEffectSpecificUnion&& other) = default;
    };
    struct AbilityEffectSpecific {
        AbilityEffectSpecificCase variant_case;
        AbilityEffectSpecificUnion variant_union;
        AbilityEffectSpecific() {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        AbilityEffectSpecific(const AbilityEffectSpecific& other) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AbilityEffectSpecificCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AbilityEffectSpecificCase::DamageArea: variant_union.damage_area = other.variant_union.damage_area; break;
            case AbilityEffectSpecificCase::DamageOverTime: variant_union.damage_over_time = other.variant_union.damage_over_time; break;
            case AbilityEffectSpecificCase::LinkedFire: variant_union.linked_fire = other.variant_union.linked_fire; break;
            case AbilityEffectSpecificCase::SpellOnEntityNearby: variant_union.spell_on_entity_nearby = other.variant_union.spell_on_entity_nearby; break;
            case AbilityEffectSpecificCase::TimedSpell: variant_union.timed_spell = other.variant_union.timed_spell; break;
            case AbilityEffectSpecificCase::Collector: variant_union.collector = other.variant_union.collector; break;
            case AbilityEffectSpecificCase::Aura: variant_union.aura = other.variant_union.aura; break;
            case AbilityEffectSpecificCase::MovingIntervalCast: variant_union.moving_interval_cast = other.variant_union.moving_interval_cast; break;
            case AbilityEffectSpecificCase::Other: variant_union.other = other.variant_union.other; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        AbilityEffectSpecific& operator=(const AbilityEffectSpecific& other) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AbilityEffectSpecificCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AbilityEffectSpecificCase::DamageArea: variant_union.damage_area = other.variant_union.damage_area; break;
            case AbilityEffectSpecificCase::DamageOverTime: variant_union.damage_over_time = other.variant_union.damage_over_time; break;
            case AbilityEffectSpecificCase::LinkedFire: variant_union.linked_fire = other.variant_union.linked_fire; break;
            case AbilityEffectSpecificCase::SpellOnEntityNearby: variant_union.spell_on_entity_nearby = other.variant_union.spell_on_entity_nearby; break;
            case AbilityEffectSpecificCase::TimedSpell: variant_union.timed_spell = other.variant_union.timed_spell; break;
            case AbilityEffectSpecificCase::Collector: variant_union.collector = other.variant_union.collector; break;
            case AbilityEffectSpecificCase::Aura: variant_union.aura = other.variant_union.aura; break;
            case AbilityEffectSpecificCase::MovingIntervalCast: variant_union.moving_interval_cast = other.variant_union.moving_interval_cast; break;
            case AbilityEffectSpecificCase::Other: variant_union.other = other.variant_union.other; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        AbilityEffectSpecific(AbilityEffectSpecificDamageArea v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::DamageArea;
            variant_union.damage_area = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificDamageOverTime v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::DamageOverTime;
            variant_union.damage_over_time = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificLinkedFire v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::LinkedFire;
            variant_union.linked_fire = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificSpellOnEntityNearby v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::SpellOnEntityNearby;
            variant_union.spell_on_entity_nearby = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificTimedSpell v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::TimedSpell;
            variant_union.timed_spell = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificCollector v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::Collector;
            variant_union.collector = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificAura v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::Aura;
            variant_union.aura = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificMovingIntervalCast v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::MovingIntervalCast;
            variant_union.moving_interval_cast = v;
        }
        AbilityEffectSpecific(AbilityEffectSpecificOther v) {
            memset(this, 0, sizeof(AbilityEffectSpecific));
            variant_case = AbilityEffectSpecificCase::Other;
            variant_union.other = v;
        }
    };
    inline void to_json(nlohmann::json& j, const AbilityEffectSpecific& v) {
        switch (v.variant_case) {
        case AbilityEffectSpecificCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case AbilityEffectSpecificCase::DamageArea: j = nlohmann::json{ { "DamageArea", v.variant_union.damage_area} }; break;
        case AbilityEffectSpecificCase::DamageOverTime: j = nlohmann::json{ { "DamageOverTime", v.variant_union.damage_over_time} }; break;
        case AbilityEffectSpecificCase::LinkedFire: j = nlohmann::json{ { "LinkedFire", v.variant_union.linked_fire} }; break;
        case AbilityEffectSpecificCase::SpellOnEntityNearby: j = nlohmann::json{ { "SpellOnEntityNearby", v.variant_union.spell_on_entity_nearby} }; break;
        case AbilityEffectSpecificCase::TimedSpell: j = nlohmann::json{ { "TimedSpell", v.variant_union.timed_spell} }; break;
        case AbilityEffectSpecificCase::Collector: j = nlohmann::json{ { "Collector", v.variant_union.collector} }; break;
        case AbilityEffectSpecificCase::Aura: j = nlohmann::json{ { "Aura", v.variant_union.aura} }; break;
        case AbilityEffectSpecificCase::MovingIntervalCast: j = nlohmann::json{ { "MovingIntervalCast", v.variant_union.moving_interval_cast} }; break;
        case AbilityEffectSpecificCase::Other: j = nlohmann::json{ { "Other", v.variant_union.other} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, AbilityEffectSpecific& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "DamageArea") { v.variant_union.damage_area = value.template get<AbilityEffectSpecificDamageArea>(); v.variant_case = AbilityEffectSpecificCase::DamageArea; }
            if (key == "DamageOverTime") { v.variant_union.damage_over_time = value.template get<AbilityEffectSpecificDamageOverTime>(); v.variant_case = AbilityEffectSpecificCase::DamageOverTime; }
            if (key == "LinkedFire") { v.variant_union.linked_fire = value.template get<AbilityEffectSpecificLinkedFire>(); v.variant_case = AbilityEffectSpecificCase::LinkedFire; }
            if (key == "SpellOnEntityNearby") { v.variant_union.spell_on_entity_nearby = value.template get<AbilityEffectSpecificSpellOnEntityNearby>(); v.variant_case = AbilityEffectSpecificCase::SpellOnEntityNearby; }
            if (key == "TimedSpell") { v.variant_union.timed_spell = value.template get<AbilityEffectSpecificTimedSpell>(); v.variant_case = AbilityEffectSpecificCase::TimedSpell; }
            if (key == "Collector") { v.variant_union.collector = value.template get<AbilityEffectSpecificCollector>(); v.variant_case = AbilityEffectSpecificCase::Collector; }
            if (key == "Aura") { v.variant_union.aura = value.template get<AbilityEffectSpecificAura>(); v.variant_case = AbilityEffectSpecificCase::Aura; }
            if (key == "MovingIntervalCast") { v.variant_union.moving_interval_cast = value.template get<AbilityEffectSpecificMovingIntervalCast>(); v.variant_case = AbilityEffectSpecificCase::MovingIntervalCast; }
            if (key == "Other") { v.variant_union.other = value.template get<AbilityEffectSpecificOther>(); v.variant_case = AbilityEffectSpecificCase::Other; }
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
    enum class MountStateCase {
        Invalid,
        Unmounted,
        MountingSquad,
        MountingFigure,
        MountedSquad,
        MountedFigure,
        Unknown
    };
    union MountStateUnion {
        InvalidVariant invalid_enum_variant;
        MountStateUnmounted unmounted;
        MountStateMountingSquad mounting_squad;
        MountStateMountingFigure mounting_figure;
        MountStateMountedSquad mounted_squad;
        MountStateMountedFigure mounted_figure;
        MountStateUnknown unknown;
        MountStateUnion() { invalid_enum_variant = {}; }
        ~MountStateUnion() { invalid_enum_variant = {}; }
        MountStateUnion(const MountStateUnion& other) = default;
        MountStateUnion(MountStateUnion&& other) = default;
        MountStateUnion& operator=(const MountStateUnion& other) = default;
        MountStateUnion& operator=(MountStateUnion&& other) = default;
    };
    struct MountState {
        MountStateCase variant_case;
        MountStateUnion variant_union;
        MountState() {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        MountState(const MountState& other) {
            memset(this, 0, sizeof(MountState));
            variant_case = other.variant_case;
            switch (variant_case) {
            case MountStateCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case MountStateCase::Unmounted: variant_union.unmounted = other.variant_union.unmounted; break;
            case MountStateCase::MountingSquad: variant_union.mounting_squad = other.variant_union.mounting_squad; break;
            case MountStateCase::MountingFigure: variant_union.mounting_figure = other.variant_union.mounting_figure; break;
            case MountStateCase::MountedSquad: variant_union.mounted_squad = other.variant_union.mounted_squad; break;
            case MountStateCase::MountedFigure: variant_union.mounted_figure = other.variant_union.mounted_figure; break;
            case MountStateCase::Unknown: variant_union.unknown = other.variant_union.unknown; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        MountState& operator=(const MountState& other) {
            memset(this, 0, sizeof(MountState));
            variant_case = other.variant_case;
            switch (variant_case) {
            case MountStateCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case MountStateCase::Unmounted: variant_union.unmounted = other.variant_union.unmounted; break;
            case MountStateCase::MountingSquad: variant_union.mounting_squad = other.variant_union.mounting_squad; break;
            case MountStateCase::MountingFigure: variant_union.mounting_figure = other.variant_union.mounting_figure; break;
            case MountStateCase::MountedSquad: variant_union.mounted_squad = other.variant_union.mounted_squad; break;
            case MountStateCase::MountedFigure: variant_union.mounted_figure = other.variant_union.mounted_figure; break;
            case MountStateCase::Unknown: variant_union.unknown = other.variant_union.unknown; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        MountState(MountStateUnmounted v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::Unmounted;
            variant_union.unmounted = v;
        }
        MountState(MountStateMountingSquad v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::MountingSquad;
            variant_union.mounting_squad = v;
        }
        MountState(MountStateMountingFigure v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::MountingFigure;
            variant_union.mounting_figure = v;
        }
        MountState(MountStateMountedSquad v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::MountedSquad;
            variant_union.mounted_squad = v;
        }
        MountState(MountStateMountedFigure v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::MountedFigure;
            variant_union.mounted_figure = v;
        }
        MountState(MountStateUnknown v) {
            memset(this, 0, sizeof(MountState));
            variant_case = MountStateCase::Unknown;
            variant_union.unknown = v;
        }
    };
    inline void to_json(nlohmann::json& j, const MountState& v) {
        switch (v.variant_case) {
        case MountStateCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case MountStateCase::Unmounted: j = nlohmann::json{ { "Unmounted", v.variant_union.unmounted} }; break;
        case MountStateCase::MountingSquad: j = nlohmann::json{ { "MountingSquad", v.variant_union.mounting_squad} }; break;
        case MountStateCase::MountingFigure: j = nlohmann::json{ { "MountingFigure", v.variant_union.mounting_figure} }; break;
        case MountStateCase::MountedSquad: j = nlohmann::json{ { "MountedSquad", v.variant_union.mounted_squad} }; break;
        case MountStateCase::MountedFigure: j = nlohmann::json{ { "MountedFigure", v.variant_union.mounted_figure} }; break;
        case MountStateCase::Unknown: j = nlohmann::json{ { "Unknown", v.variant_union.unknown} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, MountState& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "Unmounted") { v.variant_union.unmounted = value.template get<MountStateUnmounted>(); v.variant_case = MountStateCase::Unmounted; }
            if (key == "MountingSquad") { v.variant_union.mounting_squad = value.template get<MountStateMountingSquad>(); v.variant_case = MountStateCase::MountingSquad; }
            if (key == "MountingFigure") { v.variant_union.mounting_figure = value.template get<MountStateMountingFigure>(); v.variant_case = MountStateCase::MountingFigure; }
            if (key == "MountedSquad") { v.variant_union.mounted_squad = value.template get<MountStateMountedSquad>(); v.variant_case = MountStateCase::MountedSquad; }
            if (key == "MountedFigure") { v.variant_union.mounted_figure = value.template get<MountStateMountedFigure>(); v.variant_case = MountStateCase::MountedFigure; }
            if (key == "Unknown") { v.variant_union.unknown = value.template get<MountStateUnknown>(); v.variant_case = MountStateCase::Unknown; }
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
    enum class AspectCase {
        Invalid,
        PowerProduction,
        Health,
        Combat,
        ModeChange,
        Ammunition,
        SuperWeaponShadow,
        WormMovement,
        NPCTag,
        PlayerKit,
        Loot,
        Immunity,
        Turret,
        Tunnel,
        MountBarrier,
        SpellMemory,
        Portal,
        Hate,
        BarrierGate,
        Attackable,
        SquadRefill,
        PortalExit,
        ConstructionData,
        SuperWeaponShadowBomb,
        RepairBarrierSet,
        ConstructionRepair,
        Follower,
        CollisionBase,
        EditorUniqueID,
        Roam
    };
    union AspectUnion {
        InvalidVariant invalid_enum_variant;
        AspectPowerProduction power_production;
        AspectHealth health;
        AspectCombat combat;
        AspectModeChange mode_change;
        AspectAmmunition ammunition;
        AspectSuperWeaponShadow super_weapon_shadow;
        AspectWormMovement worm_movement;
        AspectNPCTag npc_tag;
        AspectPlayerKit player_kit;
        AspectLoot loot;
        AspectImmunity immunity;
        AspectTurret turret;
        AspectTunnel tunnel;
        AspectMountBarrier mount_barrier;
        AspectSpellMemory spell_memory;
        AspectPortal portal;
        AspectHate hate;
        AspectBarrierGate barrier_gate;
        AspectAttackable attackable;
        AspectSquadRefill squad_refill;
        AspectPortalExit portal_exit;
        AspectConstructionData construction_data;
        AspectSuperWeaponShadowBomb super_weapon_shadow_bomb;
        AspectRepairBarrierSet repair_barrier_set;
        AspectConstructionRepair construction_repair;
        AspectFollower follower;
        AspectCollisionBase collision_base;
        AspectEditorUniqueID editor_unique_id;
        AspectRoam roam;
        AspectUnion() { invalid_enum_variant = {}; }
        ~AspectUnion() { invalid_enum_variant = {}; }
        AspectUnion(const AspectUnion& other) = default;
        AspectUnion(AspectUnion&& other) = default;
        AspectUnion& operator=(const AspectUnion& other) = default;
        AspectUnion& operator=(AspectUnion&& other) = default;
    };
    struct Aspect {
        AspectCase variant_case;
        AspectUnion variant_union;
        Aspect() {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        Aspect(const Aspect& other) {
            memset(this, 0, sizeof(Aspect));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AspectCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AspectCase::PowerProduction: variant_union.power_production = other.variant_union.power_production; break;
            case AspectCase::Health: variant_union.health = other.variant_union.health; break;
            case AspectCase::Combat: variant_union.combat = other.variant_union.combat; break;
            case AspectCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case AspectCase::Ammunition: variant_union.ammunition = other.variant_union.ammunition; break;
            case AspectCase::SuperWeaponShadow: variant_union.super_weapon_shadow = other.variant_union.super_weapon_shadow; break;
            case AspectCase::WormMovement: variant_union.worm_movement = other.variant_union.worm_movement; break;
            case AspectCase::NPCTag: variant_union.npc_tag = other.variant_union.npc_tag; break;
            case AspectCase::PlayerKit: variant_union.player_kit = other.variant_union.player_kit; break;
            case AspectCase::Loot: variant_union.loot = other.variant_union.loot; break;
            case AspectCase::Immunity: variant_union.immunity = other.variant_union.immunity; break;
            case AspectCase::Turret: variant_union.turret = other.variant_union.turret; break;
            case AspectCase::Tunnel: variant_union.tunnel = other.variant_union.tunnel; break;
            case AspectCase::MountBarrier: variant_union.mount_barrier = other.variant_union.mount_barrier; break;
            case AspectCase::SpellMemory: variant_union.spell_memory = other.variant_union.spell_memory; break;
            case AspectCase::Portal: variant_union.portal = other.variant_union.portal; break;
            case AspectCase::Hate: variant_union.hate = other.variant_union.hate; break;
            case AspectCase::BarrierGate: variant_union.barrier_gate = other.variant_union.barrier_gate; break;
            case AspectCase::Attackable: variant_union.attackable = other.variant_union.attackable; break;
            case AspectCase::SquadRefill: variant_union.squad_refill = other.variant_union.squad_refill; break;
            case AspectCase::PortalExit: variant_union.portal_exit = other.variant_union.portal_exit; break;
            case AspectCase::ConstructionData: variant_union.construction_data = other.variant_union.construction_data; break;
            case AspectCase::SuperWeaponShadowBomb: variant_union.super_weapon_shadow_bomb = other.variant_union.super_weapon_shadow_bomb; break;
            case AspectCase::RepairBarrierSet: variant_union.repair_barrier_set = other.variant_union.repair_barrier_set; break;
            case AspectCase::ConstructionRepair: variant_union.construction_repair = other.variant_union.construction_repair; break;
            case AspectCase::Follower: variant_union.follower = other.variant_union.follower; break;
            case AspectCase::CollisionBase: variant_union.collision_base = other.variant_union.collision_base; break;
            case AspectCase::EditorUniqueID: variant_union.editor_unique_id = other.variant_union.editor_unique_id; break;
            case AspectCase::Roam: variant_union.roam = other.variant_union.roam; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Aspect& operator=(const Aspect& other) {
            memset(this, 0, sizeof(Aspect));
            variant_case = other.variant_case;
            switch (variant_case) {
            case AspectCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case AspectCase::PowerProduction: variant_union.power_production = other.variant_union.power_production; break;
            case AspectCase::Health: variant_union.health = other.variant_union.health; break;
            case AspectCase::Combat: variant_union.combat = other.variant_union.combat; break;
            case AspectCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case AspectCase::Ammunition: variant_union.ammunition = other.variant_union.ammunition; break;
            case AspectCase::SuperWeaponShadow: variant_union.super_weapon_shadow = other.variant_union.super_weapon_shadow; break;
            case AspectCase::WormMovement: variant_union.worm_movement = other.variant_union.worm_movement; break;
            case AspectCase::NPCTag: variant_union.npc_tag = other.variant_union.npc_tag; break;
            case AspectCase::PlayerKit: variant_union.player_kit = other.variant_union.player_kit; break;
            case AspectCase::Loot: variant_union.loot = other.variant_union.loot; break;
            case AspectCase::Immunity: variant_union.immunity = other.variant_union.immunity; break;
            case AspectCase::Turret: variant_union.turret = other.variant_union.turret; break;
            case AspectCase::Tunnel: variant_union.tunnel = other.variant_union.tunnel; break;
            case AspectCase::MountBarrier: variant_union.mount_barrier = other.variant_union.mount_barrier; break;
            case AspectCase::SpellMemory: variant_union.spell_memory = other.variant_union.spell_memory; break;
            case AspectCase::Portal: variant_union.portal = other.variant_union.portal; break;
            case AspectCase::Hate: variant_union.hate = other.variant_union.hate; break;
            case AspectCase::BarrierGate: variant_union.barrier_gate = other.variant_union.barrier_gate; break;
            case AspectCase::Attackable: variant_union.attackable = other.variant_union.attackable; break;
            case AspectCase::SquadRefill: variant_union.squad_refill = other.variant_union.squad_refill; break;
            case AspectCase::PortalExit: variant_union.portal_exit = other.variant_union.portal_exit; break;
            case AspectCase::ConstructionData: variant_union.construction_data = other.variant_union.construction_data; break;
            case AspectCase::SuperWeaponShadowBomb: variant_union.super_weapon_shadow_bomb = other.variant_union.super_weapon_shadow_bomb; break;
            case AspectCase::RepairBarrierSet: variant_union.repair_barrier_set = other.variant_union.repair_barrier_set; break;
            case AspectCase::ConstructionRepair: variant_union.construction_repair = other.variant_union.construction_repair; break;
            case AspectCase::Follower: variant_union.follower = other.variant_union.follower; break;
            case AspectCase::CollisionBase: variant_union.collision_base = other.variant_union.collision_base; break;
            case AspectCase::EditorUniqueID: variant_union.editor_unique_id = other.variant_union.editor_unique_id; break;
            case AspectCase::Roam: variant_union.roam = other.variant_union.roam; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Aspect(AspectPowerProduction v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::PowerProduction;
            variant_union.power_production = v;
        }
        Aspect(AspectHealth v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Health;
            variant_union.health = v;
        }
        Aspect(AspectCombat v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Combat;
            variant_union.combat = v;
        }
        Aspect(AspectModeChange v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::ModeChange;
            variant_union.mode_change = v;
        }
        Aspect(AspectAmmunition v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Ammunition;
            variant_union.ammunition = v;
        }
        Aspect(AspectSuperWeaponShadow v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::SuperWeaponShadow;
            variant_union.super_weapon_shadow = v;
        }
        Aspect(AspectWormMovement v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::WormMovement;
            variant_union.worm_movement = v;
        }
        Aspect(AspectNPCTag v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::NPCTag;
            variant_union.npc_tag = v;
        }
        Aspect(AspectPlayerKit v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::PlayerKit;
            variant_union.player_kit = v;
        }
        Aspect(AspectLoot v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Loot;
            variant_union.loot = v;
        }
        Aspect(AspectImmunity v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Immunity;
            variant_union.immunity = v;
        }
        Aspect(AspectTurret v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Turret;
            variant_union.turret = v;
        }
        Aspect(AspectTunnel v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Tunnel;
            variant_union.tunnel = v;
        }
        Aspect(AspectMountBarrier v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::MountBarrier;
            variant_union.mount_barrier = v;
        }
        Aspect(AspectSpellMemory v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::SpellMemory;
            variant_union.spell_memory = v;
        }
        Aspect(AspectPortal v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Portal;
            variant_union.portal = v;
        }
        Aspect(AspectHate v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Hate;
            variant_union.hate = v;
        }
        Aspect(AspectBarrierGate v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::BarrierGate;
            variant_union.barrier_gate = v;
        }
        Aspect(AspectAttackable v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Attackable;
            variant_union.attackable = v;
        }
        Aspect(AspectSquadRefill v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::SquadRefill;
            variant_union.squad_refill = v;
        }
        Aspect(AspectPortalExit v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::PortalExit;
            variant_union.portal_exit = v;
        }
        Aspect(AspectConstructionData v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::ConstructionData;
            variant_union.construction_data = v;
        }
        Aspect(AspectSuperWeaponShadowBomb v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::SuperWeaponShadowBomb;
            variant_union.super_weapon_shadow_bomb = v;
        }
        Aspect(AspectRepairBarrierSet v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::RepairBarrierSet;
            variant_union.repair_barrier_set = v;
        }
        Aspect(AspectConstructionRepair v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::ConstructionRepair;
            variant_union.construction_repair = v;
        }
        Aspect(AspectFollower v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Follower;
            variant_union.follower = v;
        }
        Aspect(AspectCollisionBase v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::CollisionBase;
            variant_union.collision_base = v;
        }
        Aspect(AspectEditorUniqueID v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::EditorUniqueID;
            variant_union.editor_unique_id = v;
        }
        Aspect(AspectRoam v) {
            memset(this, 0, sizeof(Aspect));
            variant_case = AspectCase::Roam;
            variant_union.roam = v;
        }
    };
    inline void to_json(nlohmann::json& j, const Aspect& v) {
        switch (v.variant_case) {
        case AspectCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case AspectCase::PowerProduction: j = nlohmann::json{ { "PowerProduction", v.variant_union.power_production} }; break;
        case AspectCase::Health: j = nlohmann::json{ { "Health", v.variant_union.health} }; break;
        case AspectCase::Combat: j = nlohmann::json{ { "Combat", v.variant_union.combat} }; break;
        case AspectCase::ModeChange: j = nlohmann::json{ { "ModeChange", v.variant_union.mode_change} }; break;
        case AspectCase::Ammunition: j = nlohmann::json{ { "Ammunition", v.variant_union.ammunition} }; break;
        case AspectCase::SuperWeaponShadow: j = nlohmann::json{ { "SuperWeaponShadow", v.variant_union.super_weapon_shadow} }; break;
        case AspectCase::WormMovement: j = nlohmann::json{ { "WormMovement", v.variant_union.worm_movement} }; break;
        case AspectCase::NPCTag: j = nlohmann::json{ { "NPCTag", v.variant_union.npc_tag} }; break;
        case AspectCase::PlayerKit: j = nlohmann::json{ { "PlayerKit", v.variant_union.player_kit} }; break;
        case AspectCase::Loot: j = nlohmann::json{ { "Loot", v.variant_union.loot} }; break;
        case AspectCase::Immunity: j = nlohmann::json{ { "Immunity", v.variant_union.immunity} }; break;
        case AspectCase::Turret: j = nlohmann::json{ { "Turret", v.variant_union.turret} }; break;
        case AspectCase::Tunnel: j = nlohmann::json{ { "Tunnel", v.variant_union.tunnel} }; break;
        case AspectCase::MountBarrier: j = nlohmann::json{ { "MountBarrier", v.variant_union.mount_barrier} }; break;
        case AspectCase::SpellMemory: j = nlohmann::json{ { "SpellMemory", v.variant_union.spell_memory} }; break;
        case AspectCase::Portal: j = nlohmann::json{ { "Portal", v.variant_union.portal} }; break;
        case AspectCase::Hate: j = nlohmann::json{ { "Hate", v.variant_union.hate} }; break;
        case AspectCase::BarrierGate: j = nlohmann::json{ { "BarrierGate", v.variant_union.barrier_gate} }; break;
        case AspectCase::Attackable: j = nlohmann::json{ { "Attackable", v.variant_union.attackable} }; break;
        case AspectCase::SquadRefill: j = nlohmann::json{ { "SquadRefill", v.variant_union.squad_refill} }; break;
        case AspectCase::PortalExit: j = nlohmann::json{ { "PortalExit", v.variant_union.portal_exit} }; break;
        case AspectCase::ConstructionData: j = nlohmann::json{ { "ConstructionData", v.variant_union.construction_data} }; break;
        case AspectCase::SuperWeaponShadowBomb: j = nlohmann::json{ { "SuperWeaponShadowBomb", v.variant_union.super_weapon_shadow_bomb} }; break;
        case AspectCase::RepairBarrierSet: j = nlohmann::json{ { "RepairBarrierSet", v.variant_union.repair_barrier_set} }; break;
        case AspectCase::ConstructionRepair: j = nlohmann::json{ { "ConstructionRepair", v.variant_union.construction_repair} }; break;
        case AspectCase::Follower: j = nlohmann::json{ { "Follower", v.variant_union.follower} }; break;
        case AspectCase::CollisionBase: j = nlohmann::json{ { "CollisionBase", v.variant_union.collision_base} }; break;
        case AspectCase::EditorUniqueID: j = nlohmann::json{ { "EditorUniqueID", v.variant_union.editor_unique_id} }; break;
        case AspectCase::Roam: j = nlohmann::json{ { "Roam", v.variant_union.roam} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Aspect& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "PowerProduction") { v.variant_union.power_production = value.template get<AspectPowerProduction>(); v.variant_case = AspectCase::PowerProduction; }
            if (key == "Health") { v.variant_union.health = value.template get<AspectHealth>(); v.variant_case = AspectCase::Health; }
            if (key == "Combat") { v.variant_union.combat = value.template get<AspectCombat>(); v.variant_case = AspectCase::Combat; }
            if (key == "ModeChange") { v.variant_union.mode_change = value.template get<AspectModeChange>(); v.variant_case = AspectCase::ModeChange; }
            if (key == "Ammunition") { v.variant_union.ammunition = value.template get<AspectAmmunition>(); v.variant_case = AspectCase::Ammunition; }
            if (key == "SuperWeaponShadow") { v.variant_union.super_weapon_shadow = value.template get<AspectSuperWeaponShadow>(); v.variant_case = AspectCase::SuperWeaponShadow; }
            if (key == "WormMovement") { v.variant_union.worm_movement = value.template get<AspectWormMovement>(); v.variant_case = AspectCase::WormMovement; }
            if (key == "NPCTag") { v.variant_union.npc_tag = value.template get<AspectNPCTag>(); v.variant_case = AspectCase::NPCTag; }
            if (key == "PlayerKit") { v.variant_union.player_kit = value.template get<AspectPlayerKit>(); v.variant_case = AspectCase::PlayerKit; }
            if (key == "Loot") { v.variant_union.loot = value.template get<AspectLoot>(); v.variant_case = AspectCase::Loot; }
            if (key == "Immunity") { v.variant_union.immunity = value.template get<AspectImmunity>(); v.variant_case = AspectCase::Immunity; }
            if (key == "Turret") { v.variant_union.turret = value.template get<AspectTurret>(); v.variant_case = AspectCase::Turret; }
            if (key == "Tunnel") { v.variant_union.tunnel = value.template get<AspectTunnel>(); v.variant_case = AspectCase::Tunnel; }
            if (key == "MountBarrier") { v.variant_union.mount_barrier = value.template get<AspectMountBarrier>(); v.variant_case = AspectCase::MountBarrier; }
            if (key == "SpellMemory") { v.variant_union.spell_memory = value.template get<AspectSpellMemory>(); v.variant_case = AspectCase::SpellMemory; }
            if (key == "Portal") { v.variant_union.portal = value.template get<AspectPortal>(); v.variant_case = AspectCase::Portal; }
            if (key == "Hate") { v.variant_union.hate = value.template get<AspectHate>(); v.variant_case = AspectCase::Hate; }
            if (key == "BarrierGate") { v.variant_union.barrier_gate = value.template get<AspectBarrierGate>(); v.variant_case = AspectCase::BarrierGate; }
            if (key == "Attackable") { v.variant_union.attackable = value.template get<AspectAttackable>(); v.variant_case = AspectCase::Attackable; }
            if (key == "SquadRefill") { v.variant_union.squad_refill = value.template get<AspectSquadRefill>(); v.variant_case = AspectCase::SquadRefill; }
            if (key == "PortalExit") { v.variant_union.portal_exit = value.template get<AspectPortalExit>(); v.variant_case = AspectCase::PortalExit; }
            if (key == "ConstructionData") { v.variant_union.construction_data = value.template get<AspectConstructionData>(); v.variant_case = AspectCase::ConstructionData; }
            if (key == "SuperWeaponShadowBomb") { v.variant_union.super_weapon_shadow_bomb = value.template get<AspectSuperWeaponShadowBomb>(); v.variant_case = AspectCase::SuperWeaponShadowBomb; }
            if (key == "RepairBarrierSet") { v.variant_union.repair_barrier_set = value.template get<AspectRepairBarrierSet>(); v.variant_case = AspectCase::RepairBarrierSet; }
            if (key == "ConstructionRepair") { v.variant_union.construction_repair = value.template get<AspectConstructionRepair>(); v.variant_case = AspectCase::ConstructionRepair; }
            if (key == "Follower") { v.variant_union.follower = value.template get<AspectFollower>(); v.variant_case = AspectCase::Follower; }
            if (key == "CollisionBase") { v.variant_union.collision_base = value.template get<AspectCollisionBase>(); v.variant_case = AspectCase::CollisionBase; }
            if (key == "EditorUniqueID") { v.variant_union.editor_unique_id = value.template get<AspectEditorUniqueID>(); v.variant_case = AspectCase::EditorUniqueID; }
            if (key == "Roam") { v.variant_union.roam = value.template get<AspectRoam>(); v.variant_case = AspectCase::Roam; }
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
    enum class JobCase {
        Invalid,
        NoJob,
        Idle,
        Goto,
        AttackMelee,
        CastSpell,
        Die,
        Talk,
        ScriptTalk,
        Freeze,
        Spawn,
        Cheer,
        AttackSquad,
        CastSpellSquad,
        PushBack,
        Stampede,
        BarrierCrush,
        BarrierGateToggle,
        FlameThrower,
        Construct,
        Crush,
        MountBarrierSquad,
        MountBarrier,
        ModeChangeSquad,
        ModeChange,
        SacrificeSquad,
        UsePortalSquad,
        Channel,
        SpawnSquad,
        LootTargetSquad,
        Morph,
        Unknown
    };
    union JobUnion {
        InvalidVariant invalid_enum_variant;
        JobNoJob no_job;
        JobIdle idle;
        JobGoto gotoCppField;
        JobAttackMelee attack_melee;
        JobCastSpell cast_spell;
        JobDie die;
        JobTalk talk;
        JobScriptTalk script_talk;
        JobFreeze freeze;
        JobSpawn spawn;
        JobCheer cheer;
        JobAttackSquad attack_squad;
        JobCastSpellSquad cast_spell_squad;
        JobPushBack push_back;
        JobStampede stampede;
        JobBarrierCrush barrier_crush;
        JobBarrierGateToggle barrier_gate_toggle;
        JobFlameThrower flame_thrower;
        JobConstruct construct;
        JobCrush crush;
        JobMountBarrierSquad mount_barrier_squad;
        JobMountBarrier mount_barrier;
        JobModeChangeSquad mode_change_squad;
        JobModeChange mode_change;
        JobSacrificeSquad sacrifice_squad;
        JobUsePortalSquad use_portal_squad;
        JobChannel channel;
        JobSpawnSquad spawn_squad;
        JobLootTargetSquad loot_target_squad;
        JobMorph morph;
        JobUnknown unknown;
        JobUnion() { invalid_enum_variant = {}; }
        ~JobUnion() { invalid_enum_variant = {}; }
        JobUnion(const JobUnion& other) = default;
        JobUnion(JobUnion&& other) = default;
        JobUnion& operator=(const JobUnion& other) = default;
        JobUnion& operator=(JobUnion&& other) = default;
    };
    struct Job {
        JobCase variant_case;
        JobUnion variant_union;
        Job() {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        Job(const Job& other) {
            memset(this, 0, sizeof(Job));
            variant_case = other.variant_case;
            switch (variant_case) {
            case JobCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case JobCase::NoJob: variant_union.no_job = other.variant_union.no_job; break;
            case JobCase::Idle: variant_union.idle = other.variant_union.idle; break;
            case JobCase::Goto: variant_union.gotoCppField = other.variant_union.gotoCppField; break;
            case JobCase::AttackMelee: variant_union.attack_melee = other.variant_union.attack_melee; break;
            case JobCase::CastSpell: variant_union.cast_spell = other.variant_union.cast_spell; break;
            case JobCase::Die: variant_union.die = other.variant_union.die; break;
            case JobCase::Talk: variant_union.talk = other.variant_union.talk; break;
            case JobCase::ScriptTalk: variant_union.script_talk = other.variant_union.script_talk; break;
            case JobCase::Freeze: variant_union.freeze = other.variant_union.freeze; break;
            case JobCase::Spawn: variant_union.spawn = other.variant_union.spawn; break;
            case JobCase::Cheer: variant_union.cheer = other.variant_union.cheer; break;
            case JobCase::AttackSquad: variant_union.attack_squad = other.variant_union.attack_squad; break;
            case JobCase::CastSpellSquad: variant_union.cast_spell_squad = other.variant_union.cast_spell_squad; break;
            case JobCase::PushBack: variant_union.push_back = other.variant_union.push_back; break;
            case JobCase::Stampede: variant_union.stampede = other.variant_union.stampede; break;
            case JobCase::BarrierCrush: variant_union.barrier_crush = other.variant_union.barrier_crush; break;
            case JobCase::BarrierGateToggle: variant_union.barrier_gate_toggle = other.variant_union.barrier_gate_toggle; break;
            case JobCase::FlameThrower: variant_union.flame_thrower = other.variant_union.flame_thrower; break;
            case JobCase::Construct: variant_union.construct = other.variant_union.construct; break;
            case JobCase::Crush: variant_union.crush = other.variant_union.crush; break;
            case JobCase::MountBarrierSquad: variant_union.mount_barrier_squad = other.variant_union.mount_barrier_squad; break;
            case JobCase::MountBarrier: variant_union.mount_barrier = other.variant_union.mount_barrier; break;
            case JobCase::ModeChangeSquad: variant_union.mode_change_squad = other.variant_union.mode_change_squad; break;
            case JobCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case JobCase::SacrificeSquad: variant_union.sacrifice_squad = other.variant_union.sacrifice_squad; break;
            case JobCase::UsePortalSquad: variant_union.use_portal_squad = other.variant_union.use_portal_squad; break;
            case JobCase::Channel: variant_union.channel = other.variant_union.channel; break;
            case JobCase::SpawnSquad: variant_union.spawn_squad = other.variant_union.spawn_squad; break;
            case JobCase::LootTargetSquad: variant_union.loot_target_squad = other.variant_union.loot_target_squad; break;
            case JobCase::Morph: variant_union.morph = other.variant_union.morph; break;
            case JobCase::Unknown: variant_union.unknown = other.variant_union.unknown; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Job& operator=(const Job& other) {
            memset(this, 0, sizeof(Job));
            variant_case = other.variant_case;
            switch (variant_case) {
            case JobCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case JobCase::NoJob: variant_union.no_job = other.variant_union.no_job; break;
            case JobCase::Idle: variant_union.idle = other.variant_union.idle; break;
            case JobCase::Goto: variant_union.gotoCppField = other.variant_union.gotoCppField; break;
            case JobCase::AttackMelee: variant_union.attack_melee = other.variant_union.attack_melee; break;
            case JobCase::CastSpell: variant_union.cast_spell = other.variant_union.cast_spell; break;
            case JobCase::Die: variant_union.die = other.variant_union.die; break;
            case JobCase::Talk: variant_union.talk = other.variant_union.talk; break;
            case JobCase::ScriptTalk: variant_union.script_talk = other.variant_union.script_talk; break;
            case JobCase::Freeze: variant_union.freeze = other.variant_union.freeze; break;
            case JobCase::Spawn: variant_union.spawn = other.variant_union.spawn; break;
            case JobCase::Cheer: variant_union.cheer = other.variant_union.cheer; break;
            case JobCase::AttackSquad: variant_union.attack_squad = other.variant_union.attack_squad; break;
            case JobCase::CastSpellSquad: variant_union.cast_spell_squad = other.variant_union.cast_spell_squad; break;
            case JobCase::PushBack: variant_union.push_back = other.variant_union.push_back; break;
            case JobCase::Stampede: variant_union.stampede = other.variant_union.stampede; break;
            case JobCase::BarrierCrush: variant_union.barrier_crush = other.variant_union.barrier_crush; break;
            case JobCase::BarrierGateToggle: variant_union.barrier_gate_toggle = other.variant_union.barrier_gate_toggle; break;
            case JobCase::FlameThrower: variant_union.flame_thrower = other.variant_union.flame_thrower; break;
            case JobCase::Construct: variant_union.construct = other.variant_union.construct; break;
            case JobCase::Crush: variant_union.crush = other.variant_union.crush; break;
            case JobCase::MountBarrierSquad: variant_union.mount_barrier_squad = other.variant_union.mount_barrier_squad; break;
            case JobCase::MountBarrier: variant_union.mount_barrier = other.variant_union.mount_barrier; break;
            case JobCase::ModeChangeSquad: variant_union.mode_change_squad = other.variant_union.mode_change_squad; break;
            case JobCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case JobCase::SacrificeSquad: variant_union.sacrifice_squad = other.variant_union.sacrifice_squad; break;
            case JobCase::UsePortalSquad: variant_union.use_portal_squad = other.variant_union.use_portal_squad; break;
            case JobCase::Channel: variant_union.channel = other.variant_union.channel; break;
            case JobCase::SpawnSquad: variant_union.spawn_squad = other.variant_union.spawn_squad; break;
            case JobCase::LootTargetSquad: variant_union.loot_target_squad = other.variant_union.loot_target_squad; break;
            case JobCase::Morph: variant_union.morph = other.variant_union.morph; break;
            case JobCase::Unknown: variant_union.unknown = other.variant_union.unknown; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Job(JobNoJob v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::NoJob;
            variant_union.no_job = v;
        }
        Job(JobIdle v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Idle;
            variant_union.idle = v;
        }
        Job(JobGoto v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Goto;
            variant_union.gotoCppField = v;
        }
        Job(JobAttackMelee v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::AttackMelee;
            variant_union.attack_melee = v;
        }
        Job(JobCastSpell v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::CastSpell;
            variant_union.cast_spell = v;
        }
        Job(JobDie v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Die;
            variant_union.die = v;
        }
        Job(JobTalk v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Talk;
            variant_union.talk = v;
        }
        Job(JobScriptTalk v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::ScriptTalk;
            variant_union.script_talk = v;
        }
        Job(JobFreeze v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Freeze;
            variant_union.freeze = v;
        }
        Job(JobSpawn v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Spawn;
            variant_union.spawn = v;
        }
        Job(JobCheer v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Cheer;
            variant_union.cheer = v;
        }
        Job(JobAttackSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::AttackSquad;
            variant_union.attack_squad = v;
        }
        Job(JobCastSpellSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::CastSpellSquad;
            variant_union.cast_spell_squad = v;
        }
        Job(JobPushBack v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::PushBack;
            variant_union.push_back = v;
        }
        Job(JobStampede v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Stampede;
            variant_union.stampede = v;
        }
        Job(JobBarrierCrush v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::BarrierCrush;
            variant_union.barrier_crush = v;
        }
        Job(JobBarrierGateToggle v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::BarrierGateToggle;
            variant_union.barrier_gate_toggle = v;
        }
        Job(JobFlameThrower v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::FlameThrower;
            variant_union.flame_thrower = v;
        }
        Job(JobConstruct v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Construct;
            variant_union.construct = v;
        }
        Job(JobCrush v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Crush;
            variant_union.crush = v;
        }
        Job(JobMountBarrierSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::MountBarrierSquad;
            variant_union.mount_barrier_squad = v;
        }
        Job(JobMountBarrier v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::MountBarrier;
            variant_union.mount_barrier = v;
        }
        Job(JobModeChangeSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::ModeChangeSquad;
            variant_union.mode_change_squad = v;
        }
        Job(JobModeChange v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::ModeChange;
            variant_union.mode_change = v;
        }
        Job(JobSacrificeSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::SacrificeSquad;
            variant_union.sacrifice_squad = v;
        }
        Job(JobUsePortalSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::UsePortalSquad;
            variant_union.use_portal_squad = v;
        }
        Job(JobChannel v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Channel;
            variant_union.channel = v;
        }
        Job(JobSpawnSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::SpawnSquad;
            variant_union.spawn_squad = v;
        }
        Job(JobLootTargetSquad v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::LootTargetSquad;
            variant_union.loot_target_squad = v;
        }
        Job(JobMorph v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Morph;
            variant_union.morph = v;
        }
        Job(JobUnknown v) {
            memset(this, 0, sizeof(Job));
            variant_case = JobCase::Unknown;
            variant_union.unknown = v;
        }
    };
    inline void to_json(nlohmann::json& j, const Job& v) {
        switch (v.variant_case) {
        case JobCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case JobCase::NoJob: j = nlohmann::json{ { "NoJob", v.variant_union.no_job} }; break;
        case JobCase::Idle: j = nlohmann::json{ { "Idle", v.variant_union.idle} }; break;
        case JobCase::Goto: j = nlohmann::json{ { "Goto", v.variant_union.gotoCppField} }; break;
        case JobCase::AttackMelee: j = nlohmann::json{ { "AttackMelee", v.variant_union.attack_melee} }; break;
        case JobCase::CastSpell: j = nlohmann::json{ { "CastSpell", v.variant_union.cast_spell} }; break;
        case JobCase::Die: j = nlohmann::json{ { "Die", v.variant_union.die} }; break;
        case JobCase::Talk: j = nlohmann::json{ { "Talk", v.variant_union.talk} }; break;
        case JobCase::ScriptTalk: j = nlohmann::json{ { "ScriptTalk", v.variant_union.script_talk} }; break;
        case JobCase::Freeze: j = nlohmann::json{ { "Freeze", v.variant_union.freeze} }; break;
        case JobCase::Spawn: j = nlohmann::json{ { "Spawn", v.variant_union.spawn} }; break;
        case JobCase::Cheer: j = nlohmann::json{ { "Cheer", v.variant_union.cheer} }; break;
        case JobCase::AttackSquad: j = nlohmann::json{ { "AttackSquad", v.variant_union.attack_squad} }; break;
        case JobCase::CastSpellSquad: j = nlohmann::json{ { "CastSpellSquad", v.variant_union.cast_spell_squad} }; break;
        case JobCase::PushBack: j = nlohmann::json{ { "PushBack", v.variant_union.push_back} }; break;
        case JobCase::Stampede: j = nlohmann::json{ { "Stampede", v.variant_union.stampede} }; break;
        case JobCase::BarrierCrush: j = nlohmann::json{ { "BarrierCrush", v.variant_union.barrier_crush} }; break;
        case JobCase::BarrierGateToggle: j = nlohmann::json{ { "BarrierGateToggle", v.variant_union.barrier_gate_toggle} }; break;
        case JobCase::FlameThrower: j = nlohmann::json{ { "FlameThrower", v.variant_union.flame_thrower} }; break;
        case JobCase::Construct: j = nlohmann::json{ { "Construct", v.variant_union.construct} }; break;
        case JobCase::Crush: j = nlohmann::json{ { "Crush", v.variant_union.crush} }; break;
        case JobCase::MountBarrierSquad: j = nlohmann::json{ { "MountBarrierSquad", v.variant_union.mount_barrier_squad} }; break;
        case JobCase::MountBarrier: j = nlohmann::json{ { "MountBarrier", v.variant_union.mount_barrier} }; break;
        case JobCase::ModeChangeSquad: j = nlohmann::json{ { "ModeChangeSquad", v.variant_union.mode_change_squad} }; break;
        case JobCase::ModeChange: j = nlohmann::json{ { "ModeChange", v.variant_union.mode_change} }; break;
        case JobCase::SacrificeSquad: j = nlohmann::json{ { "SacrificeSquad", v.variant_union.sacrifice_squad} }; break;
        case JobCase::UsePortalSquad: j = nlohmann::json{ { "UsePortalSquad", v.variant_union.use_portal_squad} }; break;
        case JobCase::Channel: j = nlohmann::json{ { "Channel", v.variant_union.channel} }; break;
        case JobCase::SpawnSquad: j = nlohmann::json{ { "SpawnSquad", v.variant_union.spawn_squad} }; break;
        case JobCase::LootTargetSquad: j = nlohmann::json{ { "LootTargetSquad", v.variant_union.loot_target_squad} }; break;
        case JobCase::Morph: j = nlohmann::json{ { "Morph", v.variant_union.morph} }; break;
        case JobCase::Unknown: j = nlohmann::json{ { "Unknown", v.variant_union.unknown} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Job& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "NoJob") { v.variant_union.no_job = value.template get<JobNoJob>(); v.variant_case = JobCase::NoJob; }
            if (key == "Idle") { v.variant_union.idle = value.template get<JobIdle>(); v.variant_case = JobCase::Idle; }
            if (key == "Goto") { v.variant_union.gotoCppField = value.template get<JobGoto>(); v.variant_case = JobCase::Goto; }
            if (key == "AttackMelee") { v.variant_union.attack_melee = value.template get<JobAttackMelee>(); v.variant_case = JobCase::AttackMelee; }
            if (key == "CastSpell") { v.variant_union.cast_spell = value.template get<JobCastSpell>(); v.variant_case = JobCase::CastSpell; }
            if (key == "Die") { v.variant_union.die = value.template get<JobDie>(); v.variant_case = JobCase::Die; }
            if (key == "Talk") { v.variant_union.talk = value.template get<JobTalk>(); v.variant_case = JobCase::Talk; }
            if (key == "ScriptTalk") { v.variant_union.script_talk = value.template get<JobScriptTalk>(); v.variant_case = JobCase::ScriptTalk; }
            if (key == "Freeze") { v.variant_union.freeze = value.template get<JobFreeze>(); v.variant_case = JobCase::Freeze; }
            if (key == "Spawn") { v.variant_union.spawn = value.template get<JobSpawn>(); v.variant_case = JobCase::Spawn; }
            if (key == "Cheer") { v.variant_union.cheer = value.template get<JobCheer>(); v.variant_case = JobCase::Cheer; }
            if (key == "AttackSquad") { v.variant_union.attack_squad = value.template get<JobAttackSquad>(); v.variant_case = JobCase::AttackSquad; }
            if (key == "CastSpellSquad") { v.variant_union.cast_spell_squad = value.template get<JobCastSpellSquad>(); v.variant_case = JobCase::CastSpellSquad; }
            if (key == "PushBack") { v.variant_union.push_back = value.template get<JobPushBack>(); v.variant_case = JobCase::PushBack; }
            if (key == "Stampede") { v.variant_union.stampede = value.template get<JobStampede>(); v.variant_case = JobCase::Stampede; }
            if (key == "BarrierCrush") { v.variant_union.barrier_crush = value.template get<JobBarrierCrush>(); v.variant_case = JobCase::BarrierCrush; }
            if (key == "BarrierGateToggle") { v.variant_union.barrier_gate_toggle = value.template get<JobBarrierGateToggle>(); v.variant_case = JobCase::BarrierGateToggle; }
            if (key == "FlameThrower") { v.variant_union.flame_thrower = value.template get<JobFlameThrower>(); v.variant_case = JobCase::FlameThrower; }
            if (key == "Construct") { v.variant_union.construct = value.template get<JobConstruct>(); v.variant_case = JobCase::Construct; }
            if (key == "Crush") { v.variant_union.crush = value.template get<JobCrush>(); v.variant_case = JobCase::Crush; }
            if (key == "MountBarrierSquad") { v.variant_union.mount_barrier_squad = value.template get<JobMountBarrierSquad>(); v.variant_case = JobCase::MountBarrierSquad; }
            if (key == "MountBarrier") { v.variant_union.mount_barrier = value.template get<JobMountBarrier>(); v.variant_case = JobCase::MountBarrier; }
            if (key == "ModeChangeSquad") { v.variant_union.mode_change_squad = value.template get<JobModeChangeSquad>(); v.variant_case = JobCase::ModeChangeSquad; }
            if (key == "ModeChange") { v.variant_union.mode_change = value.template get<JobModeChange>(); v.variant_case = JobCase::ModeChange; }
            if (key == "SacrificeSquad") { v.variant_union.sacrifice_squad = value.template get<JobSacrificeSquad>(); v.variant_case = JobCase::SacrificeSquad; }
            if (key == "UsePortalSquad") { v.variant_union.use_portal_squad = value.template get<JobUsePortalSquad>(); v.variant_case = JobCase::UsePortalSquad; }
            if (key == "Channel") { v.variant_union.channel = value.template get<JobChannel>(); v.variant_case = JobCase::Channel; }
            if (key == "SpawnSquad") { v.variant_union.spawn_squad = value.template get<JobSpawnSquad>(); v.variant_case = JobCase::SpawnSquad; }
            if (key == "LootTargetSquad") { v.variant_union.loot_target_squad = value.template get<JobLootTargetSquad>(); v.variant_case = JobCase::LootTargetSquad; }
            if (key == "Morph") { v.variant_union.morph = value.template get<JobMorph>(); v.variant_case = JobCase::Morph; }
            if (key == "Unknown") { v.variant_union.unknown = value.template get<JobUnknown>(); v.variant_case = JobCase::Unknown; }
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
    enum class CommandCase {
        Invalid,
        BuildHouse,
        CastSpellGod,
        CastSpellGodMulti,
        ProduceSquad,
        ProduceSquadOnBarrier,
        CastSpellEntity,
        BarrierGateToggle,
        BarrierBuild,
        BarrierRepair,
        BarrierCancelRepair,
        RepairBuilding,
        CancelRepairBuilding,
        GroupAttack,
        GroupEnterWall,
        GroupExitWall,
        GroupGoto,
        GroupHoldPosition,
        GroupStopJob,
        ModeChange,
        PowerSlotBuild,
        TokenSlotBuild,
        GroupKillEntity,
        GroupSacrifice,
        PortalDefineExitPoint,
        PortalRemoveExitPoint,
        TunnelMakeExitPoint,
        Ping,
        Surrender,
        WhisperToMaster
    };
    union CommandUnion {
        InvalidVariant invalid_enum_variant;
        CommandBuildHouse build_house;
        CommandCastSpellGod cast_spell_god;
        CommandCastSpellGodMulti cast_spell_god_multi;
        CommandProduceSquad produce_squad;
        CommandProduceSquadOnBarrier produce_squad_on_barrier;
        CommandCastSpellEntity cast_spell_entity;
        CommandBarrierGateToggle barrier_gate_toggle;
        CommandBarrierBuild barrier_build;
        CommandBarrierRepair barrier_repair;
        CommandBarrierCancelRepair barrier_cancel_repair;
        CommandRepairBuilding repair_building;
        CommandCancelRepairBuilding cancel_repair_building;
        CommandGroupAttack group_attack;
        CommandGroupEnterWall group_enter_wall;
        CommandGroupExitWall group_exit_wall;
        CommandGroupGoto group_goto;
        CommandGroupHoldPosition group_hold_position;
        CommandGroupStopJob group_stop_job;
        CommandModeChange mode_change;
        CommandPowerSlotBuild power_slot_build;
        CommandTokenSlotBuild token_slot_build;
        CommandGroupKillEntity group_kill_entity;
        CommandGroupSacrifice group_sacrifice;
        CommandPortalDefineExitPoint portal_define_exit_point;
        CommandPortalRemoveExitPoint portal_remove_exit_point;
        CommandTunnelMakeExitPoint tunnel_make_exit_point;
        CommandPing ping;
        CommandSurrender surrender;
        CommandWhisperToMaster whisper_to_master;
        CommandUnion() { invalid_enum_variant = {}; }
        ~CommandUnion() { invalid_enum_variant = {}; }
        CommandUnion(const CommandUnion& other) = default;
        CommandUnion(CommandUnion&& other) = default;
        CommandUnion& operator=(const CommandUnion& other) = default;
        CommandUnion& operator=(CommandUnion&& other) = default;
    };
    struct Command {
        CommandCase variant_case;
        CommandUnion variant_union;
        Command() {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        Command(const Command& other) {
            memset(this, 0, sizeof(Command));
            variant_case = other.variant_case;
            switch (variant_case) {
            case CommandCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case CommandCase::BuildHouse: variant_union.build_house = other.variant_union.build_house; break;
            case CommandCase::CastSpellGod: variant_union.cast_spell_god = other.variant_union.cast_spell_god; break;
            case CommandCase::CastSpellGodMulti: variant_union.cast_spell_god_multi = other.variant_union.cast_spell_god_multi; break;
            case CommandCase::ProduceSquad: variant_union.produce_squad = other.variant_union.produce_squad; break;
            case CommandCase::ProduceSquadOnBarrier: variant_union.produce_squad_on_barrier = other.variant_union.produce_squad_on_barrier; break;
            case CommandCase::CastSpellEntity: variant_union.cast_spell_entity = other.variant_union.cast_spell_entity; break;
            case CommandCase::BarrierGateToggle: variant_union.barrier_gate_toggle = other.variant_union.barrier_gate_toggle; break;
            case CommandCase::BarrierBuild: variant_union.barrier_build = other.variant_union.barrier_build; break;
            case CommandCase::BarrierRepair: variant_union.barrier_repair = other.variant_union.barrier_repair; break;
            case CommandCase::BarrierCancelRepair: variant_union.barrier_cancel_repair = other.variant_union.barrier_cancel_repair; break;
            case CommandCase::RepairBuilding: variant_union.repair_building = other.variant_union.repair_building; break;
            case CommandCase::CancelRepairBuilding: variant_union.cancel_repair_building = other.variant_union.cancel_repair_building; break;
            case CommandCase::GroupAttack: variant_union.group_attack = other.variant_union.group_attack; break;
            case CommandCase::GroupEnterWall: variant_union.group_enter_wall = other.variant_union.group_enter_wall; break;
            case CommandCase::GroupExitWall: variant_union.group_exit_wall = other.variant_union.group_exit_wall; break;
            case CommandCase::GroupGoto: variant_union.group_goto = other.variant_union.group_goto; break;
            case CommandCase::GroupHoldPosition: variant_union.group_hold_position = other.variant_union.group_hold_position; break;
            case CommandCase::GroupStopJob: variant_union.group_stop_job = other.variant_union.group_stop_job; break;
            case CommandCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case CommandCase::PowerSlotBuild: variant_union.power_slot_build = other.variant_union.power_slot_build; break;
            case CommandCase::TokenSlotBuild: variant_union.token_slot_build = other.variant_union.token_slot_build; break;
            case CommandCase::GroupKillEntity: variant_union.group_kill_entity = other.variant_union.group_kill_entity; break;
            case CommandCase::GroupSacrifice: variant_union.group_sacrifice = other.variant_union.group_sacrifice; break;
            case CommandCase::PortalDefineExitPoint: variant_union.portal_define_exit_point = other.variant_union.portal_define_exit_point; break;
            case CommandCase::PortalRemoveExitPoint: variant_union.portal_remove_exit_point = other.variant_union.portal_remove_exit_point; break;
            case CommandCase::TunnelMakeExitPoint: variant_union.tunnel_make_exit_point = other.variant_union.tunnel_make_exit_point; break;
            case CommandCase::Ping: variant_union.ping = other.variant_union.ping; break;
            case CommandCase::Surrender: variant_union.surrender = other.variant_union.surrender; break;
            case CommandCase::WhisperToMaster: variant_union.whisper_to_master = other.variant_union.whisper_to_master; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Command& operator=(const Command& other) {
            memset(this, 0, sizeof(Command));
            variant_case = other.variant_case;
            switch (variant_case) {
            case CommandCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case CommandCase::BuildHouse: variant_union.build_house = other.variant_union.build_house; break;
            case CommandCase::CastSpellGod: variant_union.cast_spell_god = other.variant_union.cast_spell_god; break;
            case CommandCase::CastSpellGodMulti: variant_union.cast_spell_god_multi = other.variant_union.cast_spell_god_multi; break;
            case CommandCase::ProduceSquad: variant_union.produce_squad = other.variant_union.produce_squad; break;
            case CommandCase::ProduceSquadOnBarrier: variant_union.produce_squad_on_barrier = other.variant_union.produce_squad_on_barrier; break;
            case CommandCase::CastSpellEntity: variant_union.cast_spell_entity = other.variant_union.cast_spell_entity; break;
            case CommandCase::BarrierGateToggle: variant_union.barrier_gate_toggle = other.variant_union.barrier_gate_toggle; break;
            case CommandCase::BarrierBuild: variant_union.barrier_build = other.variant_union.barrier_build; break;
            case CommandCase::BarrierRepair: variant_union.barrier_repair = other.variant_union.barrier_repair; break;
            case CommandCase::BarrierCancelRepair: variant_union.barrier_cancel_repair = other.variant_union.barrier_cancel_repair; break;
            case CommandCase::RepairBuilding: variant_union.repair_building = other.variant_union.repair_building; break;
            case CommandCase::CancelRepairBuilding: variant_union.cancel_repair_building = other.variant_union.cancel_repair_building; break;
            case CommandCase::GroupAttack: variant_union.group_attack = other.variant_union.group_attack; break;
            case CommandCase::GroupEnterWall: variant_union.group_enter_wall = other.variant_union.group_enter_wall; break;
            case CommandCase::GroupExitWall: variant_union.group_exit_wall = other.variant_union.group_exit_wall; break;
            case CommandCase::GroupGoto: variant_union.group_goto = other.variant_union.group_goto; break;
            case CommandCase::GroupHoldPosition: variant_union.group_hold_position = other.variant_union.group_hold_position; break;
            case CommandCase::GroupStopJob: variant_union.group_stop_job = other.variant_union.group_stop_job; break;
            case CommandCase::ModeChange: variant_union.mode_change = other.variant_union.mode_change; break;
            case CommandCase::PowerSlotBuild: variant_union.power_slot_build = other.variant_union.power_slot_build; break;
            case CommandCase::TokenSlotBuild: variant_union.token_slot_build = other.variant_union.token_slot_build; break;
            case CommandCase::GroupKillEntity: variant_union.group_kill_entity = other.variant_union.group_kill_entity; break;
            case CommandCase::GroupSacrifice: variant_union.group_sacrifice = other.variant_union.group_sacrifice; break;
            case CommandCase::PortalDefineExitPoint: variant_union.portal_define_exit_point = other.variant_union.portal_define_exit_point; break;
            case CommandCase::PortalRemoveExitPoint: variant_union.portal_remove_exit_point = other.variant_union.portal_remove_exit_point; break;
            case CommandCase::TunnelMakeExitPoint: variant_union.tunnel_make_exit_point = other.variant_union.tunnel_make_exit_point; break;
            case CommandCase::Ping: variant_union.ping = other.variant_union.ping; break;
            case CommandCase::Surrender: variant_union.surrender = other.variant_union.surrender; break;
            case CommandCase::WhisperToMaster: variant_union.whisper_to_master = other.variant_union.whisper_to_master; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        Command(CommandBuildHouse v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::BuildHouse;
            variant_union.build_house = v;
        }
        Command(CommandCastSpellGod v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::CastSpellGod;
            variant_union.cast_spell_god = v;
        }
        Command(CommandCastSpellGodMulti v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::CastSpellGodMulti;
            variant_union.cast_spell_god_multi = v;
        }
        Command(CommandProduceSquad v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::ProduceSquad;
            variant_union.produce_squad = v;
        }
        Command(CommandProduceSquadOnBarrier v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::ProduceSquadOnBarrier;
            variant_union.produce_squad_on_barrier = v;
        }
        Command(CommandCastSpellEntity v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::CastSpellEntity;
            variant_union.cast_spell_entity = v;
        }
        Command(CommandBarrierGateToggle v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::BarrierGateToggle;
            variant_union.barrier_gate_toggle = v;
        }
        Command(CommandBarrierBuild v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::BarrierBuild;
            variant_union.barrier_build = v;
        }
        Command(CommandBarrierRepair v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::BarrierRepair;
            variant_union.barrier_repair = v;
        }
        Command(CommandBarrierCancelRepair v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::BarrierCancelRepair;
            variant_union.barrier_cancel_repair = v;
        }
        Command(CommandRepairBuilding v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::RepairBuilding;
            variant_union.repair_building = v;
        }
        Command(CommandCancelRepairBuilding v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::CancelRepairBuilding;
            variant_union.cancel_repair_building = v;
        }
        Command(CommandGroupAttack v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupAttack;
            variant_union.group_attack = v;
        }
        Command(CommandGroupEnterWall v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupEnterWall;
            variant_union.group_enter_wall = v;
        }
        Command(CommandGroupExitWall v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupExitWall;
            variant_union.group_exit_wall = v;
        }
        Command(CommandGroupGoto v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupGoto;
            variant_union.group_goto = v;
        }
        Command(CommandGroupHoldPosition v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupHoldPosition;
            variant_union.group_hold_position = v;
        }
        Command(CommandGroupStopJob v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupStopJob;
            variant_union.group_stop_job = v;
        }
        Command(CommandModeChange v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::ModeChange;
            variant_union.mode_change = v;
        }
        Command(CommandPowerSlotBuild v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::PowerSlotBuild;
            variant_union.power_slot_build = v;
        }
        Command(CommandTokenSlotBuild v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::TokenSlotBuild;
            variant_union.token_slot_build = v;
        }
        Command(CommandGroupKillEntity v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupKillEntity;
            variant_union.group_kill_entity = v;
        }
        Command(CommandGroupSacrifice v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::GroupSacrifice;
            variant_union.group_sacrifice = v;
        }
        Command(CommandPortalDefineExitPoint v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::PortalDefineExitPoint;
            variant_union.portal_define_exit_point = v;
        }
        Command(CommandPortalRemoveExitPoint v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::PortalRemoveExitPoint;
            variant_union.portal_remove_exit_point = v;
        }
        Command(CommandTunnelMakeExitPoint v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::TunnelMakeExitPoint;
            variant_union.tunnel_make_exit_point = v;
        }
        Command(CommandPing v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::Ping;
            variant_union.ping = v;
        }
        Command(CommandSurrender v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::Surrender;
            variant_union.surrender = v;
        }
        Command(CommandWhisperToMaster v) {
            memset(this, 0, sizeof(Command));
            variant_case = CommandCase::WhisperToMaster;
            variant_union.whisper_to_master = v;
        }
    };
    inline void to_json(nlohmann::json& j, const Command& v) {
        switch (v.variant_case) {
        case CommandCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case CommandCase::BuildHouse: j = nlohmann::json{ { "BuildHouse", v.variant_union.build_house} }; break;
        case CommandCase::CastSpellGod: j = nlohmann::json{ { "CastSpellGod", v.variant_union.cast_spell_god} }; break;
        case CommandCase::CastSpellGodMulti: j = nlohmann::json{ { "CastSpellGodMulti", v.variant_union.cast_spell_god_multi} }; break;
        case CommandCase::ProduceSquad: j = nlohmann::json{ { "ProduceSquad", v.variant_union.produce_squad} }; break;
        case CommandCase::ProduceSquadOnBarrier: j = nlohmann::json{ { "ProduceSquadOnBarrier", v.variant_union.produce_squad_on_barrier} }; break;
        case CommandCase::CastSpellEntity: j = nlohmann::json{ { "CastSpellEntity", v.variant_union.cast_spell_entity} }; break;
        case CommandCase::BarrierGateToggle: j = nlohmann::json{ { "BarrierGateToggle", v.variant_union.barrier_gate_toggle} }; break;
        case CommandCase::BarrierBuild: j = nlohmann::json{ { "BarrierBuild", v.variant_union.barrier_build} }; break;
        case CommandCase::BarrierRepair: j = nlohmann::json{ { "BarrierRepair", v.variant_union.barrier_repair} }; break;
        case CommandCase::BarrierCancelRepair: j = nlohmann::json{ { "BarrierCancelRepair", v.variant_union.barrier_cancel_repair} }; break;
        case CommandCase::RepairBuilding: j = nlohmann::json{ { "RepairBuilding", v.variant_union.repair_building} }; break;
        case CommandCase::CancelRepairBuilding: j = nlohmann::json{ { "CancelRepairBuilding", v.variant_union.cancel_repair_building} }; break;
        case CommandCase::GroupAttack: j = nlohmann::json{ { "GroupAttack", v.variant_union.group_attack} }; break;
        case CommandCase::GroupEnterWall: j = nlohmann::json{ { "GroupEnterWall", v.variant_union.group_enter_wall} }; break;
        case CommandCase::GroupExitWall: j = nlohmann::json{ { "GroupExitWall", v.variant_union.group_exit_wall} }; break;
        case CommandCase::GroupGoto: j = nlohmann::json{ { "GroupGoto", v.variant_union.group_goto} }; break;
        case CommandCase::GroupHoldPosition: j = nlohmann::json{ { "GroupHoldPosition", v.variant_union.group_hold_position} }; break;
        case CommandCase::GroupStopJob: j = nlohmann::json{ { "GroupStopJob", v.variant_union.group_stop_job} }; break;
        case CommandCase::ModeChange: j = nlohmann::json{ { "ModeChange", v.variant_union.mode_change} }; break;
        case CommandCase::PowerSlotBuild: j = nlohmann::json{ { "PowerSlotBuild", v.variant_union.power_slot_build} }; break;
        case CommandCase::TokenSlotBuild: j = nlohmann::json{ { "TokenSlotBuild", v.variant_union.token_slot_build} }; break;
        case CommandCase::GroupKillEntity: j = nlohmann::json{ { "GroupKillEntity", v.variant_union.group_kill_entity} }; break;
        case CommandCase::GroupSacrifice: j = nlohmann::json{ { "GroupSacrifice", v.variant_union.group_sacrifice} }; break;
        case CommandCase::PortalDefineExitPoint: j = nlohmann::json{ { "PortalDefineExitPoint", v.variant_union.portal_define_exit_point} }; break;
        case CommandCase::PortalRemoveExitPoint: j = nlohmann::json{ { "PortalRemoveExitPoint", v.variant_union.portal_remove_exit_point} }; break;
        case CommandCase::TunnelMakeExitPoint: j = nlohmann::json{ { "TunnelMakeExitPoint", v.variant_union.tunnel_make_exit_point} }; break;
        case CommandCase::Ping: j = nlohmann::json{ { "Ping", v.variant_union.ping} }; break;
        case CommandCase::Surrender: j = nlohmann::json{ { "Surrender", v.variant_union.surrender} }; break;
        case CommandCase::WhisperToMaster: j = nlohmann::json{ { "WhisperToMaster", v.variant_union.whisper_to_master} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, Command& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "BuildHouse") { v.variant_union.build_house = value.template get<CommandBuildHouse>(); v.variant_case = CommandCase::BuildHouse; }
            if (key == "CastSpellGod") { v.variant_union.cast_spell_god = value.template get<CommandCastSpellGod>(); v.variant_case = CommandCase::CastSpellGod; }
            if (key == "CastSpellGodMulti") { v.variant_union.cast_spell_god_multi = value.template get<CommandCastSpellGodMulti>(); v.variant_case = CommandCase::CastSpellGodMulti; }
            if (key == "ProduceSquad") { v.variant_union.produce_squad = value.template get<CommandProduceSquad>(); v.variant_case = CommandCase::ProduceSquad; }
            if (key == "ProduceSquadOnBarrier") { v.variant_union.produce_squad_on_barrier = value.template get<CommandProduceSquadOnBarrier>(); v.variant_case = CommandCase::ProduceSquadOnBarrier; }
            if (key == "CastSpellEntity") { v.variant_union.cast_spell_entity = value.template get<CommandCastSpellEntity>(); v.variant_case = CommandCase::CastSpellEntity; }
            if (key == "BarrierGateToggle") { v.variant_union.barrier_gate_toggle = value.template get<CommandBarrierGateToggle>(); v.variant_case = CommandCase::BarrierGateToggle; }
            if (key == "BarrierBuild") { v.variant_union.barrier_build = value.template get<CommandBarrierBuild>(); v.variant_case = CommandCase::BarrierBuild; }
            if (key == "BarrierRepair") { v.variant_union.barrier_repair = value.template get<CommandBarrierRepair>(); v.variant_case = CommandCase::BarrierRepair; }
            if (key == "BarrierCancelRepair") { v.variant_union.barrier_cancel_repair = value.template get<CommandBarrierCancelRepair>(); v.variant_case = CommandCase::BarrierCancelRepair; }
            if (key == "RepairBuilding") { v.variant_union.repair_building = value.template get<CommandRepairBuilding>(); v.variant_case = CommandCase::RepairBuilding; }
            if (key == "CancelRepairBuilding") { v.variant_union.cancel_repair_building = value.template get<CommandCancelRepairBuilding>(); v.variant_case = CommandCase::CancelRepairBuilding; }
            if (key == "GroupAttack") { v.variant_union.group_attack = value.template get<CommandGroupAttack>(); v.variant_case = CommandCase::GroupAttack; }
            if (key == "GroupEnterWall") { v.variant_union.group_enter_wall = value.template get<CommandGroupEnterWall>(); v.variant_case = CommandCase::GroupEnterWall; }
            if (key == "GroupExitWall") { v.variant_union.group_exit_wall = value.template get<CommandGroupExitWall>(); v.variant_case = CommandCase::GroupExitWall; }
            if (key == "GroupGoto") { v.variant_union.group_goto = value.template get<CommandGroupGoto>(); v.variant_case = CommandCase::GroupGoto; }
            if (key == "GroupHoldPosition") { v.variant_union.group_hold_position = value.template get<CommandGroupHoldPosition>(); v.variant_case = CommandCase::GroupHoldPosition; }
            if (key == "GroupStopJob") { v.variant_union.group_stop_job = value.template get<CommandGroupStopJob>(); v.variant_case = CommandCase::GroupStopJob; }
            if (key == "ModeChange") { v.variant_union.mode_change = value.template get<CommandModeChange>(); v.variant_case = CommandCase::ModeChange; }
            if (key == "PowerSlotBuild") { v.variant_union.power_slot_build = value.template get<CommandPowerSlotBuild>(); v.variant_case = CommandCase::PowerSlotBuild; }
            if (key == "TokenSlotBuild") { v.variant_union.token_slot_build = value.template get<CommandTokenSlotBuild>(); v.variant_case = CommandCase::TokenSlotBuild; }
            if (key == "GroupKillEntity") { v.variant_union.group_kill_entity = value.template get<CommandGroupKillEntity>(); v.variant_case = CommandCase::GroupKillEntity; }
            if (key == "GroupSacrifice") { v.variant_union.group_sacrifice = value.template get<CommandGroupSacrifice>(); v.variant_case = CommandCase::GroupSacrifice; }
            if (key == "PortalDefineExitPoint") { v.variant_union.portal_define_exit_point = value.template get<CommandPortalDefineExitPoint>(); v.variant_case = CommandCase::PortalDefineExitPoint; }
            if (key == "PortalRemoveExitPoint") { v.variant_union.portal_remove_exit_point = value.template get<CommandPortalRemoveExitPoint>(); v.variant_case = CommandCase::PortalRemoveExitPoint; }
            if (key == "TunnelMakeExitPoint") { v.variant_union.tunnel_make_exit_point = value.template get<CommandTunnelMakeExitPoint>(); v.variant_case = CommandCase::TunnelMakeExitPoint; }
            if (key == "Ping") { v.variant_union.ping = value.template get<CommandPing>(); v.variant_case = CommandCase::Ping; }
            if (key == "Surrender") { v.variant_union.surrender = value.template get<CommandSurrender>(); v.variant_case = CommandCase::Surrender; }
            if (key == "WhisperToMaster") { v.variant_union.whisper_to_master = value.template get<CommandWhisperToMaster>(); v.variant_case = CommandCase::WhisperToMaster; }
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
    enum class CommandRejectionReasonCase {
        Invalid,
        CardRejected,
        NotEnoughPower,
        SpellDoesNotExist,
        EntityDoesNotExist,
        InvalidEntityType,
        CanNotCast,
        EntityNotOwned,
        EntityOwnedBySomeoneElse,
        NoModeChange,
        EntityAlreadyInThisMode,
        ModeNotExist,
        InvalidCardIndex,
        InvalidCard
    };
    union CommandRejectionReasonUnion {
        InvalidVariant invalid_enum_variant;
        CommandRejectionReasonCardRejected card_rejected;
        CommandRejectionReasonNotEnoughPower not_enough_power;
        CommandRejectionReasonSpellDoesNotExist spell_does_not_exist;
        CommandRejectionReasonEntityDoesNotExist entity_does_not_exist;
        CommandRejectionReasonInvalidEntityType invalid_entity_type;
        CommandRejectionReasonCanNotCast can_not_cast;
        CommandRejectionReasonEntityNotOwned entity_not_owned;
        CommandRejectionReasonEntityOwnedBySomeoneElse entity_owned_by_someone_else;
        CommandRejectionReasonNoModeChange no_mode_change;
        CommandRejectionReasonEntityAlreadyInThisMode entity_already_in_this_mode;
        CommandRejectionReasonModeNotExist mode_not_exist;
        CommandRejectionReasonInvalidCardIndex invalid_card_index;
        CommandRejectionReasonInvalidCard invalid_card;
        CommandRejectionReasonUnion() { invalid_enum_variant = {}; }
        ~CommandRejectionReasonUnion() { invalid_enum_variant = {}; }
        CommandRejectionReasonUnion(const CommandRejectionReasonUnion& other) = default;
        CommandRejectionReasonUnion(CommandRejectionReasonUnion&& other) = default;
        CommandRejectionReasonUnion& operator=(const CommandRejectionReasonUnion& other) = default;
        CommandRejectionReasonUnion& operator=(CommandRejectionReasonUnion&& other) = default;
    };
    struct CommandRejectionReason {
        CommandRejectionReasonCase variant_case;
        CommandRejectionReasonUnion variant_union;
        CommandRejectionReason() {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::Invalid;
            variant_union.invalid_enum_variant = {};
        }
        CommandRejectionReason(const CommandRejectionReason& other) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = other.variant_case;
            switch (variant_case) {
            case CommandRejectionReasonCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case CommandRejectionReasonCase::CardRejected: variant_union.card_rejected = other.variant_union.card_rejected; break;
            case CommandRejectionReasonCase::NotEnoughPower: variant_union.not_enough_power = other.variant_union.not_enough_power; break;
            case CommandRejectionReasonCase::SpellDoesNotExist: variant_union.spell_does_not_exist = other.variant_union.spell_does_not_exist; break;
            case CommandRejectionReasonCase::EntityDoesNotExist: variant_union.entity_does_not_exist = other.variant_union.entity_does_not_exist; break;
            case CommandRejectionReasonCase::InvalidEntityType: variant_union.invalid_entity_type = other.variant_union.invalid_entity_type; break;
            case CommandRejectionReasonCase::CanNotCast: variant_union.can_not_cast = other.variant_union.can_not_cast; break;
            case CommandRejectionReasonCase::EntityNotOwned: variant_union.entity_not_owned = other.variant_union.entity_not_owned; break;
            case CommandRejectionReasonCase::EntityOwnedBySomeoneElse: variant_union.entity_owned_by_someone_else = other.variant_union.entity_owned_by_someone_else; break;
            case CommandRejectionReasonCase::NoModeChange: variant_union.no_mode_change = other.variant_union.no_mode_change; break;
            case CommandRejectionReasonCase::EntityAlreadyInThisMode: variant_union.entity_already_in_this_mode = other.variant_union.entity_already_in_this_mode; break;
            case CommandRejectionReasonCase::ModeNotExist: variant_union.mode_not_exist = other.variant_union.mode_not_exist; break;
            case CommandRejectionReasonCase::InvalidCardIndex: variant_union.invalid_card_index = other.variant_union.invalid_card_index; break;
            case CommandRejectionReasonCase::InvalidCard: variant_union.invalid_card = other.variant_union.invalid_card; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        CommandRejectionReason& operator=(const CommandRejectionReason& other) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = other.variant_case;
            switch (variant_case) {
            case CommandRejectionReasonCase::Invalid: throw std::runtime_error("Invalid default variant!");
            case CommandRejectionReasonCase::CardRejected: variant_union.card_rejected = other.variant_union.card_rejected; break;
            case CommandRejectionReasonCase::NotEnoughPower: variant_union.not_enough_power = other.variant_union.not_enough_power; break;
            case CommandRejectionReasonCase::SpellDoesNotExist: variant_union.spell_does_not_exist = other.variant_union.spell_does_not_exist; break;
            case CommandRejectionReasonCase::EntityDoesNotExist: variant_union.entity_does_not_exist = other.variant_union.entity_does_not_exist; break;
            case CommandRejectionReasonCase::InvalidEntityType: variant_union.invalid_entity_type = other.variant_union.invalid_entity_type; break;
            case CommandRejectionReasonCase::CanNotCast: variant_union.can_not_cast = other.variant_union.can_not_cast; break;
            case CommandRejectionReasonCase::EntityNotOwned: variant_union.entity_not_owned = other.variant_union.entity_not_owned; break;
            case CommandRejectionReasonCase::EntityOwnedBySomeoneElse: variant_union.entity_owned_by_someone_else = other.variant_union.entity_owned_by_someone_else; break;
            case CommandRejectionReasonCase::NoModeChange: variant_union.no_mode_change = other.variant_union.no_mode_change; break;
            case CommandRejectionReasonCase::EntityAlreadyInThisMode: variant_union.entity_already_in_this_mode = other.variant_union.entity_already_in_this_mode; break;
            case CommandRejectionReasonCase::ModeNotExist: variant_union.mode_not_exist = other.variant_union.mode_not_exist; break;
            case CommandRejectionReasonCase::InvalidCardIndex: variant_union.invalid_card_index = other.variant_union.invalid_card_index; break;
            case CommandRejectionReasonCase::InvalidCard: variant_union.invalid_card = other.variant_union.invalid_card; break;
            default: throw std::runtime_error("Invalid variant");
            }
        }
        CommandRejectionReason(CommandRejectionReasonCardRejected v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::CardRejected;
            variant_union.card_rejected = v;
        }
        CommandRejectionReason(CommandRejectionReasonNotEnoughPower v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::NotEnoughPower;
            variant_union.not_enough_power = v;
        }
        CommandRejectionReason(CommandRejectionReasonSpellDoesNotExist v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::SpellDoesNotExist;
            variant_union.spell_does_not_exist = v;
        }
        CommandRejectionReason(CommandRejectionReasonEntityDoesNotExist v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::EntityDoesNotExist;
            variant_union.entity_does_not_exist = v;
        }
        CommandRejectionReason(CommandRejectionReasonInvalidEntityType v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::InvalidEntityType;
            variant_union.invalid_entity_type = v;
        }
        CommandRejectionReason(CommandRejectionReasonCanNotCast v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::CanNotCast;
            variant_union.can_not_cast = v;
        }
        CommandRejectionReason(CommandRejectionReasonEntityNotOwned v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::EntityNotOwned;
            variant_union.entity_not_owned = v;
        }
        CommandRejectionReason(CommandRejectionReasonEntityOwnedBySomeoneElse v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::EntityOwnedBySomeoneElse;
            variant_union.entity_owned_by_someone_else = v;
        }
        CommandRejectionReason(CommandRejectionReasonNoModeChange v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::NoModeChange;
            variant_union.no_mode_change = v;
        }
        CommandRejectionReason(CommandRejectionReasonEntityAlreadyInThisMode v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::EntityAlreadyInThisMode;
            variant_union.entity_already_in_this_mode = v;
        }
        CommandRejectionReason(CommandRejectionReasonModeNotExist v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::ModeNotExist;
            variant_union.mode_not_exist = v;
        }
        CommandRejectionReason(CommandRejectionReasonInvalidCardIndex v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::InvalidCardIndex;
            variant_union.invalid_card_index = v;
        }
        CommandRejectionReason(CommandRejectionReasonInvalidCard v) {
            memset(this, 0, sizeof(CommandRejectionReason));
            variant_case = CommandRejectionReasonCase::InvalidCard;
            variant_union.invalid_card = v;
        }
    };
    inline void to_json(nlohmann::json& j, const CommandRejectionReason& v) {
        switch (v.variant_case) {
        case CommandRejectionReasonCase::Invalid: throw std::runtime_error("Invalid default variant!");
        case CommandRejectionReasonCase::CardRejected: j = nlohmann::json{ { "CardRejected", v.variant_union.card_rejected} }; break;
        case CommandRejectionReasonCase::NotEnoughPower: j = nlohmann::json{ { "NotEnoughPower", v.variant_union.not_enough_power} }; break;
        case CommandRejectionReasonCase::SpellDoesNotExist: j = nlohmann::json{ { "SpellDoesNotExist", v.variant_union.spell_does_not_exist} }; break;
        case CommandRejectionReasonCase::EntityDoesNotExist: j = nlohmann::json{ { "EntityDoesNotExist", v.variant_union.entity_does_not_exist} }; break;
        case CommandRejectionReasonCase::InvalidEntityType: j = nlohmann::json{ { "InvalidEntityType", v.variant_union.invalid_entity_type} }; break;
        case CommandRejectionReasonCase::CanNotCast: j = nlohmann::json{ { "CanNotCast", v.variant_union.can_not_cast} }; break;
        case CommandRejectionReasonCase::EntityNotOwned: j = nlohmann::json{ { "EntityNotOwned", v.variant_union.entity_not_owned} }; break;
        case CommandRejectionReasonCase::EntityOwnedBySomeoneElse: j = nlohmann::json{ { "EntityOwnedBySomeoneElse", v.variant_union.entity_owned_by_someone_else} }; break;
        case CommandRejectionReasonCase::NoModeChange: j = nlohmann::json{ { "NoModeChange", v.variant_union.no_mode_change} }; break;
        case CommandRejectionReasonCase::EntityAlreadyInThisMode: j = nlohmann::json{ { "EntityAlreadyInThisMode", v.variant_union.entity_already_in_this_mode} }; break;
        case CommandRejectionReasonCase::ModeNotExist: j = nlohmann::json{ { "ModeNotExist", v.variant_union.mode_not_exist} }; break;
        case CommandRejectionReasonCase::InvalidCardIndex: j = nlohmann::json{ { "InvalidCardIndex", v.variant_union.invalid_card_index} }; break;
        case CommandRejectionReasonCase::InvalidCard: j = nlohmann::json{ { "InvalidCard", v.variant_union.invalid_card} }; break;
        default: throw std::runtime_error("Invalid variant");
        }
    }
    inline void from_json(const nlohmann::json& j, CommandRejectionReason& v) {
        for (auto& [key, value] : j.items()) {
            if (key == "CardRejected") { v.variant_union.card_rejected = value.template get<CommandRejectionReasonCardRejected>(); v.variant_case = CommandRejectionReasonCase::CardRejected; }
            if (key == "NotEnoughPower") { v.variant_union.not_enough_power = value.template get<CommandRejectionReasonNotEnoughPower>(); v.variant_case = CommandRejectionReasonCase::NotEnoughPower; }
            if (key == "SpellDoesNotExist") { v.variant_union.spell_does_not_exist = value.template get<CommandRejectionReasonSpellDoesNotExist>(); v.variant_case = CommandRejectionReasonCase::SpellDoesNotExist; }
            if (key == "EntityDoesNotExist") { v.variant_union.entity_does_not_exist = value.template get<CommandRejectionReasonEntityDoesNotExist>(); v.variant_case = CommandRejectionReasonCase::EntityDoesNotExist; }
            if (key == "InvalidEntityType") { v.variant_union.invalid_entity_type = value.template get<CommandRejectionReasonInvalidEntityType>(); v.variant_case = CommandRejectionReasonCase::InvalidEntityType; }
            if (key == "CanNotCast") { v.variant_union.can_not_cast = value.template get<CommandRejectionReasonCanNotCast>(); v.variant_case = CommandRejectionReasonCase::CanNotCast; }
            if (key == "EntityNotOwned") { v.variant_union.entity_not_owned = value.template get<CommandRejectionReasonEntityNotOwned>(); v.variant_case = CommandRejectionReasonCase::EntityNotOwned; }
            if (key == "EntityOwnedBySomeoneElse") { v.variant_union.entity_owned_by_someone_else = value.template get<CommandRejectionReasonEntityOwnedBySomeoneElse>(); v.variant_case = CommandRejectionReasonCase::EntityOwnedBySomeoneElse; }
            if (key == "NoModeChange") { v.variant_union.no_mode_change = value.template get<CommandRejectionReasonNoModeChange>(); v.variant_case = CommandRejectionReasonCase::NoModeChange; }
            if (key == "EntityAlreadyInThisMode") { v.variant_union.entity_already_in_this_mode = value.template get<CommandRejectionReasonEntityAlreadyInThisMode>(); v.variant_case = CommandRejectionReasonCase::EntityAlreadyInThisMode; }
            if (key == "ModeNotExist") { v.variant_union.mode_not_exist = value.template get<CommandRejectionReasonModeNotExist>(); v.variant_case = CommandRejectionReasonCase::ModeNotExist; }
            if (key == "InvalidCardIndex") { v.variant_union.invalid_card_index = value.template get<CommandRejectionReasonInvalidCardIndex>(); v.variant_case = CommandRejectionReasonCase::InvalidCardIndex; }
            if (key == "InvalidCard") { v.variant_union.invalid_card = value.template get<CommandRejectionReasonInvalidCard>(); v.variant_case = CommandRejectionReasonCase::InvalidCard; }
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
