#include "../misc/pcg.hpp"

//TODO(Jesse): Rehash.  Erase / Deletions

//NOTE(Jesse): Currently admits pointer types but that is bug prone.. //requires (not is_pointer_v<K>) //HashTable<K, V> requires K to be a non-pointer type."
template <typename K, typename V> 
class HashTable {
public:
	struct Entry {
		K k;
		V v;
		//b32 deleted;
	};

	HashTable(): Entries(1024), HashEntries(256) {
		BuildHashEntries(4);
	}	

	HashTable(u32 capacity): Entries(capacity), HashEntries(capacity / 4) {
		BuildHashEntries(4);
	}

	HashTable(HashTable&& other): Entries(move(other.Entries)), HashEntries(move(other.HashEntries)) {}

	template <typename U>
	Entry const * const
	Find(U&& key) requires Hashable<K> or IsSliceCompatible<K> {
		u32 hash = Hash<K>{}(key) | 1u;

		u32 bin_idx = hash % HashEntries.Capacity();
		for (auto& b: HashEntries[bin_idx]) {
			if (b.hash != hash) {
				continue;
			}

			if (Entries[b.entry_idx].k != key) {
				continue;
			}

			return &Entries[b.entry_idx];
		}

		return end();
	}

	template <typename U>
	V& 
	operator[](U&& key) requires Hashable<K> or IsSliceCompatible<K> {
		//NOTE(Jesse): The 0 hash value is reserved for tombstones (deleted KV pairs).
		u32 hash = Hash<K>{}(key) | 1u;

		u32 bin_idx = hash % HashEntries.Capacity();
		for (auto& b: HashEntries[bin_idx]) {
			if (b.hash != hash) {
				continue;
			}

			if (Entries[b.entry_idx].k != key) {
				continue;
			}

			return Entries[b.entry_idx].v;
		}

		//PotentiallyReHash();

		HashEntries[bin_idx].PushBack(HashEntry{hash, Entries.Size()});

		return Entries.PushBack(Entry{forward<U>(key), {}}).v;
	}

	HashTable&
	operator=(HashTable const& other) = delete;

	HashTable&
	operator=(HashTable&& other) {
		if (this == &other) {
			return *this;
		}

		Entries = move(other.Entries);
		HashEntries = move(other.HashEntries);

		return *this;
	}

	//TODO(Jesse): Handle tombstones.  May benefit from a separate
	// Vector<bool> to track HashEntry deletions.
	// For HashEntry, the
	auto begin() {
		return &Entries[0];
	}

	//TODO(Jesse): next() const that is tombstone aware to skip deletions.

	auto end() {
		return &Entries[Entries.Size()];
	}

	inline bool
	Found(Entry const * const e) {
		return (e < end()) and (e >= begin());
	}

	inline u32 
	Size() {
		//TODO(Jesse): Handle deletions
		return Entries.Size();
	}

private:
	struct HashEntry {
		u32 hash;
		u32 entry_idx;
	};

	f32 
	GetLoadFactor() { //TODO(Jesse): Factor in tombstone count.
		return (f32)Entries.Size() / (f32)Entries.Capacity(); 
	}

	/*
	void 
	PotentiallyReHash() {
		f32 load_factor = GetLoadFactor();
		if (load_factor >= a_max) {

		} 
		else if (PassedLowWaterMark and load_factor < (a_max / 4.0f)) {

		}
	}
	*/

	void
	BuildHashEntries(u32 capacity) {
		for (u32 he_idx = 0; he_idx < HashEntries.Capacity(); ++he_idx) {
			HashEntries[he_idx] = Vector<HashEntry>(capacity);
		}
	}

	constexpr internal inline f32 a_max = 0.7f;

	Vector<Entry> Entries;
	Vector<Vector<HashEntry>> HashEntries;
	bool PassedLowWaterMark = false;
};
