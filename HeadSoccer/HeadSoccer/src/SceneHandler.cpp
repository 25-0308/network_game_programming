#include "../include/SceneHandler.hpp"
#include "../include/TitleScene.hpp"
#include "../include/CharacterScene.hpp"
#include "../include/StageScene.hpp"
#include "../include/InGameScene.hpp"
#include "../include/ResultScene.hpp"
#include "../include/SoundHandler.hpp"

void SceneHandler::init( ) {
	scenes_[ etoi( SceneType::Title ) ] = new TitleScene( "Title Scene" );
	scenes_[ etoi( SceneType::SelectCharacter ) ] = new CharacterScene( "Select Character Scene" );
	scenes_[ etoi( SceneType::SelectStage ) ] = new StageScene( "Select Stage Scene" );
	scenes_[ etoi( SceneType::InGame ) ] = new InGameScene( "In Game Scene" );
	scenes_[ etoi( SceneType::Result ) ] = new ResultScene( "Result Scene" );

	assert( scenes_[ etoi( SceneType::Title ) ] != nullptr );
	assert( scenes_[ etoi( SceneType::SelectCharacter ) ] != nullptr );
	assert( scenes_[ etoi( SceneType::SelectStage ) ] != nullptr );
	assert( scenes_[ etoi( SceneType::InGame ) ] != nullptr );
	assert( scenes_[ etoi( SceneType::Result ) ] != nullptr );

	currSceneType_ = SceneType::Title;
	currScene_ = scenes_[ etoi( SceneType::Title ) ];
	currScene_->entry( );
	SoundHandler::playMenuMusic( );
}

void SceneHandler::changeScene( SceneType type ) {
	if ( type == currSceneType_ ) {
		return; // 같은 장면으로의 중복 전환과 효과음 재생을 막는다.
	}

	currScene_->exit( );
	currScene_ = scenes_[ etoi( type ) ];
	currSceneType_ = type;
	currScene_->entry( );

	if ( type == SceneType::InGame ) {
		SoundHandler::startMatch( );
	}
	else if ( type == SceneType::Result ) {
		SoundHandler::finishMatch( );
	}
	else {
		SoundHandler::playMenuMusic( );
	}
}

std::array<Scene*, etoi( SceneType::EOE )> SceneHandler::scenes_;
Scene* SceneHandler::currScene_;
SceneType SceneHandler::currSceneType_ = SceneType::Title;
