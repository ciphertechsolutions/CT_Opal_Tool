#ifndef PROGRESS_H_
#define PROGRESS_H_


/* Function prototypes. */
void ProcessMessages(HWND hWnd);
void UpdateProgressBox(HWND hWnd, DWORD Current);
BOOL IsProgressCancelled(HWND hWnd);
HWND CreateProgressBox(HWND hWndParent, LPTSTR Name, DWORD Size);


#endif /* PROGRESS_H_ */
