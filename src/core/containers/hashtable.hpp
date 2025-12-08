#include "../misc/pcg.hpp"

//TODO(Jesse): Erase / Deletions

//NOTE(Jesse): Currently admits pointer types but that is bug prone.. //requires (not is_pointer_v<K>) //HashTable<K, V> requires K to be a non-pointer type."
template <typename K, typename V> 
class HashTable {
public:
	struct Entry {
		K k;
		V v;
		//b32 deleted;
	};

	HashTable(): Entries(1024), HashEntries(1024 / 16, 1024 / 16) {}	
	HashTable(u32 capacity): Entries(capacity), HashEntries(capacity / 16, capacity / 16) {}
	HashTable(HashTable&& other): Entries(move(other.Entries)), HashEntries(move(other.HashEntries)) {}

	template <typename U>
	Entry const * const
	Find(U&& key) requires Hashable<K> or IsSliceCompatible<K> {
		u32 hash = Hash<K>{}(key);
		hash = hash == 0 ? 1 : hash;

		u32 bin_idx = hash % HashEntries.Capacity();
		auto& hash_bin = HashEntries[bin_idx];
		for (auto& b: hash_bin) {
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
		if (unlikely(LoadFactor() > LoadFactorMax)) {
			ReHash();
		}

		//NOTE(Jesse): The 0 hash value is reserved for tombstones (deleted KV pairs).
		u32 hash = Hash<K>{}(key);
		hash = hash == 0 ? 1 : hash;

		u32 bin_idx = hash % HashEntries.Capacity();
		auto& hash_bin = HashEntries[bin_idx];
		for (auto& b: hash_bin) {
			if (b.hash != hash) {
				continue;
			}

			if (Entries[b.entry_idx].k != key) {
				continue;
			}

			return Entries[b.entry_idx].v;
		}

		hash_bin.EmplaceBack(hash, Entries.Size());

		return Entries.EmplaceBack(forward<U>(key), V{}).v;
	}

	f32 inline
	LoadFactor() {
		return (f32)Entries.Size() / (f32)Entries.Capacity();
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

	auto begin() const { //TODO(Jesse): Handle deletions
		return &Entries[0];
	}

	auto end() const { //TODO(Jesse): Handle deletions
		return &Entries[0] + Entries.Size();
	}

	inline bool
	Found(Entry const * const e) {
		return (e >= begin()) and (e < end());
	}

	inline u32 
	Size() { //TODO(Jesse): Handle deletions
		return Entries.Size();
	}

private:
	struct HashEntry {
		u32 hash = 0;
		u32 entry_idx = 0;
	};

	f32 inline
	GetLoadFactor() { //TODO(Jesse): Factor in tombstone count.
		return (f32)Entries.Size() / (f32)Entries.Capacity(); 
	}

	no_inline()
	void 
	ReHash() {
		Vector<Vector<HashEntry>> NewHashEntries(HashEntries.Size() * 2, HashEntries.Size() * 2);
		for (auto& v_he: HashEntries) {
			for (auto& he: v_he) {
				u32 new_bin = he.hash % NewHashEntries.Capacity();
				NewHashEntries[new_bin].PushBack(move(he));
			}
		}

		HashEntries = move(NewHashEntries);
		Entries.Grow();
	}

	constexpr internal inline f32 LoadFactorMax = 0.7f;

	Vector<Entry> Entries;
	Vector<Vector<HashEntry>> HashEntries;
};
