#include "../core/core.hpp"

//TODO(Jesse): Install signal handler and test fail cases
#include "../game/path_finding/a_star.cpp"

int main() {
	StringStream logger;
	defer(logger << "All tests passed or failed successfully!");

	{
		auto bob = new u32[3];
		u32 sally[4];

		bob = (u32*)Norns_Realloc(bob, sizeof(sally));
		delete[] bob;
	}

	{
		u32 history[32] = {};
		PCG32Uni01 rng_ctx{};
		
		u32 num_samples = 1000000;
		for (u32 s_idx = 0; s_idx < num_samples; ++s_idx) {
			f32 sample = ((clamp(SampleStandardGaussian(rng_ctx), -4.0f, 4.0f) / 4.0f) + 1.0f) * 0.5f;
			history[u16(sample * (size(history) - 1))] += 1;
		}

		for (u16 h_idx = 0; h_idx < size(history) / 2 - 1; ++h_idx) {
			assert(history[h_idx] < history[h_idx + 1]);
		}

		for (u16 h_idx = size(history) / 2; h_idx < size(history) - 1; ++h_idx) {
			assert(history[h_idx] > history[h_idx + 1]);
		}
	}

	{
		GridCell answer[4] = {
			{3, 0}, {2, 0}, {1, 0}, {0, 0}
		};

		GridCell goal = {
			0, 0,
		};

		GridCell start = {
			3, 0,
		};

		u8 grid_memory[1][4] = {};
		Grid2D<u8> cost_grid = {(u8*)grid_memory, size(grid_memory[0]), size(grid_memory)};
		for (u16 y = 0; y < cost_grid.GetY(); ++y) {
			for (u16 x = 0; x < cost_grid.GetX(); ++x) {
				cost_grid[GridCell{x, y}] = x;
			}
		}

		assert(not cost_grid.IsValid(GridCell{-1, -1}));
		assert(not cost_grid.IsValid(GridCell{0, -1}));
		assert(not cost_grid.IsValid(GridCell{-1, 0}));
		assert(cost_grid.IsValid(GridCell{0, 0}));

		auto path = AStar(cost_grid, start, goal);
		
		assert(path[0] == start);
		assert(path.Back() == goal);

		for (u32 p_idx = 0; p_idx < path.Size(); ++p_idx) {
			assert(path[p_idx] == answer[p_idx]);
		}
	}

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
		auto v = Vector<u32>();
		v.Reverse();
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
