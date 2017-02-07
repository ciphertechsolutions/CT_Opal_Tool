#include"stdafx.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"Token.h"
#include"Uid.h"
#include"Memory.h"
// TODO: Better error codes


/* The feature descriptor header. */
typedef struct tagFeatureDescriptor {
	WORD		FeatureCode;
	BYTE		Version;
	BYTE		Length;
} FEATURE, *LPFEATURE;


/* The Locking Feature Descriptor. */
typedef struct tagLockingFeature {
	FEATURE		Feature;
	BYTE		Features;
	BYTE		Padding[11];
} LOCKINGFEATURE, *LPLOCKINGFEATURE;


/* The common header for an SSC. */
typedef struct tagSSC {
	FEATURE		Feature;
	WORD		BaseComId;
	WORD		NumberComIds;
} SSC, *LPSSC;


/* The default size of a packet. */
#define DEFAULTPACKETSIZE	2048


/*
 * Send the COM Packet within the session.
 */
static BOOL TrustedSendSession(LPTCGSESSION Session)
{
	int		Length;

	/* Round up length fields to indicate padding. */
	Length = BE_TO_LE_32(Session->hDrive->ComPacket->ComPacket.Length) & 3;
	if(Length != 0) {
		Session->hDrive->ComPacket->ComPacket.Length = LE_TO_BE_32((BE_TO_LE_32(Session->hDrive->ComPacket->ComPacket.Length) + 4 - Length));
		Session->hDrive->ComPacket->Packet.Length = LE_TO_BE_32((BE_TO_LE_32(Session->hDrive->ComPacket->Packet.Length) + 4 - Length));
	}

	/* Determine the length from the packet length. */
	Length = BE_TO_LE_32(Session->hDrive->ComPacket->ComPacket.Length) + sizeof(Session->hDrive->ComPacket->ComPacket);
	Length = (Length + 511) >> 9;

	/* Send the packet. */
	return TrustedSend(Session->hDrive->hDrive, Session->hDrive->UseDMA, 1, Session->hDrive->FirstComID, (LPBYTE)(Session->hDrive->ComPacket), (WORD)Length);
}


/*
 * Receive the COM Packet within the session.
 */
static BOOL TrustedReceiveSession(LPTCGSESSION Session)
{
	LPBYTE	ReceiveBuffer;
	DWORD	OutstandingData;
	DWORD	Length;
	BOOL	Result;

	/* Set the receive buffer. */
	ReceiveBuffer = (LPBYTE)(Session->hDrive->ComPacket);

	/* Give the drive some time to perform the command. */
	Sleep(10);

	/* Wait until the command has completed. */
	do {
		/* Get the result of the command. */
		Result = TrustedReceive(Session->hDrive->hDrive, Session->hDrive->UseDMA, 1, Session->hDrive->FirstComID, ReceiveBuffer, 1);

		/* Convert the outstanding data to a format we can use. */
		OutstandingData = BE_TO_LE_32(Session->hDrive->ComPacket->ComPacket.OutstandingData);

		/* Get the length of the ComPacket. */
		Length = BE_TO_LE_32(Session->hDrive->ComPacket->ComPacket.Length);

		/* Make sure that we need to wait on the drive, and there just isn't 1 byte of outstanding data. */
		if((OutstandingData == 1) && (Length != 0)) {
			break;
		}

		/* If there's outstanding data, wait a while. */
		if(OutstandingData == 1) {
			Sleep(100);
		}
	} while((Result != FALSE) && (OutstandingData == 1));

	/* The command has completed, so return immediately on failure. */
	if(Result == FALSE) {
		return FALSE;
	}

	/* Some Samsung drives don't set the OutstandingData field correctly, so check for that. */
	if((OutstandingData == 0) && (Length > (512 - sizeof(Session->hDrive->ComPacket->ComPacket)))) {
		Length = (Length + sizeof(Session->hDrive->ComPacket->ComPacket) + 511) >> 9;
		return TrustedReceive(Session->hDrive->hDrive, Session->hDrive->UseDMA, 1, Session->hDrive->FirstComID, ReceiveBuffer, (WORD)Length);
	}

	/* If we received all the information, also return. */
	if(OutstandingData == 0) {
		return TRUE;
	}

	/* If we received data, advance the receive buffer. */
	if(Session->hDrive->ComPacket->ComPacket.Length != 0) {
		ReceiveBuffer += 512;
	}

	/* Read the remaining data. */
	Result = TrustedReceive(Session->hDrive->hDrive, Session->hDrive->UseDMA, 1, Session->hDrive->FirstComID, ReceiveBuffer, (WORD)((OutstandingData + 511) >> 9));

	/* Return the result of the operation. */
	return Result;
}


/*
 * Send a command and receive a response in a single function call.
 */
