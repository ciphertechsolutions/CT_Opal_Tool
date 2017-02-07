#include"stdafx.h"
#include"Memory.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"TableDisplay.h"
#include"Uid.h"
#include"GetSp.h"
#include"GetUser.h"
#include"GetTable.h"
#include"resource.h"


typedef struct tagDisplayInfo {
	LPTABLE		Table;
	LPTSTR		Title;
	LPTSTR		*ColumnHeadings;
} DISPLAYINFO, *LPDISPLAYINFO;

/* The subclass property string. */
static LPTSTR SubClassString = _T("List Subclass");

/* The caption string to display on error messages. */
static LPTSTR Caption = _T("Table Display");


/*
 * This is my custom window drawing program.
 */
static LRESULT DrawWindow(HWND hWnd, HDC hDC)
{
	PAINTSTRUCT		ps;
	HBITMAP			hBitmapOld;
	HBITMAP			hBitmap;
	HFONT			hFontOld;
	HFONT			hFont;
	RECT			ClientRect;
	HDC				hMemDC;

	/* Start the painting process. */
	BeginPaint(hWnd, &ps);

	/* Use the specified device context, or the default one. */
	if(hDC == NULL) {
		hDC= ps.hdc;
	}

	/* Get the size of the window's client area. */
	GetClientRect(hWnd, &ClientRect);

	/* Create a memory device context, a new bitmap, and select it into the context. */
	hMemDC = CreateCompatibleDC(hDC);
	hBitmap = CreateCompatibleBitmap(hDC, ClientRect.right, ClientRect.bottom);
	hBitmapOld = (HBITMAP)SelectObject(hMemDC, hBitmap);

	/* Get the font. */
	hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)SelectObject(hMemDC, hFont);

	/* Erase the background. */
	FillRect(hMemDC, &ClientRect, GetSysColorBrush(COLOR_WINDOW));

	/* Draw the text. */
	DrawText(hMemDC, _T("Empty Table"), -1, &ClientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	/* Draw the bitmap on the device context. */
	BitBlt(hDC, 0, 0, ClientRect.right, ClientRect.bottom, hMemDC, 0, 0, SRCCOPY);

	/* Clean up and remove our objects. */
	SelectObject(hMemDC, hBitmapOld);
	SelectObject(hMemDC, hFontOld);
	DeleteObject(hBitmap);
	DeleteObject(hMemDC);

	/* End the paiting process. */
	EndPaint(hWnd, &ps);

	/* Return the result. */
	return 0;
}


/*
 * This is the subclassed function for the list view window.
 */
