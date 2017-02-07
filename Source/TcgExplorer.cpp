#include"stdafx.h"
#include<initguid.h>
#include<winioctl.h>
#include"TcgExplorer.h"
#include"AtaDrive.h"
#include"Table.h"
#include"Tcg.h"
#include"SpManagement.h"
#include"ChangePassword.h"
#include"TableDisplay.h"
#include"Uid.h"
#include"Memory.h"
#include"TestPassword.h"
#include"ByteTable.h"
#include"CryptInfo.h"
#include"Ranges.h"
#include"MbrControl.h"
#include"Msid.h"
#include"Level0.h"
#include"Users.h"
#include"Random.h"
#include"resource.h"


/* SetupAPI library needed. */
#pragma comment(lib, "setupapi.lib")

/* Level 1 information column headers. */
static LPTSTR	Level1Cols[] = {_T("Property"), _T("Value")};

/* Caption for error messages. */
static LPTSTR	Caption = _T("TCG Explorer");

/* If we can't include WinIOCtl.h, uncomment the following line. */
//DEFINE_GUID(GUID_DEVINTERFACE_DISK,                   0x53f56307L, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);


/*
 * Adds the drive to the list of drives.
 */
static void AddDriveToList(HWND hWnd, LPTCGDRIVE hDrive, LPTSTR DriveString)
{
	LPTSTR	Text;
	LVITEM	Item;
	TCHAR	ItemData[10];
	DWORD	DriveNumber;
	WORD	DriveType;
	int		Index;

	ZeroMemory(&Item, sizeof(Item));
	DriveNumber = hDrive->hDrive->DriveNumber;
	wsprintf(ItemData, _T("%d"), DriveNumber);
	Item.iItem = DriveNumber;
	Item.pszText = ItemData;
	Item.mask = LVIF_TEXT | LVIF_PARAM;
	Text = (LPTSTR)MemAlloc((lstrlen(DriveString) + 1) * sizeof(TCHAR));
	lstrcpy(Text, DriveString);
	Item.lParam = (LPARAM)Text;
	Index = SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTITEM, 0, (LPARAM)&Item);
	Item.mask = LVIF_TEXT;
	Item.iSubItem = 1;
	Item.pszText = hDrive->hDrive->Model;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_SETITEMTEXT, Index, (LPARAM)&Item);
	Item.iSubItem = 2;
	Item.pszText = hDrive->hDrive->Serial;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_SETITEMTEXT, Index, (LPARAM)&Item);
	Item.iSubItem = 3;
	DriveType = GetTCGDriveType(hDrive);
	Item.pszText = GetDriveTypeString(DriveType);
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_SETITEMTEXT, Index, (LPARAM)&Item);
	Item.iSubItem = 4;
	Item.pszText = GetDriveBusString(hDrive->hDrive);
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_SETITEMTEXT, Index, (LPARAM)&Item);
}


/*
 * Find all hard drives by number: PhysicalDrive0, etc
 */
static void EnumDrivesByNumber(HWND hWnd)
{
	LPTCGDRIVE	hDrive;
	TCHAR		DriveString[100];
	int			i;

	for(i=0; i<MAXSUPPORTEDDRIVE; i++) {
		wsprintf(DriveString, _T("\\\\.\\PhysicalDrive%d"), i);
		hDrive = OpenTcgDrive(DriveString);
		if(hDrive != NULL) {
			AddDriveToList(hWnd, hDrive, DriveString);
			CloseTcgDrive(hDrive);
		}
	}
}


/*
 * Find all hard drives by GUID.
 */
