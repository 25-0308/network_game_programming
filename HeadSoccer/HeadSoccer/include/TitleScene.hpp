#ifndef __title_scene_hpp
#define __title_scene_hpp

#include "Scene.hpp"

class TitleScene : public Scene {
public:
	TitleScene( ) {}
	TitleScene( std::string_view sceneName )
		: Scene( sceneName ) {}
	
	virtual ~TitleScene( ) {}

	void entry( ) override;
	void exit( ) override;
};

#endif  // __title_scene_hpp