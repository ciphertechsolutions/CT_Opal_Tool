#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"Msid.h"
#include"Memory.h"
#include"Resource.h"


/* The caption for error messages. */
static LPTSTR	Caption = _T("MSID");

/*
 * This is the dialog function for displaying the MSID for the drive.
 */
static BOOL CALLBACK MsidDisplayFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL		hMem;
	LPTSTR		Text;
	DWORD		TextSize;

	switch(Msg) {
		case WM_INITDIALOG:
			SetDlgItemText(hWnd, IDC_MSID, (LPTSTR)lParam);
			return TRUE;
			break;
		case WM_CTLCOLORSTATIC:
			return (BOOL)GetSysColorBrush(COLOR_WINDOW);
			break;
		case WM_SYSCOMMAND:
			switch(wParam & 0xfff0) {
				case SC_CLOSE:
					EndDialog(hWnd, 0);
					return TRUE;
					break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					EndDialog(hWnd, 0);
					break;
				case ID_CLIPBOARD:
					TextSize = SendDlgItemMessage(hWnd, IDC_MSID, WM_GETTEXTLENGTH, 0, 0) + 1;
					hMem = GlobalAlloc(GMEM_MOVEABLE, TextSize * sizeof(TCHAR));
					if(OpenClipboard(NULL) == FALSE) {
						MessageBox(hWnd, _T("Unable to copy MSID to the Clipboard."), _T("Clipboard Error"), MB_ICONERROR | MB_OK);
						GlobalFree(hMem);
					}
					EmptyClipboard();
					Text = (LPTSTR)GlobalLock(hMem);
					SendDlgItemMessage(hWnd, IDC_MSID, WM_GETTEXT, TextSize, (LPARAM)Text);
					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT, hMem);
					CloseClipboard();
					break;
			}
			return TRUE;
			break;
	}

	return FALSE;
}


/*
 * Get the MSID for the drive and display it to the user.
 */
void GetMSID(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGAUTH	TcgAuth;
	LPTSTR	MSID[33];

	/* Read the MSID. */
	if(ReadMSID(hDrive, &TcgAuth) == FALSE) {
		MessageBox(hWndParent, _T("An error occurred reading the MSID."), Caption, MB_OK);
		return;
	}

	/* Convert it to a nul-terminated string. */
	memset(MSID, 0, sizeof(MSID));
	memcpy(MSID, TcgAuth.Credentials, TcgAuth.Size);

	/* Display the MSID. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MSID), hWndParent, MsidDisplayFunc, (LPARAM)MSID);
}
