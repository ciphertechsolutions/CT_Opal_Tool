#ifndef TABLE_H_
#define TABLE_H_

#define TABLE_TYPE_INT		0			/* Data is integer data. */
#define TABLE_TYPE_STRING	1			/* Data is a string. */
#define TABLE_TYPE_BINARY	2			/* Data is a sequence of bytes. */


/* The structure used to store table cells. */
typedef struct tagTableCell {
	int					Col;		/* The column corresponding to the entry. */
	int					Type;		/* The type of data stored in this cell. */
	QWORD				IntData;	/* Integer data, if appropriate. */
	int					Size;		/* The size of the bytes array below. */
	struct tagTableCell	*Next;		/* Pointer to the next cell in the table. */
	unsigned char		Bytes[1];	/* The bytes in the table, if string or binary. */
} TABLECELL, *LPTABLECELL;


/* The structure for a table row. */
typedef struct tagTableRow {
	LPTABLECELL			Cells;		/* The cells of the row. */
	struct tagTableRow	*Next;		/* Pointer to the next row. */
} TABLEROW, *LPTABLEROW;


/* The top level table structure. */
typedef struct tagTable {
	int				NumRows;		/* The number of rows in the table. */
	int				NumColumns;		/* The number of columns in the table. */
	LPTABLEROW		Rows;			/* The rows of the table. */
} TABLE, *LPTABLE;


/* Function prototypes. */
int GetRows(LPTABLE Table);
int GetCols(LPTABLE Table);
void AddCellToTable(LPTABLE Table, LPTABLECELL Cell, int Row);
LPTABLECELL AddCell(LPTABLE Table, int Row, int Column, QWORD IntData, LPBYTE Bytes);
LPTABLECELL GetTableCell(LPTABLE Table, int Row, int Column);
void SortTable(LPTABLE Table);
LPTABLE CreateTable(void);
void FreeTable(LPTABLE Table);


#endif /* TABLE_H_ */
