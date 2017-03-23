#include <Windows.h>
int D3DDemo(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);
int FixPipeline(HINSTANCE hinstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd);

int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	D3DDemo(hinstance, prevInstance, cmdLine, showCmd);
	return 0;
}