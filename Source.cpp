#include <windows.h>
#include <gl/gl.h>
#include<string>
#include<math.h>

#include"OpenGl/main.h"
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"


#pragma comment(lib, "opengl32.lib")

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);





BOOL IsCoordInMap(float x, float y)
{
	return (x >= 0) && (x < mapW) && (y >= 0) && (y < mapH);
}

float Map_GetHeight(float x, float y)
{
	if (!IsCoordInMap(x, y))return 0;
	int cX = (int)x;
	int cY = (int)y;
	float h1 = ((1 - (x - cX)) * map[cX][cY].z + (x - cX) * map[cX + 1][cY].z);
	float h2 = ((1 - (x - cX)) * map[cX][cY + 1].z + (x - cX) * map[cX + 1][cY + 1].z);
	return (1 - (y - cY)) * h1 + (y - cY) * h2;
}

void LoadTexture(const char *file_name, int *target)
{
	int width, height,cnt;
	unsigned char *data = stbi_load(file_name, &width, &height, &cnt, 0);

	glGenTextures(1, (GLuint*)target);
	glBindTexture(GL_TEXTURE_2D, *target);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, cnt == 4 ? GL_RGBA : GL_RGB,
		   GL_UNSIGNED_BYTE, data);

	   glBindTexture(GL_TEXTURE_2D,0);
	   stbi_image_free(data);

}

#define sqr(a) (a)*(a)
void CalcNormals(TCell a, TCell b, TCell c, TCell* n)
{
	float wrk1;
	TCell v1, v2;
	v1.x = a.x - b.x;
	v1.y = a.y - b.y;
	v1.z = a.z - b.z;
	v2.x = b.x - c.x;
	v2.y = b.y - c.y;
	v2.z = b.z - c.z;

	n->x = (v1.y * v2.z - v1.z * v2.y);
	n->y = (v1.z * v2.x - v1.x * v2.z);
	n->z = (v1.x * v2.y - v1.y * v2.x);
	wrk1 = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));
	n->x /= wrk1;
	n->y /= wrk1;
	n->z /= wrk1;
}

void Map_CreateHill(int posX, int posY, int rad, int height)
{
	for (int i = posX - rad; i <= posX + rad; i++)
	{
		for (int j = posY - rad; j <= posY + rad; j++)
		{
			if (IsCoordInMap(i, j)) {
				float len = sqrt(pow(posX - i, 2) + pow(posY - j, 2));

				if (len < rad)
				{
					len = len / rad * M_PI_2;
					map[i][j].z += cos(len) * height;
				}
			}
		}
	}
}



void Tree_Create(TobjGroup* obj, int type, int x, int y)
{
	obj->type = type;
	float z = Map_GetHeight(x + 0.5, y + 0.5) - 0.5;
	int logs = 6;
	int leafs = 5 * 5 * 2 - 2 + 3 * 3 * 2;

	obj->itemsCnt = logs+leafs;
	obj->items = (TObject*)malloc(sizeof(TObject) * obj->itemsCnt);

	for (int i = 0; i < logs; i++)
	{
		obj->items[i].type = 1;
		obj->items[i].x = x;
		obj->items[i].y = y;
		obj->items[i].z = z + i;
	}
	int pos = logs;
	for (int k = 0; k < 2; k++)
	{
		for (int i =  x- 2; i <= x+2; i++)
		{
			for (int j = y-2; j <= y+2; j++)
			{
				if ((i != x) || (y != j))
				{
					obj->items[pos].type = 2;
					obj->items[pos].x = i;
					obj->items[pos].y = j;
					obj->items[pos].z = z + logs - 2 + k;
					pos++;
				}
			}
		}
	}

	for (int k = 0; k < 2; k++)
	{
		for (int i = x - 1; i <= x + 1; i++)
		{
			for (int j = y - 1; j <= y + 1; j++)
			{
					obj->items[pos].type = 2;
					obj->items[pos].x = i;
					obj->items[pos].y = j;
					obj->items[pos].z = z + logs+ k;
					pos++;
				
			}
		}
	}
	
}

void Tree_Show(TobjGroup obj)
{
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, cube);
	glColor3f(0.7, 0.7, 0.7);
	glNormal3f(0, 0, 1);
	glBindTexture(GL_TEXTURE_2D, obj.type);
	for (int i = 0; i < obj.itemsCnt; i++)
	{
		
		if (obj.items[i].type == 1)glTexCoordPointer(2, GL_FLOAT, 0, cubeUVLog);
		else glTexCoordPointer(2, GL_FLOAT, 0, cubeUVLeaf);
		glPushMatrix();
		glTranslatef(obj.items[i].x, obj.items[i].y, obj.items[i].z);
		glDrawElements(GL_TRIANGLES, cubeInCnt, GL_UNSIGNED_INT, cubeInd);
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}



