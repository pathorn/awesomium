#include "TestFramework.h"
#include "WebCore.h"
#include <windows.h>

#define WIDTH	600
#define HEIGHT	600
#define LENGTH_SEC	5

class Test_RenderSync : public Test
{
	Awesomium::WebView* webView;
public:
	Test_RenderSync() : Test("RenderSync")
	{
		webView = Awesomium::WebCore::Get().createWebView(WIDTH, HEIGHT);
		webView->loadFile("tests/RenderTest.html");
		Sleep(100);
	}

	~Test_RenderSync()
	{
		webView->destroy();
	}

	bool run()
	{
		log("Running");

		unsigned char* buffer = new unsigned char[WIDTH * HEIGHT * 4];

		timer t;
		t.start();
		int loopCount = 0;
		int renderCount = 0;

		while(t.elapsed_time() < LENGTH_SEC)
		{
			if(webView->isDirty())
			{
				renderCount++;
				webView->render(buffer, WIDTH * 4, 4);
			}

			loopCount++;
			Sleep(1);
		}

		delete[] buffer;

		logTestValue("RenderSync_LoopCount", loopCount / (double)LENGTH_SEC);
		logTestValue("RenderSync_RenderCount", renderCount / (double)LENGTH_SEC);

		return true;
	}
};