static BOOL TrustedCommand(LPTCGSESSION Session)
{
	BOOL	Result;

	/* Send the packet. */
	Result = TrustedSendSession(Session);

	/* Get the response. */
	if(Result) {
		Result = TrustedReceiveSession(Session);
	}

	/* Return the result of the commands. */
	return Result;
}


/*
 * Perform a level 0 discovery on the drive.
 */
BOOL Level0Discovery(LPTCGDRIVE hDrive)
{
	return TrustedReceive(hDrive->hDrive, hDrive->UseDMA, 1, 1, hDrive->hDrive->Scratch, 1);
}


/*
 * Return a bitmask of the Locking Features.
 */
DWORD GetLockingFeatures(LPTCGDRIVE hDrive)
{
	LPLOCKINGFEATURE	LockingFeature;
	LPFEATURE			Feature;

	/* Perform a level 0 discovery. */
	if(Level0Discovery(hDrive) == FALSE) {
		return 0x80000000;
	}

	/* Search for the Locking Feature Descriptor. */
	Feature = (LPFEATURE)&(hDrive->hDrive->Scratch[48]);
	while((BE_TO_LE_16(Feature->FeatureCode) != 0) && (BE_TO_LE_16(Feature->FeatureCode) < 2)) {
		Feature = (LPFEATURE)((LPBYTE)Feature + Feature->Length + sizeof(FEATURE));
	}

	/* Make sure there's one. */
	if(BE_TO_LE_16(Feature->FeatureCode) != 2) {
		return 0x80000000;
	}

	/* Return the feature. */
	LockingFeature = (LPLOCKINGFEATURE)Feature;
	return LockingFeature->Features;
}


/*
 * Quick check to determine whether the Locking SP is active.
 */
BOOL IsLockingEnabled(LPTCGDRIVE hDrive)
{
	/* Get the Locking feature.  Note: on error, the locking enable bit is zero.*/
	return (GetLockingFeatures(hDrive) & 0x02);
}


/*
 * Get the drive type from the SSC.
 */
WORD GetTCGDriveType(LPTCGDRIVE hDrive)
{
	LPFEATURE	Feature;

	/* Send a level 0 discovery. */
	if(Level0Discovery(hDrive) == FALSE) {
		return 0;
	}

	/* Search for the first SSC. */
	Feature = (LPFEATURE)&(hDrive->hDrive->Scratch[48]);
	while((BE_TO_LE_16(Feature->FeatureCode) != 0) && (BE_TO_LE_16(Feature->FeatureCode) < 0x100)) {
		Feature = (LPFEATURE)((LPBYTE)Feature + Feature->Length + sizeof(FEATURE));
	}

	/* Make sure there's one. */
	if(BE_TO_LE_16(Feature->FeatureCode) == 0) {
		return 0;
	}

	/* Return the feature code. */
	return BE_TO_LE_16(Feature->FeatureCode);
}


/*
 * Get the first statically allocated Com ID.
 */
static WORD GetFirstComID(LPTCGDRIVE hDrive)
{
	LPFEATURE		Feature;
	LPSSC			Ssc;

	/* Send a level 0 discovery. */
	if(Level0Discovery(hDrive) == FALSE) {
		return 0;
	}

	/* Search for the first SSC. */
	Feature = (LPFEATURE)&(hDrive->hDrive->Scratch[48]);
	while((BE_TO_LE_16(Feature->FeatureCode) != 0) && (BE_TO_LE_16(Feature->FeatureCode) < 0x100)) {
		Feature = (LPFEATURE)((LPBYTE)Feature + Feature->Length + sizeof(FEATURE));
	}

	/* Make sure there's one. */
	if(BE_TO_LE_16(Feature->FeatureCode) == 0) {
		return 0;
	}

	/* Return the Com ID. */
	Ssc = (LPSSC)Feature;
	return BE_TO_LE_16(Ssc->BaseComId);
}


/*
 * Converts a drive type into a readable string.
 */
LPTSTR GetDriveTypeString(WORD Type)
{
	switch(Type) {
		case 0:
			return _T("N/A");
			break;
		case 0x100:
			return _T("Enterprise");
			break;
		case 0x200:
			return _T("Opal");
			break;
		case 0x203:
			return _T("Opal 2.0");
			break;
		case 0x301:
			return _T("Opalite");
			break;
		case 0x302:
			return _T("Pyrite");
			break;
		default:
			return _T("Unknown");
			break;
	}
}


/*
 * Initialize a ComPacket with the session information.
 */
