#include"stdafx.h"
#include"Memory.h"
#include"AtaDrive.h"
#include"Level0.h"
#include"resource.h"


/* The subclass property string. */
static LPTSTR SubClassString = _T("Level0 Subclass");


/* These must all be byte-aligned. */
#pragma pack(push, 1)

/* The feature descriptor header. */
typedef struct tagFeatureDescriptor {
	WORD		FeatureCode;
	BYTE		Version;
	BYTE		Length;
} FEATURE, *LPFEATURE;


/* The TPer Feature Descriptor. */
typedef struct tagTperFeature {
	FEATURE		Feature;
	BYTE		Features;
	BYTE		Padding[11];
} TPERFEATURE, *LPTPERFEATURE;


/* The Locking Feature Descriptor. */
typedef struct tagLockingFeature {
	FEATURE		Feature;
	BYTE		Features;
	BYTE		Padding[11];
} LOCKINGFEATURE, *LPLOCKINGFEATURE;


/* The Geometry Feature Descriptor. */
typedef struct tagGeometryFeature {
	FEATURE		Feature;
	BYTE		Align;
	BYTE		Padding[7];
	DWORD		LogicalBlockSize;
	QWORD		AlignmentGranularity;
	QWORD		LowestAlignedLBA;
} GEOMETRYFEATURE, *LPGEOMETRYFEATURE;


/* The DataStore Feature Descriptor. */
typedef struct tagDataStoreFeature {
	FEATURE		Feature;
	WORD		Padding;
	WORD		MaxNumber;
	DWORD		TotalSize;
	DWORD		SizeAlignment;
} DATASTOREFEATURE, *LPDATASTOREFEATURE;


/* The Single Used Mode Feature Descriptor. */
typedef struct tagSingleUserModeFeature {
	FEATURE		Feature;
	DWORD		NumLocking;
	BYTE		Flags;
} SINGLEUSERMODEFEATURE, *LPSINGLEUSERMODEFEATURE;


/* The common header for an Opal SSC. */
/* Note that Opalite and Pyrite use a subset of these. */
typedef struct tagOPALSSC {
	FEATURE		Feature;
	WORD		BaseComId;
	WORD		NumberComIds;
	BYTE		RangeCrossing;
	/* The following are for Opal 2.0. */
	WORD		NumAdmins;
	WORD		NumUsers;
	BYTE		PinIndicator;
	BYTE		PinRevert;
} OPALSSC, *LPOPALSSC;

/* Restore packing alignment. */
#pragma pack(pop)


/*
 * Counts the number of lines in a string.
 */
static int CountLines(LPTSTR String)
{
	int		i;
	int		Count;

	Count = 1;
	for(i=0; String[i]!=0 && String[i+1]!=0; i++) {
		if((String[i] == _T('\r')) && (String[i+1] == _T('\n'))) {
			Count++;
		}
	}
	return Count;
}


/*
 * This is the subclassed function for the edit window.
 */
static LRESULT CALLBACK NewEditProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC		OrigProc;
	LRESULT		Result;

	/* Get the original procedure. */
	OrigProc = (WNDPROC)GetProp(hWnd, SubClassString);

	/* Process the messages we care about. */
	switch(Msg) {
		case WM_GETDLGCODE:
			Result = CallWindowProc(OrigProc, hWnd, Msg, wParam, lParam);
			return Result & ~DLGC_HASSETSEL;
			break;
		case WM_DESTROY:
			RemoveProp(hWnd, SubClassString);
			SetWindowLong(hWnd, GWL_WNDPROC, (LONG)OrigProc);
			break;
		default:
			break;
	}

	/* Call the original routine. */
	return CallWindowProc(OrigProc, hWnd, Msg, wParam, lParam);
}


/*
 * This is the dialog function for displaying the level 0 discovery information for the drive.
 */
