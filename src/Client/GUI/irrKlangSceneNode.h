#ifndef __IRRKLANG_SCENE_NODE_H_INCLUDED__
#define __IRRKLANG_SCENE_NODE_H_INCLUDED__

// This file contains a scene node for the Irrlicht engine which is able to
// play back sounds and music in 3D. Just place it into 3d space and set
// what sound it should play.
// It uses the free irrKlang sound engine (http://www.ambiera.com/irrklang).
// This file also contains a sound engine factory, CIrrKlangSceneNodeFactory. Just 
// register this factory in your scene manager, and you will be able to load and
// save irrKlang scene nodes from and into .irr files:
//
// irrklang::ISoundEngine* soundEngine = createIrrKlangDevice();
// CIrrKlangSceneNodeFactory* factory = new CIrrKlangSceneNodeFactory(soundEngine, sceneManager);
// sceneManager->registerSceneNodeFactory(factory);
// factory->drop();

#include "irrlicht/irrlicht.h"
#include "irrklang/irrKlang.h"

using namespace irr;
using namespace irrklang;
using namespace scene;


//! A scene node for  playing 3d audio using the free irrKlang sound engine.
/** Use it like this:

\begincode
irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
IrrlichtDevice* device = createDevice(video::EDT_OPENGL, core::dimension2d<s32>(640, 480), 16, false);
scene::ISceneManager* smgr = device->getSceneManager();

// .. other code here

CIrrKlangSceneNode* soundNode = 
  new CIrrKlangSceneNode(soundEngine, smgr->getRootSceneNode(), smgr, 666);

  soundNode->setSoundFilename("yourfile.wav");
  soundNode->setMinMaxSoundDistance(30.0f); // influences the distance where the sound can be heard
  soundNode->setRandomMode(1000, 5000); // plays sound multiple times with random interval

  // other modes would be:
  // soundNode->setLoopingStreamMode() 
  // or
  // soundNode->setPlayOnceMode();

  soundNode->drop();
\endcode
*/
class CIrrKlangSceneNode : public scene::ISceneNode
{
public:

	CIrrKlangSceneNode(irrklang::ISoundEngine* soundEngine,
					   scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
	~CIrrKlangSceneNode();

	// play modes:

	//! Sets the play mode to 'play once', a sound file is played once, and 
	//! the scene node deletes itself then, if wished.
	void setPlayOnceMode(bool deleteWhenFinished=false);

	//! Sets the play mode to 'looping stream', plays a looped sound over and over again.
	void setLoopingStreamMode();

	//! Sets the play mode to 'random'. Plays a sound with a variable, random interval
	//! over and over again.
	//! \param minTimeMsInterval: Minimal wait time in milli seconds before the sound is played again.
	//! \param maxTimeMsInterval: Maximal wait time in milli seconds before the sound is played again.
	void setRandomMode(int minTimeMsInterval=1000, int maxTimeMsInterval=5000);

	// Sound parameters

	//! Adds a sound filename to play
    void addSoundFileName(const char* soundFilename);

	//! Gets the sound filename to play
    const char* getSoundFileName(u32 id) const;

	//! Sets the minimal and maximal 3D sound distances.
	//! Set to negative values if you want to use the default values of the sound engine.
	void setMinMaxSoundDistance(f32 minDistance=1.0f, f32 maxDistance=10000000.0f);

	//! stops playback, releases sound, sets playmode to 'nothing'
	void stop();

	// rendering functions:

	virtual void OnAnimate(u32 timeMs);
	virtual void OnRegisterSceneNode();
	virtual void render();
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual ESCENE_NODE_TYPE getType() const;

    scene::ISceneNode *getDebugCube(void) { return cube; }
    scene::ITextSceneNode *getDebugText(void) { return text; }

protected:

	enum EPlayMode
	{
		EPM_NOTHING = 0,
		EPM_RANDOM,
		EPM_LOOPING,
		EPM_ONCE
	};

	core::aabbox3d<f32> Box;
	irrklang::ISoundEngine* SoundEngine;
	irrklang::ISound* Sound;

    core::array<core::stringc> SoundFileNames;
	f32 MinDistance;
	f32 MaxDistance;

	EPlayMode PlayMode;
	u32 TimeMsDelayFinished;
	bool DeleteWhenFinished;
	s32 MaxTimeMsInterval;
	s32 MinTimeMsInterval;
	s32 PlayedCount;
    scene::ISceneNode *cube;
    scene::ITextSceneNode *text;
};



// own factory implementation to make it possible to load and save irrKlang scene
// nodes fomr and to .irr files. Create an instance of this and use
// ISceneManager::registerSceneNodeFactory() for this.
class CIrrKlangSceneNodeFactory : public ISceneNodeFactory
{
public:

	CIrrKlangSceneNodeFactory(irrklang::ISoundEngine* soundEngine, ISceneManager* mgr);
	~CIrrKlangSceneNodeFactory();

	virtual ISceneNode* addSceneNode(ESCENE_NODE_TYPE type, ISceneNode* parent=0);
	virtual ISceneNode* addSceneNode(const c8* typeName, ISceneNode* parent=0);
	virtual u32 getCreatableSceneNodeTypeCount() const;
	virtual const c8* getCreateableSceneNodeTypeName(u32 idx) const;
	virtual ESCENE_NODE_TYPE getCreateableSceneNodeType(u32 idx) const;
	virtual const c8* getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const;

private:

	ESCENE_NODE_TYPE getTypeFromName(const c8* name);
	ISceneManager* Manager;
	irrklang::ISoundEngine* SoundEngine;
};


#endif