static void InitComPacket(LPTCGSESSION Session, LPBYTE InvokingUID, LPBYTE MethodUID)
{
	/* Zero out the ComPacket. */
	memset(Session->hDrive->ComPacket, 0, Session->hDrive->BufferSize);

	/* Set the Com ID. We don't set the Extended Com IDs size we only use static COM IDs. */
	Session->hDrive->ComPacket->ComPacket.ComID = BE_TO_LE_16(Session->hDrive->FirstComID);

	/* Set the session numbers. */
	Session->hDrive->ComPacket->Packet.HostSessionNumber = BE_TO_LE_32(Session->HostNumber);
	Session->hDrive->ComPacket->Packet.TperSessionNumber = BE_TO_LE_32(Session->TperNumber);

	/* Set the length fields. */
	Session->hDrive->ComPacket->ComPacket.Length = LE_TO_BE_32(sizeof(TCGPACKET) + sizeof(TCGSUBPACKET));
	Session->hDrive->ComPacket->Packet.Length = LE_TO_BE_32(sizeof(TCGSUBPACKET));

	/* Add the function call. */
	if(InvokingUID != NULL) {
		AddTokenCall(Session->hDrive->ComPacket, InvokingUID, MethodUID);
	}
}


/*
 * Perform a Level 1 Discovery on the drive.
 */
LPTABLE Level1Discovery(LPTCGDRIVE hDrive, LPPROPERTIES Properties)
{
	TCGSESSION	Session;
	LPTOKENS	Tokens;
	LPTABLE		Table;
	BYTE		Response;
	BOOL		Result;
	int			CurrentRow;
	int			i;


	/* Initialize session variables. */
	Session.HostNumber = 0;
	Session.TperNumber = 0;
	Session.hDrive = hDrive;

	/* Initialize the ComPacket. */
	InitComPacket(&Session, SM.Uid, SM_PROPERTIES.Uid);

	/* Add the properties we want to communicate to the TPer. */
	if(Properties != NULL) {
		AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session.hDrive->ComPacket, 0);
		AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_STARTLIST);
		for(i=0; Properties[i].String!=NULL; i++) {
			AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_STARTNAME);
			AddTokenBytes(Session.hDrive->ComPacket, (LPBYTE)Properties[i].String, lstrlen(Properties[i].String));
			AddTokenUnsignedInt(Session.hDrive->ComPacket, Properties[i].Value);
			AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_ENDNAME);
		}
		AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_ENDLIST);
		AddSimpleToken(Session.hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session.hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(&Session);
	if(Result == FALSE) {
		return NULL;
	}

	/* Parse the response for validity. */
	Tokens = ParseResponse(Session.hDrive->ComPacket);

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session.hDrive->ComPacket);

	/* Create a blank table. */
	Table = CreateTable();
	if(Table == NULL) {
		return NULL;
	}

	/* Parse the information into a table. */
	CurrentRow = 0;
	if(Response == 0) {
		for(i=0; i<Tokens->Count; i++) {
			if(Tokens->Tokens[i].Type == TCG_TOKEN_STARTNAME) {
				AddCell(Table, CurrentRow, 0, Tokens->Tokens[i+1].Integer, Tokens->Tokens[i+1].Bytes);
				AddCell(Table, CurrentRow, 1, Tokens->Tokens[i+2].Integer, Tokens->Tokens[i+2].Bytes);
				CurrentRow++;
			}
		}
	}

	/* Free up allocated memory. */
	MemFree(Tokens);

	/* Return the table of information. */
	return Table;
}


/*
 * Determine the TPer's buffer size.
 */
static void NotifyTperBufferSize(LPTCGDRIVE hDrive)
{
	LPTABLECELL		Cell;
	PROPERTIES		Properties[8];
	LPTABLE			Table;
	int				Rows;
	int				i;

	/* Fill up the properties list. */
	Properties[0].String = _T("MaxComPacketSize");
	Properties[0].Value = hDrive->BufferSize;
	Properties[1].String = _T("MaxResponseComPacketSize");
	Properties[1].Value = hDrive->BufferSize;
	Properties[2].String = _T("MaxPacketSize");
	Properties[2].Value = hDrive->BufferSize - 20;
	Properties[3].String = _T("MaxIndTokenSize");
	Properties[3].Value = hDrive->BufferSize - 56;
	Properties[4].String = _T("MaxPackets");
	Properties[4].Value = 1;
	Properties[5].String = _T("MaxSubpackets");
	Properties[5].Value = 1;
	Properties[6].String = _T("MaxMethods");
	Properties[6].Value = 1;
	Properties[7].String = NULL;
	Properties[7].Value = 0;

	/* Perform a level 1 discovery. */
	Table = Level1Discovery(hDrive, Properties);

	/* If there was an error, do nothing. */
	if(Table == NULL) {
		hDrive->BufferSize = DEFAULTPACKETSIZE;
		return;
	}

	/* Find the values of MaxComPacketSize and MaxResponseComPacketSize. */
	Rows = GetRows(Table);
	for(i=0; i<Rows; i++) {
		Cell = GetTableCell(Table, i, 0);
		if((lstrcmpiA((LPSTR)Cell->Bytes, "MaxComPacketSize") == 0) || (lstrcmpiA((LPSTR)Cell->Bytes, "MaxResponseComPacketSize") == 0)) {
			Cell = GetTableCell(Table, i, 1);
			hDrive->BufferSize = min(hDrive->BufferSize, (DWORD)Cell->IntData);
		}
	}

	/* Free up resources. */
	FreeTable(Table);
}