static LRESULT CALLBACK NewListProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC		OrigProc;

	/* Get the original procedure. */
	OrigProc = (WNDPROC)GetProp(hWnd, SubClassString);

	/* Process the messages we care about. */
	switch(Msg) {
		case WM_PAINT:
			return DrawWindow(hWnd, (HDC)wParam);
			break;
		case WM_ERASEBKGND:
			return 0;
			break;
		case WM_SIZE:
		case WM_SIZING:
			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
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
 * Get the coordinates of the client area in screen coordinates.
 */
static void GetClientWindowRect(HWND hWnd, LPRECT Rect)
{
	POINT	Point;

	/* Get the client area. */
	GetClientRect(hWnd, Rect);

	/* Convert the upper left coordinate. */
	Point.x = Rect->left;
	Point.y = Rect->top;
	ClientToScreen(hWnd, &Point);
	Rect->left = Point.x;
	Rect->top = Point.y;

	/* Convert the lower right coordinate. */
	Point.x = Rect->right;
	Point.y = Rect->bottom;
	ClientToScreen(hWnd, &Point);
	Rect->right = Point.x;
	Rect->bottom = Point.y;
}


/*
 * End the dialog and clean up.
 */
static void TerminateDialog(HWND hWnd)
{
	MemFree((LPBYTE)GetWindowLong(hWnd, DWL_USER));
	EndDialog(hWnd, 0);
}


/*
 * This is the dialog function for displaying a table.
 */
static BOOL CALLBACK TableDisplayFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPDISPLAYINFO	DisplayInfo;
	LPTABLECELL		Iter;
	LVCOLUMN		Column;
	WNDPROC			OldWndProc;
	LPTABLE			Table;
	LVITEM			Item;
	LPRECT			Margins;
	LPTSTR			DynamicText;
	TCHAR			Text[100];
	LONG			Style;
	RECT			ClientRect;
	RECT			Rect;
	HWND			hWndList;
	HWND			hWndOk;
	int				Width;
	int				Rows, Cols;
	int				Top, Left;
	int				i, j, k;


	switch(Msg) {
		case WM_INITDIALOG:
			DisplayInfo = (LPDISPLAYINFO)lParam;
			SetWindowText (hWnd, DisplayInfo->Title);
			Table = DisplayInfo->Table;
			Rows = GetRows(Table);
			Cols = GetCols(Table);
			hWndList = GetDlgItem(hWnd, IDC_TABLELIST);

			GetWindowRect(hWndList, &ClientRect);
			GetClientWindowRect(hWnd, &Rect);
			Margins = (LPRECT)MemAlloc(sizeof(RECT));
			Margins->bottom = Rect.bottom - ClientRect.bottom;
			Margins->top = ClientRect.top - Rect.top;
			Margins->left = ClientRect.left - Rect.left;
			Margins->right = Rect.right - ClientRect.right;
			SetWindowLong(hWnd, DWL_USER, (LONG)Margins);

			GetClientRect(hWndList, &ClientRect);
			Width = ClientRect.right - ClientRect.left;
			if((Cols == 0) || (Rows == 0)) {
				/* Display an empty table, subclass the list window. */
				OldWndProc = (WNDPROC)GetWindowLong(hWndList, GWL_WNDPROC);
				SetProp(hWndList, SubClassString, OldWndProc);
				SetWindowLong(hWndList, GWL_WNDPROC, (LONG)NewListProc);
				Style = GetWindowLong(hWndList, GWL_STYLE);
				Style |= LVS_NOCOLUMNHEADER;
				SetWindowLong(hWndList, GWL_STYLE, Style);
			}
			SendMessage(hWndList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
			/* Create header with columns. We create one extra to delete later, because windows does not
			   let you format the first column. */
			if(Cols != 0) {
				for(i=0; i<=Cols; i++) {
					memset(&Column, 0, sizeof(Column));
					if((i==0) || (DisplayInfo->ColumnHeadings == NULL)) {
						wsprintf(Text, _T("%d"), i);
						Column.pszText = Text;
					} else {
						Column.pszText = DisplayInfo->ColumnHeadings[i-1];
					}
					Column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
					Column.fmt = LVCFMT_CENTER;
					Column.cx = Width/Cols;
					if(i == Cols) {
						Column.cx += Width % Cols;
					}
					SendMessage(hWndList, LVM_INSERTCOLUMN, i, (LPARAM)&Column);
				}
				SendMessage(hWndList, LVM_DELETECOLUMN, 0, 0);
			}

			/* Create rows. */
			memset(&Item, 0, sizeof(Item));
			for(i=0; i<Rows; i++) {
				Item.iItem = i;
				SendMessage(hWndList, LVM_INSERTITEM, 0, (LPARAM)&Item);
			}

			/* Insert each table element. */
			for(i=0; i<Rows; i++) {
				for(j=0; j<Cols; j++) {
					Iter = GetTableCell(Table, i, j);
					if(Iter != NULL) {
						DynamicText = NULL;
						memset(&Item, 0, sizeof(Item));
						Item.mask = LVIF_TEXT;
						Item.iItem = i;
						Item.iSubItem = j;
						if(Iter->Type == TABLE_TYPE_INT) {
							wsprintf(Text, _T("%d"), Iter->IntData);
							Item.pszText = Text;
						} else if(Iter->Type == TABLE_TYPE_STRING) {
							Item.pszText = (LPSTR)Iter->Bytes;
						} else if(Iter->Type == TABLE_TYPE_BINARY) {
							DynamicText = (LPTSTR)MemAlloc((3*(Iter->Size) + 2) * sizeof(TCHAR));
							for(k=0; k<Iter->Size; k++) {
								wsprintf(&DynamicText[3*k], _T("%02X "), Iter->Bytes[k]);
							}
							Item.pszText = DynamicText;
						} else {
							Item.pszText = _T("");
						}
						SendMessage(hWndList, LVM_SETITEM, 0, (LPARAM)&Item);
						if(DynamicText != NULL) {
							MemFree(DynamicText);
						}
					}
				}
			}

			return TRUE;
			break;
		case WM_SIZE:
		case WM_SIZING:
			Margins = (LPRECT)GetWindowLong(hWnd, DWL_USER);
			hWndList = GetDlgItem(hWnd, IDC_TABLELIST);
			GetClientRect(hWnd, &ClientRect);
			InvalidateRect(hWndList, NULL, FALSE);
			MoveWindow(hWndList, ClientRect.left + Margins->left, ClientRect.top + Margins->top, ClientRect.right - Margins->right - Margins->left, ClientRect.bottom - Margins->bottom - Margins->top, TRUE);
			hWndOk = GetDlgItem(hWnd, IDOK);
			GetClientRect(hWndOk, &Rect);
			Top = (2*ClientRect.bottom - Margins->bottom)/2 - (Rect.bottom - Rect.top)/2;
			Left = (ClientRect.right - ClientRect.left)/2 - (Rect.right - Rect.left)/2;
			MoveWindow(hWndOk, Left, Top, Rect.right, Rect.bottom, TRUE);
			break;
		case WM_SYSCOMMAND:
			switch(wParam & 0xfff0) {
				case SC_CLOSE:
					TerminateDialog(hWnd);
					return TRUE;
					break;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					TerminateDialog(hWnd);
					break;
				case IDCANCEL:
					TerminateDialog(hWnd);
					break;
			}
			return TRUE;
			break;
	}

	return FALSE;
}


/*
 * Displays a table structure.
 */
void DisplayGenericTable(HWND hWndParent, LPTABLE Table, LPTSTR Title, LPTSTR *ColumnHeadings)
{
	DISPLAYINFO		DisplayInfo;

	/* Initialize information to pass to the dialog box. */
	DisplayInfo.Table = Table;
	DisplayInfo.Title = Title;
	DisplayInfo.ColumnHeadings = ColumnHeadings;

	/* Display the table. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TABLEDISPLAY), hWndParent, TableDisplayFunc, (LPARAM)&DisplayInfo);
}


/*
 * Display the given table in the specified SP.
 */
void DisplayTable(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGAUTH			TcgAuth;
	LPTABLE			Table;
	BOOL			RetVal;
	BYTE			Sp[8];
	BYTE			TableUid[8];


	/* Query the user for the SP to activate. */
	RetVal = GetSP(hWndParent, hDrive, Sp, Caption);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Query the user for the authority to use. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, Sp, NULL, Caption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Read the table of tables. */
	Table = ReadTableNoSession(hDrive, Sp, TABLE_TABLE.Uid, &TcgAuth);

	/* If there was a problem, return. */
	if(Table == NULL) {
		MessageBox(hWndParent, _T("An error occurred reading the table of tables."), Caption, MB_OK);
		return;
	}

	/* Query the user for the table to display. */
	RetVal = GetTable(hWndParent, Table, TableUid);

	/* Free the table. */
	FreeTable(Table);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Read the selected table. */
	Table = ReadTableNoSession(hDrive, Sp, TableUid, &TcgAuth);

	/* If there was a problem, return. */
	if(Table == NULL) {
		MessageBox(hWndParent, _T("An error occurred reading the selected table."), Caption, MB_OK);
		return;
	}

	/* Display the table. */
	DisplayGenericTable(hWndParent, Table, _T("Table Display"), NULL);

	/* Free up used memory. */
	FreeTable(Table);
}
