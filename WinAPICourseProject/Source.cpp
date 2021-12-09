#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <wingdi.h>
#include <debugapi.h>
#include "resource.h"
#include <math.h>
#include <objidl.h>
#include <gdiplus.h>
#include "EdmondKarpAlgorithm.h"
using namespace Gdiplus;



#define DRAW_AREA_UPPER_LEFT_X 20
#define DRAW_AREA_UPPER_LEFT_Y 20
#define DRAW_AREA_LOWER_RIGHT_X 570
#define DRAW_AREA_LOWER_RIGHT_Y 520
#define DRAW_AREA_WIDTH 550
#define DRAW_AREA_HEIGHT 500



#define MENU_AREA_UPPER_LEFT_X 590
#define MENU_AREA_UPPER_LEFT_Y 5
#define MENU_AREA_WIDTH 170
#define MENU_AREA_HEIGHT 525



#define COLOR_TO_BE_TRANSPARENT RGB(153, 255, 204)


#define SRCBUTTONID 13
#define SNKBUTTONID 14
#define MEDBUTTONID 15
#define MAXFLOWBUTTONID 16

/// 
/// ////////////////////////////////////////////////////
///

typedef struct tagVertex {
	int medNumber;
	RECT vRect;
} VERTEX;



#define VERTEX_MAX 20
VERTEX vertices[VERTEX_MAX];
int v_ind = 0;


/// 
/// ////////////////////////////////////////////////////
///

typedef struct tagEdge {
	VERTEX* u;
	VERTEX* v;
	int cap;
} EDGE;

#define EDGE_MAX 100
EDGE edges[EDGE_MAX];
int e_ind = 0;

/// 
/// ////////////////////////////////////////////////////
///

short int SRC_IS_SELECTED = 0;
short int SRC_CREATED = 0;

short int SNK_IS_SELECTED = 0;
short int SNK_CREATED = 0;

short int MED_IS_SELECTED = 0;
short int MED_CREATED = 0;

short int DRAG_ACTIVE = 0;

static HBITMAP HSRCBitmap;
static HBITMAP HSRCMonoBitmap;
static HBITMAP HSelectedSRCBitmap;


static HBITMAP HSNKBitmap;
static HBITMAP HSNKMonoBitmap;
static HBITMAP HSelectedSNKBitmap;

#define MAX_MED 18
static HBITMAP HBMPs[MAX_MED];
static HBITMAP HBMPs_Selected[MAX_MED];
static HBITMAP HBMPs_Mono[MAX_MED];
static short int alreadyCreated[MAX_MED];

VERTEX* firstChosenP = NULL;
VERTEX* secondChosenP = NULL;

VERTEX* sinkP = NULL;
VERTEX* sourceP = NULL;

short int CAP_RETURN_RES = 0;
int MAX_FLOW_RES = 0;

BOOL CALLBACK RequestCapacityDlgProc(HWND, UINT, WPARAM, LPARAM);

netEdge (*globalPointerToNetwork)[100] = NULL;





//=================================================================================================================
wchar_t currFileName[256] = L"Безымянный.nw";
wchar_t saveFileName[256] = L"\0";
OPENFILENAME ofn;
OPENFILENAME sfn;
HWND groupButton;
//=================================================================================================================


int writeAllDataToFile(void) {

	FILE* file;
	file = _wfopen(saveFileName, L"w");
	
	if (file == NULL) return -1;


	fwprintf(file, L"%d\n", v_ind);
	for (int i = 0; i < v_ind; i++) {
		fwprintf(file, L"%d %d %d %d %d\n",
			vertices[i].vRect.left,
			vertices[i].vRect.top,
			vertices[i].vRect.right,
			vertices[i].vRect.bottom,
			vertices[i].medNumber);
	}

	fwprintf(file, L"%d\n", e_ind);
	for (int i = 0; i < e_ind; i++) {
		fwprintf(file, L"%d %d %d\n", edges[i].u->medNumber, edges[i].v->medNumber, edges[i].cap);
	}

	fclose(file);

	return 0;
}

int SaveAllDataToFile(void) {

	FILE* file;
	file = _wfopen(currFileName, L"w");

	if (file == NULL) return -1;


	fwprintf(file, L"%d\n", v_ind);
	for (int i = 0; i < v_ind; i++) {
		fwprintf(file, L"%d %d %d %d %d\n",
			vertices[i].vRect.left,
			vertices[i].vRect.top,
			vertices[i].vRect.right,
			vertices[i].vRect.bottom,
			vertices[i].medNumber);
	}

	fwprintf(file, L"%d\n", e_ind);
	for (int i = 0; i < e_ind; i++) {
		fwprintf(file, L"%d %d %d\n", edges[i].u->medNumber, edges[i].v->medNumber, edges[i].cap);
	}

	fclose(file);

	return 0;
}

void ResetEverything(void) {
	wcscpy(currFileName, L"Безымянный.nw");
	SetWindowText(groupButton, currFileName);
	e_ind = 0;
	v_ind = 0;
	sinkP = NULL;
	sourceP = NULL;
	firstChosenP = NULL;
	secondChosenP = NULL;
	SNK_IS_SELECTED = 0;    SNK_CREATED = 0;
	SRC_IS_SELECTED = 0;	SRC_CREATED = 0;
	MED_IS_SELECTED = 0;	MED_CREATED = 0;
	DRAG_ACTIVE = 0;

	for (int i = 0; i < MAX_MED; i++)
		alreadyCreated[i] = 0;
}

