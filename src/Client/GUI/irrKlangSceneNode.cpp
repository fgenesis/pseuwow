#include "irrKlangSceneNode.h"

// id for our scene node, 'ikng', short for 'irrklang'.
// If this line doesn't work, you are probably using irrlicht 1.3 or earlier.
// Then remove the MAKE_IRR_ID thing and replace it with a random number.
const int IRRKLANG_SCENE_NODE_ID = MAKE_IRR_ID('i','k','n','g');

// type name for our scene node
const char* irrKlangSceneNodeTypeName = "irrKlangSceneNode";



CIrrKlangSceneNode::CIrrKlangSceneNode(irrklang::ISoundEngine* soundEngine, 
									   scene::ISceneNode* parent,
									   scene::ISceneManager* mgr, s32 id)
	: scene::ISceneNode(parent, mgr, id), SoundEngine(soundEngine)
{
	setAutomaticCulling(scene::EAC_OFF);

	MinDistance = 20.0f; // a usual good value for irrlicht engine applications
	MaxDistance = -1.0f;
	PlayMode = EPM_RANDOM;
	TimeMsDelayFinished = 0;
	DeleteWhenFinished = false;
	MaxTimeMsInterval = 5000;
	MinTimeMsInterval = 1000;
	Sound = 0;
	PlayedCount = 0;

	if (SoundEngine)
		SoundEngine->grab();
}


CIrrKlangSceneNode::~CIrrKlangSceneNode()
{
	stop();

	if (SoundEngine)
		SoundEngine->drop();
}


void CIrrKlangSceneNode::OnRegisterSceneNode()
{
	if (IsVisible && DebugDataVisible!=0)
		SceneManager->registerNodeForRendering(this, irr::scene::ESNRP_TRANSPARENT);

	ISceneNode::OnRegisterSceneNode();
}


void CIrrKlangSceneNode::OnAnimate(u32 timeMs)
{
	ISceneNode::OnAnimate(timeMs);

	// play the sound

	core::vector3df pos = getAbsolutePosition();

	if (Sound)
		Sound->setPosition(pos);

	switch(PlayMode)
	{
	case EPM_NOTHING:
		return;
	case EPM_RANDOM:
		{
			if (Sound && Sound->isFinished())
			{
				Sound->drop();
				Sound = 0;

				// calculate next play time

				s32 delta = MaxTimeMsInterval - MinTimeMsInterval;

				if (delta < 2)
					delta = 2;

				TimeMsDelayFinished = timeMs + (rand()%delta) + MinTimeMsInterval;
			}
			else
			if (!Sound && (!TimeMsDelayFinished || timeMs > TimeMsDelayFinished))
			{
				// play new sound		

				if (SoundFileName.size())
					Sound = SoundEngine->play3D(SoundFileName.c_str(), pos, false, true, true);

				if (Sound)
				{
					if (MinDistance > 0 )
						Sound->setMinDistance(MinDistance);
					if (MaxDistance > 0 )
						Sound->setMaxDistance(MaxDistance);

					Sound->setIsPaused(false);
				}
			}
		}
		break;
	case EPM_LOOPING:
		{
			if (!Sound)
			{
				if (SoundFileName.size())
					Sound = SoundEngine->play3D(SoundFileName.c_str(), pos, true, true, true);

				if (Sound)
				{
					if (MinDistance > 0 )
						Sound->setMinDistance(MinDistance);
					if (MaxDistance > 0 )
						Sound->setMaxDistance(MaxDistance);

					Sound->setIsPaused(false);
				}
				else
				{
					// sound could not be loaded
					stop();
				}
			}
		}
		break;
	case EPM_ONCE:
		{
			if (PlayedCount)
			{
				// stop

				if (Sound && Sound->isFinished())
				{
					stop();

					if (DeleteWhenFinished)
						SceneManager->addToDeletionQueue(this);
				}
			}
			else
			{
				// start

				if (SoundFileName.size())
					Sound = SoundEngine->play3D(SoundFileName.c_str(), pos, false, true, true);

				if (Sound)
				{
					if (MinDistance > 0 )
						Sound->setMinDistance(MinDistance);
					if (MaxDistance > 0 )
						Sound->setMaxDistance(MaxDistance);

					Sound->setIsPaused(false);
					++PlayedCount;
				}
				else
				{
					// sound could not be loaded
					stop();
				}
			}
		}
		break;
	}
}


