#include"stdafx.h"
#include"Table.h"
#include"AtaDrive.h"
#include"Tcg.h"
#include"Uid.h"
#include"GetUser.h"
#include"GetSp.h"
#include"Memory.h"
#include"resource.h"


/*
 * Note: It would be nice to determine when a user is locked out of authenticating, but only admins
 * can access that information (according to the Opal spec).  If you have an admin password, you have
 * the keys to the kingdom!
 */


/*
 * Get the entry corresponding to the name of the row in the table.
 */
static LPBYTE GetName(LPTABLE Table, int Row)
{
	LPTABLECELL	Cell;

	/* Look for column 1. */
	Cell = GetTableCell(Table, Row, 1);
	if(Cell != NULL) {
		return Cell->Bytes;
	}

	/* Not found. */
	return (LPBYTE)"Unknown";
}


/*
 * Test a user entered password against all users and all SPs.
 */
void TestPassword(HWND hWndParent, LPTCGDRIVE hDrive)
{
	LPTABLECELL	SpCell;
	LPTABLECELL	UserCell;
	TCGAUTH		TcgAuth;
	LPTABLE		SpTable;
	LPTABLE		UserTable;
	LPBYTE		UserName;
	LPBYTE		SpName;
	TCHAR		Text[1000];
	DWORD		Count;
	BOOL		Result;
	int			SpRows;
	int			UserRows;
	int			i, j;

	/* Query the user for the password. */
	Result = GetPassword(hWndParent, hDrive, &TcgAuth);

	/* If the user cancels, just return. */
	if(Result == FALSE)	{
		return;
	}

	/* Get the list of SPs on the drive. */
	SpTable = GetSpTable(hDrive);

	/* Verify the table. */
	if(SpTable == NULL) {
		MessageBox(hWndParent, _T("There was an error reading the SP table."), _T("Password Test"), MB_ICONERROR | MB_OK);
		return;
	}

	/* For each SP, get a list of users within the SP. */
	Count = 0;
	SpRows = GetRows(SpTable);
	for(i=0; i<SpRows; i++) {
		/* Get the table cell. */
		SpCell = GetTableCell(SpTable, i, 0);

		/* Get a list of users. */
		UserTable = GetUserTable(hDrive, SpCell->Bytes);

		/* Verify the table. */
		if(UserTable == NULL) {
			MessageBox(hWndParent, _T("There was an error reading a user table."), _T("Password Test"), MB_ICONERROR | MB_OK);
			continue;
		}

		/* Loop through users. */
		UserRows = GetRows(UserTable);
		for(j=0; j<UserRows; j++) {
			/* Get the table cell. */
			UserCell = GetTableCell(UserTable, j, 0);

			/* Copy the authority. */
			memcpy(TcgAuth.Authority, UserCell->Bytes, sizeof(TcgAuth.Authority));

			/* Check the authorization value. */
			Result = CheckAuth(hDrive, SpCell->Bytes, &TcgAuth);

			/* If it works, notify the user. */
			if(Result) {
				SpName = GetName(SpTable, i);
				UserName = GetName(UserTable, j);
				wsprintf(Text, _T("Password verified for user '%s' in SP '%s'"), UserName, SpName);
				MessageBox(hWndParent, Text, _T("Password Test"), MB_OK);
				Count++;
			}
		}

		/* Free up memory. */
		FreeTable(UserTable);
	}

	/* If the password did not verify at all, notify the user. */
	if(Count == 0) {
		MessageBox(hWndParent, _T("The password was not verified."), _T("Password Test"), MB_OK);
	} else {
		MessageBox(hWndParent, _T("Password verification complete."), _T("Password Test"), MB_OK);
	}

	/* Free up memory. */
	FreeTable(SpTable);
}
