#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"MbrControl.h"
#include"GetUser.h"
#include"Memory.h"
#include"Uid.h"
#include"Resource.h"


/* Caption to display on error messages. */
static LPTSTR ControlCaption = _T("MBR Control");


/* The bits we examine from the locking feature descriptor. */
#define	MBR_ENABLE_BIT	0x10
#define	MBR_DONE_BIT	0x20


/* The structure to pass information to the dialog box. */
typedef struct tagDlgInfo {
	LPTCGAUTH	TcgAuth;			/* Authorization information. */
	LPTCGDRIVE	hDrive;				/* Drive on which we are modifying ranges. */
} DLGINFO, *LPDLGINFO;


/*
 * Set or clear the MBR enable value and the MBR done value.
 */
static void ChangeMbrState(HWND hWnd, LPDLGINFO DlgInfo, BOOL IsEnable, BOOL IsDone)
{
	TCGSESSION	Session;
	BYTE		Result1;
	BYTE		Result2;

	/* Start a session to the Locking SP. */
	if(StartSession(&Session, DlgInfo->hDrive, SP_LOCKING.Uid, DlgInfo->TcgAuth) != 0) {
		MessageBox(hWnd, _T("There was an error authenticating the user to the Locking SP."), ControlCaption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Set the MBR state. Some Samsung drives can't change both bits at the same time. */
	Result1 = SetMbrState(&Session, IsEnable, -1);
	Result2 = SetMbrState(&Session, -1, IsDone);

	/* If there was an error, alert the user. */
	if((Result1 != 0) || (Result2 != 0)) {
		MessageBox(hWnd, _T("There was an error setting the state of the Shadow MBR area."), ControlCaption, MB_ICONERROR | MB_OK);
	}

	/* Close the session. */
	EndSession(&Session);

}


/*
 * This is the dialog function for MBR Control.
 */
static BOOL CALLBACK MbrControlFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPDLGINFO	DlgInfo;
	DWORD		LockingBits;
	int			IsEnable;
	int			IsDone;

	switch(Msg) {
		case WM_INITDIALOG:
			/* Save off the structure for later. */
			DlgInfo = (LPDLGINFO)lParam;
			SetWindowLong(hWnd, DWL_USER, lParam);

			/* Get the MBR bits to determine the state. */
			LockingBits = GetLockingFeatures(DlgInfo->hDrive);

			/* Set the check boxes appropriately. */
			if(LockingBits & MBR_ENABLE_BIT) {
				SendDlgItemMessage(hWnd, IDC_ENABLE, BM_SETCHECK, BST_CHECKED, 0);
			}
			if(LockingBits & MBR_DONE_BIT) {
				SendDlgItemMessage(hWnd, IDC_DONE, BM_SETCHECK, BST_CHECKED, 0);
			}

			/* Return TRUE so the dialog box sets the focus to the first control. */
			return TRUE;
			break;
		case WM_SYSCOMMAND:
			switch(wParam & 0xfff0) {
				case SC_CLOSE:
					SendMessage(hWnd, WM_COMMAND, IDCANCEL, 0);
					return TRUE;
					break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					/* Get our structure. */
					DlgInfo = (LPDLGINFO)GetWindowLong(hWnd, DWL_USER);

					/* Get the state of the MBR. */
					LockingBits = GetLockingFeatures(DlgInfo->hDrive);

					/* Determine whether the check boxes are checked. */
					if(SendDlgItemMessage(hWnd, IDC_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						IsEnable = MBR_ENABLE_BIT;
					} else {
						IsEnable = 0;
					}
					if(SendDlgItemMessage(hWnd, IDC_DONE, BM_GETCHECK, 0, 0) == BST_CHECKED) {
						IsDone = MBR_ENABLE_BIT;
					} else {
						IsDone = 0;
					}

					/* For anything that hasn't changed, set the flag to -1 so we don't try to change it. */
					if((LockingBits & MBR_ENABLE_BIT) == (DWORD)IsEnable) {
						IsEnable = -1;
					}
					if((LockingBits & MBR_DONE_BIT) == (DWORD)IsDone) {
						IsDone = -1;
					}

					/* If we need to change something, do it now. */
					if((IsEnable != -1) || (IsDone != -1)) {
						ChangeMbrState(hWnd, DlgInfo, IsEnable, IsDone);
					}

					/* All done. */
					EndDialog(hWnd, 0);
					break;
				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			break;
	}

	return FALSE;
}


/*
 * Allow a user to enable/disable the MBR Shadow area, or to set/clear the Done flag.
 */
void MbrControl(HWND hWnd, LPTCGDRIVE hDrive)
{
	TCGAUTH		TcgAuth;
	DLGINFO		DlgInfo;

	/* Check that the Locking SP is enabled. */
	if(IsLockingEnabled(hDrive) == FALSE) {
		MessageBox(hWnd, _T("The Locking SP is not enabled."), ControlCaption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Get the user information. */
	if(GetUserAuthInfo(hWnd, hDrive, SP_LOCKING.Uid, NULL, ControlCaption, &TcgAuth) == FALSE) {
		return;
	}

	/* Verify the user is valid. */
	if(CheckAuth(hDrive, SP_LOCKING.Uid, &TcgAuth) == FALSE) {
		MessageBox(hWnd, _T("There was an error authenticating the user to the Locking SP."), ControlCaption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Set up the structure to pass to the dialog box. */
	DlgInfo.TcgAuth = &TcgAuth;
	DlgInfo.hDrive = hDrive;

	/* Display the dialog box. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MBRCONTROL), hWnd, MbrControlFunc, (LPARAM)&DlgInfo);
}
