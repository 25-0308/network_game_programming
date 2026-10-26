#ifndef __character_scene_hpp
#define __character_scene_hpp

#include "Scene.hpp"

class CharacterScene : public Scene {
public:
	CharacterScene( ) {}
	CharacterScene( std::string_view sceneName )
		: Scene( sceneName ) {}

	virtual ~CharacterScene( ) { }

	void entry( ) override;
	void exit( ) override;
};

#endif  // __character_scene_hpp