/*
 * Start a session with an SP.
 */
BYTE StartSession(LPTCGSESSION Session, LPTCGDRIVE hDrive, LPBYTE Sp, LPTCGAUTH TcgAuth)
{
	LPTOKENS	Tokens;
	BYTE		Response;
	BOOL		Result;

	/* Initialize session variables. */
	Session->HostNumber = 0;
	Session->TperNumber = 0;
	Session->hDrive = hDrive;

	/* Initialize the ComPacket. */
	InitComPacket(Session, SM.Uid, SM_STARTSESSION.Uid);

	/* Add parameters. */
	Session->HostNumber = 1;
	AddTokenUnsignedInt(Session->hDrive->ComPacket, Session->HostNumber);
	AddTokenBytes(Session->hDrive->ComPacket, Sp, 8);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);

	/* Add authentication parameters, if requested. */
	if((TcgAuth != NULL) && (TcgAuth->IsValid != FALSE)) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 0);
		AddTokenBytes(Session->hDrive->ComPacket, TcgAuth->Credentials, TcgAuth->Size);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 3);
		AddTokenBytes(Session->hDrive->ComPacket, TcgAuth->Authority, 8);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Parse the repsonse for validity. */
	Tokens = ParseResponse(Session->hDrive->ComPacket);

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* The TPer session should be the 6th non-empty token. */
	if((Response == 0) && (Tokens->Count >= 6)) {
		Session->TperNumber = Tokens->Tokens[5].Integer;
	}

	/* Free up allocated resources. */
	MemFree(Tokens);

	/* Return the response code from the drive. */
	return Response;
}


/*
 * Check a user and password against an SP.
 */
BOOL CheckAuth(LPTCGDRIVE hDrive, LPBYTE SpUid, LPTCGAUTH TcgAuth)
{
	TCGSESSION	Session;
	BYTE		Result;

	/* Start an authorized session to the SP. */
	Result = StartSession(&Session, hDrive, SpUid, TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		return FALSE;
	}

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Return success. */
	return TRUE;
}


/*
 * Retrieve the next row of a table.
 */
static BYTE GetNextRow(LPTCGSESSION Session, LPBYTE Table, LPBYTE CurrentRow, LPBYTE NextRow)
{
	LPBYTE		Ptr;
	BYTE		Response;
	BOOL		Result;

	/* Early Samsung drives don't support the Next method, so return an error. */
	if(Session->hDrive->IsSamsung != FALSE) {
		return 0x3f;
	}

	/* Initialize the ComPacket. */
	InitComPacket(Session, Table, METHOD_NEXT.Uid);

	/* Add the current row parameter. */
	if(CurrentRow != NULL) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 0);
		AddTokenBytes(Session->hDrive->ComPacket, CurrentRow, 8);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}

	/* Add the count of rows to return. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Copy the next row. */
	if(Response == 0) {
		Ptr = GetResponseBytes(Session->hDrive->ComPacket, NULL);
		if(Ptr != NULL) {
			memcpy(NextRow, Ptr, 8);
		} else {
			Response = 0x3f;
		}
	}

	/* Return the response code from the drive. */
	return Response;
}


/*
 * Read specific columns from a row in a table.
 */
static BOOL ReadRowColumns(LPTCGSESSION Session, LPBYTE Row, int StartColumn, int EndColumn)
{
	/* Initialize the ComPacket. */
	InitComPacket(Session, Row, METHOD_GET.Uid);

	/* Add the cell block information. */
	AddTokenCellBlock(Session->hDrive->ComPacket, NULL, NULL, -1, -1, StartColumn, EndColumn);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	return TrustedCommand(Session);
}


/*
 * Read bytes from a byte table.
 */
