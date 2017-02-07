#include"stdafx.h"
#include"Resource.h"
#include"Memory.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"ChangePassword.h"
#include"GetSp.h"
#include"GetUser.h"
#include"Uid.h"


/* The caption to display on error messages. */
static LPTSTR	Caption = _T("ChangePassword");


/*
 * Change the password for a user or admin authority.
 */
void ChangePassword(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGSESSION	Session;
	TCGAUTH		NewTcgAuth;
	TCGAUTH		TcgAuth;
	BYTE		Sp[8];
	BYTE		Result;
	BOOL		RetVal;

	/* Query the user for the SP. */
	RetVal = GetSP(hWndParent, hDrive, Sp, Caption);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Get the user to use for changing the password. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, Sp, NULL, Caption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Get the new password information. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, Sp, _T("Authority To Change Password"), Caption, &NewTcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Start an authorized session to the SP. */
	Result = StartSession(&Session, hDrive, Sp, &TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		MessageBox(hWndParent, _T("An error occurred opening a session to the SP."), Caption, MB_OK);
		return;
	}

	/* Get the user's PIN table entry. */
	if(ReadTableCellBytes(&Session, NewTcgAuth.Authority, 10, NewTcgAuth.Authority, NULL) == FALSE) {
		/* If there was an error, just assume it's a user and use the appropriate credentials. */
		NewTcgAuth.Authority[3] = 0x0b;
	}

	/* Set the user's password. */
	Result = ChangeAuth(&Session, &NewTcgAuth);

	/* Close the session. */
	EndSession(&Session);

	/* Display the result of the change. */
	if(Result == 0) {
		MessageBox(hWndParent, _T("The password has been changed."), Caption, MB_OK);
	} else {
		MessageBox(hWndParent, _T("There was an error changing the password."), Caption, MB_OK);
	}
}
