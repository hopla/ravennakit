/*
 * Owllab License Agreement
 *
 * This software is provided by Owllab and may not be used, copied, modified,
 * merged, published, distributed, sublicensed, or sold without a valid and
 * explicit agreement with Owllab.
 *
 * Copyright (c) 2025 Owllab. All rights reserved.
 */

#pragma once
#include "nmos_flow_audio_raw.hpp"

namespace rav::nmos {

/// Flow is a generic structure that can hold different types of flows.
/// Note: at this moment the Flow is a FlowAudioRaw, when the next supported type is added, Flow should become or hold a
/// variant or use dynamic polymorphism.
struct Flow {
    std::variant<FlowAudioRaw> any_of;

    [[nodiscard]] boost::uuids::uuid id() const {
        return std::visit(
            [](const auto& flow_variant) {
                return flow_variant.id;
            },
            any_of
        );
    }
};

inline void tag_invoke(const boost::json::value_from_tag& tag, boost::json::value& jv, const Flow& flow) {
    std::visit(
        [&tag, &jv](const auto& flow_variant) {
            tag_invoke(tag, jv, flow_variant);
        },
        flow.any_of
    );
}

}  // namespace rav::nmos
