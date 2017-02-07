#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"CryptInfo.h"
#include"Uid.h"
#include"TableDisplay.h"
#include"GetUser.h"
#include"GetRange.h"
#include"Memory.h"


/* The algorithms used by Opal 1.0. */
enum {ALG_AES128, ALG_AES256};

/* The basic column headers. */
static LPTSTR	BasicHeaders[] = {_T("Range"), _T("Algorithm"), _T("Mode")};

/* The advanced column headers. */
static LPTSTR	AdvancedHeaders[] = {_T("Range"), _T("Start"), _T("Length"), _T("Read Lock Enabled"), _T("Read Locked"), _T("Write Lock Enabled"), _T("Write Locked"), _T("Algorithm"), _T("Mode")};

/* The caption for error messages. */
static LPTSTR	Caption = _T("Cryptographic Information");

/* The structure to contain the range information. */
typedef struct tagRangeInfo {
	QWORD		Start;				/* The start of the range. */
	QWORD		Length;				/* The length of the range. */
	BOOL		ReadLockEnabled;	/* True if read lock is enabled. */
	BOOL		WriteLockEnabled;	/* True if write lock is enabled. */
	BOOL		ReadLocked;			/* True if locked for reading. */
	BOOL		WriteLocked;		/* True if locked for writing. */
	int			Mode;				/* The block chaining mode. */
	int			Algorithm;			/* The algorithm (AES128 or AES256 as of Opal 1). */
} RANGEINFO, *LPRANGEINFO;


/*
 * Convert an algorithm identifier to a string.
 */
static LPTSTR AlgorithmToString(int Algorithm)
{
	switch(Algorithm) {
		case ALG_AES128:
			return _T("AES 128");
			break;
		case ALG_AES256:
			return _T("AES 256");
			break;
		default:
			return _T("Unknown");
			break;
	}
}


/*
 * Convert a boolean value to a string.
 */
static LPTSTR BoolToString(int Value)
{
	switch(Value) {
		case 0:
			return _T("False");
			break;
		default:
			return _T("True");
			break;
	}
}


/*
 * Convert a block mode identifier to a string.
 */
static LPTSTR ModeToString(int Mode)
{
	switch(Mode) {
		case 0:
			return _T("ECB");
			break;
		case 1:
			return _T("CBC");
			break;
		case 2:
			return _T("CFB");
			break;
		case 3:
			return _T("OFB");
			break;
		case 4:
			return _T("GCM");
			break;
		case 5:
			return _T("CTR");
			break;
		case 6:
			return _T("CCM");
			break;
		case 7:
			return _T("XTS");
			break;
		case 8:
			return _T("LRW");
			break;
		case 9:
			return _T("EME");
			break;
		case 10:
			return _T("CMC");
			break;
		case 11:
			return _T("XEX");
			break;
		default:
			return _T("Unknown");
			break;
	}
}


/*
 * Convert a range UID to an index: Global = 0, Range1 = 1, etc
 */
static int GetRangeIndex(LPBYTE Uid)
{
	int		Range;

	/* Check for the global range first. */
	if(memcmp(Uid, LOCKING_GLOBALRANGE.Uid, 8) == 0) {
		return 0;
	}

	/* Check for the other ranges. */
	if(memcmp(Uid, LOCKING_RANGE.Uid, 6) != 0) {
		return -1;
	}

	/* Determine the range number. */
	Range = ((int)Uid[6] << 8) + Uid[7];

	/* Return the range. */
	return Range;
}


/*
 * Get advanced information for the ranges.
 */
