using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using byte = u8;

using f32 = float;
using f64 = double;

//NOTE(Jesse): Move many _language_ features only exposed through the std
// into the global namespace and treat them as language keywords.

using std::forward;
using std::move;
using std::atomic; //NOTE(Jesse): To be able to use atomic<> without std reference.
using std::nullptr_t;
using std::is_pointer_v;
using std::remove_pointer_t;
using std::is_void_v;
using std::exchange;

const f32 f32_max = FLT_MAX;
const f32 f32_min = FLT_MIN;
const f32 f32_epsilon = FLT_EPSILON;

const f64 f64_max = DBL_MAX;
const f64 f64_min = DBL_MIN;
const f64 f64_epsilon = DBL_EPSILON;

const u64 u64_max = UINT64_MAX;
const u64 u32_max = UINT32_MAX;
const u64 u16_max = UINT16_MAX;
const u64 u8_max = UINT8_MAX;

const i64 i64_max = INT64_MAX;
const i64 i32_max = INT32_MAX;
const i64 i16_max = INT16_MAX;
const i64 i8_max = INT8_MAX;

#define global static
#define internal static

#define nil 0
