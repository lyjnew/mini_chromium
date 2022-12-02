#if defined(_MSC_VER)

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shlwapi.lib ")
#pragma comment(lib, "propsys.lib ")
#pragma comment(lib, "setupapi.lib ")
#pragma comment(lib, "powrprof.lib ")
#pragma comment(lib, "userenv.lib ")  // For CreateEnvironmentBlock
#pragma comment(lib, "ntdll.lib")  // For RtlGetVersion

#endif  // defined(_MSC_VER)