void Map_Init()
{

	LoadTexture("textures/pole.png", &tex_pole);
	LoadTexture("textures/trava.png", &tex_trava);
	LoadTexture("textures/flower.png", &tex_flower);
	LoadTexture("textures/flower2.png", &tex_flower2);
	LoadTexture("textures/grib.png", &tex_grib);
	LoadTexture("textures/tree.png", &tex_tree);
	LoadTexture("textures/tree2.png", &tex_tree2);
	LoadTexture("textures/wood.png", &tex_wood);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.99);


	for (int i = 0; i < mapW; i++)
		for (int j = 0; j < mapH; j++)
		{
			map[i][j].x = i;
			map[i][j].y = j;
			map[i][j].z = (rand() % 10) * 0.05;

			mapUV[i][j].u = i;
			mapUV[i][j].v = j;
		}

	for (int i = 0; i < mapW - 1; i++)
	{
		int pos = i * mapH;
		for (int j = 0; j < mapH - 1; j++)
		{
			mapind[i][j][0] = pos;
			mapind[i][j][1] = pos + 1;
			mapind[i][j][2] = pos + 1 + mapH;

			mapind[i][j][3] = pos + 1 + mapH;
			mapind[i][j][4] = pos + mapH;
			mapind[i][j][5] = pos;

			pos++;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		Map_CreateHill(rand() % mapW, rand() % mapH, rand() % 50, rand() % 10);

	}

	for (int i = 0; i < mapW - 1; i++)
	{
		for (int j = 0; j < mapH - 1; j++)
		{
			CalcNormals(map[i][j], map[i + 1][j], map[i][j + 1],&mapNormal[i][j]);
		}
	}


	int travaN = 2000;
	int gribN = 30;
	int treeN = 40;
	plantCnt = travaN +gribN+treeN;
	plantMas = (TObject*)realloc(plantMas,sizeof(*plantMas)*plantCnt);

	for (int i = 0; i < plantCnt; i++)
	{
		if (i < travaN) {
			plantMas[i].type = rand()%100 != 0?tex_trava:(rand()%2 == 0?tex_flower:tex_flower2);
			plantMas[i].scale = 0.7 + (rand() % 5) * 0.1;
		}
		else if (i < (travaN + gribN))
		{
			plantMas[i].type = tex_grib;
			plantMas[i].scale = 0.2 + (rand() % 10) * 0.01;
		}
		else
		{
			
				plantMas[i].type = rand()%2==0?tex_tree :tex_tree2;
				plantMas[i].scale = 4 + (rand() % 14);
		}

		plantMas[i].x = rand() % mapW;
		plantMas[i].y = rand() % mapW;
		plantMas[i].z = Map_GetHeight(plantMas[i].x, plantMas[i].y);

	}

	treeCNt = 50;
	tree = (TobjGroup*)realloc(tree, sizeof(*tree) * treeCNt);

	for (int i = 0; i < treeCNt; i++)
		Tree_Create(tree + i, tex_wood, rand() % mapW, rand() % mapH);
}


void Player_Move()
{
	camera::Camera_MoveDirection(
		GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0),
		GetKeyState('D')<0?1:(GetKeyState('A')<0?-1:0),
		0.1);
	camera::Camera_AutoMoveByMouse(400, 400, 0.2);
	camera::camera.z = Map_GetHeight(camera::camera.x, camera::camera.y)+1.7;
}

void Map_Show()
{

	glClearColor(0.6f, 0.8f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	Player_Move();
	camera::Camera_Apply();

	GLfloat position[] = { 1,0,2,0 };
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
    	glVertexPointer(3, GL_FLOAT, 0, map);
		glTexCoordPointer(2, GL_FLOAT, 0, mapUV);
			glColor3f(0.7, 0.7, 0.7);
	    glNormalPointer(GL_FLOAT, 0, mapNormal);
		glBindTexture(GL_TEXTURE_2D, tex_pole);
	glDrawElements(GL_TRIANGLES, mapIndCount, GL_UNSIGNED_INT, mapind);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	 glVertexPointer(3, GL_FLOAT, 0, plant);
	glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
	glColor3f(0.7, 0.7, 0.7);
	glNormal3f(0, 0, 1);
	for (int i = 0; i < plantCnt; i++)
	{
		glBindTexture(GL_TEXTURE_2D, plantMas[i].type);
		glPushMatrix();
		glTranslatef(plantMas[i].x, plantMas[i].y, plantMas[i].z);
		glScalef(plantMas[i].scale, plantMas[i].scale,plantMas[i].scale);
		glDrawElements(GL_TRIANGLES, plantIndCnt, GL_UNSIGNED_INT, plantInd);
		glPopMatrix();
	}
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);


	for (int i = 0; i < treeCNt; i++)
	{
		Tree_Show(*tree);
	}

	glPopMatrix();

	
}

void WndResize(int x,int y)
{
	glViewport(0, 0, x, y);
	float k = x / (float)y;
	float sz = 0.1;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-k * sz, k * sz, -sz, sz, sz * 2, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hwnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	BOOL bQuit = FALSE;
	float theta = 0.0f;

	/* register window class */
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = TEXT("GLSample");
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


	if (!RegisterClassEx(&wcex))
		return 0;

	/* create main window */
	hwnd = CreateWindowEx(0,
		TEXT("GLSample"),
		TEXT("OpenGL Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1100,
		700,
		NULL,
		NULL,
		hInstance,
		NULL);

	ShowWindow(hwnd, nCmdShow);

	/* enable OpenGL for the window */
	EnableOpenGL(hwnd, &hDC, &hRC);

	RECT rct;
	GetClientRect(hwnd,&rct);
	WndResize(rct.right, rct.bottom);
	Map_Init();
	glEnable(GL_DEPTH_TEST);

	while (!bQuit)
	{
	
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			
			if (msg.message == WM_QUIT)
			{
				bQuit = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			
			Map_Show();

			SwapBuffers(hDC);

			theta += 1.0f;
			Sleep(1);
		}
	}


	DisableOpenGL(hwnd, hDC, hRC);

	DestroyWindow(hwnd);

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_DESTROY:
		return 0;
	case WM_SIZE:
		WndResize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	}
	break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
	PIXELFORMATDESCRIPTOR pfd;

	int iFormat;

	/* get the device context (DC) */
	*hDC = GetDC(hwnd);

	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof(pfd));

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(*hDC, &pfd);

	SetPixelFormat(*hDC, iFormat, &pfd);

	/* create and enable the render context (RC) */
	*hRC = wglCreateContext(*hDC);

	wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hwnd, hDC);
}
