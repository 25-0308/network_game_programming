#ifndef __scene_handler_hpp
#define __scene_handler_hpp

#include "common.hpp"
#include "Scene.hpp"

class SceneHandler {
public:
	static void init( );

	static void update( ) {
		currScene_->update( );
	}

	static void render( HDC hdc ) {
		currScene_->render( hdc );
	}

	static void release( ) {
		for ( auto scene : scenes_ ) {
			delete scene;
		}
	}

	static Scene* getCurrScene( ) {
		return currScene_;
	}

	static SceneType getCurrSceneType( ) {
		return currSceneType_;
	}

	static void changeScene( SceneType type );

private:
	static std::array<Scene*, etoi( SceneType::EOE )> scenes_;
	static Scene* currScene_;
	static SceneType currSceneType_;
};

#endif	// __scene_handler_hpp