static void EnumDrivesByGuid(HWND hWnd)
{
	PSP_DEVICE_INTERFACE_DETAIL_DATA	DetailData;
	SP_DEVICE_INTERFACE_DATA			DevInterfaceData;
	SP_DEVINFO_DATA						DevInfoData;
	LPTCGDRIVE							hDrive;
	HDEVINFO							DevInfo;
	DWORD								Size;
	DWORD								Index;
	BOOL								Result;

	DevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if(DevInfo == INVALID_HANDLE_VALUE) {
		return;
	}

	Index = 0;
	do {
		DevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		Result = SetupDiEnumDeviceInterfaces(DevInfo, NULL, &GUID_DEVINTERFACE_DISK, Index, &DevInterfaceData);
		Index++;
		if(Result) {
			Size = 0;
			SetupDiGetDeviceInterfaceDetail(DevInfo, &DevInterfaceData, NULL, 0, &Size, NULL);
			DetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)MemCalloc(Size);
			DetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			SetupDiGetDeviceInterfaceDetail(DevInfo, &DevInterfaceData, DetailData, Size, &Size, &DevInfoData);
			hDrive = OpenTcgDrive(DetailData->DevicePath);
			if(hDrive != NULL) {
				AddDriveToList(hWnd, hDrive, DetailData->DevicePath);
				CloseTcgDrive(hDrive);
			}
			MemFree(DetailData);
		}
	} while(GetLastError() != ERROR_NO_MORE_ITEMS);

	SetupDiDestroyDeviceInfoList(DevInfo);
}


/*
 * Enumerate hard drives.
 */
static void EnumDrives(HWND hWnd)
{
	MENUITEMINFO	mii;
	HMENU			hMenu;

	/* Remove all drives from the list. */
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_DELETEALLITEMS, 0, 0);

	/* Get the submenu with the user's enumeration selection. */
	hMenu = GetSubMenu(GetMenu(hWnd), 0);

	/* Determine whether the first is checked. */
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	GetMenuItemInfo(hMenu, IDM_BYNUMBER, FALSE, &mii);

	/* Enumerate appropriately. */
	if(mii.fState & MFS_CHECKED) {
		EnumDrivesByNumber(hWnd);
	} else {
		EnumDrivesByGuid(hWnd);
	}
}


/*
 * Opens a drive given the index of the drive in the list.
 */
static LPTCGDRIVE OpenDriveByIndex(HWND hWnd, int Index)
{
	LVITEM	Item;

	/* Get the lParam of the item. */
	memset(&Item, 0, sizeof(Item));
	Item.mask = LVIF_PARAM;
	Item.iItem = Index;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_GETITEM, Index, (LPARAM)&Item);

	return OpenTcgDrive((LPTSTR)Item.lParam);
}


/*
 * The handler for the WM_SYSCOMMAND message.
 */
static BOOL SysCommandHandler(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	/* Mask off the system bits. */
	wParam &= 0xfff0;

	/* We only handle the close message, and end the dialog box. */
	if(wParam == SC_CLOSE) {
		EndDialog(hWnd, 0);
		return TRUE;
	}

	/* We didn't handle any other messages. */
	return FALSE;
}


/*
 * The handler for the WM_INITDIALOG message.
 */
static BOOL InitDialogHandler(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO	mii;
	LVCOLUMN		Column;
	HICON			hIcon;
	HMENU			hMenu;
	RECT			Rect;
	int				Width;
	int				Total;

	/* Set the checkmark on the menu item. */
	hMenu = GetSubMenu(GetMenu(hWnd), 0);
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_CHECKED;
	SetMenuItemInfo(hMenu, IDM_BYGUID, FALSE, &mii);

	/* Set the icons for the dialog box. */
	hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DRIVE), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DRIVE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	/* Get the width of the report header. */
	GetWindowRect(GetDlgItem(hWnd, IDC_LISTDRIVES), &Rect);
	Width = Rect.right - Rect.left - 2*GetSystemMetrics(SM_CXEDGE);
	Total = 0;

	/* Set list view styles. */
	ListView_SetExtendedListViewStyle(GetDlgItem(hWnd, IDC_LISTDRIVES), LVS_EX_FULLROWSELECT);

	/* Add a dummy column to delete later. The first column has limited formatting. */
	ZeroMemory(&Column, sizeof(Column));
	Column.pszText = _T("Dummy");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem = 0;
	Column.cx = 1;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);

	/* Initialize the report header. */
	Column.pszText = _T("Drive");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem++;
	Column.cx = Width/10;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);
	Total += Column.cx;
	Column.pszText = _T("Model");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem++;
	Column.cx = 45*Width/100;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);
	Total += Column.cx;
	Column.pszText = _T("Serial");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem++;
	Column.cx = 25*Width/100;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);
	Total += Column.cx;
	Column.pszText = _T("Type");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem++;
	Column.cx = Width/10;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);
	Total += Column.cx;
	Column.pszText = _T("Bus");
	Column.cchTextMax = lstrlen(Column.pszText);
	Column.iSubItem++;
	Column.cx = Width - Total;
	Column.fmt = LVCFMT_CENTER;
	Column.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT;
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_INSERTCOLUMN, Column.iSubItem, (LPARAM)&Column);

	/* Delete the dummy column. */
	SendDlgItemMessage(hWnd, IDC_LISTDRIVES, LVM_DELETECOLUMN, 0, 0);

	/* Query each drive in turn. */
	EnumDrives(hWnd);

	/* Let the default control get the focus. */
	return TRUE;
}


