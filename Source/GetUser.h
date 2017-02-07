#ifndef GETUSER_H_
#define GETUSER_H_


LPTABLE GetUserTable(LPTCGDRIVE hDrive, LPBYTE SpUid);
BOOL GetUserAuthInfo(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Sp, LPTSTR Title, LPTSTR Caption, LPTCGAUTH TcgAuth);
BOOL GetPassword(HWND hWndParent, LPTCGDRIVE hDrive, LPTCGAUTH TcgAuth);


#endif /* GETUSER _H_ */
