#include "Ogre.h"
#include "InputManager.h"
#include "WebCore.h"
#include "OgrePanelOverlayElement.h"
#include "KeyboardHook.h"
#include "OgreTimer.h"
#include "TerrainCamera.h"
#include <direct.h>
#include <stdlib.h>

std::string getCurrentWorkingDirectory();

void GetMeshInformation(const Ogre::MeshPtr mesh,
                         size_t &vertex_count,
                         Ogre::Vector3* &vertices,
                         size_t &index_count,
                         unsigned long* &indices,
                         const Ogre::Vector3 &position,
                         const Ogre::Quaternion &orient,
                         const Ogre::Vector3 &scale );

using namespace Ogre;
using namespace Awesomium;

#define ASYNC_RENDER 1
#define SUPER_QUALITY 0

class Application : public OIS::MouseListener, public OIS::KeyListener, public HookListener, public WebViewListener
{
public:
	bool shouldQuit;
	Ogre::RenderWindow* renderWin;
	SceneManager* sceneMgr;
	InputManager* inputMgr;
	Camera* camera;
	Viewport* viewport;
	Ogre::Overlay* overlay;
	Ogre::PanelOverlayElement* panel;
	Ogre::TexturePtr webTexture;
	WebCore* core; 
	WebView* view;
	KeyboardHook* hook;
	int width, height;
	bool isTransparent;
	bool isOverlayMode;
	bool isKeyboardFocused;
	Ogre::Timer timer;
	TerrainCamera* terrainCam;
	SceneNode* planeNode;
	Ogre::RaySceneQuery* raySceneQuery;

	Application() : shouldQuit(false), renderWin(0), sceneMgr(0), camera(0), viewport(0), inputMgr(0), view(0), core(0), 
		hook(0), isTransparent(false), isOverlayMode(true), isKeyboardFocused(false), terrainCam(0), raySceneQuery(0)
	{
		Root *root = new Root("plugins.cfg", "ogre.cfg", "ogre.log");
		shouldQuit = !root->showConfigDialog();
		if(shouldQuit)
			return;

		root->initialise(true, "Awesomium Demo - Push Esc to Exit");
		renderWin = root->getAutoCreatedWindow();

		static const ColourValue skyColor(0.6, 0.7, 0.8);

		sceneMgr = root->createSceneManager("TerrainSceneManager");
		
		parseResources();
		sceneMgr->setWorldGeometry("terrain.cfg");

		// Set the ambient light and fog
		sceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
		sceneMgr->setFog(FOG_LINEAR, skyColor, 0, 400, 1000);

		// Create sun-light
		Light* light = sceneMgr->createLight("Sun");
		light->setType(Light::LT_DIRECTIONAL);
		light->setDirection(Vector3(0, -1, -0.8));

		camera = sceneMgr->createCamera("mainCamera");
		viewport = renderWin->addViewport(camera);
		viewport->setBackgroundColour(skyColor);
		SceneNode* camNode = sceneMgr->getRootSceneNode()->createChildSceneNode("camNode");
		terrainCam = new TerrainCamera(camNode, camera);

		camera->setFarClipDistance(950);
		camera->setNearClipDistance(1);

		if(root->getRenderSystem()->getCapabilities()->getNonPOW2TexturesLimited())
		{
			std::cerr << "Error! Rendering device has limited/no support for NPOT textures, falling back to 512x512 dimensions.\n";
			width = height = 512;
		}
		else
		{
			width = viewport->getActualWidth();
			height = viewport->getActualHeight();
		}

		sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
		sceneMgr->setShadowFarDistance(260);
		sceneMgr->setShadowColour(ColourValue(0.7, 0.7, 0.7));
		sceneMgr->setShadowTextureSize(width > 800 ? 1024 : 512);
#if SUPER_QUALITY
		sceneMgr->setShadowTextureSize(2048);
#endif

		loadInputSystem();

		createOverlayAndMaterials();

		setupAwesomium();

		Plane plane(Vector3::UNIT_X, 0);
		MeshManager::getSingleton().createPlane("webPlane",
           ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
           width,height,2,2,true,1,1,1,Vector3::UNIT_Y);
		Entity* planeEnt = sceneMgr->createEntity("webPlaneEntity", "webPlane");
		planeEnt->setMaterialName("webPlaneMaterial");
		planeEnt->setCastShadows(true);
		planeNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
		planeNode->attachObject(planeEnt);
		planeNode->scale(0.25, 0.25, 0.25);
		planeNode->setVisible(false);

		hook = new KeyboardHook(this);

		raySceneQuery = sceneMgr->createRayQuery(Ray());

		sceneMgr->getRootSceneNode()->setVisible(false);
	}

