#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)