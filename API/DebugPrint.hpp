#pragma once

#include <sstream>
#include "Types.hpp"

std::string Debug(const api::CommandRejectionReason& reason) {
    std::stringstream ss;
    std::visit([&](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, api::CommandRejectionReasonCardRejected>) {
                ss << "Can not play card " << std::hex << arg.reason;
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonNotEnoughPower>) {
                ss << "NotEnoughPower";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonSpellDoesNotExist>) {
                ss << "SpellDoesNotExist";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonEntityDoesNotExist>) {
                ss << "EntityDoesNotExist";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonInvalidEntityType>) {
                ss << "entity_type " << arg.entity_type << " is not valid";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonCanNotCast>) {
                ss << "CanNotCast";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonEntityNotOwned>) {
                ss << "EntityNotOwned";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonEntityOwnedBySomeoneElse>) {
                ss << "EntityOwnedBySomeoneElse";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonNoModeChange>) {
                ss << "NoModeChange";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonEntityAlreadyInThisMode>) {
                ss << "EntityAlreadyInThisMode";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonModeNotExist>) {
                ss << "ModeNotExist";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonInvalidCardIndex>) {
                ss << "InvalidCardIndex";
            }
            else if constexpr (std::is_same_v<T, api::CommandRejectionReasonInvalidCard>) {
                ss << "InvalidCard";
            }
        }, reason.v);
    ss << std::endl;
    return ss.str();
}