void processReadData(void) {
	sinkP = NULL;
	sourceP = NULL;
	firstChosenP = NULL;
	secondChosenP = NULL;
	SNK_IS_SELECTED = 0;    SNK_CREATED = 0;
	SRC_IS_SELECTED = 0;	SRC_CREATED = 0;
	MED_IS_SELECTED = 0;	MED_CREATED = 0;
	DRAG_ACTIVE = 0;

	for (int i = 0; i < MAX_MED; i++)
		alreadyCreated[i] = 0;

	for (int i = 0; i < v_ind; i++) {
		if (vertices[i].medNumber == -1) {
			sourceP = &vertices[i];
			SRC_CREATED = 1;
		}
		else if (vertices[i].medNumber == -2) {
			sinkP = &vertices[i];
			SNK_CREATED = 1;
		}
		else {
			alreadyCreated[vertices[i].medNumber] = 1;
		}
	}
}

int readAllDataFromFile(void) {
	FILE* file;
	file = _wfopen(currFileName, L"r");

	if (file == NULL) return -1;
	int buf;

	buf = fwscanf(file, L"%d\n", &v_ind);

	if (buf != 1) {
		ResetEverything();
		return -1;
	}

	for (int i = 0; i < v_ind; i++) {
		buf = fwscanf(file, L"%d %d %d %d %d\n",
			&vertices[i].vRect.left,
			&vertices[i].vRect.top,
			&vertices[i].vRect.right,
			&vertices[i].vRect.bottom,
			&vertices[i].medNumber);

		if (buf != 5) {
			ResetEverything();
			return -1;
		}
	}


	buf = fwscanf(file, L"%d\n", &e_ind);
	if (buf != 1) {
		ResetEverything();
		return -1;
	}

	for (int i = 0; i < e_ind; i++) {
		int u, v, cap;
		buf = fwscanf(file, L"%d %d %d\n", &u, &v, &cap);

		edges[i].u = NULL;
		edges[i].v = NULL;

		for (int j = 0; j < v_ind; j++)
			if (vertices[j].medNumber == u) {
				edges[i].u = &vertices[j];
				break;
			}

		if (edges[i].u == NULL) {
			ResetEverything();
			return -1;
		}


		for (int j = 0; j < v_ind; j++)
			if (vertices[j].medNumber == v) {
				edges[i].v = &vertices[j];
				break;
			}

		if (edges[i].v == NULL) {
			ResetEverything();
			return -1;
		}

		edges[i].cap = cap;

		if (buf != 3) {
			ResetEverything();
			return -1;
		}



	}

	fclose(file);

	processReadData();
	return 0;
}



void printCurrentStateDebug(void) {
	OutputDebugStringA("\n\n===============================================================================\n\n");
	char str[256];
	sprintf(str, "TOTAL NUMBER OF VERTICES: %d\n", v_ind);
	OutputDebugStringA(str);
	for (int i = 0; i < v_ind; i++) {
		sprintf(str, "\nARRAY INDEX: %d\nvMEDNUMBER: %d\n", i, vertices[i].medNumber);
		OutputDebugStringA(str);
		if (&vertices[i] == sourceP) OutputDebugStringA("IT'S A SOURCE!!!!!!!\n\n");
		if (&vertices[i] == sinkP) OutputDebugStringA("IT'S A SINK!!!!!!!\n\n");
	}
	OutputDebugStringA("\n\n===============================================================================\n\n");
}

int getGreatestVertexNumber(void) {
	int greatest = -1;
	for (int i = 0; i < v_ind; i++)
		if (vertices[i].medNumber > greatest)
			greatest = vertices[i].medNumber;
	return greatest;
}

void printCurrentStateDebugDifferent(void) {
	OutputDebugStringA("\n\n===============================================================================\n\n");
	char str[256];
	sprintf(str, "TOTAL NUMBER OF VERTICES: %d\n", v_ind);
	OutputDebugStringA(str);
	sprintf(str, "TOTAL NUMBER OF EDGES: %d\n", e_ind);
	OutputDebugStringA(str);

	int greatest = getGreatestVertexNumber() + 2;

	for (int i = 0; i < e_ind; i++) {
		sprintf(str, "\nFrom: %d to: %d with %d\n", (edges[i].u->medNumber == -2) ? greatest : edges[i].u->medNumber + 1, (edges[i].v->medNumber == -2) ? greatest : edges[i].v->medNumber + 1, edges[i].cap);
		OutputDebugStringA(str);
	}
	OutputDebugStringA("\n\n===============================================================================\n\n");
}

void RedrawDrawAreaBackground(HDC hMemDC) {
	RECT rect;

	SetRect(&rect, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
	FillRect(hMemDC, &rect, (HBRUSH)(COLOR_3DLIGHT + 1));
}


int getNotCreatedMedIndex(void);

int CreateVertex(int x, int y) {
	if (v_ind == VERTEX_MAX) {
		return -VERTEX_MAX;
	}
	else {

		if (SRC_IS_SELECTED) {

			if (SRC_CREATED) {
				return -1;
			}
			else {
				vertices[v_ind].medNumber = -1;
				sourceP = &vertices[v_ind];
				SRC_CREATED = 1;
			}

		}

		if (SNK_IS_SELECTED) {

			if (SNK_CREATED) {
				return -2;
			}
			else {
				vertices[v_ind].medNumber = -2;
				sinkP = &vertices[v_ind];
				SNK_CREATED = 1;
			}
		}

		if (MED_IS_SELECTED) {

			int res = getNotCreatedMedIndex();
			if (res == MAX_MED) {
				return -3;
			}
			else {
				vertices[v_ind].medNumber = res;
				MED_CREATED = 1;
			}

		}

		if (SRC_IS_SELECTED || SNK_IS_SELECTED || MED_IS_SELECTED) {
			SetRect(&vertices[v_ind].vRect, x - 21, y - 21, x + 21, y + 21);
			v_ind++;
			return 0;
		}
		else {
			return -5;
		}

	}
}

int CreateEdge(VERTEX *u, VERTEX *v, int capacity) {

	if (e_ind == EDGE_MAX) {
		return -EDGE_MAX;
	}
	else {
		edges[e_ind].u = u;
		edges[e_ind].v = v;
		edges[e_ind].cap = capacity;

		e_ind++;

		return 0;
	}

}

EDGE *EdgeExists(VERTEX* u, VERTEX* v) {
	for (int i = 0; i < e_ind; i++)
		if (edges[i].u == u && edges[i].v == v) return &edges[i];
	return NULL;
}

void DrawVertex(HDC hdc, VERTEX* v) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	int x = v->vRect.left;
	int y = v->vRect.top;

	if (v == sourceP)
		SelectObject(hMemDC, HSRCBitmap);
	else if (v == sinkP)
		SelectObject(hMemDC, HSNKBitmap);
	else {
		SelectObject(hMemDC, HBMPs[v->medNumber]);
		MED_CREATED = 0;
		alreadyCreated[v->medNumber] = 1;
	}

	TransparentBlt(hdc, x, y, 41, 41,
		hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);

	DeleteDC(hMemDC);
}