BYTE ReadByteTable(LPTCGSESSION Session, LPBYTE TableUid, LPBYTE Buffer, DWORD Start, DWORD Size)
{
	LPBYTE		Ptr;
	BYTE		Response;
	BYTE		TableRow[8];
	BOOL		Result;

	/* Convert the table uid into table row 0. */
	memset(TableRow, 0, sizeof(TableRow));
	memcpy(TableRow, &TableUid[4], 4);

	/* Initialize the ComPacket. */
	InitComPacket(Session, TableRow, METHOD_GET.Uid);

	/* Add the data information. */
	AddTokenCellBlock(Session->hDrive->ComPacket, NULL, NULL, Start, Start+Size-1, -1, -1);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Save off buffer. */
	if(Response == 0) {
		Ptr = GetResponseBytes(Session->hDrive->ComPacket, NULL);
		if(Ptr != NULL) {
			memcpy(Buffer, Ptr, Size);
		} else {
			Response = 0x3f;
		}
	}

	/* Return the response code from the drive. */
	return Response;
}


/*
 * Write bytes to a byte table.
 */
BYTE WriteByteTable(LPTCGSESSION Session, LPBYTE TableUid, LPBYTE Buffer, DWORD Start, DWORD Size)
{
	BYTE	TableRow[8];
	BOOL	Result;

	/* Convert the table uid into table row 0. */
	memset(TableRow, 0, sizeof(TableRow));
	memcpy(TableRow, &TableUid[4], 4);

	/* Initialize the ComPacket. */
	InitComPacket(Session, TableRow, METHOD_SET.Uid);

	/* Add the data information. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 0);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, Start);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddTokenBytes(Session->hDrive->ComPacket, Buffer, Size);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Read a single cell from a table.
 */
LPTABLECELL ReadTableCell(LPTCGSESSION Session, LPBYTE RowUid, int Column)
{
	LPTABLECELL	TableCell;
	LPTOKENS	Tokens;
	BYTE		Response;
	int			i;

	/* Read the specific column. */
	if(ReadRowColumns(Session, RowUid, Column, Column) == FALSE) {
		return NULL;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Check the response first. */
	if(Response != 0) {
		return NULL;
	}

	/* Find the contents of the cell. */
	Tokens = ParseResponse(Session->hDrive->ComPacket);
	for(i=0; i<Tokens->Count; i++) {
		if(Tokens->Tokens[i].Type == TCG_TOKEN_STARTNAME) {
			TableCell = AddCell(NULL, 0, Column, Tokens->Tokens[i+2].Integer, Tokens->Tokens[i+2].Bytes);
			break;
		}
	}
	MemFree(Tokens);

	/* Return the table cell. */
	return TableCell;
}


/*
 * Read a single cell from a table containing only bytes.
 */
BOOL ReadTableCellBytes(LPTCGSESSION Session, LPBYTE RowUid, int Column, LPBYTE Buffer, LPDWORD Size)
{
	LPBYTE	Ptr;
	DWORD	LocalSize;

	/* Set the size in case there's an error. */
	if(Size != NULL) {
		*Size = 0;
	}

	/* Read the specific column. */
	if(ReadRowColumns(Session, RowUid, Column, Column) == FALSE) {
		return FALSE;
	}

	/* Check the response code from the drive. */
	if(GetResponseResultCode(Session->hDrive->ComPacket) != 0) {
		return FALSE;
	}

	/* Find the contents of the cell. */
	Ptr = GetResponseBytes(Session->hDrive->ComPacket, &LocalSize);

	/* Let the user know if there were no bytes. */
	if(Ptr == NULL) {
		return FALSE;
	}

	/* Copy the bytes. */
	memcpy(Buffer, Ptr, LocalSize);

	/* Set the size. */
	if(Size != NULL) {
		*Size = LocalSize;
	}

	/* Return success. */
	return TRUE;
}


/*
 * Read a single cell from a table containing an integer (32-bit).
 */
BOOL ReadTableCellDword(LPTCGSESSION Session, LPBYTE RowUid, int Column, LPDWORD Value)
{
	LPTOKENS	Tokens;
	BYTE		Response;
	BOOL		Result;
	int			i;

	/* Read the specific column. */
	if(ReadRowColumns(Session, RowUid, Column, Column) == FALSE) {
		return FALSE;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Check the response first. */
	if(Response != 0) {
		return FALSE;
	}

	/* Find the contents of the cell.  A valid response may be returned, although with no data. */
	Result = FALSE;
	Tokens = ParseResponse(Session->hDrive->ComPacket);
	for(i=0; i<Tokens->Count; i++) {
		if(Tokens->Tokens[i].Type == TCG_TOKEN_STARTNAME) {
			if(Value != NULL) {
				*Value = (DWORD)(Tokens->Tokens[i+2].Integer);
			}
			Result = TRUE;
			break;
		}
	}
	MemFree(Tokens);

	/* Return the result of whether the value was found. */
	return Result;
}


/*
 * Read a row of a table into memory.
 */
static void ReadTableRow(LPTCGSESSION Session, LPBYTE RowUid, int RowNum, LPTABLE Table)
{
	LPTOKENS	Tokens;
	BYTE		Response;
	int			i;

	/* Add the row to the table separately, in case we couldn't read it. */
	AddCell(Table, RowNum, 0, 8, RowUid);

	/* Read the entire row. */
	if(ReadRowColumns(Session, RowUid, -1, -1) == FALSE) {
		return;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Save row information. */
	if(Response == 0) {
		Tokens = ParseResponse(Session->hDrive->ComPacket);
		for(i=0; i<Tokens->Count; i++) {
			if(Tokens->Tokens[i].Type == TCG_TOKEN_STARTNAME) {
				AddCell(Table, RowNum, Tokens->Tokens[i+1].Integer, Tokens->Tokens[i+2].Integer, Tokens->Tokens[i+2].Bytes);
			}
		}
		MemFree(Tokens);
	}
}


/*
 * Read an entire table into memory.
 */
LPTABLE ReadTable(LPTCGSESSION Session, LPBYTE TableUid)
{
	LPTABLE		Table;
	BYTE		RealTableUid[8];
	BYTE		Row[8];
	BYTE		Result;
	int			CurrentRow;

	/* Convert the Table UID to the actual UID. */
	memset(&RealTableUid[4], 0, 4);
	memcpy(RealTableUid, &TableUid[4], 4);

	/* Get the first row of the table. */
	Result = GetNextRow(Session, RealTableUid, NULL, Row);

	/* Initialize variables. */
	Table = CreateTable();
	if(Table == NULL) {
		return NULL;
	}
	CurrentRow = 0;

	while(Result == 0) {
		/* Read the entire row. */
		ReadTableRow(Session, Row, CurrentRow, Table);

		/* Get the next row of the table. */
		Result = GetNextRow(Session, RealTableUid, Row, Row);

		/* Go to the next row. */
		CurrentRow++;
	}

	/* Check for no table entries. */
	if((Table->NumRows == 0) || (Table->NumColumns == 0)) {
		FreeTable(Table);
		return NULL;
	}

	/* Add text descriptions, if they are missing. */
	AddTextDescriptions(Table);

	/* Sort the table by UID. */
	SortTable(Table);

	/* Return the table information. */
	return Table;
}


/*
 * Read a table from the drive without already having an open session.
 */
LPTABLE ReadTableNoSession(LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE TableUid, LPTCGAUTH TcgAuth)
{
	TCGSESSION	Session;
	LPTABLE		Table;
	BYTE		Result;

	/* Initialize the table, in case we don't read one. */
	Table = NULL;

	/* Start the session. */
	Result = StartSession(&Session, hDrive, Sp, TcgAuth);

	/* If there was a problem, return. */
	if(Result == 0) {
		/* Read the table. */
		Table = ReadTable(&Session, TableUid);

		/* Close the session, we're done for now. */
		EndSession(&Session);
	}

	/* Return the table. */
	return Table;
}


/*
 * Get random bytes from the drive.  This is an optional command in the
 * current specification, although Seagate supports it.  Further, Seagate's
 * support is limited to returning up to 32 bytes.
 */
BYTE GetRandom(LPTCGSESSION Session, int Size, LPBYTE Bytes)
{
	LPBYTE		Ptr;
	BYTE		Response;
	BOOL		Result;

	/* Early Samsung drives don't support the Random method, so quit now. */
	if(Session->hDrive->IsSamsung != FALSE) {
		return 0x3f;
	}

	/* Initialize the ComPacket. */
	InitComPacket(Session, SP_THIS.Uid, METHOD_RANDOM.Uid);

	/* Request random bytes from the drive. */
	AddTokenUnsignedInt(Session->hDrive->ComPacket, Size);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Get the response code from the drive. */
	Response = GetResponseResultCode(Session->hDrive->ComPacket);

	/* Copy the returned bytes. */
	if(Response == 0) {
		Ptr = GetResponseBytes(Session->hDrive->ComPacket, NULL);
		if(Ptr != NULL) {
			memcpy(Bytes, Ptr, Size);
		} else {
			Response = 0x3f;
		}
	}

	/* Return the response code from the drive. */
	return Response;
}


/*
 * Activates an SP.
 */
BYTE Activate_SP(LPTCGSESSION Session, LPBYTE Sp)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, Sp, METHOD_OPAL_ACTIVATE.Uid);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Revert an SP through the Admin SP.
 */
BYTE Revert(LPTCGSESSION Session, LPBYTE Sp)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, Sp, METHOD_OPAL_REVERT.Uid);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Revert an SP through the specified SP.
 */
BYTE RevertSP(LPTCGSESSION Session, BOOL KeepGlobalKey)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, SP_THIS.Uid, METHOD_REVERTSP.Uid);

	/* Convert the boolean to a 0/1 value. */
	if(KeepGlobalKey != 0) {
		KeepGlobalKey = 1;
	}

	/* Add the parameter to keep the global key. Although an optional parameter, some Samsung drives require it. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 0x60000);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, KeepGlobalKey);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Change the user's authorization value.
 */
BYTE ChangeAuth(LPTCGSESSION Session, LPTCGAUTH TcgAuth)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, TcgAuth->Authority, METHOD_SET.Uid);

	/* Add the column to change and its new value. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 3);
	AddTokenBytes(Session->hDrive->ComPacket, TcgAuth->Credentials, TcgAuth->Size);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Creates or modifies a range.
 */
