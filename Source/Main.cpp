#include"stdafx.h"
#include"Memory.h"
#include"Admin.h"
#include"TcgExplorer.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	/* Make sure we're admin. */
	if(ElevateMe() == FALSE) {
		MessageBox(NULL, _T("You must be an administrator to run this program."), _T("TCG Explorer"), MB_OK | MB_ICONERROR);
		return 0;
	}

	/* Initialize common controls. */
	InitCommonControls();

	/* Initialize memory. */
	MemInit(NULL);

	/* Start the explorer. */
	TcgExplorer();

	/* Cleanup. */
	MemCleanup();

	return 0;
}