static BOOL CALLBACK Level0DisplayFunc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC		OldWndProc;
	LPTSTR		String;
	HFONT		hFontOld;
	HFONT		hFont;
	HWND		hWndEdit;
	RECT		Rect;
	SIZE		Size;
	HDC			hDC;
	int			NumLines;

	switch(Msg) {
		case WM_INITDIALOG:
			hWndEdit = GetDlgItem(hWnd, IDC_LEVEL0INFO);
			String = (LPTSTR)lParam;
			SetWindowText(hWndEdit, String);
			hDC = GetDC(hWnd);
			hFont = CreateFont(-MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE | FIXED_PITCH, _T("Courier"));
			ReleaseDC(hWnd, hDC);
			SendMessage(hWndEdit, WM_SETFONT, (WPARAM)hFont, FALSE);
			hDC = GetDC(hWndEdit);
			hFontOld = (HFONT)SelectObject(hDC, hFont);
			GetTextExtentPoint32(hDC, String, lstrlen(String), &Size);
			SelectObject(hDC, hFontOld);
			ReleaseDC(hWnd, hDC);
			GetClientRect(hWndEdit, &Rect);
			NumLines = CountLines(String);
			if(NumLines <= (Rect.bottom / Size.cy)) {
				ShowScrollBar(hWndEdit, SB_VERT, FALSE);
			}
			OldWndProc = (WNDPROC)GetWindowLong(hWndEdit, GWL_WNDPROC);
			SetProp(hWndEdit, SubClassString, OldWndProc);
			SetWindowLong(hWndEdit, GWL_WNDPROC, (LONG)NewEditProc);
			return TRUE;
			break;
		case WM_CTLCOLORSTATIC:
			return (BOOL)GetSysColorBrush(COLOR_WINDOW);
			break;
		case WM_SYSCOMMAND:
			switch(wParam & 0xfff0) {
				case SC_CLOSE:
					EndDialog(hWnd, 0);
					return TRUE;
					break;
			}
			break;
	}

	return FALSE;
}


/*
 * Prints the header information for each feature.
 */
static void PrintFeatureHeader(LPFEATURE Feature, LPTSTR Header, LPTSTR Buffer)
{
	TCHAR		Text[100];
	int			i;

	/* Print the carriage return for the previous line. */
	if(Buffer[0] != 0) {
		lstrcat(Buffer, _T("\r\n\r\n"));
	}

	/* Add the header. */
	lstrcat(Buffer, _T(" "));
	lstrcat(Buffer, Header);
	lstrcat(Buffer, _T("\r\n"));

	/* Add the underline. */
	Text[0] = ' ';
	for(i=0; Header[i]; i++) {
		Text[i+1] = '-';
	}

	/* Add the version information. */
	Text[i+1] = 0;
	lstrcat(Buffer, Text);
	lstrcat(Buffer, _T("\r\n"));
	wsprintf(Text, _T(" Version:            %d"), (Feature->Version) >> 4);
	lstrcat(Buffer, Text);
}


/*
 * To save on multiple if statments, print strings based on true or false values.
 */
static void PrintBoolean(LPTSTR Buffer, LPTSTR String, BOOL Value, LPTSTR TrueString, LPTSTR FalseString)
{
	lstrcat(Buffer, String);
	if(Value) {
		lstrcat(Buffer, TrueString);
	} else {
		lstrcat(Buffer, FalseString);
	}
}


/*
 * This is used in multiple places, so print the action of the PIN as defined by the vendor.
 */
static void PrintPinFeature(LPTSTR Buffer, BYTE Value)
{
	switch(Value) {
		case 0x00:
			lstrcat(Buffer, _T("MSID"));
			break;
		case 0xff:
			lstrcat(Buffer, _T("Vendor Defined"));
			break;
		default:
			lstrcat(Buffer, _T("Unknown"));
			break;
	}
}


/*
 * Print information from the Tper Feature Descriptor.
 */
static void PrintTperFeatures(LPTPERFEATURE Feature, LPTSTR Buffer)
{
	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("TPer Features"), Buffer);

	/* Print the rest of the information. */
	PrintBoolean(Buffer, _T("\r\n ComID Management:   "), Feature->Features & 0x40, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n Streaming           "), Feature->Features & 0x10, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n Buffer Management:  "), Feature->Features & 0x08, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n ACK/NACK:           "), Feature->Features & 0x04, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n Async:              "), Feature->Features & 0x02, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n Sync:               "), Feature->Features & 0x01, _T("Supported"), _T("Not Supported"));
}


/*
 * Print information from the Locking Feature Descriptor.
 */
