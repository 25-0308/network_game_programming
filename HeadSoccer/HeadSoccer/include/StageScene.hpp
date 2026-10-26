#ifndef __stage_scene_hpp
#define __stage_scene_hpp

#include "Scene.hpp"

class StageScene : public Scene {
public:
	StageScene( ) {}
	StageScene( std::string_view sceneName )
		: Scene( sceneName ) {
	}

	virtual ~StageScene( ) { }

	void entry( ) override;
	void exit( ) override;
};

#endif  // __stage_scene_hpp