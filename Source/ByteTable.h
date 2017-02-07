#ifndef BYTETABLE_H_
#define BYTETABLE_H_


void ReadByteTableByUid(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Uid, LPBYTE Sp);
void WriteByteTableByUid(HWND hWndParent, LPTCGDRIVE hDrive, LPBYTE Uid, LPBYTE Sp);
void ReadArbitraryByteTable(HWND hWndParent, LPTCGDRIVE hDrive);
void WriteArbitraryByteTable(HWND hWndParent, LPTCGDRIVE hDrive);


#endif /* BYTETABLE_H_ */