	~Application()
	{
		if(raySceneQuery)
			delete raySceneQuery;

		if(terrainCam)
			delete terrainCam;

		if(hook)
			delete hook;

		if(view)
		{
			view->destroy();
			delete core;
		}

		if(sceneMgr)
			sceneMgr->clearScene();

		webTexture.setNull();

		if(Root::getSingletonPtr())
			delete Root::getSingletonPtr();
	}

	void setupAwesomium()
	{
		core = new WebCore(LOG_VERBOSE);
		core->setBaseDirectory(getCurrentWorkingDirectory() + "..\\media\\");

		core->setCustomResponsePage(404, "404response.html");

		view = core->createWebView(width, height, false, ASYNC_RENDER, 90);

		view->setProperty("welcomeMsg", "Hey there, thanks for checking out the Awesomium v1.0 Demo.");
		view->setProperty("renderSystem", Root::getSingleton().getRenderSystem()->getName());
		view->setCallback("requestFPS");
		view->setListener(this);

		view->loadFile("demo.html");
	}

	void createOverlayAndMaterials()
	{
		Ogre::String texName = "webTexture"; 
		Ogre::String materialName = "webMaterial";
		Ogre::String planeMaterialName = "webPlaneMaterial";

		webTexture = Ogre::TextureManager::getSingleton().createManual(
			texName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_BYTE_BGRA,
			Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = webTexture->getBuffer();
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
		size_t dstBpp = Ogre::PixelUtil::getNumElemBytes(pixelBox.format);
		size_t dstPitch = pixelBox.rowPitch * dstBpp;

		Ogre::uint8* dstData = static_cast<Ogre::uint8*>(pixelBox.data);
		memset(dstData, 255, dstPitch * height);
		pixelBuffer->unlock();

		// Create Overlay Material
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(materialName, 
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		Ogre::Pass* pass = material->getTechnique(0)->getPass(0);
		pass->setDepthCheckEnabled(false);
		pass->setDepthWriteEnabled(false);
		pass->setLightingEnabled(false);
		pass->setSceneBlending(Ogre::SBT_REPLACE);

		Ogre::TextureUnitState* texUnit = pass->createTextureUnitState(texName);
		texUnit->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		// Create Plane Material
		material = Ogre::MaterialManager::getSingleton().create(planeMaterialName, 
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		pass = material->getTechnique(0)->getPass(0);
		pass->setLightingEnabled(false);
		pass->setSceneBlending(Ogre::SBT_REPLACE);
		pass->setCullingMode(Ogre::CULL_NONE);

		texUnit = pass->createTextureUnitState(texName);
		texUnit->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
		texUnit->setTextureAnisotropy(4);
#if SUPER_QUALITY
		texUnit->setTextureAnisotropy(16);
#endif

		OverlayManager& overlayManager = OverlayManager::getSingleton();

		panel = static_cast<PanelOverlayElement*>(overlayManager.createOverlayElement("Panel", "webPanel"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setMaterialName(materialName);
		panel->setDimensions(width, height);
		panel->setPosition(0, 0);
		
		overlay = overlayManager.create("webOverlay");
		overlay->add2D(panel);
		overlay->show();
	}

	void resizeOverlay(int width, int height)
	{
		this->width = width;
		this->height = height;

		Ogre::MaterialPtr mat = MaterialManager::getSingleton().getByName("webMaterial");
		Ogre::Pass* pass = mat->getTechnique(0)->getPass(0);
		pass->removeAllTextureUnitStates();

		Ogre::TextureManager::getSingleton().remove("webTexture");
		webTexture.setNull();

		webTexture = Ogre::TextureManager::getSingleton().createManual(
			"webTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_BYTE_BGRA,
			Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		Ogre::TextureUnitState* texUnit = pass->createTextureUnitState("webTexture");
		texUnit->setTextureFiltering(Ogre::FO_NONE, Ogre::FO_NONE, Ogre::FO_NONE);

		panel->setDimensions(width, height);
	}

	void draw()
	{
		if(!view->isDirty())
			return;

		Ogre::HardwarePixelBufferSharedPtr pixelBuffer = webTexture->getBuffer();
		pixelBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
		const Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();
		size_t dstBpp = Ogre::PixelUtil::getNumElemBytes(pixelBox.format);
		size_t dstPitch = pixelBox.rowPitch * dstBpp;

		Ogre::uint8* dstData = static_cast<Ogre::uint8*>(pixelBox.data);
		
		view->render(dstData, (int)dstPitch, (int)dstBpp, 0);

		pixelBuffer->unlock();
	}

	void parseResources()
	{
		ConfigFile cf;
		cf.load("resources.cfg");
		ConfigFile::SectionIterator seci = cf.getSectionIterator();

		String secName, typeName, archName;
		while(seci.hasMoreElements())
		{
			secName = seci.peekNextKey();
			ConfigFile::SettingsMultiMap *settings = seci.getNext();
			ConfigFile::SettingsMultiMap::iterator i;
			for(i = settings->begin(); i != settings->end(); ++i)
			{
				typeName = i->first;
				archName = i->second;
				ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
			}
		}

		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}

	void loadInputSystem()
	{
		inputMgr = InputManager::getSingletonPtr();
		inputMgr->initialise(renderWin);
		inputMgr->addMouseListener(this, "MouseListener");
		inputMgr->addKeyListener(this, "KeyListener");
	}

	void update()
	{
		draw();
		Ogre::WindowEventUtilities::messagePump();
		Root::getSingleton().renderOneFrame();
		InputManager::getSingletonPtr()->capture();
		core->update();

		static long lastTime = 0;

		if(!isKeyboardFocused)
		{
			Ogre::Real delta = 165 * (timer.getMilliseconds() - lastTime) / (Ogre::Real)1000;
			
			OIS::Keyboard* keyboard = inputMgr->getKeyboard();

			bool isTranslating = false;
			Vector3 translation(0, 0, 0);

			if(keyboard->isKeyDown(OIS::KC_W))
			{
				translation += Vector3(0, 0, delta);
				isTranslating = true;
			}

			if(keyboard->isKeyDown(OIS::KC_S))
			{
				translation += Vector3(0, 0, -delta);
				isTranslating = true;
			}

			if(keyboard->isKeyDown(OIS::KC_A))
			{
				translation += Vector3(delta, 0, 0);
				isTranslating = true;
			}

			if(keyboard->isKeyDown(OIS::KC_D))
			{
				translation += Vector3(-delta, 0, 0);
				isTranslating = true;
			}

			if(isTranslating)
			{
				terrainCam->translate(translation);
				terrainCam->clampToTerrain();
			}
		}

		lastTime = timer.getMilliseconds();
	}

	void onBeginNavigation(const std::string& url, const std::wstring& frameName)
	{
		std::cout << "Navigating to URL: " << url << ", Frame: ";
		std::wcout << frameName << std::endl;
	}

	void onBeginLoading(const std::string& url, const std::wstring& frameName, int statusCode, const std::wstring& mimeType)
	{
		std::cout << "Begin Loading URL: " << url << "\n\twith a status code of: " << statusCode << "\n\tand a mime-type of: ";
		std::wcout << mimeType << L", Frame: " << frameName << std::endl;
	}

	void onFinishLoading()
	{
		std::cout << "Finished Loading" << std::endl;
	}

	void onCallback(const std::string& name, const Awesomium::JSArguments& args)
	{
		if(name == "requestFPS")
		{
			const RenderTarget::FrameStats& stats = viewport->getTarget()->getStatistics();
			view->setProperty("fps", (int)stats.lastFPS);
			view->executeJavascript("updateFPS()");
		}
	}
	
	void onReceiveTitle(const std::wstring& title, const std::wstring& frameName)
	{
		std::cout << "Receieved Title: " << title << ", Frame: ";
		std::wcout << frameName << std::endl;
	}

	void onChangeCursor(const HCURSOR& cursor)
	{
	}

	void onChangeTooltip(const std::wstring& tooltip)
	{
		if(tooltip.length())
			std::wcout << "Tooltip: " << tooltip << std::endl;
	}

	void onChangeKeyboardFocus(bool isFocused)
	{
		isKeyboardFocused = isFocused;

		if(isFocused)
			std::cout << "Keyboard is Focused" << std::endl;
		else
			std::cout << "Keyboard is Un-Focused" << std::endl;
	}

	void onChangeTargetURL(const std::string& url)
	{
		std::cout << "Target URL: " << url << std::endl;
	}

	bool mouseMoved(const OIS::MouseEvent &arg)
	{
		if(arg.state.buttonDown(OIS::MB_Right))
		{
			// If we are in camera-pivot state, spin/pitch the camera based
			// on relative mouse movement.
			terrainCam->spin(Degree(-arg.state.X.rel * 0.14));
			terrainCam->pitch(Degree(-arg.state.Y.rel * 0.1));
			return true;
		}

		if(isOverlayMode)
		{
			if(arg.state.Z.rel)
				view->injectMouseWheel(arg.state.Z.rel / 2);
			else
				view->injectMouseMove(arg.state.X.abs, arg.state.Y.abs);
		}
		else
		{
			int localX, localY;

			if(webPlaneHitTest(arg.state.X.abs, arg.state.Y.abs, arg.state.width, arg.state.height, localX, localY))
			{
				if(arg.state.Z.rel)
					view->injectMouseWheel(arg.state.Z.rel / 2);
				else
					view->injectMouseMove(localX, localY);
			}
		}

		return true;
	}

	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
	{
		if(id == OIS::MB_Left)
		{
			if(isOverlayMode)
			{
				view->injectMouseDown(LEFT_MOUSE_BTN);
			}
			else
			{
				int localX, localY;

				if(webPlaneHitTest(arg.state.X.abs, arg.state.Y.abs, arg.state.width, arg.state.height, localX, localY))
				{
					view->injectMouseDown(LEFT_MOUSE_BTN);
				}
			}
		}

		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
	{
		if(id == OIS::MB_Left)
			view->injectMouseUp(LEFT_MOUSE_BTN);

		return true;
	}

	void handleKeyMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		view->injectKeyboardEvent(hwnd, msg, wParam, lParam);
	}

	bool keyPressed( const OIS::KeyEvent &arg )
	{
		return true;
	}

	bool keyReleased( const OIS::KeyEvent &arg )
	{
		if(arg.key == OIS::KC_ESCAPE)
		{
			shouldQuit = true;
		}
		else if(arg.key == OIS::KC_F1)
		{
			isOverlayMode = !isOverlayMode;
			view->unfocus();

			if(isOverlayMode)
			{
				overlay->show();
				planeNode->setVisible(false);
				sceneMgr->getRootSceneNode()->setVisible(false);
			}
			else
			{
				overlay->hide();
				terrainCam->orientPlaneToCamera(planeNode, height * 0.25);
				planeNode->setVisible(true);
				sceneMgr->getRootSceneNode()->setVisible(true);
			}
		}
		else if(arg.key == OIS::KC_F2)
		{
			isTransparent = !isTransparent;

			view->setTransparent(isTransparent);

			Ogre::Pass* overlayMatPass = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().
				getByName("webMaterial"))->getTechnique(0)->getPass(0);

			Ogre::Pass* planeMatPass = static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().
				getByName("webPlaneMaterial"))->getTechnique(0)->getPass(0);

			if(isTransparent)
			{
				std::cout << "Rendering the WebView as transparent." << std::endl;
				overlayMatPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
				planeMatPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			}
			else
			{
				std::cout << "Rendering the WebView as opaque." << std::endl;
				overlayMatPass->setSceneBlending(Ogre::SBT_REPLACE);
				planeMatPass->setSceneBlending(Ogre::SBT_REPLACE);
			}
		}
		else if(arg.key == OIS::KC_F3)
		{
			view->loadFile("demo.html");
		}
		else if(arg.key == OIS::KC_F4)
		{
			view->zoomIn();
		}
		else if(arg.key == OIS::KC_F5)
		{
			view->zoomOut();
		}
		else if(arg.key == OIS::KC_F6)
		{
			const RenderTarget::FrameStats& stats = viewport->getTarget()->getStatistics();
			std::cout << "Current FPS: " << (int)stats.lastFPS << std::endl;
		}
		else if(arg.key == OIS::KC_F7)
		{
			static bool isShadowsDisabled = true;
			isShadowsDisabled = !isShadowsDisabled;

			if(isShadowsDisabled)
			{
				std::cout << "Shadows Disabled." << std::endl;
				sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
			}
			else
			{
				std::cout << "Shadows Enabled." << std::endl;
				sceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
			}
		}
		else if(arg.key == OIS::KC_F9 || arg.key == OIS::KC_WEBBACK)
		{
			view->goToHistoryOffset(-1);
		}
		else if(arg.key == OIS::KC_F10 || arg.key == OIS::KC_WEBFORWARD)
		{
			view->goToHistoryOffset(1);
		}
		
		return true;
	}

	// Based on code from the 'BrowserPlaneDemo'
	bool webPlaneHitTest(int x, int y, int width, int height, int& outX, int& outY)
	{
		bool hit = false;

		Ray mouseRay = camera->getCameraToViewportRay((Ogre::Real)x / width, (Ogre::Real)y / height);
		raySceneQuery->setRay(mouseRay);
		raySceneQuery->setSortByDistance(true);

		RaySceneQueryResult &result = raySceneQuery->execute();

		for(RaySceneQueryResult::iterator i = result.begin(); i != result.end(); i++)
			if(i->movable && i->movable->getName().substr(0, strlen("webPlaneEntity")) == "webPlaneEntity")
				 hit = rayHitPlane((Ogre::Entity*)i->movable, mouseRay, outX, outY);

		raySceneQuery->clearResults();

		return hit;
	}

	// Based on code from the 'BrowserPlaneDemo'
	bool rayHitPlane(Ogre::Entity* plane, const Ray& ray, int& outX, int& outY)
	{
		bool hit = false;
		Ogre::Real closest_distance = -1.0f;
  
		size_t vertexCount, indexCount;
		Ogre::Vector3 *vertices;
		unsigned long *indices;

		/**
		* Note: Since this is just a demo, care hasn't been taken to optimize everything
		*		to the extreme, however, for a production setting, it would be a bit
		*		more efficient to save the mesh information for each Entity we care
		*		about so that we aren't needlessly calling the following function every
		*		time we wish to a little mouse-picking.
		*/

		// get the mesh information
		GetMeshInformation(plane->getMesh(), vertexCount, vertices, indexCount, indices,             
			plane->getParentNode()->_getDerivedPosition(), plane->getParentNode()->_getDerivedOrientation(),
			plane->getParentNode()->getScale());

		// test for hitting individual triangles on the mesh
		bool new_closest_found = false;
		std::pair<bool, Ogre::Real> intersectTest;
		for(int i = 0; i < (int)indexCount; i += 3)
		{
			/**
			* Note: We could also check for hits on the back-side of the plane by swapping
			*		the 'true' and 'false' tokens in the below statement.
			*/

			// check for a hit against this triangle
			intersectTest = Ogre::Math::intersects(ray, vertices[indices[i]], vertices[indices[i+1]], 
			vertices[indices[i+2]], true, false);

			// if it was a hit check if its the closest
			if(intersectTest.first)
			{
				if((closest_distance < 0.0f) || (intersectTest.second < closest_distance))
				{
					// this is the closest so far, save it off
					closest_distance = intersectTest.second;
					new_closest_found = true;
				}
			}
		}

		// free the vertices and indices memory
		delete[] vertices;
		delete[] indices;

		// get the parent node
		Ogre::SceneNode* browserNode = plane->getParentSceneNode();

		// if we found a new closest raycast for this object, update the
		// closest_result before moving on to the next object.
		if(browserNode && new_closest_found)
		{
			Ogre::Vector3 pointOnPlane = ray.getPoint(closest_distance);
			Ogre::Quaternion quat = browserNode->_getDerivedOrientation().Inverse();
			Ogre::Vector3 result = quat * pointOnPlane;
			Ogre::Vector3 positionInWorld = quat * browserNode->_getDerivedPosition();
			Ogre::Vector3 scale = browserNode->_getDerivedScale();

			outX = (width/2) - ((result.z - positionInWorld.z)/scale.z);
			outY = (height/2) - ((result.y - positionInWorld.y)/scale.y);

			hit = true;
		}

		return hit;
	}

};

std::string getCurrentWorkingDirectory()
{
	std::string workingDirectory = "";
	char currentPath[_MAX_PATH];
	getcwd(currentPath, _MAX_PATH);
	workingDirectory = currentPath;

	return workingDirectory + "\\";
}

// Get the mesh information for the given mesh.
// Code found on this forum link: http://www.ogre3d.org/wiki/index.php/RetrieveVertexData
void GetMeshInformation( const Ogre::MeshPtr mesh,
                         size_t &vertex_count,
                         Ogre::Vector3* &vertices,
                         size_t &index_count,
                         unsigned long* &indices,
                         const Ogre::Vector3 &position,
                         const Ogre::Quaternion &orient,
                         const Ogre::Vector3 &scale )
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    vertex_count = index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh( i );

        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }

        // Add the indices
        index_count += submesh->indexData->indexCount;
    }


    // Allocate space for the vertices and indices
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned long[index_count];

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if((!submesh->useSharedVertices)||(submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //      Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);

                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }


        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                    static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
}