BYTE CreateRange(LPTCGSESSION Session, LPBYTE RangeUid, QWORD RangeStart, QWORD RangeLength, BOOL EnableReadLock, BOOL EnableWriteLock)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, RangeUid, METHOD_SET.Uid);

	/* Ensure the boolean values are 0/1 values. */
	if((EnableReadLock != 0) && (EnableReadLock != -1)) {
		EnableReadLock = 1;
	}
	if((EnableWriteLock != 0) && (EnableWriteLock != -1)) {
		EnableWriteLock = 1;
	}

	/* Add the information to change and its new values. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	if(RangeStart != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 3);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, RangeStart);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	if(RangeLength != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 4);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, RangeLength);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	if(EnableReadLock != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 5);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, EnableReadLock);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	if(EnableWriteLock != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 6);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, EnableWriteLock);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Set or clear the read lock value for a range.
 */
BYTE SetReadLock(LPTCGSESSION Session, LPBYTE RangeUid, BOOL ReadLock)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, RangeUid, METHOD_SET.Uid);

	/* Ensure the boolean values are 0/1 values. */
	if(ReadLock != 0) {
		ReadLock = 1;
	}

	/* Add the information to change and its new values. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 7);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, ReadLock);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Set or clear the write lock value for a range.
 */
