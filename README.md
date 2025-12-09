A* Path tracing over a field of gaussian noise to the center pixel.

![AStar](https://github.com/JesseRMeyer/Norns/raw/main/media/ducks_webm.mp4)

The purpose of this project was to learn common techniques used in
so called "Modern C++" codebases, including but not limited to:

Ownership models with reference counted pointers (UniquePtr, SharedPtr, etc).

Template / Generic programming for common data structures and algorithms.

Move semantics with rvalue types.

Inheritance modeling.

Implementing a simpler alternative to STL, currently provided, in various stages of completeness:
* HashTable
* Vector
* Queue
* Heap
* PriorityQueue
* PCG random number generator (used as hashtable hashing function)
* Futex
* Threads
* Slice
* Window Message Queue
* Time

Cross Platform - Tests pass for both Linux and Win32.

Concepts, used with HashTable for sensible hashing defaults for common types.

Simple build system.  Just call the build script for your platform and it's done. 
If the linker yells at you for missing a library, install it through your package manager. 
Use `./build.sh optimize` to enable optimizations.

Tests are always ran as part of the build step, with address and undefined sanitizers enabled.

There is a "core" namespace that reimagines parts of the STL, as needed, as the project grew.
It's somewhat opinionated in a few ways:
	Global availablility so code files don't need to #include the bits and pieces they need.
	Folders within those directories, particularly in os, house cross-platform implementations if necessary.

The public facing api are provided as loose .hpp files under either containers, os, or misc directories.

new and delete (and friends) have been overloaded to use a simple TLSF allocator with some a/tsan integration.

Since many modern C++ features are actually exposed by the standard library (like forward and move),
these have been imported into the global namespace and treated as language keywords.

Third party libraries are included under the third_party directory.

A "game" is being written in the game directory that acts as a kind of holistic test. It's
cross platform, uses a client-server architecture where the platform message queue is
ran on a separate thread than the game thread.  The game thread will feature multithreaded A* path finding, where you are a Momma Duck escorting a dozen ducklings across a map to safety navigate
currents of flowing water.  The 2D renderer will be multithreaded in software on the CPU.

--

A few common or more recent ideas not explored deeply, if at all:

Exceptions.  They are disabled at the compiler flag level. 
While I do have serious reservations about the use of exceptions for the purpose of handling 
expected error cases (an all too common use for them in practice), 
this was ultimately decided to simplify the scope of the project.
In principle the entire codebase is Exception Safe.

Modules.  These complicated the build system significantly and didn't
appear to address any actual problems in the architecture or development
of the project.  Compilation times are already very fast due to very conservative
use of STL, as well as a "Unity" organizational structure that keeps linking times down to a minimum.

Coroutines.

Const correctness.  Tho const is used in a few situations where it's vital, like an iterator returned from a HashTable's Find() of a key/value pair as mutating the key would be disasterous.

--

I also used this as an oppertunity to integrate LLMs more deeply into the development process.
See the llm.txt file for details.

