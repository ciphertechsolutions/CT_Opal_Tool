#include"stdafx.h"
#include"Admin.h"


/* Internal admin status. */
#define ADMIN_STATUS_NORMAL			1
#define ADMIN_STATUS_ELEVATED		2
#define ADMIN_STATUS_NONELEVATED	3


/*
 * Determine whether the process is elevated.  If not, determine whether it can be elevated.
 */
static int GetAdminStatus(void)
{
	TOKEN_ELEVATION_TYPE	tet;
	TOKEN_ELEVATION			te;
	HANDLE					hToken;
	DWORD					dwReturnLength;


	/* Open a handle to our token. */
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		return ADMIN_STATUS_NORMAL;
	}

	/* Get elevation type information. */
	GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenElevationType, &tet, sizeof(tet), &dwReturnLength);
	GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)TokenElevation, &te, sizeof(te), &dwReturnLength);

	/* Close the handle to the token. */
	CloseHandle(hToken);

	/* The following can only happen if UAC is enabled and the user is an admin. */
	if(tet == TokenElevationTypeFull) {
		return ADMIN_STATUS_ELEVATED;
	}
	if(tet == TokenElevationTypeLimited) {
		return ADMIN_STATUS_NONELEVATED;
	}

	/* If we get here, either UAC is enabled and we're a normal user, or UAC is disabled. */
	if(te.TokenIsElevated) {
		return ADMIN_STATUS_ELEVATED;
	} else {
		return ADMIN_STATUS_NORMAL;
	}
}


/*
 * If we're not elevated, but can be, execute this process again at an elevated status and terminate this process.
 */
BOOL ElevateMe(void)
{
	STARTUPINFO		StartupInfo;
	DWORD			nShow;
	TCHAR			CurrentDirectory[MAX_PATH];
	TCHAR			CurrentExecutable[MAX_PATH];

	/* Get our admin status. */
	switch(GetAdminStatus()) {
		case ADMIN_STATUS_ELEVATED:
			return TRUE;				/* Already elevated. */
			break;
		case ADMIN_STATUS_NORMAL:
			return FALSE;				/* Not elevated, can't elevate. */
			break;
	}

	/* Get our startup information so we can replicate it. */
	GetStartupInfo(&StartupInfo);
	GetCurrentDirectory(MAX_PATH, CurrentDirectory);
	GetModuleFileName(NULL, CurrentExecutable, MAX_PATH);

	/* Determine the default window state. */
	if(StartupInfo.dwFlags & STARTF_USESHOWWINDOW) {
		nShow = StartupInfo.wShowWindow;
	} else {
		nShow = SW_NORMAL;
	}

	/* Execute. */
	ShellExecute(NULL, _T("runas"), CurrentExecutable, NULL, CurrentDirectory, nShow);

	/* Terminate ourselves. */
	ExitProcess(0);

	/* Make compiler happy. */
	return FALSE;
}
