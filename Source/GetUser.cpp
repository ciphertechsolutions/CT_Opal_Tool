#include"stdafx.h"
#include"Table.h"
#include"AtaDrive.h"
#include"Tcg.h"
#include"GetUser.h"
#include"Memory.h"
#include"Uid.h"
#include"resource.h"


typedef struct tagUserDialog {
	LPTCGAUTH	TcgAuth;
	LPTABLE		Table;
	LPTCGDRIVE	hDrive;
	LPTSTR		Title;
} USERDIALOG, *LPUSERDIALOG;


/*
 * Verify whether a character is a hex character.
 */
static BYTE ConvertHex(TCHAR Char)
{
	if((Char >= _T('0')) && (Char <= _T('9'))) return Char - _T('0');
	if((Char >= _T('a')) && (Char <= _T('f'))) return Char - _T('a') + 0x0a;
	if((Char >= _T('A')) && (Char <= _T('F'))) return Char - _T('A') + 0x0a;

	return 0xff;
}


/*
 * This is the dialog box message handler.
 */
static BOOL CALLBACK GetUserDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPUSERDIALOG	UserDlg;
	LPTABLECELL		Iter;
	LRESULT			Index;
	DWORD			TextLength;
	DWORD			i;
	TCHAR			AuthString[65];
	HWND			hWndCombo;
	BOOL			IsHex;
	BOOL			IsReverse;
	BOOL			IsMsid;
	BYTE			Temp;
	BYTE			Auth[32];
	int				NumRows;
	int				Row;
	int				j;


	switch(Msg) {
		case WM_INITDIALOG:
			UserDlg = (LPUSERDIALOG)lParam;
			SetWindowLong(hWnd, DWL_USER, (LONG)UserDlg);
			if(UserDlg->Table != NULL) {
				hWndCombo = GetDlgItem(hWnd, IDC_COMBO1);
				Index = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)_T("No User"));
				SendMessage(hWndCombo, CB_SETITEMDATA, Index, -2);
				NumRows = GetRows(UserDlg->Table);
				for(j=0; j<NumRows; j++) {
					Iter = GetTableCell(UserDlg->Table, j, 1);
					if(Iter != NULL) {
						Index = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)(Iter->Bytes));
						SendMessage(hWndCombo, CB_SETITEMDATA, Index, j);
					}
				}
				SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
			}
			if(UserDlg->Title != NULL) {
				SetWindowText(hWnd, UserDlg->Title);
			}
			return TRUE;
			break;
		case WM_SYSCOMMAND:
			switch(wParam & 0xfff0) {
				case SC_CLOSE:
					EndDialog(hWnd, -1);
					return TRUE;
					break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					UserDlg = (LPUSERDIALOG)GetWindowLong(hWnd, DWL_USER);
					IsMsid = SendDlgItemMessage(hWnd, IDC_USEMSID, BM_GETCHECK, 0, 0) == BST_CHECKED;
					if(IsMsid) {
						/* Read the MSID. */
						if(ReadMSID(UserDlg->hDrive, UserDlg->TcgAuth) == FALSE) {
							MessageBox(hWnd, _T("An error occurred reading the MSID."), NULL, MB_OK);
							break;
						}

						/* Indicate we have valid credentials. */
						UserDlg->TcgAuth->IsValid = TRUE;
					} else {
						IsHex = SendDlgItemMessage(hWnd, IDC_HEX, BM_GETCHECK, 0, 0) == BST_CHECKED;
						IsReverse = SendDlgItemMessage(hWnd, IDC_REVERSE, BM_GETCHECK, 0, 0) == BST_CHECKED;
						TextLength = SendDlgItemMessage(hWnd, IDC_PASSWORD, WM_GETTEXTLENGTH, 0, 0);
						if(IsHex && (TextLength & 1)) {
							MessageBox(hWnd, _T("The hexadecimal string must be an even length."), NULL, MB_ICONERROR | MB_OK);
							break;
						}
						if(IsHex && (TextLength > 64)) {
							MessageBox(hWnd, _T("The hexadecimal string must be at most 64 characters."), NULL, MB_ICONERROR | MB_OK);
							break;
						}
						if(!IsHex && (TextLength > 32)) {
							MessageBox(hWnd, _T("The password must be at most 32 characters."), NULL, MB_ICONERROR | MB_OK);
							break;
						}
						GetDlgItemText(hWnd, IDC_PASSWORD, AuthString, TextLength+1);

						if(IsHex) {
							for(i=0; i<TextLength; i++) {
								if(ConvertHex(AuthString[i]) == 0xff) {
									MessageBox(hWnd, _T("The hexadecimal string contains invalid characters."), NULL, MB_ICONERROR | MB_OK);
									break;
								}
							}
							if(i != TextLength) break;

							TextLength /= 2;
							for(i=0; i<TextLength; i++) {
								Auth[i] = (ConvertHex(AuthString[2*i]) << 4) | ConvertHex(AuthString[2*i+1]);
							}
						} else {
							memcpy(Auth, AuthString, lstrlen(AuthString));
						}

						if(IsReverse) {
							for(i=0; i<TextLength/2; i++) {
								Temp = Auth[i];
								Auth[i] = Auth[TextLength-i-1];
								Auth[TextLength-i-1] = Temp;
							}
						}
						UserDlg->TcgAuth->Size = TextLength;
						memcpy(UserDlg->TcgAuth->Credentials, Auth, TextLength);
					}

					if(UserDlg->Table != NULL) {
						hWndCombo = GetDlgItem(hWnd, IDC_COMBO1);
						Index = SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
						Row = SendMessage(hWndCombo, CB_GETITEMDATA, Index, 0);
					} else {
						Row = 0;
					}
					if(Row >= 0) {
						UserDlg->TcgAuth->IsValid = TRUE;
					}
					EndDialog(hWnd, Row);
					break;
				case IDCANCEL:
					EndDialog(hWnd, -1);
					break;
			}
			return TRUE;
			break;
	}

	return FALSE;
}


