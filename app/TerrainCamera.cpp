#include "TerrainCamera.h"

using namespace Ogre;

TerrainCamera::TerrainCamera(Ogre::SceneNode *baseNode, Ogre::Camera *camera, Vector3 offset, Real height) : baseNode(baseNode), camera(camera), height(height)
{
	pivotNode = baseNode->createChildSceneNode(baseNode->getName()+"_PivotNode", Ogre::Vector3(700, 300, 700));
	camNode = pivotNode->createChildSceneNode(baseNode->getName()+"_CameraNode", offset);
	camNode->yaw(Degree(180));
	camNode->attachObject(camera);
	rayQuery = camera->getSceneManager()->createRayQuery(Ray(pivotNode->getPosition(), Vector3::NEGATIVE_UNIT_Y));
	clampToTerrain();
}

TerrainCamera::~TerrainCamera()
{
	camNode->detachAllObjects();
	baseNode->removeAndDestroyChild(pivotNode->getName());
}

Camera* TerrainCamera::getCamera()
{
	return camera;
}

void TerrainCamera::spin(const Ogre::Radian &angle)
{
	pivotNode->yaw(angle);
}

void TerrainCamera::pitch(const Ogre::Radian &angle)
{
	camNode->pitch(angle);
}

void TerrainCamera::translate(const Ogre::Vector3& displacement)
{
	pivotNode->translate(displacement, Ogre::Node::TS_LOCAL);
}

void TerrainCamera::clampToTerrain()
{
	rayQuery->setRay(Ray(pivotNode->getPosition() + Vector3(0, 200, 0), Vector3::NEGATIVE_UNIT_Y));
	RaySceneQueryResult &qryResult = rayQuery->execute();
	RaySceneQueryResult::iterator i = qryResult.begin();
	if(i != qryResult.end() && i->worldFragment)
		pivotNode->setPosition(pivotNode->getPosition().x, i->worldFragment->singleIntersection.y + 1 + height, pivotNode->getPosition().z);
}

void TerrainCamera::orientPlaneToCamera(Ogre::SceneNode* planeNode, int planeHeight)
{
	pivotNode->translate(Ogre::Vector3(0, 0, 270), Ogre::Node::TS_LOCAL);

	planeNode->setPosition(pivotNode->getPosition());
	
	pivotNode->translate(Ogre::Vector3(0, 0, -270), Ogre::Node::TS_LOCAL);

	planeNode->setOrientation(pivotNode->getOrientation());
	planeNode->yaw(Degree(90));

	rayQuery->setRay(Ray(planeNode->getPosition() + Vector3(0, 170, 0), Vector3::NEGATIVE_UNIT_Y));
	RaySceneQueryResult &qryResult = rayQuery->execute();
	RaySceneQueryResult::iterator i = qryResult.begin();
	if(i != qryResult.end() && i->worldFragment)
		planeNode->setPosition(planeNode->getPosition().x, i->worldFragment->singleIntersection.y + planeHeight / 2, planeNode->getPosition().z);
}