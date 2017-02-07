#include"stdafx.h"
#include"Memory.h"
#include"Table.h"


/*
 * Determine if the data is a printable string or a sequence of bytes.
 */
static int GetTableByteType(LPBYTE Bytes, QWORD Size)
{
	QWORD		i;

	/* If no bytes, it's an integer. */
	if(Bytes == NULL) {
		return TABLE_TYPE_INT;
	}

	/* Look for non-printable bytes. */
	for(i=0; i<Size; i++) {
		if((Bytes[i] < 0x20) || (Bytes[i] >= 0x7f)) {
			return TABLE_TYPE_BINARY;
		}
	}

	/* All bytes are ASCII. */
	return TABLE_TYPE_STRING;
}


/* 
 * Get the number of rows in the table.
 */
int GetRows(LPTABLE Table)
{
	return Table->NumRows;
}


/*
 * Get the number of columns in the table.
 */
int GetCols(LPTABLE Table)
{
	return Table->NumColumns;
}


/*
 * Get a pointer to the specified row.
 */
static LPTABLEROW GetTableRow(LPTABLE Table, int Row)
{
	LPTABLEROW		TableRow;
	int				i;

	/* Get to the right row. */
	TableRow=Table->Rows;
	for(i=0; i<Row; i++) {
		TableRow=TableRow->Next;
	}

	/* Return the row. */
	return TableRow;
}


/*
 * Get the table entry specified by the row and column.
 */
LPTABLECELL GetTableCell(LPTABLE Table, int Row, int Column)
{
	LPTABLECELL		Iter;
	LPTABLEROW		TableRow;

	/* Check that the entry may exist. */
	if((Row >= Table->NumRows) || (Column >= Table->NumColumns)) {
		return NULL;
	}

	/* Get to the right row. */
	TableRow = GetTableRow(Table, Row);

	/* Get to the right cell. */
	for(Iter=TableRow->Cells; Iter!=NULL; Iter=Iter->Next) {
		if(Iter->Col == Column) {
			break;
		}
	}

	/* Return the cell. */
	return Iter;
}


/*
 * Adds a new row to the table.
 */
static void AddRow(LPTABLE Table)
{
	LPTABLEROW	NewRow;
	LPTABLEROW	Iter;

	/* Allocate memory for the row. */
	NewRow = (LPTABLEROW)MemCalloc(sizeof(TABLEROW));

	/* Update the table. */
	Table->NumRows++;
	if(Table->Rows == NULL) {
		Table->Rows = NewRow;
	} else {
		for(Iter=Table->Rows; Iter->Next!=NULL; Iter=Iter->Next);
		Iter->Next = NewRow;
	}
}


/*
 * Adds a single table cell to a larger table.
 */
void AddCellToTable(LPTABLE Table, LPTABLECELL Cell, int Row)
{
	LPTABLECELL	Next;
	LPTABLEROW	TableRow;

	/* Add rows, if necessary. */
	while(Row >= Table->NumRows) {
		AddRow(Table);
	}

	/* Update table information. */
	Table->NumColumns = max(Table->NumColumns, Cell->Col+1);

	/* Find the appropriate row. */
	TableRow = GetTableRow(Table, Row);

	/* If this is the first entry, just place it in the table. */
	if(TableRow->Cells == NULL) {
		TableRow->Cells = Cell;
		return;
	}

	/* Add the entry to the table. */
	for(Next=TableRow->Cells; Next->Next; Next=Next->Next);
	Next->Next = Cell;
}


/*
 * Adds an entry to the table.
 */
LPTABLECELL AddCell(LPTABLE Table, int Row, int Column, QWORD IntData, LPBYTE Bytes)
{
	LPTABLECELL	NewEntry;
	int			Type;
	int			ToAlloc;

	/* This takes care of a bug(?) in Seagate drives which have a column 0xffff0000 in the Locking table. */
	if(Column < 0) {
		return NULL;
	}

	/* Check for a duplicate item first, and don't add it. */
	if(Table != NULL) {
		NewEntry = GetTableCell(Table, Row, Column);
		if(NewEntry != NULL) {
			return NewEntry;
		}
	}

	/* Determine the type of data first, as it determines how many bytes to allocate. */
	Type = GetTableByteType(Bytes, IntData);

	/* Determine the number of bytes to allocate. */
	ToAlloc = sizeof(TABLECELL);
	if(Type != TABLE_TYPE_INT) {
		ToAlloc += (int)IntData + 1;
	}

	/* Allocate memory for a new entry. */
	NewEntry = (LPTABLECELL)MemCalloc(ToAlloc);
	if(NewEntry == NULL) {
		return NULL;
	}

	/* Fill in the entry information. */
	NewEntry->Col = Column;
	NewEntry->Type = Type;
	NewEntry->Size = (int)IntData;
	NewEntry->IntData = IntData;
	NewEntry->Next = NULL;

	/* Fill in type specific information. */
	switch(NewEntry->Type) {
		case TABLE_TYPE_STRING:
			NewEntry->Size++;
			/* Fall through. */
		case TABLE_TYPE_BINARY:
			memcpy(NewEntry->Bytes, Bytes, (int)IntData);
			break;
		case TABLE_TYPE_INT:
			NewEntry->Size = 0;
			break;
	}

	/* Add the new entry to the table. */
	if(Table != NULL) {
		AddCellToTable(Table, NewEntry, Row);
	}

	/* Return the entry. */
	return NewEntry;
}


/*
 * Determine whether a table entry contains a Uid in column 0.
 */
static BOOL IsUid(LPTABLECELL Table)
{
	return ((Table != NULL) && (Table->Type == TABLE_TYPE_BINARY) && (Table->Size == 8));
}


/*
 * Sort a table by UID in column 0.
 */
void SortTable(LPTABLE Table)
{
	LPTABLECELL	Cell1;
	LPTABLECELL	Cell2;
	LPTABLECELL	Temp;
	LPTABLEROW	Row1;
	LPTABLEROW	Row2;
	int			i, j;

	/* Determine the sort order. */
	for(i=0; i<Table->NumRows; i++) {
		Cell1 = GetTableCell(Table, i, 0);
		if(IsUid(Cell1)) {
			for(j=i+1; j<Table->NumRows; j++) {
				Cell2 = GetTableCell(Table, j, 0);
				if(IsUid(Cell2)) {
					if(memcmp(Cell1->Bytes, Cell2->Bytes, 8) > 0) {
						Row1 = GetTableRow(Table, i);
						Row2 = GetTableRow(Table, j);
						Temp = Row1->Cells;
						Row1->Cells = Row2->Cells;
						Row2->Cells = Temp;
						Cell1 = Cell2;
					}
				}
			}
		}
	}
}


/*
 * Create an empty table.
 */
LPTABLE CreateTable(void)
{
	/* Allocate memory for the table. */
	return (LPTABLE)MemCalloc(sizeof(TABLE));
}


/*
 * Free up a table row.
 */
static void FreeTableRow(LPTABLEROW TableRow)
{
	LPTABLECELL		Cell;
	LPTABLECELL		Next;

	Cell = TableRow->Cells;
	while(Cell != NULL) {
		Next = Cell;
		Cell = Cell->Next;
		MemFree(Next);
	}
	MemFree(TableRow);
}


/*
 * Free up a table linked list.
 */
void FreeTable(LPTABLE Table)
{
	LPTABLEROW		Row;
	LPTABLEROW		Next;

	if(Table != NULL) {
		Row = Table->Rows;
		while(Row != NULL) {
			Next = Row;
			Row = Row->Next;
			FreeTableRow(Next);
		}
		MemFree(Table);
	}
}
