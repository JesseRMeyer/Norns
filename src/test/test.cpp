#include "../core/core.hpp"

//TODO(Jesse): Install signal handler and test fail cases

int main() {
	StringStream logger;
	defer(logger << "All tests passed or failed successfully!");

	{
		//NOTE(Jesse): Queue uses a head/tail index pair
		// to track front and open slot.
		// Currently the slot just under the tail
		// is reserved (unusable) to determine queue vacancy.
		auto q = AtomicFixedSizeQueue<int>(2);
		q.Put(1);
		assert(q.IsFull());

		q.Pop();
		assert(q.HasRoom());
		assert(q.IsEmpty());
	}

	{
		auto ht = HashTable<i32, f32>();
		ht[4] = 4.0f;
		assert(ht[4] == 4.0f);

		i32 bob = 6;
		f32 sally = 42.0f;

		ht[bob] = sally;
		assert(ht[bob] == sally);

		auto& zero_val = ht[8];
		assert(zero_val == 0.0f);

		auto th = move(ht);
	}

	{
		auto pq = PriorityQueue<u32, f32>(128);
		pq.Put(1, 1.0f);
		pq.Put(10, 0.0f);
		pq.Put(10000, 1000.0f);
		pq.Put(100, 10.0f);
		pq.Put(1000, 100.0f);
		pq.Put(100000, 100000.0f);
		
		auto last = pq.TopPriority();
		pq.Pop();
		while (not pq.IsEmpty()) {
			assert(last > pq.TopPriority());
			last = pq.TopPriority();
			pq.Pop();
		}
		assert(pq.IsEmpty());
	}

	{
		auto pq = PriorityQueue<u32, f32, greater>(128);
		pq.Put(1, 1.0f);
		pq.Put(10, 0.0f);
		pq.Put(10000, 1000.0f);
		pq.Put(100, 10.0f);
		pq.Put(1000, 100.0f);
		pq.Put(100000, 100000.0f);

		auto last = pq.TopPriority();
		pq.Pop();
		while (not pq.IsEmpty()) {
			assert(last < pq.TopPriority());
			last = pq.TopPriority();
			pq.Pop();
		}
		assert(pq.IsEmpty());
	}

	{
		u32 bob[5] = {1, 2, 3, 4, 5};
		auto bob_slice = Slice<u32>(bob, 5);

		auto ht = HashTable<Slice<u32>, bool>();
		ht[bob_slice] = true; //NOTE(Jesse): bob_slice is MOVED
		assert(ht[bob_slice] == true);

		auto sally_slice = Slice<u32>(bob, 4);
		ht[move(sally_slice)] = true;
		//logger << sally_slice[3]; //NOTE(Jesse): This will crash because sally_slice is now invalid.

		assert(ht[Slice<u32>(bob, 5)] == true);
		assert(ht[Slice<u32>(bob, 4)] == true);
	}

	{
		assert(os::Time::Now() > 0);

		auto st = os::Time::StartTimer();
		os::Time::Second actual_sleep_duration = 0.05f;
		os::Time::Sleep(actual_sleep_duration);
		auto ns = os::Time::StopTimer(st);

		auto sleep_time = os::Time::NanoToSeconds(ns);
		assert(abs(sleep_time - actual_sleep_duration) < 0.001f);
	}

	{
		auto t = os::Thread();
		assert(not t.IsAlive());
	}

	{
		auto f = os::Futex();
		f.Lock();
		
		assert(f.TryLock() == false);

		f.Unlock();

		assert(f.TryLock() == true);

		f.Unlock();
	}

	{
		auto f = new os::Futex;
		delete f;
	}

	{
		auto f = UniquePtr<os::Futex>();
	}

	{
		auto f = SharedPtr<os::Futex>();
		assert(f.RefCount() == 1);

		auto b = f;
		assert(b.RefCount() == 2);
	}

	{
		auto a = Vector<int>(32);
		a.PushBack(4);
		assert(a.Size() == 1);

		int sally = 8;
		a.PushBack(move(sally));

		assert(a.Size() == 2);

		assert(a[0] == 4);
		assert(a[1] == 8);
	}

	return 0;
}