void CIrrKlangSceneNode::render()
{
	// draw scene node as billboard when debug data is visible

	if (!DebugDataVisible)
		return;

	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
      
	scene::ICameraSceneNode* camera = SceneManager->getActiveCamera();
	if (camera)
	{
		video::S3DVertex vertices[4];
		u16 indices[6];

		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;
		indices[3] = 0;
		indices[4] = 3;
		indices[5] = 2;

		vertices[0].TCoords.set(0.0f, 1.0f);
		vertices[1].TCoords.set(0.0f, 0.0f);
		vertices[2].TCoords.set(1.0f, 0.0f);
		vertices[3].TCoords.set(1.0f, 1.0f);

		vertices[0].Color.set(128,255,255,255);
		vertices[1].Color.set(128,255,255,255);
		vertices[2].Color.set(128,255,255,255);
		vertices[3].Color.set(128,255,255,255);

		core::vector3df pos = getAbsolutePosition();
		core::vector3df campos = camera->getAbsolutePosition();
		core::vector3df target = camera->getTarget();
		core::vector3df up = camera->getUpVector();
		core::vector3df view = target - campos;
		view.normalize();

		core::vector3df horizontal = up.crossProduct(view);
		horizontal.normalize();

		core::vector3df vertical = horizontal.crossProduct(view);
		vertical.normalize();

		const f32 Size = 5.0f;
		horizontal *= Size / 2.0f;
		vertical *= Size / 2.0f;	

		vertices[0].Pos = pos + horizontal + vertical;
		vertices[1].Pos = pos + horizontal - vertical;
		vertices[2].Pos = pos - horizontal - vertical;
		vertices[3].Pos = pos - horizontal + vertical;

		view *= -1.0f;

		for (s32 i=0; i<4; ++i)
			vertices[i].Normal = view;

		// draw billboard

		video::SMaterial material;
		material.Lighting = false;
		material.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
		material.MaterialTypeParam = 255;
		material.TextureLayer[0].Texture = driver->getTexture("textures/editor_defaults/default_sound.png");

		core::matrix4 mat;
		driver->setTransform(video::ETS_WORLD, mat);
		driver->setMaterial(material);

		driver->drawIndexedTriangleList(vertices, 4, indices, 2);
	}
}


const core::aabbox3d<f32>& CIrrKlangSceneNode::getBoundingBox() const
{
	return Box;
}


const c8* const IrrKlangPlayModeNames[] =
{
	"nothing", "random", "looping",	"play_once", 0
};


//! Writes attributes of the scene node.
void CIrrKlangSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	if (!out)
		return;

	ISceneNode::serializeAttributes(out, options);
	const char** soundNames = 0;

	if ( SoundEngine && options && ( options->Flags & io::EARWF_FOR_EDITOR ) )
	{
		// show a list of all loaded sound files in editor

		int count = SoundEngine->getSoundSourceCount();
		soundNames = new const char*[count+3];

		for (int i=0; i<count; ++i)
			soundNames[i] = SoundEngine->getSoundSource(i)->getName();

		soundNames[count] = "";
		soundNames[count+1] = "<select>"; // editor file selector
		soundNames[count+2] = 0;

		out->addEnum("SoundFileName", SoundFileName.c_str(), soundNames);

		delete [] soundNames; 
	}
	else
	{
		// only store string of sound file if not in editor 
		out->addString("SoundFileName", SoundFileName.c_str());
	}

	// add play modes

	out->addEnum("PlayMode", PlayMode, IrrKlangPlayModeNames);
	out->addFloat("MinDistance", MinDistance);
	out->addFloat("MaxDistance", MaxDistance);

	// only save the necessary attributes

	switch(PlayMode)
	{
	case EPM_ONCE:
		out->addBool("DeleteWhenFinished", DeleteWhenFinished);
		break;
	case EPM_RANDOM:
		out->addInt("MinTimeMsInterval", MinTimeMsInterval);
		out->addInt("MaxTimeMsInterval", MaxTimeMsInterval);
		break;
	}
}


