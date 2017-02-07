#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"GetSp.h"
#include"GetUser.h"
#include"Memory.h"
#include"Uid.h"
#include"resource.h"


/* Caption for error messages. */
static LPTSTR	Caption = _T("Enable/Disable Users");


/* The subclass property string. */
static LPTSTR SubClassString = _T("ListView Subclass");


/* Information on each user. */
typedef struct tagUserInfo {
	LPTSTR		Name;			/* The name of the user. */
	BOOL		IsEnabled;		/* True if the user is enabled. */
	BOOL		ToEnable;		/* True if the user should be enabled. */
} USERINFO, *LPUSERINFO;


/* The structure to pass information to the dialog box. */
typedef struct tagDlgInfo {
	int			NumUsers;			/* The number of user entities. */
	LPUSERINFO	UserInfo;			/* Information for each user. */
} DLGINFO, *LPDLGINFO;


/*
 * This is the subclassed function for the list view window.
 */
static LRESULT CALLBACK NewProgressProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LVHITTESTINFO	HitTest;
	WNDPROC			OrigProc;
	BOOL			IsChecked;

	/* Get the original procedure. */
	OrigProc = (WNDPROC)GetProp(hWnd, SubClassString);

	/* Process the messages we care about. */
	switch(Msg) {
		case WM_LBUTTONDOWN:
			GetCursorPos(&HitTest.pt);
			ScreenToClient(hWnd, &HitTest.pt);
			ListView_HitTest(hWnd, &HitTest);
			if(HitTest.flags == LVHT_ONITEMLABEL) {
				IsChecked = ListView_GetCheckState(hWnd, HitTest.iItem);
				ListView_SetCheckState(hWnd, HitTest.iItem, !IsChecked);
				return 0;
			}
			break;
		case WM_DESTROY:
			RemoveProp(hWnd, SubClassString);
			SetWindowLong(hWnd, GWL_WNDPROC, (LONG)OrigProc);
			break;
		default:
			break;
	}

	/* Call the original routine. */
	return CallWindowProc(OrigProc, hWnd, Msg, wParam, lParam);
}


/*
 * This is the dialog box message handler.
 */
static BOOL CALLBACK UserFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPDLGINFO	DlgInfo;
	LVCOLUMN	Column;
	WNDPROC		OldWndProc;
	LVITEM		Item;
	RECT		Rect;
	HWND		hWndList;
	int			NumItems;
	int			Index;
	int			i;

	switch(Msg) {
		case WM_INITDIALOG:
			DlgInfo = (LPDLGINFO)lParam;
			SetWindowLong(hWnd, DWL_USER, (LONG)DlgInfo);
			hWndList = GetDlgItem(hWnd, IDC_LIST1);
			OldWndProc = (WNDPROC)GetWindowLong(hWndList, GWL_WNDPROC);
			SetProp(hWndList, SubClassString, OldWndProc);
			SetWindowLong(hWndList, GWL_WNDPROC, (LONG)NewProgressProc);
			ListView_SetExtendedListViewStyle(hWndList, LVS_EX_CHECKBOXES);
			GetClientRect(hWndList, &Rect);
			ZeroMemory(&Column, sizeof(LVCOLUMN));
			Column.mask = LVCF_WIDTH;
			Column.cx = Rect.right - GetSystemMetrics(SM_CXVSCROLL);
			ListView_InsertColumn(hWndList, 0, &Column);
			for(i=0; i<DlgInfo->NumUsers; i++) {
				if(DlgInfo->UserInfo[i].Name != NULL) {
					ZeroMemory(&Item, sizeof(LVITEM));
					Item.mask = LVIF_TEXT | LVIF_PARAM;
					Item.pszText = DlgInfo->UserInfo[i].Name;
					Item.iItem = i;
					Item.lParam = i;
					Index = SendMessage(hWndList, LVM_INSERTITEM, 0, (LPARAM)&Item);
					ListView_SetCheckState(hWndList, Index, DlgInfo->UserInfo[i].IsEnabled);
				}
			}
			return TRUE;
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
					DlgInfo = (LPDLGINFO)GetWindowLong(hWnd, DWL_USER);
					hWndList = GetDlgItem(hWnd, IDC_LIST1);
					NumItems = ListView_GetItemCount(hWndList);
					for(i=0; i<NumItems; i++) {
						ZeroMemory(&Item, sizeof(Item));
						Item.iItem = i;
						Item.mask = LVIF_PARAM;
						ListView_GetItem(hWndList, &Item);
						Index = Item.lParam;
						if(ListView_GetCheckState(hWndList, i) == 0) {
							DlgInfo->UserInfo[Index].ToEnable = FALSE;
						} else {
							DlgInfo->UserInfo[Index].ToEnable = TRUE;
						}
					}
					EndDialog(hWnd, 0);
					break;
				case IDCANCEL:
					EndDialog(hWnd, 0);
					break;
			}
			return TRUE;
			break;
	}

	return FALSE;
}