/*
 * Given a table of valid SPs, query the user for the SP to authenticate to.
 */
static BOOL GetUser(HWND hWndParent, LPTCGDRIVE hDrive, LPTSTR Title, LPTABLE Table, LPTCGAUTH TcgAuth)
{
	LPTABLECELL		Cell;
	USERDIALOG		UserDlg;
	int				Row;

	/* Initialize the structure to pass to the dialog box. */
	UserDlg.TcgAuth = TcgAuth;
	UserDlg.Table = Table;
	UserDlg.hDrive = hDrive;
	UserDlg.Title = Title;

	/* Query the user for the drive user, returning the row in the table of the selection. */
	Row = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GETUSER), hWndParent, GetUserDialog, (LPARAM)&UserDlg);

	/* If the row is -1, the user cancelled the selection. */
	if(Row == -1) {
		return FALSE;
	}

	/* Copy the user ID. */
	Cell = GetTableCell(Table, Row, 0);
	if(Cell != NULL) {
		memcpy(TcgAuth->Authority, Cell->Bytes, 8);
	}

	/* Return TRUE indicating a valid selection. */
	return TRUE;
}


/*
 * Get a default user table when none can be read (like early Samsung SSDs)
 */
static LPTABLE GetDefaultUserTable(LPBYTE SpUid)
{
	LPTABLE		Table;
	BYTE		Uid[8];

	/* Create an empty table. */
	Table = CreateTable();
	if(Table == NULL) {
		return NULL;
	}

	/* Handle Admin SP. */
	if(memcmp(SpUid, SP_ADMIN.Uid, 8) == 0) {
		AddCell(Table, 0, 0, sizeof(AUTHORITY_ANYBODY.Uid), AUTHORITY_ANYBODY.Uid);
		AddCell(Table, 1, 0, sizeof(AUTHORITY_ADMINS.Uid), AUTHORITY_ADMINS.Uid);
		AddCell(Table, 2, 0, sizeof(AUTHORITY_MAKERS.Uid), AUTHORITY_MAKERS.Uid);
		AddCell(Table, 3, 0, sizeof(AUTHORITY_SID.Uid), AUTHORITY_SID.Uid);
	/* Handle Locking SP. */
	} else if(memcmp(SpUid, SP_LOCKING.Uid, 8) == 0) {
		AddCell(Table, 0, 0, sizeof(AUTHORITY_ANYBODY.Uid), AUTHORITY_ANYBODY.Uid);
		AddCell(Table, 1, 0, sizeof(AUTHORITY_ADMINS.Uid), AUTHORITY_ADMINS.Uid);
		memcpy(Uid, AUTHORITY_ADMIN.Uid, sizeof(Uid));
		Uid[7] = 1;
		AddCell(Table, 2, 0, sizeof(Uid), Uid);
		AddCell(Table, 3, 0, sizeof(AUTHORITY_USERS.Uid), AUTHORITY_USERS.Uid);
		memcpy(Uid, AUTHORITY_USER.Uid, sizeof(Uid));
		Uid[7] = 1;
		AddCell(Table, 4, 0, sizeof(Uid), Uid);
		Uid[7] = 2;
		AddCell(Table, 5, 0, sizeof(Uid), Uid);
		Uid[7] = 3;
		AddCell(Table, 6, 0, sizeof(Uid), Uid);
		Uid[7] = 4;
		AddCell(Table, 7, 0, sizeof(Uid), Uid);
	}

	/* Add text descriptions. */
	AddTextDescriptions(Table);

	/* Return the table. */
	return Table;
}


