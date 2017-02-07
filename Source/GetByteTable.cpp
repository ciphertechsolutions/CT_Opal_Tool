#include"stdafx.h"
#include"Table.h"
#include"AtaDrive.h"
#include"Tcg.h"
#include"GetUser.h"
#include"GetSp.h"
#include"Memory.h"
#include"Uid.h"
#include"resource.h"


/*
 * Determine whether the table is a byte table or not.
 */
static BOOL IsByteTable(LPTABLE Table, int Row)
{
	LPTABLECELL	Cell;

	/* Find the fifth column. */
	Cell = GetTableCell(Table, Row, 4);

	/* Return whether the table is a byte table. */
	if(Cell != NULL) {
		return (Cell->IntData == 2);
	}

	/* No data was found. */
	return FALSE;
}


/*
 * This is the dialog box message handler.
 */
static BOOL CALLBACK GetByteTableDialog(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
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
				if((Iter != NULL) && IsByteTable(Table, i)) {
					Index = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)(Iter->Bytes));
					SendMessage(hWndCombo, CB_SETITEMDATA, Index, i);
				}
			}
			if(SendMessage(hWndCombo, CB_GETCOUNT, 0, 0) == 0) {
				MessageBox(hWnd, _T("There are no byte tables in this SP."), _T("Get Byte Table"), MB_OK);
				EndDialog(hWnd, -1);
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
 * Given a table of tables in an SP, query the user for the byte table to read/write.
 */
BOOL GetByteTable(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Uid, LPBYTE Sp)
{
	LPTABLECELL		Iter;
	LPTABLE			Table;
	BOOL			RetVal;
	int				Row;

	/* Query the user for the SP. */
	RetVal = GetSP(hWndParent, hDrive, Sp, _T("Get Byte Table"));

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return FALSE;
	}

	/* Open an unauthenticated session to the SP. */
	Table = ReadTableNoSession(hDrive, Sp, TABLE_TABLE.Uid, NULL);

	/* Verify the table was returned. */
	if(Table == NULL) {
		MessageBox(hWndParent, _T("There was an error reading the table of tables from the SP."), _T("Get Byte Table"), MB_ICONERROR | MB_OK);
		return FALSE;
	}

	/* Query the user for the byte table, returning the row in the table of the selection. */
	Row = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_GETBYTETABLE), hWndParent, GetByteTableDialog, (LPARAM)Table);

	/* If the row is -1, the user cancelled the selection. */
	if(Row == -1) {
		FreeTable(Table);
		return FALSE;
	}

	/* Copy the UID. */
	Iter = GetTableCell(Table, Row, 0);
	if(Iter != NULL) {
		memcpy(Uid, Iter->Bytes, 8);
	}

	/* Free resources. */
	FreeTable(Table);

	/* Return TRUE indicating a valid selection. */
	return TRUE;
}
