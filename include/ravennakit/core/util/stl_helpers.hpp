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

#include <algorithm>

namespace rav {

template<typename Container, typename Pred>
size_t stl_remove_if(Container& container, Pred pred) {
    auto old_size = container.size();
    container.erase(std::remove_if(container.begin(), container.end(), pred), container.end());
    return old_size - container.size();
}

template<typename Map, typename Key>
auto stl_get_or_default(const Map& map, const Key& key) -> typename Map::mapped_type {
    if (auto it = map.find(key); it != map.end())
        return it->second;
    return typename Map::mapped_type{};
}

}  // namespace rav
