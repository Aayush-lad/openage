// Copyright 2017-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <iostream>
#include <optional>
#include <unordered_map>

#include "curve/map_filter_iterator.h"
#include "time/time.h"


namespace openage::curve {

/**
 * Map that keeps track of the lifetime of the contained elements.
 * Make sure that no key is reused.
 */
template <typename key_t, typename val_t>
class UnorderedMap {
	/** Internal container to access all data and metadata */
	struct map_element {
		map_element(const val_t &v, const time::time_t &a, const time::time_t &d) :
			value(v),
			alive(a),
			dead(d) {}

		val_t value;
		time::time_t alive;
		time::time_t dead;
	};

	/**
	 * Data holder. Maps keys to map elements.
	 * Map elements themselves store when they are valid.
	 */
	std::unordered_map<key_t, map_element> container;

public:
	using const_iterator = typename std::unordered_map<key_t, map_element>::const_iterator;

	std::optional<MapFilterIterator<key_t, val_t, UnorderedMap>>
	operator()(const time::time_t &, const key_t &) const;

	std::optional<MapFilterIterator<key_t, val_t, UnorderedMap>>
	at(const time::time_t &, const key_t &) const;

	MapFilterIterator<key_t, val_t, UnorderedMap>
	begin(const time::time_t &e = std::numeric_limits<time::time_t>::max()) const;

	MapFilterIterator<key_t, val_t, UnorderedMap>
	end(const time::time_t &e = std::numeric_limits<time::time_t>::max()) const;

	MapFilterIterator<key_t, val_t, UnorderedMap>
	insert(const time::time_t &birth, const key_t &, const val_t &);

	MapFilterIterator<key_t, val_t, UnorderedMap>
	insert(const time::time_t &birth, const time::time_t &death, const key_t &key, const val_t &value);

	MapFilterIterator<key_t, val_t, UnorderedMap>
	between(const time::time_t &start, const time::time_t &to) const;

	void birth(const time::time_t &, const key_t &);
	void birth(const time::time_t &,
	           const MapFilterIterator<val_t, val_t, UnorderedMap> &);

	void kill(const time::time_t &, const key_t &);
	void kill(const time::time_t &,
	          const MapFilterIterator<val_t, val_t, UnorderedMap> &);

	// remove all dead elements before that point in time
	void clean(const time::time_t &);

	/**
	 * gdb helper method.
	 */
	void dump() {
		for (auto i : container) {
			std::cout << "Element: " << i.second.value << std::endl;
		}
	}
};

template <typename key_t, typename val_t>
std::optional<MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>>
UnorderedMap<key_t, val_t>::operator()(const time::time_t &time,
                                       const key_t &key) const {
	return this->at(time, key);
}

template <typename key_t, typename val_t>
std::optional<MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>>
UnorderedMap<key_t, val_t>::at(const time::time_t &time,
                               const key_t &key) const {
	auto e = this->container.find(key);
	if (e != this->container.end() and e->second.alive <= time and e->second.dead > time) {
		return MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>(
			e,
			this,
			time,
			std::numeric_limits<time::time_t>::max());
	}
	else {
		return {};
	}
}

template <typename key_t, typename val_t>
MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>
UnorderedMap<key_t, val_t>::begin(const time::time_t &time) const {
	return MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>(
		this->container.begin(),
		this,
		time,
		std::numeric_limits<time::time_t>::max());
}

template <typename key_t, typename val_t>
MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>
UnorderedMap<key_t, val_t>::end(const time::time_t &time) const {
	return MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>(
		this->container.end(),
		this,
		-std::numeric_limits<time::time_t>::max(),
		time);
}

template <typename key_t, typename val_t>
MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>
UnorderedMap<key_t, val_t>::between(const time::time_t &from, const time::time_t &to) const {
	auto it = MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>(
		this->container.begin(),
		this,
		from,
		to);

	if (!it.valid()) {
		++it;
	}
	return it;
}

template <typename key_t, typename val_t>
MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>
UnorderedMap<key_t, val_t>::insert(const time::time_t &alive,
                                   const key_t &key,
                                   const val_t &value) {
	return this->insert(
		alive,
		std::numeric_limits<time::time_t>::max(),
		key,
		value);
}

template <typename key_t, typename val_t>
MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>
UnorderedMap<key_t, val_t>::insert(const time::time_t &alive,
                                   const time::time_t &dead,
                                   const key_t &key,
                                   const val_t &value) {
	map_element e(value, alive, dead);
	auto it = this->container.insert(std::make_pair(key, e));
	return MapFilterIterator<key_t, val_t, UnorderedMap<key_t, val_t>>(
		it.first,
		this,
		alive,
		dead);
}

template <typename key_t, typename val_t>
void UnorderedMap<key_t, val_t>::birth(const time::time_t &time,
                                       const key_t &key) {
	auto it = this->container.find(key);
	if (it != this->container.end()) {
		it->second.alive = time;
	}
}

template <typename key_t, typename val_t>
void UnorderedMap<key_t, val_t>::birth(const time::time_t &time,
                                       const MapFilterIterator<val_t, val_t, UnorderedMap> &it) {
	it->second.alive = time;
}

template <typename key_t, typename val_t>
void UnorderedMap<key_t, val_t>::kill(const time::time_t &time,
                                      const key_t &key) {
	auto it = this->container.find(key);
	if (it != this->container.end()) {
		it->second.dead = time;
	}
}

template <typename key_t, typename val_t>
void UnorderedMap<key_t, val_t>::kill(const time::time_t &time,
                                      const MapFilterIterator<val_t, val_t, UnorderedMap> &it) {
	it->second.dead = time;
}

template <typename key_t, typename val_t>
void UnorderedMap<key_t, val_t>::clean(const time::time_t &) {
	// TODO save everything to a file and be happy.
}

} // namespace openage::curve
