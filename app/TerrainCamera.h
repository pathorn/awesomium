#ifndef __TerrainCamera_H__
#define __TerrainCamera_H__

#include "Ogre.h"

class TerrainCamera
{
	Ogre::SceneNode *baseNode, *pivotNode, *camNode;
	Ogre::Camera *camera;
	Ogre::RaySceneQuery *rayQuery;
	Ogre::Real height;
public:
	TerrainCamera(Ogre::SceneNode *baseNode, Ogre::Camera *camera, Ogre::Vector3 offset = Ogre::Vector3(0, 40, -60), Ogre::Real height = 20);

	~TerrainCamera();

	Ogre::Camera* getCamera();

	void spin(const Ogre::Radian &angle);

	void pitch(const Ogre::Radian &angle);

	void translate(const Ogre::Vector3& displacement);

	void clampToTerrain();

	void orientPlaneToCamera(Ogre::SceneNode* planeNode, int planeHeight);
};

#endif