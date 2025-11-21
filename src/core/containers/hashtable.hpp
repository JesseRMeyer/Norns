#include "../misc/pcg.hpp"

//NOTE(Jesse): Very simple hash table with no fancy bells or whistles.
//TODO(Jesse): Rehash.  Erase.

template <typename K, typename V>
class HashTable {
public:
	HashTable(): Entries(1024), HashEntries(256) {
		BuildHashEntries(4);
	}	

	HashTable(u32 capacity): Entries(capacity), HashEntries(capacity / 4) {
		BuildHashEntries(4);
	}

	HashTable(HashTable&& other): Entries(move(other.Entries)), HashEntries(move(other.HashEntries)) {}

	template <typename U>
	V& 
	operator[](U&& key) {
		//NOTE(Jesse): The 0 hash value is reserved for tombstones (deleted KV pairs).
		u32 hash = pcg32_hash((byte*)&key, sizeof(key)) | 1u;

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

		HashEntries[bin_idx].Emplace({hash, Entries.Size()});

		return Entries.Emplace({move(forward<U>(key)), {}}).v;
	}

	HashTable&
	operator=(HashTable& other) = delete;

	HashTable&
	operator=(HashTable&& other) {
		if (this == &other) {
			return *this;
		}

		Entries = move(other.Entries);
		HashEntries = move(other.HashEntries);

		other.Entries.size = 0;
		other.HashEntries.size = 0;

		return *this;
	}

	auto begin() const {
		return &Entries[0];
	}

	auto end() const {
		return &Entries[Entries.Size()];
	}

private:
	struct HashEntry {
		u32 hash;
		u32 entry_idx;
	};

	struct Entry {
		K k;
		V v;
	};

	f32 
	GetLoadFactor() {
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