//! Reads attributes of the scene node.
void CIrrKlangSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	if (!in)
		return;

	ISceneNode::deserializeAttributes(in);

	core::stringc oldFileName = SoundFileName;
	SoundFileName = in->getAttributeAsString("SoundFileName");
	EPlayMode newMode = (EPlayMode)in->getAttributeAsEnumeration("PlayMode", IrrKlangPlayModeNames);
	
	f32 minDistance = in->getAttributeAsFloat("MinDistance");
	f32 maxDistance = in->getAttributeAsFloat("MaxDistance");	

	setMinMaxSoundDistance(minDistance, maxDistance);

	DeleteWhenFinished = in->getAttributeAsBool("DeleteWhenFinished");

	if (in->existsAttribute("MinTimeMsInterval"))
		MinTimeMsInterval = in->getAttributeAsInt("MinTimeMsInterval");
	if (in->existsAttribute("MaxTimeMsInterval"))
		MaxTimeMsInterval = in->getAttributeAsInt("MaxTimeMsInterval");

	if (newMode != PlayMode || oldFileName != SoundFileName)
	{
		stop();
		TimeMsDelayFinished = 0;
		PlayMode = newMode;
	}
}



//! Creates a clone of this scene node and its children.
ISceneNode* CIrrKlangSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
	// this method is only implemented to let irrEdit be able to copy the
	// scene node via CTRL+C, it's not necessary

	if (!newParent) newParent = Parent;
	if (!newManager) newManager = SceneManager;

	CIrrKlangSceneNode* nb = new CIrrKlangSceneNode(SoundEngine, newParent, newManager, ID);

	nb->cloneMembers(this, newManager);

	nb->SoundFileName = SoundFileName;
	nb->MinDistance = MinDistance;
	nb->Box = Box;
	nb->PlayMode = PlayMode;
	nb->TimeMsDelayFinished = TimeMsDelayFinished;
	nb->DeleteWhenFinished = DeleteWhenFinished;
	nb->MaxTimeMsInterval = MaxTimeMsInterval;
	nb->MinTimeMsInterval = MinTimeMsInterval;

	nb->drop();
	return nb;
}


//! Returns type of the scene node
ESCENE_NODE_TYPE CIrrKlangSceneNode::getType() const
{
	return (ESCENE_NODE_TYPE)IRRKLANG_SCENE_NODE_ID;
}


void CIrrKlangSceneNode::stop()
{
	PlayMode = EPM_NOTHING;
	PlayedCount = 0;

	if (Sound)
	{
		Sound->stop();
		Sound->drop();
		Sound = 0;
	}
}


//! Sets the play mode to 'play once', a sound file is played once, and 
//! the scene node deletes itself then, if wished.
void CIrrKlangSceneNode::setPlayOnceMode(bool deleteWhenFinished)
{
	stop();
	PlayMode = EPM_ONCE;
	DeleteWhenFinished = deleteWhenFinished;
	PlayedCount = 0;
}


//! Sets the play mode to 'looping stream', plays a looped sound over and over again.
void CIrrKlangSceneNode::setLoopingStreamMode()
{
	stop();
	PlayMode = EPM_LOOPING;
}


