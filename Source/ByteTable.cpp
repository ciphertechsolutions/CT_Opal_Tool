#include"stdafx.h"
#include"Table.h"
#include"AtaDrive.h"
#include"Tcg.h"
#include"GetUser.h"
#include"GetByteTable.h"
#include"Uid.h"
#include"Memory.h"
#include"Progress.h"


/*
 * Save a byte table to a file.
 */
static void SaveByteTableToFile(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE Uid, LPTCGAUTH TcgAuth)
{
	OPENFILENAME	ofn;
	TCGSESSION		Session;
	HANDLE			hFile;
	LPBYTE			Buffer;
	DWORD			TableSize;
	DWORD			ReadSize;
	DWORD			BufferSize;
	DWORD			nWritten;
	DWORD			i;
	TCHAR			FileName[MAX_PATH];
	HWND			hWndDlg;
	BOOL			Result;


	/* Determine the size of the data store. */
	TableSize = GetByteTableSize(hDrive, Sp, Uid);

	/* Determine, and allocate, a buffer. */
	BufferSize = min((hDrive->BufferSize - 1024) & (~0x3ff), 63*1024);
	Buffer = (LPBYTE)MemAlloc(BufferSize);
	if(Buffer == NULL) {
		MessageBox(hWndParent, _T("Out of memory."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
		return;
	}

	/* Query the user for the file name to save the data store to. */
	memset(&ofn, 0, sizeof(ofn));
	FileName[0] = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWndParent;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	if(GetSaveFileName(&ofn) == FALSE) {
		MemFree(Buffer);
		return;
	}

	/* Create the file. */
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		MessageBox(hWndParent, _T("There was an error creating the file."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
		MemFree(Buffer);
		return;
	}

	/* Authenticate to the Locking SP. */
	Result = StartSession(&Session, hDrive, Sp, TcgAuth);
	if(Result != 0) {
		MessageBox(hWndParent, _T("There was an error authenticating to the Locking SP."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
		CloseHandle(hFile);
		MemFree(Buffer);
		return;
	}

	/* Start the progress bar. */
	hWndDlg = CreateProgressBox(hWndParent, _T("Reading byte table..."), TableSize);

	/* Samsung drives can't read the last byte of the MBR table, so test for this and adjust accordingly. */
	Result = ReadByteTable(&Session, Uid, Buffer, TableSize-1, 1);
	if(Result != 0) {
		TableSize--;
	}

	/* Save the datastore. */
	for(i=0; i<TableSize; i+=BufferSize) {
		/* Has the user cancelled the operation? */
		if(IsProgressCancelled(hWndDlg) != FALSE) {
			break;
		}

		/* Update the progress bar. */
		UpdateProgressBox(hWndDlg, i);

		/* Process any messages for the progess dialog box. */
		ProcessMessages(hWndDlg);

		/* Read from the data store. */
		ReadSize = min(BufferSize, TableSize-i);
		Result = ReadByteTable(&Session, Uid, Buffer, i, ReadSize);
		if(Result != 0) {
			MessageBox(hWndParent, _T("There was an error reading the datastore."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
			break;
		}

		/* Write to the file. */
		nWritten = 0;
		Result = WriteFile(hFile, Buffer, ReadSize, &nWritten, NULL);
		if((Result == FALSE) || (nWritten != ReadSize)) {
			MessageBox(hWndParent, _T("There was an error writing to the file."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
			break;
		}
	}

	/* Terminate the progress bar. Wait a short amount of time so the user notices the completion. */
	UpdateProgressBox(hWndDlg, TableSize);
	Sleep(400);
	DestroyWindow(hWndDlg);

	/* Close open handles. */
	CloseHandle(hFile);

	/* Close sessions. */
	EndSession(&Session);

	/* Free memory. */
	MemFree(Buffer);
}


/*
 * Copy the contents of a file to a byte table.
 */
static void WriteByteTableFromFile(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Sp, LPBYTE Uid, LPTCGAUTH TcgAuth)
{
	OPENFILENAME	ofn;
	TCGSESSION		Session;
	HANDLE			hFile;
	LPBYTE			Buffer;
	DWORD			TableSize;
	DWORD			FileSize;
	DWORD			BufferSize;
	DWORD			nRead;
	DWORD			i;
	TCHAR			FileName[MAX_PATH];
	HWND			hWndDlg;
	BOOL			Result;


	/* Determine the size of the data store. */
	TableSize = GetByteTableSize(hDrive, Sp, Uid);

	/* Determine, and allocate, a buffer. */
	BufferSize = min((hDrive->BufferSize - 1024) & (~0x3ff), 63*1024);
	Buffer = (LPBYTE)MemAlloc(BufferSize);
	if(Buffer == NULL) {
		MessageBox(hWndParent, _T("Out of memory."), _T("Read Byte Table"), MB_ICONERROR | MB_OK);
		MemFree(Buffer);
		return;
	}

	/* Query the user for the file name to save the data store to. */
	memset(&ofn, 0, sizeof(ofn));
	FileName[0] = 0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWndParent;
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if(GetOpenFileName(&ofn) == FALSE) {
		MemFree(Buffer);
		return;
	}

	/* Open the file. */
	hFile = CreateFile(FileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		MessageBox(hWndParent, _T("There was an error creating the file."), _T("Write Byte Table"), MB_ICONERROR | MB_OK);
		MemFree(Buffer);
		return;
	}

	/* Get the file size. */
	FileSize = GetFileSize(hFile, NULL);

	/* Check that the file can fit in the datastore. */
	if(FileSize > TableSize) {
		MessageBox(hWndParent, _T("The file is too big for the datastore."), _T("Write Byte Table"), MB_ICONWARNING | MB_OK);
		CloseHandle(hFile);
		MemFree(Buffer);
		return;
	}
	TableSize = FileSize;

	/* Authenticate to the Locking SP. */
	Result = StartSession(&Session, hDrive, Sp, TcgAuth);
	if(Result != 0) {
		MessageBox(hWndParent, _T("There was an error authenticating to the Locking SP."), _T("Write Byte Table"), MB_ICONERROR | MB_OK);
		CloseHandle(hFile);
		MemFree(Buffer);
		return;
	}

	/* Start the progress bar. */
	hWndDlg = CreateProgressBox(hWndParent, _T("Writing byte table..."), TableSize);

	/* Write the datastore. */
	for(i=0; i<TableSize; i+=BufferSize) {
		/* Has the user cancelled the operation? */
		if(IsProgressCancelled(hWndDlg) != FALSE) {
			break;
		}

		/* Update the progress bar. */
		UpdateProgressBox(hWndDlg, i);

		/* Process any messages for the progess dialog box. */
		ProcessMessages(hWndDlg);

		/* Read from the file. */
		nRead = 0;
		Result = ReadFile(hFile, Buffer, BufferSize, &nRead, NULL);
		if(Result == FALSE) {
			MessageBox(hWndParent, _T("There was an error reading from the file."), _T("Write Byte Table"), MB_ICONERROR | MB_OK);
			break;
		}

		/* Write to the data store. */
		Result = WriteByteTable(&Session, Uid, Buffer, i, nRead);
		if(Result != 0) {
			MessageBox(hWndParent, _T("There was an error writing to the datastore."), _T("Write Byte Table"), MB_ICONERROR | MB_OK);
			break;
		}
	}

	/* Terminate the progress bar. Wait a short amount of time so the user notices the completion. */
	UpdateProgressBox(hWndDlg, TableSize);
	Sleep(400);
	DestroyWindow(hWndDlg);

	/* Close open handles. */
	CloseHandle(hFile);

	/* Close sessions. */
	EndSession(&Session);

	/* Free memory. */
	MemFree(Buffer);
}


/*
 * Save the byte table to a file.
 */
void ReadByteTableByUid(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Uid, LPBYTE Sp)
{
	TCGAUTH	TcgAuth;
	BOOL	Result;

	/* Get the user and the user's auth info. */
	Result = GetUserAuthInfo(hWndParent, hDrive, Sp, NULL, _T("Read Byte Table"), &TcgAuth);
	if(Result != FALSE) {
		/* Save the byte table to the file. */
		SaveByteTableToFile(hWndParent, hDrive, Sp, Uid, &TcgAuth);
	}
}


/*
 * Write the data store from a file.
 */
void WriteByteTableByUid(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Uid, LPBYTE Sp)
{
	TCGAUTH	TcgAuth;
	BOOL	Result;

	/* Get the user and the user's auth info. */
	Result = GetUserAuthInfo(hWndParent, hDrive, Sp, NULL, _T("Write Byte Table"), &TcgAuth);
	if(Result != FALSE) {
		/* Write a file to the table. */
		WriteByteTableFromFile(hWndParent, hDrive, Sp, Uid, &TcgAuth);
	}
}


/*
 * Save the byte table to a file.
 */
void ReadArbitraryByteTable(HWND hWndParent, LPTCGDRIVE hDrive)
{
	BYTE	Uid[8];
	BYTE	Sp[8];
	BOOL	Result;

	/* Get the byte table to read. */
	Result = GetByteTable(hWndParent, hDrive, Uid, Sp);
	if(Result == FALSE) {
		return;
	}

	/* Read the table. */
	ReadByteTableByUid(hWndParent, hDrive, Uid, Sp);
}


/*
 * Write the data store from a file.
 */
void WriteArbitraryByteTable(HWND hWndParent, LPTCGDRIVE hDrive)
{
	BYTE	Uid[8];
	BYTE	Sp[8];
	BOOL	Result;

	/* Get the byte table to write. */
	Result = GetByteTable(hWndParent, hDrive, Uid, Sp);
	if(Result == FALSE) {
		return;
	}

	/* Write the byte table. */
	WriteByteTableByUid(hWndParent, hDrive, Uid, Sp);
}