static BOOL GetAdvancedInformation(HWND hWndParent, LPTCGDRIVE hDrive, LPRANGEINFO RangeInfo, int NumRanges)
{
	LPTABLECELL	Cell;
	LPTABLECELL	Iter;
	TCGAUTH		TcgAuth;
	LPTABLE		Table;
	BOOL		Result;
	int			Index;
	int			Reply;
	int			Rows;
	int			i;

	/* Ask the user if they want advanced information. */
	Reply = MessageBox(hWndParent, _T("Do you want full range information?"), Caption, MB_YESNO);
	if(Reply == IDNO) {
		return FALSE;
	}

	/* Get the user's authorization information. */
	Result = GetUserAuthInfo(hWndParent, hDrive, SP_LOCKING.Uid, NULL, Caption, &TcgAuth);
	if(Result == FALSE) {
		return FALSE;
	}

	/* Get the range information. */
	Table = GetRangeTable(hDrive, &TcgAuth);

	/* If there was an error reading the table, notify the user. */
	if(Table == NULL) {
		MessageBox(hWndParent, _T("There was an error reading range information."), Caption, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	/* Check that we read enough - some users can read minimal information. */
	if((GetRows(Table) <= NumRanges) || (GetCols(Table) < 9)) {
		MessageBox(hWndParent, _T("There was an error reading range information."), Caption, MB_ICONERROR | MB_OK);
		FreeTable(Table);
		return FALSE;
	}

	/* Parse the table. */
	Rows = GetRows(Table);
	for(i=0; i<Rows; i++) {
		Iter = GetTableCell(Table, i, 0);
		Index = GetRangeIndex(Iter->Bytes);
		if(Index != -1) {
			Cell = GetTableCell(Table, i, 3);
			RangeInfo[Index].Start = Cell->IntData;
			Cell = GetTableCell(Table, i, 4);
			RangeInfo[Index].Length = Cell->IntData;
			Cell = GetTableCell(Table, i, 5);
			RangeInfo[Index].ReadLockEnabled = (int)Cell->IntData;
			Cell = GetTableCell(Table, i, 6);
			RangeInfo[Index].WriteLockEnabled = (int)Cell->IntData;
			Cell = GetTableCell(Table, i, 7);
			RangeInfo[Index].ReadLocked = (int)Cell->IntData;
			Cell = GetTableCell(Table, i, 8);
			RangeInfo[Index].WriteLocked = (int)Cell->IntData;
		}
	}

	/* Free up used memory. */
	FreeTable(Table);

	/* Return true - we have all the range information. */
	return TRUE;
}


/*
 * Retrieve and display cryptographic information about the drive.
 */
void CryptInfo(HWND hWndParent, LPTCGDRIVE hDrive)
{
	LPRANGEINFO	RangeInfo;
	TCGSESSION	Session;
	LPTABLE		Table;
	LPTSTR		*Headers;
	LPTSTR		String;
	TCHAR		Text[100];
	BOOL		IsAdvanced;
	BYTE		Uid[8];
	BYTE		Result;
	int			NumRanges;
	int			i;

	/* Check that the Locking SP is enabled. */
	if(IsLockingEnabled(hDrive) == FALSE) {
		MessageBox(hWndParent, _T("The Locking SP is not enabled."), Caption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Start an unauthenticated session to the Locking SP. */
	Result = StartSession(&Session, hDrive, SP_LOCKING.Uid, NULL);
	if(Result != 0) {
		MessageBox(hWndParent, _T("There was an error opening a session to the Locking SP."), Caption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Get the number of ranges. */
	NumRanges = GetNumberRanges(&Session);

	/* Allocate memory for the range information. */
	RangeInfo = (LPRANGEINFO)MemAlloc((NumRanges + 1) * sizeof(RANGEINFO));
	if(RangeInfo == NULL) {
		MessageBox(hWndParent, _T("Out of resources - unable to allocate memory."), Caption, MB_ICONERROR | MB_OK);
		EndSession(&Session);
		return;
	}

	/* Determine the algorithm by reading the mode information. */
	if(ReadTableCellDword(&Session, K_AES_128_GLOBALRANGEKEY.Uid, 4, (LPDWORD)&(RangeInfo[0].Mode)) != FALSE) {
		RangeInfo[0].Algorithm = ALG_AES128;
		memcpy(Uid, K_AES_128_RANGEKEY.Uid, 8);
	} else {
		if(ReadTableCellDword(&Session, K_AES_256_GLOBALRANGEKEY.Uid, 4, (LPDWORD)&(RangeInfo[0].Mode)) != FALSE) {
			RangeInfo[0].Algorithm = ALG_AES256;
			memcpy(Uid, K_AES_256_RANGEKEY.Uid, 8);
		} else {
			MemFree(RangeInfo);
			MessageBox(hWndParent, _T("There was an error finding algorithm information."), Caption, MB_ICONERROR | MB_OK);
			EndSession(&Session);
			return;
		}
	}

	for(i=1; i<=NumRanges; i++) {
		/* Get the mode for the each range. */
		Uid[6] = (i >> 8) & 0xff;
		Uid[7] = (i >> 0) & 0xff;
		RangeInfo[i].Algorithm = RangeInfo[0].Algorithm;
		if(ReadTableCellDword(&Session, Uid, 4, (LPDWORD)&(RangeInfo[i].Mode)) == FALSE) {
			MessageBox(hWndParent, _T("There was an error determining the encryption mode for a range."), Caption, MB_ICONERROR | MB_OK);
			EndSession(&Session);
			MemFree(RangeInfo);
			return;
		}
	}

	/* End the session. */
	EndSession(&Session);

	/* Get advanced information. */
	IsAdvanced = GetAdvancedInformation(hWndParent, hDrive, RangeInfo, NumRanges);

	/* Initialize the table. */
	Table = CreateTable();

	/* Display basic information. */
	if(IsAdvanced == FALSE) {
		/* Set header information. */
		Headers = BasicHeaders;

		/* Add in global information. */
		wsprintf(Text, _T("Global Range"));
		AddCell(Table, 0, 0, lstrlen(Text), (LPBYTE)Text);
		String = AlgorithmToString(RangeInfo[0].Algorithm);
		AddCell(Table, 0, 1, lstrlen(String), (LPBYTE)String);
		String = ModeToString(RangeInfo[0].Mode);
		AddCell(Table, 0, 2, lstrlen(String), (LPBYTE)String);

		/* Add in the rest of the information. */
		for(i=1; i<=NumRanges; i++) {
			wsprintf(Text, _T("Range %d"), i);
			AddCell(Table, i, 0, lstrlen(Text), (LPBYTE)Text);
			String = AlgorithmToString(RangeInfo[i].Algorithm);
			AddCell(Table, i, 1, lstrlen(String), (LPBYTE)String);
			String = ModeToString(RangeInfo[i].Mode);
			AddCell(Table, i, 2, lstrlen(String), (LPBYTE)String);
		}
	} else {
		/* Set header information. */
		Headers = AdvancedHeaders;

		/* Add in global information. */
		wsprintf(Text, _T("Global Range"));
		AddCell(Table, 0, 0, lstrlen(Text), (LPBYTE)Text);
		AddCell(Table, 0, 1, RangeInfo[0].Start, NULL);
		AddCell(Table, 0, 2, RangeInfo[0].Length, NULL);
		String = BoolToString(RangeInfo[0].ReadLockEnabled);
		AddCell(Table, 0, 3, lstrlen(String), (LPBYTE)String);
		String = BoolToString(RangeInfo[0].ReadLocked);
		AddCell(Table, 0, 4, lstrlen(String), (LPBYTE)String);
		String = BoolToString(RangeInfo[0].WriteLockEnabled);
		AddCell(Table, 0, 5, lstrlen(String), (LPBYTE)String);
		String = BoolToString(RangeInfo[0].WriteLocked);
		AddCell(Table, 0, 6, lstrlen(String), (LPBYTE)String);
		String = AlgorithmToString(RangeInfo[0].Algorithm);
		AddCell(Table, 0, 7, lstrlen(String), (LPBYTE)String);
		String = ModeToString(RangeInfo[0].Mode);
		AddCell(Table, 0, 8, lstrlen(String), (LPBYTE)String);

		/* Add in the rest of the information. */
		for(i=1; i<=NumRanges; i++) {
			wsprintf(Text, _T("Range %d"), i);
			AddCell(Table, i, 0, lstrlen(Text), (LPBYTE)Text);
			AddCell(Table, i, 1, RangeInfo[i].Start, NULL);
			AddCell(Table, i, 2, RangeInfo[i].Length, NULL);
			String = BoolToString(RangeInfo[i].ReadLockEnabled);
			AddCell(Table, i, 3, lstrlen(String), (LPBYTE)String);
			String = BoolToString(RangeInfo[i].ReadLocked);
			AddCell(Table, i, 4, lstrlen(String), (LPBYTE)String);
			String = BoolToString(RangeInfo[i].WriteLockEnabled);
			AddCell(Table, i, 5, lstrlen(String), (LPBYTE)String);
			String = BoolToString(RangeInfo[i].WriteLocked);
			AddCell(Table, i, 6, lstrlen(String), (LPBYTE)String);
			String = AlgorithmToString(RangeInfo[i].Algorithm);
			AddCell(Table, i, 7, lstrlen(String), (LPBYTE)String);
			String = ModeToString(RangeInfo[i].Mode);
			AddCell(Table, i, 8, lstrlen(String), (LPBYTE)String);
		}
	}

	/* Display the table. */
	DisplayGenericTable(hWndParent, Table, Caption, Headers);

	/* Free up resources. */
	FreeTable(Table);
	MemFree(RangeInfo);
}