BYTE SetWriteLock(LPTCGSESSION Session, LPBYTE RangeUid, BOOL WriteLock)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, RangeUid, METHOD_SET.Uid);

	/* Ensure the boolean values are 0/1 values. */
	if(WriteLock != 0) {
		WriteLock = 1;
	}

	/* Add the information to change and its new values. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 8);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, WriteLock);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * CryptoErase a range.
 */
BYTE EraseRange(LPTCGSESSION Session, LPBYTE Uid)
{
	BOOL		Result;

	/* Early Samsung drives don't support the GenKey method, so quit now. */
	if(Session->hDrive->IsSamsung != FALSE) {
		return 0x3f;
	}

	/* Initialize the ComPacket. */
	InitComPacket(Session, Uid, METHOD_GENKEY.Uid);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Set or clear the write lock value for a range.
 */
BYTE SetMbrState(LPTCGSESSION Session, BOOL IsEnable, BOOL IsDone)
{
	BOOL		Result;

	/* If there's nothing to do, return success. */
	if((IsEnable == -1) && (IsDone == -1)) {
		return 0;
	}

	/* Initialize the ComPacket. */
	InitComPacket(Session, MBRCONTROL.Uid, METHOD_SET.Uid);

	/* Ensure the boolean values are 0/1/-1 values. */
	if((IsEnable != 0) && (IsEnable != -1)) {
		IsEnable = 1;
	}
	if((IsDone != 0) && (IsDone != -1)) {
		IsDone = 1;
	}

	/* Add the information to change and its new values. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	if(IsEnable != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, IsEnable);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	if(IsDone != -1) {
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, 2);
		AddTokenUnsignedInt(Session->hDrive->ComPacket, IsDone);
		AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	}
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Enable or disable a user.
 */
BYTE ChangeUserState(LPTCGSESSION Session, LPBYTE User, BOOL Enable)
{
	BOOL		Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, User, METHOD_SET.Uid);

	/* Ensure the boolean values are 0/1 values. */
	if(Enable != 0) {
		Enable = 1;
	}

	/* Add the information to change and its new values. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 1);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_STARTNAME);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, 5);
	AddTokenUnsignedInt(Session->hDrive->ComPacket, Enable);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDLIST);
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDNAME);

	/* Signify end of parameters and expected result. */
	AddTokenEndParameters(Session->hDrive->ComPacket, 0);

	/* Send the packet and get the response. */
	Result = TrustedCommand(Session);
	if(Result == FALSE) {
		return 0x3f;
	}

	/* Return the response code from the drive. */
	return GetResponseResultCode(Session->hDrive->ComPacket);
}


/*
 * Read the MSID for the drive..
 */
BOOL ReadMSID(LPTCGDRIVE hDrive, LPTCGAUTH TcgAuth)
{
	TCGSESSION		Session;
	BYTE			Result;
	BOOL			Response;

	/* Start an unauthenticated session to the Admin SP. */
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, NULL);

	/* If there was a problem, return. */
	if(Result != 0) {
		return FALSE;
	}

	/* Read the MSID. */
	Response = ReadTableCellBytes(&Session, C_PIN_MSID.Uid, 3, TcgAuth->Credentials, &(TcgAuth->Size));

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Return the result. */
	return Response;
}