void DrawSelectedVertex(HDC hdc, VERTEX* v) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	int x = v->vRect.left;
	int y = v->vRect.top;

	if (v == sourceP)
		SelectObject(hMemDC, HSelectedSRCBitmap);
	else if (v == sinkP)
		SelectObject(hMemDC, HSelectedSNKBitmap);
	else
		SelectObject(hMemDC, HBMPs_Selected[v->medNumber]);

	TransparentBlt(hdc, x, y, 41, 41,
		hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);

	DeleteDC(hMemDC);
}

void DrawAllVerticesBack(HDC hMemDC) {
	HDC diffHDC = CreateCompatibleDC(hMemDC);
	for (int i = 0; i < v_ind; i++) {
		int x = vertices[i].vRect.left;
		int y = vertices[i].vRect.top;

		if (firstChosenP == &vertices[i]) {
			if (&vertices[i] == sourceP)
				SelectObject(diffHDC, HSelectedSRCBitmap);
			else if (&vertices[i] == sinkP)
				SelectObject(diffHDC, HSelectedSNKBitmap);
			else
				SelectObject(diffHDC, HBMPs_Selected[vertices[i].medNumber]);
		}
		else {

			if (&vertices[i] == sourceP)
				SelectObject(diffHDC, HSRCBitmap);
			else if (&vertices[i] == sinkP)
				SelectObject(diffHDC, HSNKBitmap);
			else
				SelectObject(diffHDC, HBMPs[vertices[i].medNumber]);
		}

		TransparentBlt(hMemDC, x - 20, y - 20, 41, 41,
			diffHDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);
	}
	DeleteDC(diffHDC);
}

double getVectorLength(double x, double y) {
	return sqrt(x * x + y * y);
}

void DrawAllEdgesBack(HDC hMemDC) {

	for (int i = 0; i < e_ind; i++) {
		int upperLeftX = edges[i].u->vRect.left;
		int upperLeftY = edges[i].u->vRect.top;

		int x1 = upperLeftX + 20;
		int y1 = upperLeftY + 20;

		int lowerRightX = edges[i].v->vRect.right;
		int lowerRightY = edges[i].v->vRect.bottom;

		int x2 = lowerRightX - 20;
		int y2 = lowerRightY - 20;

		double origLength = getVectorLength((double) x2 - x1, (double)y2 - y1);
		double newLength = origLength - 25;

		double k = newLength / origLength;

		int newX2 = (int) floor(x1 + ((double)x2 - x1) * k);
		int newY2 = (int) floor(y1 + ((double)y2 - y1) * k);

		Graphics graphics(hMemDC);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		Pen      pen(Color(255, 0, 0, 255));
		pen.SetEndCap(LineCapArrowAnchor);
		graphics.DrawLine(&pen, x1 - 20, y1 - 20, newX2 - 20, newY2 - 20);

		RECT rect;
		SetRect(&rect, (x1 + x2) / 2 - 11 - 20, (y1 + y2) / 2 - 11 - 20, (x1 + x2) / 2 + 11 - 20, (y1 + y2) / 2 + 11 - 20);
		char str[256];
		_itoa(edges[i].cap, str, 10);
		DrawTextA(hMemDC, str, -1, &rect, (UINT)NULL);
	}
}


void DrawAllEdgesBackToRes(HDC hMemDC) {

	for (int i = 0; i < e_ind; i++) {
		int upperLeftX = edges[i].u->vRect.left;
		int upperLeftY = edges[i].u->vRect.top;

		int x1 = upperLeftX + 20;
		int y1 = upperLeftY + 20;

		int lowerRightX = edges[i].v->vRect.right;
		int lowerRightY = edges[i].v->vRect.bottom;

		int x2 = lowerRightX - 20;
		int y2 = lowerRightY - 20;

		double origLength = getVectorLength((double)x2 - x1, (double)y2 - y1);
		double newLength = origLength - 25;

		double k = newLength / origLength;

		int newX2 = (int)floor(x1 + ((double)x2 - x1) * k);
		int newY2 = (int)floor(y1 + ((double)y2 - y1) * k);

		Graphics graphics(hMemDC);
		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		Pen      pen(Color(255, 0, 0, 255));
		pen.SetEndCap(LineCapArrowAnchor);
		graphics.DrawLine(&pen, x1 - 20, y1 - 20, newX2 - 20, newY2 - 20);

		RECT rect;
		SetRect(&rect, (x1 + x2) / 2 - 11 - 20, (y1 + y2) / 2 - 11 - 20, (x1 + x2) / 2 + 31 - 20, (y1 + y2) / 2 + 11 - 20);

		char str[256];
		int u = edges[i].u->medNumber;
		int v = edges[i].v->medNumber;

		if (u == -1) u = 0;
		else u++;
		if (v == -2) v = getGreatestVertexNumber() + 2;
		else v++;

		_itoa(globalPointerToNetwork[u][v].flow, str, 10);


		char str2[256];
		_itoa(globalPointerToNetwork[u][v].capacity, str2, 10);

		strcat(str, "/");
		strcat(str, str2);

		DrawTextA(hMemDC, str, -1, &rect, (UINT)NULL);
	}
}