//! Sets the play mode to 'random'. Plays a sound with a variable, random interval
//! over and over again.
//! \param minTimeMs: Minimal wait time in milli seconds before the sound is played again.
//! \param maxTimeMs: Maximal wait time in milli seconds before the sound is played again.
void CIrrKlangSceneNode::setRandomMode(int minTimeMs, int maxTimeMs)
{
	stop();
	PlayMode = EPM_RANDOM;
	MinTimeMsInterval = minTimeMs;
	MaxTimeMsInterval = maxTimeMs;
}


//! Sets the sound filename to play
void CIrrKlangSceneNode::setSoundFileName(const char* soundFilename)
{
	if (soundFilename)
		SoundFileName = soundFilename;
	else
		SoundFileName = "";
}


//! Gets the sound filename to play
const char* CIrrKlangSceneNode::getSoundFileName() const
{
	return SoundFileName.c_str();
}


//! Sets the minimal and maximal 3D sound distances
void CIrrKlangSceneNode::setMinMaxSoundDistance(f32 minDistance, f32 maxDistance)
{
	MinDistance = minDistance;
	MaxDistance = maxDistance;

	if (Sound)
	{
		if (MinDistance > 0)
			Sound->setMinDistance(MinDistance);

		if (MaxDistance > 0)
			Sound->setMaxDistance(MaxDistance);
	}
}



// ------------------------------------------------------------------------
// Factory
// ------------------------------------------------------------------------

CIrrKlangSceneNodeFactory::CIrrKlangSceneNodeFactory(irrklang::ISoundEngine* sengine, ISceneManager* mgr)
: Manager(mgr), SoundEngine(sengine)
{
	if (SoundEngine)
		SoundEngine->grab();

	// don't grab the manager here, to avoid cyclic references
}


CIrrKlangSceneNodeFactory::~CIrrKlangSceneNodeFactory()
{
	if (SoundEngine)
		SoundEngine->drop();
}


//! adds a scene node to the scene graph based on its type id
ISceneNode* CIrrKlangSceneNodeFactory::addSceneNode(ESCENE_NODE_TYPE type, ISceneNode* parent)
{
	if (!parent)
		parent = Manager->getRootSceneNode();

	if (type == IRRKLANG_SCENE_NODE_ID)
	{
		CIrrKlangSceneNode* node = new CIrrKlangSceneNode(SoundEngine, parent, Manager, -1);
		node->drop();
		return node; 
	}

	return 0;
}


//! adds a scene node to the scene graph based on its type name
ISceneNode* CIrrKlangSceneNodeFactory::addSceneNode(const c8* typeName, ISceneNode* parent)
{
	return addSceneNode( getTypeFromName(typeName), parent );
}


//! returns amount of scene node types this factory is able to create
u32 CIrrKlangSceneNodeFactory::getCreatableSceneNodeTypeCount() const
{
	return 1;
}


//! returns type of a createable scene node type
ESCENE_NODE_TYPE CIrrKlangSceneNodeFactory::getCreateableSceneNodeType(u32 idx) const
{
	if (idx==0)
		return (ESCENE_NODE_TYPE)IRRKLANG_SCENE_NODE_ID;

	return ESNT_UNKNOWN;
}


//! returns type name of a createable scene node type 
const c8* CIrrKlangSceneNodeFactory::getCreateableSceneNodeTypeName(u32 idx) const
{
	if (idx==0)
		return irrKlangSceneNodeTypeName;

	return 0;
}


//! returns type name of a createable scene node type 
const c8* CIrrKlangSceneNodeFactory::getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const
{
	if (type == IRRKLANG_SCENE_NODE_ID)
		return irrKlangSceneNodeTypeName;

	return 0;
}


ESCENE_NODE_TYPE CIrrKlangSceneNodeFactory::getTypeFromName(const c8* name)
{
	if (!strcmp(name, irrKlangSceneNodeTypeName))
		return (ESCENE_NODE_TYPE)IRRKLANG_SCENE_NODE_ID;

	return ESNT_UNKNOWN;
}
