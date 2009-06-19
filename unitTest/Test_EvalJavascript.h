#include "TestFramework.h"
#include "WebCore.h"
#include <windows.h>

#define LENGTH_SEC	5

class Test_EvalJavascript : public Test, public Awesomium::WebViewListener
{
	Awesomium::WebView* webView;
	bool hasLoaded;
public:
	Test_EvalJavascript() : Test("EvalJavascript"), hasLoaded(false)
	{
		webView = Awesomium::WebCore::Get().createWebView(15, 15, false, true);
		webView->loadFile("tests/EvalJavascript.html");
		webView->setListener(this);
		
		Sleep(100);
	}

	~Test_EvalJavascript()
	{
		webView->setListener(0);
		webView->destroy();
	}

	bool run()
	{
		log("Running");
		log("Waiting for page to load...");
		while(!hasLoaded)
		{
			Awesomium::WebCore::Get().update();
			Sleep(50);
		}
		log("Page loaded, running Javascript test");

		timer t;
		t.start();
		int loopCount = 0;
		double result;
		Awesomium::FutureJSValue futureVal;

		while(t.elapsed_time() < LENGTH_SEC)
		{
			futureVal = webView->executeJavascriptWithResult("test(987654321, 123456789, 'awesomium')");

			//Sleep(1);

			result = futureVal.get().toDouble();

			if((int)result != 25417574)
			{
				log("Test failed, incorrect value returned:");
				LOG(result);
				return false;
			}

			loopCount++;
		}

		logTestValue("EvalJavascript", loopCount / (double)LENGTH_SEC);

		return true;
	}

	void onBeginNavigation(const std::string& url, const std::wstring& frameName) {}
	void onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType) {}
	void onFinishLoading() { hasLoaded = true; }
	void onCallback(const std::string& name, const Awesomium::JSArguments& args) {}
	void onReceiveTitle(const std::wstring& title, const std::wstring& frameName) {}
	void onChangeTooltip(const std::wstring& tooltip) {}
#if defined(_WIN32)
	void onChangeCursor(const HCURSOR& cursor) {}
#endif
	void onChangeKeyboardFocus(bool isFocused) {}
	void onChangeTargetURL(const std::string& url) {}
};