void DrawEverythingBack(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateCompatibleBitmap(hdc, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
	SelectObject(hMemDC, memBM);

	RedrawDrawAreaBackground(hMemDC);
	DrawAllEdgesBack(hMemDC);
	DrawAllVerticesBack(hMemDC);


	BitBlt(hdc, DRAW_AREA_UPPER_LEFT_X, DRAW_AREA_UPPER_LEFT_Y, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	DeleteDC(hMemDC);
	DeleteObject(memBM);
}

void DrawEverythingBackToRes(HDC hdc) {
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP memBM = CreateCompatibleBitmap(hdc, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
	SelectObject(hMemDC, memBM);

	RedrawDrawAreaBackground(hMemDC);
	DrawAllEdgesBackToRes(hMemDC);
	DrawAllVerticesBack(hMemDC);


	BitBlt(hdc, DRAW_AREA_UPPER_LEFT_X, DRAW_AREA_UPPER_LEFT_Y, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT, hMemDC, 0, 0, SRCCOPY);
	DeleteDC(hMemDC);
	DeleteObject(memBM);
}

void DeselectButton(HWND hWnd, short int* btn) {
	HCURSOR hCurs = LoadCursor(NULL, IDC_ARROW);
	SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
	*btn = 0;
}

void DeselectButtonsIfAny(HWND hWnd) {
	if (SRC_IS_SELECTED) DeselectButton(hWnd, &SRC_IS_SELECTED);
	if (SNK_IS_SELECTED) DeselectButton(hWnd, &SNK_IS_SELECTED);
	if (MED_IS_SELECTED) DeselectButton(hWnd, &MED_IS_SELECTED);
	InvalidateRect(hWnd, NULL, TRUE);
}

void RemoveEdgesFromArray(VERTEX* p) {
	while (1) {

		int old_ind = e_ind;
		for (int i = 0; i < e_ind; i++)
			if (edges[i].u == p || edges[i].v == p) {
				for (int t = i + 1; t < e_ind; t++)
					edges[t - 1] = edges[t];

				e_ind--;
				break;
			}

		if (old_ind == e_ind) break;
	}
}

void AdjustVertexPointersInEdges(VERTEX* p) {
	for (int i = 0; i < e_ind; i++) {
		if (edges[i].u > p) edges[i].u--;
		if (edges[i].v > p) edges[i].v--;
	}
}

void DeleteVertexFromArray(HDC hdc, VERTEX* p) {

	if (p == sourceP) {
		SRC_CREATED = 0;
		SRC_IS_SELECTED = 0;
		if (sinkP != NULL) {
			if (sinkP > sourceP) sinkP--;
		}
		sourceP = NULL;
	}
	else if (p == sinkP) {
		SNK_CREATED = 0;
		SNK_IS_SELECTED = 0;
		if (sourceP != NULL) {
			if (sourceP > sinkP) sourceP--;
		}
		sinkP = NULL;
	}
	else {
		MED_CREATED = 0;
		MED_IS_SELECTED = 0;
		alreadyCreated[p->medNumber] = 0;
		if (sourceP != NULL) {
			if (sourceP > p) sourceP--;
		}
		if (sinkP != NULL) {
			if (sinkP > p) sinkP--;
		}
	}

	RemoveEdgesFromArray(p);

	for (int i = 0; i < v_ind; i++) {
		if (&vertices[i] == p) {
			int ind = i;
			for (int j = ind + 1; j < v_ind; j++) {
				vertices[j - 1] = vertices[j];
			}
			v_ind--;
			break;
		}
	}

	AdjustVertexPointersInEdges(p);

	//DrawAllVerticesBack(hdc);
	DrawEverythingBack(hdc);
}

void DeleteEdgeFromArray(HDC hdc, EDGE* p) {

	for (int i = 0; i < e_ind; i++) {
		if (&edges[i] == p) {
			int ind = i;

			for (int j = ind + 1; j < e_ind; j++) {
				edges[j - 1] = edges[j];
			}

			e_ind--;
			break;
		}
	}

	DrawEverythingBack(hdc);
}

void ProcessClickInDrawArea(HWND hWnd, HDC hdc, int x, int y) {
	if (SRC_IS_SELECTED || SNK_IS_SELECTED || MED_IS_SELECTED) {

		int res = CreateVertex(x, y);
		if (res == -VERTEX_MAX) {
			MessageBoxA(hWnd, "2 many", "Wrong!", (UINT)NULL);
		}
		else if (res == -1) {
			MessageBoxA(hWnd, "SRC has been created already", "Wrong!", (UINT)NULL);
		}
		else if (res == -2) {
			MessageBoxA(hWnd, "SNK has been created already", "Wrong!", (UINT)NULL);
		}
		else if (res == -3) {
			MessageBoxA(hWnd, "2 many med", "Wrong!", (UINT)NULL);
		}
		else if (res == 0) {
			DrawVertex(hdc, &vertices[v_ind - 1]);
		}

		DeselectButtonsIfAny(hWnd);
	}
	else {

		RECT rect;
		HRGN hRgn;

		for (int i = 0; i < v_ind; i++) {
			SetRect(&rect, vertices[i].vRect.left, vertices[i].vRect.top, vertices[i].vRect.right, vertices[i].vRect.bottom);
			hRgn = CreateRectRgnIndirect(&rect);

			if (PtInRegion(hRgn, x, y)) {

				DRAG_ACTIVE = 1;
				if (firstChosenP == NULL) {
					firstChosenP = &vertices[i];
					DrawSelectedVertex(hdc, firstChosenP);
				}
				else

					if (secondChosenP == NULL) {
						secondChosenP = &vertices[i];
						if (secondChosenP == firstChosenP) {
							DrawVertex(hdc, firstChosenP);
							firstChosenP = NULL;
							secondChosenP = NULL;
						}
						else {
							DrawVertex(hdc, firstChosenP);

							if (secondChosenP == sourceP && !EdgeExists(secondChosenP, firstChosenP)) {
								MessageBoxW(hWnd, L"В исток не могут идти никакие дуги!", L"Неверный выбор!", (UINT)NULL);
							}
							else if (firstChosenP == sinkP && !EdgeExists(secondChosenP, firstChosenP)) {
								MessageBoxW(hWnd, L"Из стока не могут идти никакие дуги!", L"Неверный выбор!", (UINT)NULL);
							}
							else {
								EDGE* pointer = EdgeExists(secondChosenP, firstChosenP);
								if (pointer != NULL) {
									DeleteEdgeFromArray(hdc, pointer);
								}
								else {

									DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_REQUESTCAPACITY), hWnd, RequestCapacityDlgProc);

									if (CAP_RETURN_RES >= 1 && CAP_RETURN_RES <= 99) {

										pointer = EdgeExists(firstChosenP, secondChosenP);
										if (pointer != NULL) {
											pointer->cap = CAP_RETURN_RES;
											//DrawEverythingBack(hdc);
										}
										else {

											int res = CreateEdge(firstChosenP, secondChosenP, CAP_RETURN_RES);
											if (res == -EDGE_MAX) {
												MessageBoxW(hWnd, L"Невозможно создать больше дуг!", L"Ошибка!", (UINT)NULL);
											}
											else {
												//DrawEverythingBack(hdc);
											}
										}
									}
									
								}

							}

							firstChosenP = NULL;
							secondChosenP = NULL;
							DrawEverythingBack(hdc);
						}
					}
				DeleteObject(hRgn);
				break;

			}
			else {
				//SetWindowText(hWnd, L"Didn't hit!");
			}

			DeleteObject(hRgn);
		}

	}
}


void DrawItemSupp(HDC hMemDC, HWND hWnd, LPDRAWITEMSTRUCT pdis,
	short int* globSelectedPointer, short int* globCreatedPointer,
	HBITMAP positive, HBITMAP negative) {
		{
			switch (pdis->itemAction) {
			case ODA_SELECT:
				if ((pdis->itemState & ODS_SELECTED) && !(*globCreatedPointer)) {
					(*globSelectedPointer) = (*globSelectedPointer) ? 0 : 1;
				}
			}

			if ((*globSelectedPointer)) {
				SelectObject(hMemDC, negative);
			}
			else {
				if (!(*globCreatedPointer)) {
					SelectObject(hMemDC, positive);
					DeselectButton(hWnd, globCreatedPointer);
				}
				else
					SelectObject(hMemDC, negative);
			}
		}
}

int getNotCreatedMedIndex(void) {
	for (int i = 0; i < MAX_MED; i++)
		if (!alreadyCreated[i])
			return i;

	return MAX_MED;
}

void DrawItemDependingOnParameters(HWND hWnd, HDC hMemDC, LPDRAWITEMSTRUCT pdis, WPARAM wParam) {
	switch (wParam) {
	case SRCBUTTONID:
		DrawItemSupp(hMemDC, hWnd, pdis, &SRC_IS_SELECTED, &SRC_CREATED, HSRCBitmap, HSRCMonoBitmap);
		break;

	case SNKBUTTONID:
		DrawItemSupp(hMemDC, hWnd, pdis, &SNK_IS_SELECTED, &SNK_CREATED, HSNKBitmap, HSNKMonoBitmap);
		break;

	case MEDBUTTONID: {
		int res = getNotCreatedMedIndex();
		if (res == MAX_MED) {
			DrawItemSupp(hMemDC, hWnd, pdis, &MED_IS_SELECTED, &MED_CREATED, HBMPs_Mono[res - 1], HBMPs_Mono[res - 1]);
			MED_CREATED = 1;
		}
		else {
			DrawItemSupp(hMemDC, hWnd, pdis, &MED_IS_SELECTED, &MED_CREATED, HBMPs[res], HBMPs_Mono[res]);
		}
		break;
	}

	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



BOOL CALLBACK RequestCapacityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:

		switch (LOWORD(wParam)) {

		case IDOK_CAP: {

			BOOL check;
			int res = GetDlgItemInt(hDlg, IDC_EDIT_CAP, &check, FALSE);

			if (check) {

				if (res < 1 || res > 99) {
					MessageBoxW(hDlg, L"Введите натуральное число от 1 до 99!", L"Некорректный ввод!", (UINT)NULL);
					CAP_RETURN_RES = -3;
				}
				else {
					CAP_RETURN_RES = res;
					EndDialog(hDlg, 0);
				}
			}
			else {
				CAP_RETURN_RES = -1;
				MessageBoxW(hDlg, L"Введите натуральное число от 1 до 99!", L"Некорректный ввод!", (UINT)NULL);
			}

			return TRUE;
		}
		case IDCANCEL_CAP: {
			EndDialog(hDlg, 0);
			CAP_RETURN_RES = -2;
			return TRUE;
		}

		}
		break;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND hTextMaxFlow;

BOOL CALLBACK ShowResDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		hTextMaxFlow = GetDlgItem(hDlg, IDC_STATIC_MAX_FLOW);
		char str[256];
		_itoa(MAX_FLOW_RES, str, 10);
		SetWindowTextA(hTextMaxFlow, str);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		case IDOK:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);
		DrawEverythingBackToRes(hdc);
		EndPaint(hDlg, &ps);
		return TRUE;
		break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HICON hIcon;

#define NEW_FILE_ID 699
#define OPEN_FILE_ID 700
#define SAVE_FILE_ID 701
#define SAVE_FILE_AS_ID 702
#define EXIT_ID 703
#define ABOUT_ID 704

LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	case WM_CREATE: {

		SetClassLong(hWnd, GCL_HICON, (LONG)hIcon);

		HMENU hMenuBar = CreateMenu();
		HMENU hFile = CreateMenu();
		AppendMenu(hMenuBar, MF_POPUP, (UINT)hFile, L"Файл");
		AppendMenu(hFile, MF_STRING, (UINT_PTR)NEW_FILE_ID, L"Новый");
		AppendMenu(hFile, MF_STRING,   (UINT_PTR)OPEN_FILE_ID, L"Открыть");
		AppendMenu(hFile, MF_STRING,   (UINT_PTR)SAVE_FILE_ID, L"Сохранить");
		AppendMenu(hFile, MF_STRING,   (UINT_PTR)SAVE_FILE_AS_ID, L"Сохранить как...");
		AppendMenu(hFile, MF_STRING,   (UINT_PTR)EXIT_ID, L"Выход");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)ABOUT_ID, L"Справка");
		SetMenu(hWnd, hMenuBar);


		HSRCBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SRC_BITMAP));
		HSRCMonoBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SRC_MONO_BITMAP));
		HSelectedSRCBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SELECTED_SRC_BITMAP));

		HSNKBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SNK_BITMAP));
		HSNKMonoBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SNK_MONO_BITMAP));
		HSelectedSNKBitmap = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(SELECTED_SNK_BITMAP));

		HBMPs[0] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP1));
		HBMPs[1] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP2));
		HBMPs[2] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP3));
		HBMPs[3] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP4));
		HBMPs[4] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP5));
		HBMPs[5] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP6));
		HBMPs[6] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP7));
		HBMPs[7] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP8));
		HBMPs[8] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP9));
		HBMPs[9] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP10));
		HBMPs[10] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP11));
		HBMPs[11] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP12));
		HBMPs[12] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP13));
		HBMPs[13] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP14));
		HBMPs[14] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP15));
		HBMPs[15] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP16));
		HBMPs[16] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP17));
		HBMPs[17] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP18));


		HBMPs_Selected[0] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP1_S));
		HBMPs_Selected[1] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP2_S));
		HBMPs_Selected[2] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP3_S));
		HBMPs_Selected[3] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP4_S));
		HBMPs_Selected[4] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP5_S));
		HBMPs_Selected[5] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP6_S));
		HBMPs_Selected[6] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP7_S));
		HBMPs_Selected[7] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP8_S));
		HBMPs_Selected[8] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP9_S));
		HBMPs_Selected[9] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP10_S));
		HBMPs_Selected[10] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP11_S));
		HBMPs_Selected[11] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP12_S));
		HBMPs_Selected[12] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP13_S));
		HBMPs_Selected[13] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP14_S));
		HBMPs_Selected[14] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP15_S));
		HBMPs_Selected[15] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP16_S));
		HBMPs_Selected[16] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP17_S));
		HBMPs_Selected[17] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP18_S));


		HBMPs_Mono[0] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP1_M));
		HBMPs_Mono[1] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP2_M));
		HBMPs_Mono[2] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP3_M));
		HBMPs_Mono[3] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP4_M));
		HBMPs_Mono[4] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP5_M));
		HBMPs_Mono[5] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP6_M));
		HBMPs_Mono[6] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP7_M));
		HBMPs_Mono[7] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP8_M));
		HBMPs_Mono[8] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP9_M));
		HBMPs_Mono[9] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP10_M));
		HBMPs_Mono[10] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP11_M));
		HBMPs_Mono[11] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP12_M));
		HBMPs_Mono[12] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP13_M));
		HBMPs_Mono[13] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP14_M));
		HBMPs_Mono[14] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP15_M));
		HBMPs_Mono[15] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP16_M));
		HBMPs_Mono[16] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP17_M));
		HBMPs_Mono[17] = (HBITMAP)LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(BMP18_M));


		for (int i = 0; i < MAX_MED; i++)
			alreadyCreated[i] = 0;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_COMMAND: {
		WORD word = LOWORD(wParam);

		switch (word) {
		case NEW_FILE_ID: {
			ResetEverything();
			InvalidateRect(hWnd, NULL, 0);
			break;
		}


		case OPEN_FILE_ID: {
			//wcscpy(currFileName, L"Souce.nk");
			//SetWindowText(groupButton, currFileName);

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = currFileName;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(currFileName);
			ofn.lpstrFilter = L"Сети\0*.nw\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetOpenFileName(&ofn) == TRUE) {

				int res = readAllDataFromFile();
				
				if (res == -1) {
					MessageBox(hWnd, L"Произошла ошибка чтения!", L"Ошибка!", MB_ICONERROR);
					InvalidateRect(hWnd, NULL, 0);
				}
				else {
					InvalidateRect(hWnd, NULL, 0);
				}

				SetWindowText(groupButton, currFileName);
			}
			else {
				MessageBox(hWnd, L"Файл не был открыт!", L"Внимание!", MB_ICONEXCLAMATION);
			}

			break;
		}

		case SAVE_FILE_ID: {
			
			int res = SaveAllDataToFile();

			if (res == -1) { 
				MessageBox(hWnd, L"Файл не был сохранен!", L"Внимание!", MB_ICONERROR); 
			}
			else {
				MessageBox(hWnd, L"Запись прошла успешно!", L"Успех!", NULL);
			}

			break;
		}

		case SAVE_FILE_AS_ID: {

			ZeroMemory(&sfn, sizeof(sfn));
			sfn.lStructSize = sizeof(sfn);
			sfn.hwndOwner = hWnd;
			sfn.lpstrFile = saveFileName;
			sfn.lpstrFile[0] = '\0';
			sfn.nMaxFile = sizeof(saveFileName);
			sfn.lpstrFilter = L"Сети\0*.nw\0";
			sfn.nFilterIndex = 0;
			sfn.lpstrFileTitle = NULL;
			sfn.nMaxFileTitle = 0;
			sfn.lpstrInitialDir = NULL;
			sfn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if (GetSaveFileName(&sfn) == TRUE) {
				if (wcsstr(saveFileName, L".nw") == NULL) wcscat(saveFileName, L".nw");

				int res = writeAllDataToFile();
				if (res == -1) {
					MessageBox(hWnd, L"Ошибка доступа к файлу!", L"Внимание!", MB_ICONERROR);
				}
				else {
					MessageBox(hWnd, L"Запись прошла успешно!", L"Успех!", NULL);
				}
			}
			else {
				MessageBox(hWnd, L"Файл не был сохранен!", L"Внимание!", MB_ICONEXCLAMATION);
			}

			break;
		}

		case EXIT_ID: {
			PostQuitMessage(0);
			break;
		}

		case ABOUT_ID: {
			MessageBoxW(hWnd, L" Курсовая работа\n студента группы 951005\n Плотникова Владислава Вадимовича\n Проверила Болтак Светлана Владимировна", L"Справка", MB_ICONINFORMATION);
			break;
		}

		case SRCBUTTONID:

			if (SNK_IS_SELECTED || MED_IS_SELECTED) {
				SRC_IS_SELECTED = 0;
				return 0;
			}

			if (!SRC_CREATED && SRC_IS_SELECTED) {
				HCURSOR hCurs = LoadCursor(NULL, IDC_CROSS);
				SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
			}
			else {
				SRC_IS_SELECTED = 0;
			}
			break;

		case SNKBUTTONID:

			if (SRC_IS_SELECTED || MED_IS_SELECTED) {
				SNK_IS_SELECTED = 0;
				return 0;
			}

			if (!SNK_CREATED && SNK_IS_SELECTED) {
				HCURSOR hCurs = LoadCursor(NULL, IDC_CROSS);
				SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
			}
			else {
				SNK_IS_SELECTED = 0;
			}

			break;

		case MEDBUTTONID: {
			if (SRC_IS_SELECTED || SNK_IS_SELECTED) {
				MED_IS_SELECTED = 0;
				return 0;
			}

			if (!MED_CREATED && MED_IS_SELECTED) {
				HCURSOR hCurs = LoadCursor(NULL, IDC_CROSS);
				SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG)(hCurs));
			}
			else {
				MED_IS_SELECTED = 0;
			}

			break;
		}

		case MAXFLOWBUTTONID: {
			//printCurrentStateDebugDifferent();

			if (sourceP == NULL) {
				MessageBoxW(hWnd, L"В введенном графе отсутствует исток!", L"Некорректный ввод!", (UINT)NULL);
			}
			else if (sinkP == NULL) {
				MessageBoxW(hWnd, L"В введенном графе отсутствует сток!", L"Некорректный ввод!", (UINT)NULL);
			}
			else {

				int N = getGreatestVertexNumber() + 3;

				int** w = (int **)malloc(sizeof(int*) * e_ind);

				for (int i = 0; i < e_ind; i++) {
					w[i] = (int*)malloc(sizeof(int) * 3);

					int u = edges[i].u->medNumber;
					int v = edges[i].v->medNumber;
					int c = edges[i].cap;

					u += 2;
					v = (v == -2) ? N : v + 2;

					w[i][0] = u;
					w[i][1] = v;
					w[i][2] = c;
				}


				MAX_FLOW_RES = EdmondKarpAlgorithmFunc(N, e_ind, w, &globalPointerToNetwork);

				switch (MAX_FLOW_RES) {
				case -1:
					MessageBoxW(hWnd, L"В введеном графе из истока не исходит ни одной дуги!", L"Некорректный ввод!", (UINT)NULL);
					break;
				case -2:
					MessageBoxW(hWnd, L"В введеном графе в сток не входит ни одна дуга!", L"Некорректный ввод!", (UINT)NULL);
					break;
				case -3:
					MessageBoxW(hWnd, L"В введеном графе одна из промежуточных вершин \nне лежит на пути из истока в сток!", L"Некорректный ввод!", (UINT)NULL);
					break;
				default: {
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RESDIALOGWIN), hWnd, ShowResDlgProc);
				}
				}

				for (int i = 0; i < e_ind; i++)
					free(w[i]);

				free(w);
			}
		}

		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_LBUTTONDOWN: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		HDC hdc = GetDC(hWnd);
		RECT rect;


		SetRect(&rect, DRAW_AREA_UPPER_LEFT_X + 30, DRAW_AREA_UPPER_LEFT_Y + 30, DRAW_AREA_LOWER_RIGHT_X - 30, DRAW_AREA_LOWER_RIGHT_Y - 30);
		HRGN hRgn = CreateRectRgnIndirect(&rect);
		SelectClipRgn(hdc, hRgn);


		if (PtInRegion(hRgn, x, y)) {
			SetFocus(hWnd);
			SelectClipRgn(hdc, NULL);
			ProcessClickInDrawArea(hWnd, hdc, x, y);
		}
		else {
			DeselectButtonsIfAny(hWnd);
		}


		ReleaseDC(hWnd, hdc);
		DeleteObject(hRgn);

		//printCurrentStateDebug();

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_LBUTTONUP: {
		DRAG_ACTIVE = 0;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//DrawAllVerticesBack(hdc);
		DrawEverythingBack(hdc);

		RECT rect;
		SetRect(&rect, 600, 15, 750, 100);
		DrawText(hdc, L"Поиск \rмаксимального потока\r в сети\rалгоритмом\rЭдмондса-Карпа", -1, &rect, DT_CENTER | DT_WORDBREAK);

		SetRect(&rect, 600, 125, 750, 150);
		DrawText(hdc, L"Исток сети", -1, &rect, DT_CENTER | DT_SINGLELINE);

		SetRect(&rect, 600, 230, 750, 270);
		DrawText(hdc, L"Промежуточная вершина", -1, &rect, DT_CENTER | DT_WORDBREAK);

		SetRect(&rect, 600, 355, 750, 375);
		DrawText(hdc, L"Сток сети", -1, &rect, DT_CENTER | DT_SINGLELINE);

		EndPaint(hWnd, &ps);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_MOUSEMOVE: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		HRGN hRgn;
		RECT drawAreaRect;
		SetRect(&drawAreaRect, DRAW_AREA_UPPER_LEFT_X + 30, DRAW_AREA_UPPER_LEFT_Y + 30, DRAW_AREA_LOWER_RIGHT_X - 30, DRAW_AREA_LOWER_RIGHT_Y - 30);
		hRgn = CreateRectRgnIndirect(&drawAreaRect);

		if (PtInRegion(hRgn, x, y)) {
			if (DRAG_ACTIVE && wParam == MK_LBUTTON && firstChosenP != NULL) {
				SetRect(&(firstChosenP->vRect), x - 21, y - 21, x + 21, y + 21);

				HDC hdc = GetDC(hWnd);
				DrawEverythingBack(hdc);
				//DrawAllVerticesBack(hdc);
				ReleaseDC(hWnd, hdc);
			}
		}

		DeleteObject(hRgn);

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	case WM_DRAWITEM: {
		LPDRAWITEMSTRUCT pdis;
		pdis = (LPDRAWITEMSTRUCT)lParam;

		HDC hMemDC = CreateCompatibleDC(pdis->hDC);

		switch (wParam) {
		case SRCBUTTONID:
			if (SNK_IS_SELECTED || MED_IS_SELECTED) {
				SRC_IS_SELECTED = 0;
				return TRUE;
			}
			break;

		case SNKBUTTONID:
			if (SRC_IS_SELECTED || MED_IS_SELECTED) {
				SNK_IS_SELECTED = 0;
				return TRUE;
			}
			break;

		case MEDBUTTONID:
			if (SRC_IS_SELECTED || SNK_IS_SELECTED) {
				MED_IS_SELECTED = 0;
				return TRUE;
			}
		}

		DrawItemDependingOnParameters(hWnd, hMemDC, pdis, wParam);

		FillRect(pdis->hDC, &pdis->rcItem, (HBRUSH)(COLOR_WINDOW + 1));
		TransparentBlt(pdis->hDC, 0, 0, 41, 41,
			hMemDC, 0, 0, 41, 41, COLOR_TO_BE_TRANSPARENT);



		DeleteDC(hMemDC);

		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BACKSPACE_VIRTUAL_CODE 0x08
	case WM_KEYDOWN: {

		if (wParam == BACKSPACE_VIRTUAL_CODE) {
			if (firstChosenP != NULL) {
				HDC hdc = GetDC(hWnd);
				DeleteVertexFromArray(hdc, firstChosenP);
				firstChosenP = NULL;
				DeselectButtonsIfAny(hWnd);
				ReleaseDC(hWnd, hdc);
			}
		}

		//printCurrentStateDebug();

		return 0;
	}


	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}


	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wcex;	//Оконный класс для окна нашего приложения

	HWND hWnd;			//Главное окно нашего приложения
	MSG msg;			//Структура для получения и отправки сообщений

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//////////////////////////////////////////////
	//////////////////////////////////////////////			//Заполнение полей оконного класса
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = MyWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"MyWindowClass";
	hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDB_ICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	wcex.hIconSm = NULL;
	//////////////////////////////////////////////
	//////////////////////////////////////////////

	RegisterClassEx(&wcex);		//Регистрация оконного класса


	//Создание главного окна приложения
	hWnd = CreateWindowEx(0, L"MyWindowClass", L"Программное средство для нахождения максимального потока в сети",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 800, 600, 0, 0, hInstance, NULL);

	//Создание рамки для области рисования
		groupButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", currFileName,
		WS_VISIBLE | WS_CHILD | WS_BORDER | BS_GROUPBOX,
		DRAW_AREA_UPPER_LEFT_X - 15,
		DRAW_AREA_UPPER_LEFT_Y - 15,
		DRAW_AREA_WIDTH + 25,
		DRAW_AREA_HEIGHT + 25, hWnd, NULL, NULL, NULL);

	//Создание рамки для меню
	HWND groupButtonAn = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | BS_GROUPBOX,
		MENU_AREA_UPPER_LEFT_X,
		MENU_AREA_UPPER_LEFT_Y,
		MENU_AREA_WIDTH,
		MENU_AREA_HEIGHT, hWnd, NULL, NULL, NULL);


	//Создание кнопки с источником сети
	HWND srcButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Btn1",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		655,
		150,
		42,
		42, hWnd, (HMENU)SRCBUTTONID, NULL, NULL);


	//Создание кнопки с промежуточными вершинами
	HWND midButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Btn2",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		655,
		270,
		42,
		42, hWnd, (HMENU)MEDBUTTONID, NULL, NULL);


	//Создание кнопки со стоком сети
	HWND snkButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Btn3",
		WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
		655,
		375,
		42,
		42, hWnd, (HMENU)SNKBUTTONID, NULL, NULL);


	//Создание кнопки для расчета максимального потока
	HWND maxFlowButton = CreateWindowEx(WS_EX_WINDOWEDGE, L"Button", L"Найти максимальный поток сети",
		WS_VISIBLE | WS_CHILD | BS_MULTILINE,
		605,
		440,
		140,
		70,
		hWnd, (HMENU)MAXFLOWBUTTONID, NULL, NULL);

	//Показ главного окна приложения
	ShowWindow(hWnd, nCmdShow);

	//Цикл обработки сообщений
	while (GetMessage(&msg, 0, 0, 0))
	{
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);

	//Возвращение результата работы
	return msg.wParam;
}