/*
 * Get the size of the byte table from the SP.
 */
DWORD GetByteTableSize(LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE TableUid)
{
	TCGSESSION	Session;
	DWORD		TableSize;
	BYTE		Result;

	/* Start an unauthorized session to the SP. */
	Result = StartSession(&Session, hDrive, Sp, NULL);

	/* If there was a problem, return. */
	if(Result != 0) {
		return 0;
	}

	/* Read the table of tables. */
	TableSize = 0;
	ReadTableCellDword(&Session, TableUid, 7, &TableSize);

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Return the size. */
	return TableSize;
}


/*
 * Ends an open session with an SP.
 */
BOOL EndSession(LPTCGSESSION Session)
{
	BOOL	Result;

	/* Initialize the ComPacket. */
	InitComPacket(Session, NULL, NULL);

	/* Add the end of session token. */
	AddSimpleToken(Session->hDrive->ComPacket, TCG_TOKEN_ENDSESSION);

	/* Send the end of session token and get the response. */
	Result = TrustedCommand(Session);

	/* Check that the TPer ended the session. */
	if(Result) {
		Result = CheckResponseForClose(Session->hDrive->ComPacket);
	}

	/* Return the result of the end session. */
	return Result;
}


/*
 * Close a TCG drive, freeing up resources.
 */
void CloseTcgDrive(LPTCGDRIVE hDrive)
{
	if(hDrive != NULL) {
		if(hDrive->ComPacket != NULL) {
			VirtualFree(hDrive->ComPacket, 0, MEM_RELEASE);
		}
		CloseAtaDrive(hDrive->hDrive);
		MemFree(hDrive);
	}
}


/*
 * Allocate a large static buffer.
 */
static void GetBuffer(LPTCGDRIVE hDrive)
{
	/* Start with a 128K buffer. */
	hDrive->BufferSize = 256*1024;
	hDrive->ComPacket = NULL;

	/* Try to allocate buffers. */
	while((hDrive->BufferSize >= 2048) && (hDrive->ComPacket == NULL)) {
		hDrive->BufferSize >>= 1;
		hDrive->ComPacket = (LPTCGFULLPACKET)VirtualAlloc(NULL, hDrive->BufferSize, MEM_COMMIT, PAGE_READWRITE);
	}
}


/*
 * Open a TCG drive and initialize the TCG parameters.
 */
LPTCGDRIVE OpenTcgDrive(LPTSTR Drive)
{
	TCGSESSION	Session;
	LPTCGDRIVE	hDrive;
	LPATADRIVE	hAtaDrive;
	BYTE		Result;

	/* Open the ata drive. */
	hAtaDrive = OpenAtaDrive(Drive);
	if(hAtaDrive == NULL) {
		return NULL;
	}

	/* Create the structure. */
	hDrive = (LPTCGDRIVE)MemAlloc(sizeof(TCGDRIVE));
	if(hDrive == NULL) {
		CloseAtaDrive(hAtaDrive);
		return NULL;
	}

	/* Fill in the rest of the structure with default values. */
	hDrive->hDrive = hAtaDrive;
	hDrive->UseDMA = TRUE;
	hDrive->BufferSize = DEFAULTPACKETSIZE;

	/* Get the first Com ID. */
	hDrive->FirstComID = GetFirstComID(hDrive);

	/* If there was an error, try a different DMA setting. */
	if(hDrive->FirstComID == 0) {
		hDrive->UseDMA = FALSE;
		hDrive->FirstComID = GetFirstComID(hDrive);
	}

	/* If there's still an error, the drive is not a TCG drive. */
	if(hDrive->FirstComID == 0) {
		return hDrive;
	}

	/* Get the buffer size we can use. */
	GetBuffer(hDrive);
	if(hDrive->ComPacket == NULL) {
		CloseTcgDrive(hDrive);
		return NULL;
	}

	/* Notify the TPer of our settings. */
	if(hDrive->ComPacket != NULL) {
		NotifyTperBufferSize(hDrive);
	}

	/* Determine if the drive is a first generation Samsung drive. */
	hDrive->IsSamsung = FALSE;
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, NULL);
	if(Result == 0) {
		if(ReadRowColumns(&Session, C_PIN_MSID.Uid, -1, -1) != FALSE) {
			hDrive->IsSamsung = (GetResponseResultCode(Session.hDrive->ComPacket) == 1);
		}
		EndSession(&Session);
	}

	/* Return the handle to the TCG drive. */
	return hDrive;
}