static void PrintLockingFeatures(LPLOCKINGFEATURE Feature, LPTSTR Buffer)
{
	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("Locking Features"), Buffer);

	/* Print the rest of the information. */
	PrintBoolean(Buffer, _T("\r\n MBR Done:           "), Feature->Features & 0x20, _T("Yes"), _T("No"));
	PrintBoolean(Buffer, _T("\r\n MBR:                "), Feature->Features & 0x10, _T("Enabled"), _T("Disabled"));
	PrintBoolean(Buffer, _T("\r\n Media Encryption:   "), Feature->Features & 0x08, _T("Supported"), _T("Not Supported"));
	PrintBoolean(Buffer, _T("\r\n Locked:             "), Feature->Features & 0x04, _T("Yes"), _T("No"));
	PrintBoolean(Buffer, _T("\r\n Locking Enabled:    "), Feature->Features & 0x02, _T("Enabled"), _T("Disabled"));
	PrintBoolean(Buffer, _T("\r\n Locking Supported:  "), Feature->Features & 0x01, _T("Supported"), _T("Not Supported"));
}


/*
 * Print information from the Geometry Feature Descriptor.
 */
static void PrintGeometryFeatures(LPGEOMETRYFEATURE Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("Geometry Features"), Buffer);

	/* Print the rest of the information. */
	PrintBoolean(Buffer, _T("\r\n Alignment required: "), Feature->Align & 0x01, _T("True"), _T("False"));
	wsprintf(Text, _T("\r\n Logical Block Size: %d"), BE_TO_LE_32(Feature->LogicalBlockSize));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Granularity:        %I64d"), BE_TO_LE_64(Feature->AlignmentGranularity));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Lowest Aligned LBA: %I64d"), BE_TO_LE_64(Feature->LowestAlignedLBA));
	lstrcat(Buffer, Text);
}


/*
 * Print information from the Opal or Enterprise Feature Descriptor.
 */
static void PrintOpalFeatures(LPOPALSSC Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("Opal Features"), Buffer);

	/* Print the rest of the information. */
	wsprintf(Text, _T("\r\n Base ComID:         0x%04x"), BE_TO_LE_16(Feature->BaseComId));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Number of ComIDs:   %d"), BE_TO_LE_16(Feature->NumberComIds));
	lstrcat(Buffer, Text);
	PrintBoolean(Buffer, _T("\r\n Range Crossing:     "), Feature->RangeCrossing & 0x01, _T("Not Supported"), _T("Supported"));

	/* Process Opal 2.0 features. */
	if(BE_TO_LE_16(Feature->Feature.FeatureCode) == 0x0203) {
		wsprintf(Text, _T("\r\n Number of Admins:   %d"), BE_TO_LE_16(Feature->NumAdmins));
		lstrcat(Buffer, Text);
		wsprintf(Text, _T("\r\n Number of Users:    %d"), BE_TO_LE_16(Feature->NumUsers));
		lstrcat(Buffer, Text);
		lstrcat(Buffer, _T("\r\n SID PIN Indicator:  "));
		PrintPinFeature(Buffer, Feature->PinIndicator);
		lstrcat(Buffer, _T("\r\n SID PIN Revert:     "));
		PrintPinFeature(Buffer, Feature->PinRevert);
	}
}


/*
 * Print information regarding additional data stores.
 */
static void PrintDatastoreFeatures(LPDATASTOREFEATURE Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("DataStore Features"), Buffer);

	/* Print the rest of the information. */
	wsprintf(Text, _T("\r\n Maximum Number:     %d"), BE_TO_LE_16(Feature->MaxNumber));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Total Size:         %d"), BE_TO_LE_32(Feature->TotalSize));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Size Alignment:     %d"), BE_TO_LE_32(Feature->SizeAlignment));
	lstrcat(Buffer, Text);
}


/*
 * Print information regarding Opalite or Pyrite features.
 */
static void PrintOpalitePyriteFeatures(LPOPALSSC Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	/* Determine if this is an Opalite drive or a Pyrite drive. */
	if(BE_TO_LE_16(Feature->Feature.FeatureCode) == 0x0301) {
		PrintFeatureHeader((LPFEATURE)Feature, _T("Opalite Features"), Buffer);
	} else {
		PrintFeatureHeader((LPFEATURE)Feature, _T("Pyrite Features"), Buffer);
	}

	/* Print the rest of the information. */
	wsprintf(Text, _T("\r\n Base ComID:         0x%04x"), BE_TO_LE_16(Feature->BaseComId));
	lstrcat(Buffer, Text);
	wsprintf(Text, _T("\r\n Number of ComIDs:   %d"), BE_TO_LE_16(Feature->NumberComIds));
	lstrcat(Buffer, Text);

	/* Process other features. */
	if(BE_TO_LE_16(Feature->Feature.FeatureCode) == 0x0203) {
		lstrcat(Buffer, _T("\r\n SID PIN Indicator:  "));
		PrintPinFeature(Buffer, Feature->PinIndicator);
		lstrcat(Buffer, _T("\r\n SID PIN Revert:     "));
		PrintPinFeature(Buffer, Feature->PinRevert);
	}
}


