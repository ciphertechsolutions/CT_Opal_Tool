#include"stdafx.h"
#include"Table.h"
#include"GetTable.h"
#include"resource.h"


/*
 * This is the dialog box message handler.
 */
static BOOL CALLBACK GetTableDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
 * Query the user for the table to display.
 */
BOOL GetTable(HWND hWndParent, LPTABLE Table, LPBYTE TableUid)
{
	LPTABLECELL	Cell;
	int			Row;

	/* Query the user for the table, returning the row in the table of the selection. */
	Row = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GETTABLE), hWndParent, GetTableDialog, (LPARAM)Table);

	/* If the row is -1, the user cancelled the selection. */
	if(Row == -1) {
		return FALSE;
	}

	/* Copy the table UID. */
	Cell = GetTableCell(Table, Row, 0);
	if(Cell != NULL) {
		memcpy(TableUid, Cell->Bytes, 8);
	}

	/* Return TRUE indicating a valid selection. */
	return TRUE;
}
