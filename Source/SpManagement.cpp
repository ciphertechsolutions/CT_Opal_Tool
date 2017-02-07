#include"stdafx.h"
#include"Memory.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"SpManagement.h"
#include"Uid.h"
#include"GetSp.h"
#include"GetUser.h"
#include"resource.h"


/* Captions for error messages. */
static LPTSTR	ActivateCaption = _T("Activate SP");
static LPTSTR	RevertCaption = _T("Revert SP");
static LPTSTR	RevertLockingCaption = _T("Revert Locking SP");
static LPTSTR	RevertDriveCaption = _T("Revert Drive");


/*
 * Activate an SP.
 */
void Activate(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGSESSION		Session;
	TCGAUTH			TcgAuth;
	BOOL			RetVal;
	BYTE			Result;
	BYTE			Sp[8];


	/* Query the user for the SP to activate. */
	RetVal = GetSP(hWndParent, hDrive, Sp, ActivateCaption);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Query the user for the drive user/admin to use. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, SP_ADMIN.Uid, NULL, ActivateCaption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Start authenticated session to the Admin SP. */
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, &TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		MessageBox(hWndParent, _T("An error occurred opening a session."), ActivateCaption, MB_OK);
		return;
	}

	/* Activate the SP. */
	RetVal = Activate_SP(&Session, Sp);

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Indicate success/error message. */
	if(RetVal != 0) {
		MessageBox(hWndParent, _T("There was an error activating the SP."), ActivateCaption, MB_OK);
	} else {
		MessageBox(hWndParent, _T("The SP was activated successfully."), ActivateCaption, MB_OK);
	}
}


/*
 * Revert an SP through the Admin SP's SP Table.
 */
void DoRevert(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGSESSION		Session;
	TCGAUTH			TcgAuth;
	BOOL			RetVal;
	BYTE			Result;
	BYTE			Sp[8];


	/* Query the user for the SP to revert. */
	RetVal = GetSP(hWndParent, hDrive, Sp, RevertCaption);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Query the user for the drive user/admin to use. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, SP_ADMIN.Uid, NULL, RevertCaption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Start authenticated session to the Admin SP. */
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, &TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		MessageBox(hWndParent, _T("An error occurred opening a session."), RevertCaption, MB_OK);
		return;
	}

	/* Revert the SP. */
	RetVal = Revert(&Session, Sp);

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Indicate success/error message. */
	if(RetVal != 0) {
		MessageBox(hWndParent, _T("There was an error reverting the SP."), RevertCaption, MB_OK);
	} else {
		MessageBox(hWndParent, _T("The SP was successfully reverted."), RevertCaption, MB_OK);
	}
}


/*
 * Revert the Locking SP through the Locking SP.
 */
void DoRevertLockingSp(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGSESSION		Session;
	TCGAUTH			TcgAuth;
	BOOL			RetVal;
	BOOL			KeepGlobalRange;
	BYTE			Result;


	/* Check that the Locking SP is enabled. */
	if(IsLockingEnabled(hDrive) == FALSE) {
		MessageBox(hWndParent, _T("The Locking SP is not enabled."), RevertCaption, MB_ICONERROR | MB_OK);
		return;
	}

	/* Query the user for the drive user/admin to use. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, SP_LOCKING.Uid, NULL, RevertCaption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Start authenticated session to the Locking SP. */
	Result = StartSession(&Session, hDrive, SP_LOCKING.Uid, &TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		MessageBox(hWndParent, _T("An error occurred opening a session to the Locking SP."), RevertCaption, MB_OK);
		return;
	}

	/* Ask the user whether or not to keep the global range key. */
	KeepGlobalRange = (MessageBox(hWndParent, _T("Do you wish to keep the global range key?"), _T("Global Range Key"), MB_YESNO) == IDYES);

	/* Revert the drive. */
	RetVal = RevertSP(&Session, KeepGlobalRange);

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Indicate success/error message. */
	if(RetVal != 0) {
		MessageBox(hWndParent, _T("There was an error reverting the Locking SP."), RevertCaption, MB_OK);
	} else {
		MessageBox(hWndParent, _T("The Locking SP was successfully reverted."), RevertCaption, MB_OK);
	}
}


/*
 * Revert a drive to its Original Factory State (OFS)
 */
void RevertDrive(HWND hWndParent, LPTCGDRIVE hDrive)
{
	TCGSESSION		Session;
	TCGAUTH			TcgAuth;
	BOOL			RetVal;
	BYTE			Result;


	/* Query the user for the drive user/admin to use. */
	RetVal = GetUserAuthInfo(hWndParent, hDrive, SP_ADMIN.Uid, NULL, RevertCaption, &TcgAuth);

	/* If the user cancelled the query, return. */
	if(RetVal == FALSE) {
		return;
	}

	/* Start authenticated session to the Admin SP. */
	Result = StartSession(&Session, hDrive, SP_ADMIN.Uid, &TcgAuth);

	/* If there was a problem, return. */
	if(Result != 0) {
		MessageBox(hWndParent, _T("An error occurred opening a session."), RevertCaption, MB_OK);
		return;
	}

	/* Revert the drive. */
	RetVal = Revert(&Session, SP_ADMIN.Uid);

	/* Close the session, we're done for now. */
	EndSession(&Session);

	/* Indicate success/error message. */
	if(RetVal != 0) {
		MessageBox(hWndParent, _T("There was an error reverting the drive."), RevertCaption, MB_OK);
	} else {
		MessageBox(hWndParent, _T("The drive was successfully reverted."), RevertCaption, MB_OK);
	}
}
