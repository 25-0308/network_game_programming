#ifndef __result_scene_hpp
#define __result_scene_hpp

#include "Scene.hpp"

class ResultScene : public Scene {
public:
	ResultScene( ) {}
	ResultScene( std::string_view sceneName )
		: Scene( sceneName ) {}

	virtual ~ResultScene( ) {}
	
	void entry( ) override;
	void exit( ) override;
};

#endif	// __result_scene_hpp