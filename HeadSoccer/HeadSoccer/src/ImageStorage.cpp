#include "../include/imageStorage.hpp"
#include "../include/ResourceHandler.hpp"
#include <filesystem>
#include "../include/Camera.hpp"
#include "../include/CharacterInfo.hpp"
#include "GameConstants.hpp"

#include<iostream>

void ImageStorage::initializeImages( const std::string& leftOrRight ) {
    // 맵은 파일 이름순이라 index_가 Protocol::Character 순서와 같음
    for ( std::size_t i = 0; i < characterNames.size( ); ++i ) {
        auto file = std::string( characterNames[ i ] ) + "_" + leftOrRight + ".png";
        addImage( file, ResourceHandler::loadImage( file, std::filesystem::canonical( "assets/" + file ).string( ), true ) );

        auto flagFile = std::string( characterFlags[ i ] );
        addFlagImage( flagFile, ResourceHandler::loadImage( flagFile, std::filesystem::canonical( "assets/" + flagFile ).string( ), false ) );
    }

    // 스탯 막대 (render에서 findImage로 꺼내 씀)
    ResourceHandler::loadImage( "stat_color.png", std::filesystem::canonical( "assets/stat_color.png" ).string( ), true );
}

void ImageStorage::update() {
    componentUpdate();
}

void ImageStorage::update(std::int32_t index) {
    setIndex(index);

    componentUpdate();
}

void ImageStorage::render(HDC hdc) {
    // 캐릭터 이미지 렌더링
    auto images = std::vector<Image*>(images_.size());
    std::transform(images_.begin(), images_.end(), images.begin(),
        [](const auto& pair) { return pair.second; });

    if (index_ >= 0 && static_cast<size_t>(index_) < images.size()) {
        auto relativePos = charpos_ - Camera::getPos();
        images[index_]->draw(hdc, relativePos);
    }

    // 플래그 이미지 렌더링
    auto flagImages = std::vector<Image*>(flagImages_.size());
    std::transform(flagImages_.begin(), flagImages_.end(), flagImages.begin(),
        [](const auto& pair) { return pair.second; });

    if (index_ >= 0 && static_cast<size_t>(index_) < flagImages.size()) {
        auto flagPos = flagpos_ - Camera::getPos();
        flagImages[index_]->draw(hdc, flagPos);
    }

	// 스탯 UI 렌더링 (스탯 값은 서버와 같은 GameConst::characterStats)
	const auto& stat = GameConst::characterStats[ index_ ];
	auto statimage = ResourceHandler::findImage("stat_color.png");
	if (statimage) {
		auto statPos = statpos_ - Camera::getPos();                            
        statimage->draw(hdc, statPos, stat.speed, -30);
		statimage->draw(hdc, statPos, stat.kick, -1);
		statimage->draw(hdc, statPos, stat.jump, 27);
	}
    
    componentRender(hdc); // Animator 렌더링
}