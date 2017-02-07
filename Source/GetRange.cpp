#include"stdafx.h"
#include"Atadrive.h"
#include"Table.h"
#include"Tcg.h"
#include"Uid.h"
#include"GetRange.h"
#include"Memory.h"


/*
 * Get the number of ranges, not including the global range.
 */
int GetNumberRanges(LPTCGSESSION Session)
{
	int			NumRanges;

	/* Early Samsung drives don't support anything other than the global range. */
	if(Session->hDrive->IsSamsung != FALSE) {
		return 0;
	}

	/* Set the default value for the number of ranges. */
	NumRanges = 4;

	/* Read the cell containing the number of ranges. */
	ReadTableCellDword(Session, LOCKINGINFO.Uid, 4, (LPDWORD)&NumRanges);

	/* Return the value. */
	return NumRanges;
}


/*
 * Read a row of the locking table cell by cell.
 */
static void ReadEntireRow(LPTCGSESSION Session, LPBYTE RowUid, int Row, LPTABLE Table)
{
	LPTABLECELL	NewEntry;
	int			i;

	/* Add the row UID to the table. */
	AddCell(Table, Row, 0, 8, RowUid);

	/* Read each of the cells that contains variable information. */
	for(i=3; i<9; i++) {
		NewEntry = ReadTableCell(Session, RowUid, i);
		if(NewEntry != NULL) {
			AddCellToTable(Table, NewEntry, Row);
		}
	}
}


/*
 * Read the range table row by row, if it's not accessible all at once.
 */
static LPTABLE ReadDefaultRangeTable(LPTCGSESSION Session)
{
	LPTABLE		Table;
	BYTE		Uid[8];
	int			NumRanges;
	int			i;

	/* Determine the number of ranges we're dealing with. */
	NumRanges = GetNumberRanges(Session);

	/* Add in the global range. */
	Table = CreateTable();
	ReadEntireRow(Session, LOCKING_GLOBALRANGE.Uid, 0, Table);

	/* Add in the other ranges. */
	for(i=1; i<=NumRanges; i++) {
		memcpy(Uid, LOCKING_RANGE.Uid, sizeof(Uid));
		Uid[6] = (i >> 8) & 0xff;
		Uid[7] = (i >> 0) & 0xff;
		ReadEntireRow(Session, Uid, i, Table);
	}

	/* Add text descriptions. */
	AddTextDescriptions(Table);

	/* Return the table. */
	return Table;
}


/*
 * Get the range table from the Locking SP.
 */
LPTABLE GetRangeTable(LPTCGDRIVE hDrive, LPTCGAUTH TcgAuth)
{
	TCGSESSION	Session;
	LPTABLE		Table;
	BYTE		Result;

	/* Start an unauthorized session to the SP. */
	Result = StartSession(&Session, hDrive, SP_LOCKING.Uid, TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		return NULL;
	}

	/* Read the range table. */
	Table = ReadTable(&Session, TABLE_LOCKING.Uid);

	/* If there was an error, read the default table. */
	if(Table == NULL) {
		Table = ReadDefaultRangeTable(&Session);
	}

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Return the table. */
	return Table;
}
