#ifndef __in_game_scene_hpp
#define __in_game_scene_hpp

#include "Scene.hpp"

class InGameScene : public Scene {
public:
	InGameScene( ) {}
	InGameScene( std::string_view sceneName )
		: Scene( sceneName ) {}

	virtual ~InGameScene( ) {}

	void entry( ) override;
	void exit( ) override;
};

#endif  // __in_game_scene_hpp