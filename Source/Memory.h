#ifndef MEMORY_H_
#define MEMORY_H_


/* The handle to the program's heap. */
extern HANDLE	g_hHeap;


#ifdef _DEBUG

/* Function prototypes */
void DbgMemInit(LPTSTR FileName);
LPVOID DbgMemAlloc(DWORD Size, DWORD Flags, LPSTR File, int Line);
void DbgMemFree(LPVOID Ptr, LPSTR File, int Line);
void DbgMemCleanup();

/* Aliases to the debug routines */
#define MemInit(x)		DbgMemInit(x)
#define	MemAlloc(x)		DbgMemAlloc(x, 0, __FILE__, __LINE__)
#define MemCalloc(x)	DbgMemAlloc(x, HEAP_ZERO_MEMORY, __FILE__, __LINE__)
#define MemFree(x)		DbgMemFree(x, __FILE__, __LINE__)
#define MemCleanup()	DbgMemCleanup()

#else

/* Aliases to the real memory allocation routines */
#define MemInit(x)		g_hHeap = GetProcessHeap()
#define	MemAlloc(x)		HeapAlloc(g_hHeap, 0, (x))
#define MemCalloc(x)	HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, (x))
#define MemFree(x)		HeapFree(g_hHeap, 0, (x))
#define MemCleanup()

#endif


#endif /* MEMORY_H_ */