/*
 * Get the User table from the SP.
 */
LPTABLE GetUserTable(LPTCGDRIVE hDrive, LPBYTE SpUid)
{
	LPTABLE		Table;

	/* Read the User table. */
	Table = ReadTableNoSession(hDrive, SpUid, TABLE_AUTHORITY.Uid, NULL);

	/* If we don't have a table, get a default one. */
	if(Table == NULL) {
		Table = GetDefaultUserTable(SpUid);
	}

	/* Return the table. */
	return Table;
}


/*
 * Get the user and the user's authorization information.
 */
BOOL GetUserAuthInfo(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Sp, LPTSTR Title, LPTSTR Caption, LPTCGAUTH TcgAuth)
{
	LPTABLE		UserTable;
	BOOL		Result;

	/* Get the list of users from the SP. */
	UserTable = GetUserTable(hDrive, Sp);
	if(UserTable == NULL) {
		MessageBox(hWndParent, _T("There was an error reading the user list from the SP."), Caption, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	/* Initialize the credentials structure. */
	TcgAuth->IsValid = FALSE;

	/* Query the user for the password. */
	Result = GetUser(hWndParent, hDrive, Title, UserTable, TcgAuth);

	/* Free up the user table - we don't need it anymore. */
	FreeTable(UserTable);

	/* Return the result. */
	return Result;
}


/*
 * Get the password from the user.
 */
BOOL GetPassword(HWND hWndParent, LPTCGDRIVE hDrive, LPTCGAUTH TcgAuth)
{
	USERDIALOG	DlgInfo;
	int			Result;

	/* Set up the structure to pass to the dialog box. */
	DlgInfo.TcgAuth = TcgAuth;
	DlgInfo.hDrive = hDrive;
	DlgInfo.Table = NULL;
	DlgInfo.Title = NULL;

	/* Initialize the credentials structure. */
	TcgAuth->IsValid = FALSE;

	/* Query the user for the new password. */
	Result = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GETPASSWORD), hWndParent, GetUserDialog, (LPARAM)&DlgInfo);

	/* If the result is -1, the user cancelled. */
	if(Result == -1) {
		return FALSE;
	}

	/* Return TRUE indicating a valid entry. */
	return TRUE;
}
