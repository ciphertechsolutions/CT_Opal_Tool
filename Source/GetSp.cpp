#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"GetSp.h"
#include"Uid.h"
#include"resource.h"


/*
 * This is the dialog box message handler.
 */
static BOOL CALLBACK GetSpDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPTABLECELL	Iter;
	LPTABLE		Table;
	LRESULT		Index;
	HWND		hWndCombo;
	int			NumRows;
	int			Row;
	int			i;


	switch(Msg) {
		case WM_INITDIALOG:
			Table = (LPTABLE)lParam;
			hWndCombo = GetDlgItem(hWnd, IDC_COMBO1);
			NumRows = GetRows(Table);
			for(i=0; i<NumRows; i++) {
				Iter = GetTableCell(Table, i, 1);
				if(Iter != NULL) {
					Index = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)(Iter->Bytes));
					SendMessage(hWndCombo, CB_SETITEMDATA, Index, i);
				}
			}
			SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
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
					hWndCombo = GetDlgItem(hWnd, IDC_COMBO1);
					Index = SendMessage(hWndCombo, CB_GETCURSEL, 0, 0);
					Row = SendMessage(hWndCombo, CB_GETITEMDATA, Index, 0);
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
 * Create a default SP table for Opal drives.  This is needed for Samsung SSD drives that
 * don't implement the Next method, and so the ReadTable function fails.
 */
static LPTABLE GetDefaultSpTable(void)
{
	LPTABLE		Table;

	/* Create an empty table. */
	Table = CreateTable();
	if(Table == NULL) {
		return NULL;
	}

	/* Add the entries. */
	AddCell(Table, 0, 0, sizeof(SP_ADMIN.Uid), SP_ADMIN.Uid);
	AddCell(Table, 0, 1, lstrlen(SP_ADMIN.Description), (LPBYTE)SP_ADMIN.Description);
	AddCell(Table, 1, 0, sizeof(SP_LOCKING.Uid), SP_LOCKING.Uid);
	AddCell(Table, 1, 1, lstrlen(SP_LOCKING.Description), (LPBYTE)SP_LOCKING.Description);

	/* Return the table. */
	return Table;
}


/*
 * Get the SP table from the Admin SP.
 */
LPTABLE GetSpTable(LPTCGDRIVE hDrive)
{
	LPTABLE		Table;

	/* Read the SP table. */
	Table = ReadTableNoSession(hDrive, SP_ADMIN.Uid, TABLE_SP.Uid, NULL);

	/* If there was no table read, get the default one. */
	if(Table == NULL) {
		Table = GetDefaultSpTable();
	}

	/* Return the table. */
	return Table;
}


/*
 * Given a table of valid SPs, query the user for the SP to authenticate to.
 */
BOOL GetSP(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Sp, LPTSTR Caption)
{
	LPTABLECELL	Iter;
	LPTABLE		Table;
	int			Row;

	/* Get the list of SPs. */
	Table = GetSpTable(hDrive);

	/* Verify the table was read. */
	if(Table == NULL) {
		MessageBox(hWndParent, _T("There was an error reading the table of tables."), Caption, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	/* Query the user for the SP, returning the row in the table of the selection. */
	Row = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GETSP), hWndParent, GetSpDialog, (LPARAM)Table);

	/* If the row is -1, the user cancelled the selection. */
	if(Row == -1) {
		FreeTable(Table);
		return FALSE;
	}

	/* Copy the SP UID. */
	Iter = GetTableCell(Table, Row, 0);
	if(Iter != NULL) {
		memcpy(Sp, Iter->Bytes, 8);
	}

	/* Free the table. */
	FreeTable(Table);

	/* Return TRUE indicating a valid selection. */
	return TRUE;
}