/*
 * Print information associated with Single User Mode.
 */
static void PrintSingleUserModeFeatures(LPSINGLEUSERMODEFEATURE Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	/* Print the feature header. */
	PrintFeatureHeader((LPFEATURE)Feature, _T("Single User Mode Features"), Buffer);

	/* Print the remaining features. */
	wsprintf(Text, _T("\r\n Locking Objects:    %d"), BE_TO_LE_32(Feature->NumLocking));
	lstrcat(Buffer, Text);
	PrintBoolean(Buffer, _T("\r\n Any Objects in SUM: "), Feature->Flags & 0x01, _T("True"), _T("False"));
	PrintBoolean(Buffer, _T("\r\n All Objects in SUM: "), Feature->Flags & 0x02, _T("True"), _T("False"));
	PrintBoolean(Buffer, _T("\r\n Policy:             "), Feature->Flags & 0x04, _T("Admin Controlled"), _T("User Controlled"));
}


/*
 * Prints information for each feature into the buffer.
 */
static void PrintFeatureInformation(LPFEATURE Feature, LPTSTR Buffer)
{
	TCHAR	Text[100];

	switch(BE_TO_LE_16(Feature->FeatureCode)) {
		case 0x0001:
			PrintTperFeatures((LPTPERFEATURE)Feature, Buffer);
			break;
		case 0x0002:
			PrintLockingFeatures((LPLOCKINGFEATURE)Feature, Buffer);
			break;
		case 0x0003:
			PrintGeometryFeatures((LPGEOMETRYFEATURE)Feature, Buffer);
			break;
		case 0x0100:
		case 0x0200:
		case 0x0203:
			PrintOpalFeatures((LPOPALSSC)Feature, Buffer);
			break;
		case 0x0201:
			PrintSingleUserModeFeatures((LPSINGLEUSERMODEFEATURE)Feature, Buffer);
			break;
		case 0x0202:
			PrintDatastoreFeatures((LPDATASTOREFEATURE)Feature, Buffer);
			break;
		case 0x0301:
		case 0x0302:
			PrintOpalitePyriteFeatures((LPOPALSSC)Feature, Buffer);
			break;
		default:
			if(BE_TO_LE_16(Feature->FeatureCode) < 0xc000) {
				wsprintf(Text, _T("Unknown Features: 0x%04x"), BE_TO_LE_16(Feature->FeatureCode));
			} else {
				wsprintf(Text, _T("Unknown Vendor Features: 0x%04x"), BE_TO_LE_16(Feature->FeatureCode));
			}
			PrintFeatureHeader(Feature, Text, Buffer);
			break;
	}
}


/*
 * Display level 0 information about the drive.
 */
void Level0Info(HWND hWnd, LPBYTE Buffer)
{
	LPBYTE		Offset;
	LPTSTR		TextBuffer;

	/* Allocate memory for the buffer. */
	TextBuffer = (LPTSTR)MemAlloc(64*1024);
	if(TextBuffer == NULL) {
		MessageBox(hWnd, _T("Out of resources - not enough memory."), _T("Level 0 Discovery Information"), MB_ICONERROR | MB_OK);
	}
	TextBuffer[0] = 0;

	/* Iterate through the descriptors. */
	Offset = &Buffer[0x30];
	while(((LPFEATURE)Offset)->Length != 0) {
		PrintFeatureInformation((LPFEATURE)Offset, TextBuffer);
		Offset += ((LPFEATURE)Offset)->Length + sizeof(FEATURE);
	}

	/* Display the information. */
	DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LEVEL0), hWnd, Level0DisplayFunc, (LPARAM)TextBuffer);

	/* Free up resources. */
	MemFree(TextBuffer);
}
