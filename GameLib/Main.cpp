#include <Windows.h>
#pragma comment(lib, "gdiplus.lib")
int D3DDemo(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);
int FixPipeline(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	FixPipeline(hinstance, prevInstance, cmdLine, showCmd);
	return 0;
}