/*
 * Allow an admin to enable or disable users.
 */
void Users(HWND hWnd, LPTCGDRIVE hDrive)
{
	LPTABLECELL	Entry;
	LPTABLECELL	Iter;
	TCGSESSION	Session;
	TCGAUTH		TcgAuth;
	DLGINFO		DlgInfo;
	LPTABLE		Table;
	BOOL		RetVal;
	BOOL		ReadOne;
	BYTE		Result;
	BYTE		Sp[8];
	BOOL		MakeChanges;
	int			Rows;
	int			i;


	/* Query the user for the SP. */
	RetVal = GetSP(hWnd, hDrive, Sp, Caption);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Query the user for the drive user/admin to use. */
	RetVal = GetUserAuthInfo(hWnd, hDrive, Sp, NULL, Caption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Get the user table. */
	Table = GetUserTable(hDrive, Sp);

	/* Check that we received a valid table. */
	if(Table == NULL) {
		MessageBox(hWnd, _T("There was an error reading the authority table."), Caption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Start a session to the SP. */
	Result = StartSession(&Session, hDrive, Sp, &TcgAuth);

	/* If there was an error, notify the user. */
	if(Result != 0) {
		MessageBox(hWnd, _T("There was an error authenticating to the SP."), Caption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Determine the number of rows in the table. */
	Rows = GetRows(Table);

	/* Get the enable/disable state for each user. */
	ReadOne = FALSE;
	for(i=0; i<Rows; i++) {
		Entry = GetTableCell(Table, i, 0);
		Entry = ReadTableCell(&Session, Entry->Bytes, 5);
		if(Entry == NULL) {
			AddCell(Table, i, 5, 0, NULL);
		} else {
			AddCellToTable(Table, Entry, i);
			ReadOne = TRUE;
		}
	}

	/* End the session. */
	EndSession(&Session);

	/* If we weren't able to read everything, tell the user. */
	if(ReadOne == FALSE) {
		MessageBox(hWnd, _T("Unable to read the entire authority table. Try authenticating as a different authority."), Caption, MB_ICONERROR | MB_OK);
		FreeTable(Table);
		return;
	}

	/* Initialize the structure to pass to the dialog box. */
	DlgInfo.NumUsers = GetRows(Table);
	DlgInfo.UserInfo = (LPUSERINFO)MemAlloc(DlgInfo.NumUsers * sizeof(USERINFO));
	if(DlgInfo.UserInfo == NULL) {
		MessageBox(hWnd, _T("Out of resources - Not enough memory."), Caption, MB_ICONERROR | MB_OK);
		FreeTable(Table);
		return;
	}

	/* Initialize the user information. */
	for(i=0; i<DlgInfo.NumUsers; i++) {
		Iter = GetTableCell(Table, i, 1);
		if(Iter != NULL) {
			Entry = GetTableCell(Table, i, 5);
			DlgInfo.UserInfo[i].Name = (LPTSTR)Iter->Bytes;
			DlgInfo.UserInfo[i].IsEnabled = FALSE;
			if((Entry != NULL) && (Entry->IntData != 0)) {
				DlgInfo.UserInfo[i].IsEnabled = TRUE;
			}
		} else {
			DlgInfo.UserInfo[i].Name = NULL;
			DlgInfo.UserInfo[i].IsEnabled = FALSE;
		}
		DlgInfo.UserInfo[i].ToEnable = DlgInfo.UserInfo[i].IsEnabled;
	}

	/* Display the dialog box. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_USERS), hWnd, UserFunc, (LPARAM)&DlgInfo);

	/* Determine whether we need to make changes. */
	MakeChanges = FALSE;
	for(i=0; i<DlgInfo.NumUsers; i++) {
		if(DlgInfo.UserInfo[i].IsEnabled != DlgInfo.UserInfo[i].ToEnable) {
			MakeChanges = TRUE;
			break;
		}
	}

	/* Apply changes, if any. */
	while(MakeChanges != FALSE) {
		Result = StartSession(&Session, hDrive, Sp, &TcgAuth);
		if(Result != 0) {
			MessageBox(hWnd, _T("There was an error authenticating to the SP."), Caption, MB_ICONERROR | MB_OK);
			break;
		}
		for(i=0; i<DlgInfo.NumUsers; i++) {
			if(DlgInfo.UserInfo[i].IsEnabled != DlgInfo.UserInfo[i].ToEnable) {
				Entry = GetTableCell(Table, i, 0);
				Result = ChangeUserState(&Session, Entry->Bytes, DlgInfo.UserInfo[i].ToEnable);
				if(Result != 0) {
					MessageBox(hWnd, _T("There was an error enabling or disabling a user."), Caption, MB_ICONERROR | MB_OK);
					break;
				}
			}
		}
		EndSession(&Session);
		MakeChanges = FALSE;
	}

	/* Free resources. */
	MemFree(DlgInfo.UserInfo);
	FreeTable(Table);
}