/*
 * Internal helper routine to get the selected drive number.
 */
static int GetSelectedDrive(HWND hWnd)
{
	int		Index;

	/* Get the index of the selected row. */
	Index = ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTDRIVES), -1, LVNI_SELECTED);

	/* If nothing is selected, notify the user. */
	if(Index == -1) {
		MessageBox(hWnd, _T("No drive was selected!"), Caption, MB_ICONSTOP | MB_OK);
	}

	/* Return the index. */
	return Index;
}


/*
 * This handles the WM_COMMAND message.
 */
static BOOL CommandHandler(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO	mii;
	LPTCGDRIVE		hDrive;
	LPTABLE			Table;
	HMENU			hMenu;
	BOOL			Result;
	int				Index;

	switch(LOWORD(wParam)) {
		case IDM_EXIT:
			EndDialog(hWnd, 0);
			break;
		case IDM_REFRESH:
			EnumDrives(hWnd);
			break;
		case IDM_BYNUMBER:
			hMenu = GetSubMenu(GetMenu(hWnd), 0);
			ZeroMemory(&mii, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_STATE;
			mii.fState = MFS_CHECKED;
			SetMenuItemInfo(hMenu, IDM_BYNUMBER, FALSE, &mii);
			mii.fState = MFS_UNCHECKED;
			SetMenuItemInfo(hMenu, IDM_BYGUID, FALSE, &mii);
			EnumDrives(hWnd);
			break;
		case IDM_BYGUID:
			hMenu = GetSubMenu(GetMenu(hWnd), 0);
			ZeroMemory(&mii, sizeof(mii));
			mii.cbSize = sizeof(mii);
			mii.fMask = MIIM_STATE;
			mii.fState = MFS_UNCHECKED;
			SetMenuItemInfo(hMenu, IDM_BYNUMBER, FALSE, &mii);
			mii.fState = MFS_CHECKED;
			SetMenuItemInfo(hMenu, IDM_BYGUID, FALSE, &mii);
			EnumDrives(hWnd);
			break;
		case IDM_LEVEL0:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				Result = Level0Discovery(hDrive);
				if(Result == FALSE) {
					MessageBox(hWnd, _T("There was an error retrieving Level 0 Discovery information."), Caption, MB_ICONERROR | MB_OK);
				} else {
					Level0Info(hWnd, hDrive->hDrive->Scratch);
				}
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_LEVEL1:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				Table = Level1Discovery(hDrive, NULL);
				CloseTcgDrive(hDrive);
				if(Table != NULL) {
					DisplayGenericTable(hWnd, Table, _T("Level 1 Information"), Level1Cols);
					FreeTable(Table);
				} else {
					MessageBox(hWnd, _T("There was an error retrieving the Level 1 Discovery information."), Caption, MB_OK);
				}
			}
			break;
		case IDM_LOCKRANGES:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				LockRanges(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_ERASERANGE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				EraseRanges(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_RANGE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				ModifyRanges(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_ENCRYPTION:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				CryptInfo(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_TESTPASSWORD:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				TestPassword(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_SAVEDATASTORE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				ReadByteTableByUid(hWnd, hDrive, TABLE_DATASTORE.Uid, SP_LOCKING.Uid);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_WRITEDATASTORE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				WriteByteTableByUid(hWnd, hDrive, TABLE_DATASTORE.Uid, SP_LOCKING.Uid);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_SAVEMBR:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				ReadByteTableByUid(hWnd, hDrive, TABLE_MBR.Uid, SP_LOCKING.Uid);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_WRITEMBR:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				WriteByteTableByUid(hWnd, hDrive, TABLE_MBR.Uid, SP_LOCKING.Uid);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_READARBTABLE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				ReadArbitraryByteTable(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_WRITEARBTABLE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				WriteArbitraryByteTable(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_MBRCONTROL:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				MbrControl(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_TABLES:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				DisplayTable(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_ACTIVATE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				Activate(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_GETMSID:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				GetMSID(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_CHANGEPASS:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				ChangePassword(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_REVERT:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				DoRevert(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_REVERTLOCKINGSP:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				DoRevertLockingSp(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_REVERTDRIVE:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				RevertDrive(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_USERS:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				Users(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
		case IDM_RANDOM:
			Index = GetSelectedDrive(hWnd);
			if(Index != -1) {
				hDrive = OpenDriveByIndex(hWnd, Index);
				Random(hWnd, hDrive);
				CloseTcgDrive(hDrive);
			}
			break;
	}

	/* We handle these messages. */
	return TRUE;
}


/*
 * This is the handler for the WM_NOTIFY message.
 */
static BOOL NotifyHandler(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNMLISTVIEW	ListView;
	LVITEM			Item;
	LPNMHDR			Hdr;
	POINT			Point;
	HMENU			hMenu;

	/* Make sure it's for the list view control. */
	if(wParam != IDC_LISTDRIVES) {
		return TRUE;
	}

	/* Handle right click messages. */
	Hdr = (LPNMHDR)lParam;
	switch(Hdr->code) {
		case NM_RCLICK:
			/* If nothing is selected, return. */
			if(ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTDRIVES), -1, LVNI_SELECTED) == -1) {
				return TRUE;
			}

			/* Get the popup menu we wish to display. */
			hMenu = GetSubMenu(GetMenu(hWnd), 1);

			/* Get the location of the mouse pointer in screen coordinates. */
			GetCursorPos(&Point);

			/* Display the pop-up menu. */
			TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, Point.x, Point.y, hWnd, NULL);
		break;
		case LVN_DELETEALLITEMS:
			return FALSE;
			break;
		case LVN_DELETEITEM:
			ListView = (LPNMLISTVIEW)lParam;
			ZeroMemory(&Item, sizeof(Item));
			Item.iItem = ListView->iItem;
			Item.mask = LVIF_PARAM;
			ListView_GetItem(GetDlgItem(hWnd, IDC_LISTDRIVES), &Item);
			MemFree((LPVOID)Item.lParam);
			break;
		default:
			break;
	}

	/* Always return true, since we process this message. */
	return TRUE;
}


/*
 * The message handler for the TCG Explorer dialog box.
 */
static BOOL CALLBACK ExplorerFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	/* Handle our messages here. */
	switch(Msg) {
		case WM_INITDIALOG:
			return InitDialogHandler(hWnd, wParam, lParam);
			break;
		case WM_SYSCOMMAND:
			return SysCommandHandler(hWnd, wParam, lParam);
			break;
		case WM_COMMAND:
			return CommandHandler(hWnd, wParam, lParam);
			break;
		case WM_NOTIFY:
			return NotifyHandler(hWnd, wParam, lParam);
			break;
	}

	/* Indicate we didn't process the message, or we still want the system to process it. */
	return FALSE;
}


/*
 * Start the TCG Explorer dialog box.
 */
void TcgExplorer(void)
{
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), HWND_DESKTOP, ExplorerFunc);
}
