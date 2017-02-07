#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"Random.h"
#include"Uid.h"
#include"resource.h"


/*
 * This is the dialog function for displaying the random bytes.
 */
static BOOL CALLBACK RandomDisplayFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HFONT	hFont;
	HDC		hDC;

	switch(Msg) {
		case WM_INITDIALOG:
			hDC = GetDC(hWnd);
			hFont = CreateFont(-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE | FIXED_PITCH, _T("Courier"));
			ReleaseDC(hWnd, hDC);
			SendDlgItemMessage(hWnd, IDC_RANDOM, WM_SETFONT, (WPARAM)hFont, FALSE);
			SetDlgItemText(hWnd, IDC_RANDOM, (LPTSTR)lParam);
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
			}
			return TRUE;
			break;
	}

	return FALSE;
}


/*
 * Get 32 random bytes from a drive and display them to the user.
 */
void Random(HWND hWnd, LPTCGDRIVE hDrive)
{
	TCGSESSION	Session;
	TCHAR		Text[200];
	TCHAR		Temp[5];
	BYTE		Result;
	BYTE		Bytes[32];
	int			i;

	/* Open a session to the admin SP. */
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, NULL);
	if(Result != 0) {
		MessageBox(hWnd, _T("There was an error opening a session to the Admin SP."), _T("Get Random Bytes"), MB_ICONERROR | MB_OK);
		return;
	}

	/* Get random bytes. */
	Result = GetRandom(&Session, 32, Bytes);

	/* Close the session. */
	EndSession(&Session);

	/* Display an error message, if needed. */
	if(Result != 0) {
		MessageBox(hWnd, _T("There was an error reading random bytes from the drive."), _T("Get Random Bytes"), MB_ICONERROR | MB_OK);
		return;
	}

	/* Convert bytes to a string. */
	Text[0] = 0;
	for(i=0; i<32; i++) {
		wsprintf(Temp, _T("%02X"), Bytes[i]);
		lstrcat(Text, Temp);
		if(((i%8) == 7) && (i != 31)) {
			lstrcat(Text, _T("\r\n"));
		} else if(i != 31) {
			lstrcat(Text, _T(" "));
		}
	}

	/* Display the random bytes. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RANDOM), hWnd, RandomDisplayFunc, (LPARAM)Text);
}
