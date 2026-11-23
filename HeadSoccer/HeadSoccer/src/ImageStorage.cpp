#include "../include/imageStorage.hpp"
#include "../include/ResourceHandler.hpp"
#include <filesystem>
#include "../include/Camera.hpp"

#include<iostream>

void ImageStorage::initializeImages( const std::string& leftOrRight ) {
    std::vector<std::string> image = {
        "korea_" + leftOrRight+ ".png",
        "argentina_" + leftOrRight + ".png",
        "france_" + leftOrRight + ".png",
        "america_" + leftOrRight + ".png"
    };
    std::vector<std::string> flag = {
        "k_flag.png",
        "ag_flag.png",
        "f_flag.png",
        "a_flag.png"
    };
    std::vector<std::string> stat = {
        "stat_color.png",
        "stat_color.png",
        "stat_color.png",
        "stat_color.png"
    };
    for (const auto& file : image) {
        auto filePath = std::filesystem::path("assets/" + file);
        Image* image = ResourceHandler::loadImage(file, std::filesystem::canonical(filePath).string(), true);
        addImage(file, image);
    }
    for (const auto& file : flag) {
        auto flagPath = std::filesystem::path("assets/" + file);
        Image* flagImage = ResourceHandler::loadImage(file, std::filesystem::canonical(flagPath).string(), false);
        addFlagImage(file, flagImage);
    }
    for (const auto& file : stat) {
        auto statPath = std::filesystem::path("assets/" + file);
        Image* statImage = ResourceHandler::loadImage(file, std::filesystem::canonical(statPath).string(), true);
        addStatImage(file, statImage);
    }

    // statData ÃÊ±âÈ­ (america, argentina, france, korea ¼ø¼­·Î ¸ÂÃã)
    statData = {
        {8, 5, 5}, // america
        {5, 8, 5}, // argentina
        {7, 5, 7}, // france
        {6, 6, 6}  // korea
    };
}

void ImageStorage::update() {
    componentUpdate();
}

void ImageStorage::update(std::int32_t index) {
    setIndex(index);
    setFlagIndex(index);
    setStatIndex(index);
    allstatindex = (statIndex_ + 4) % 4;

    componentUpdate();
}

void ImageStorage::render(HDC hdc) {
    // Ä³¸¯ÅÍ ÀÌ¹ÌÁö ·»´õ¸µ
    auto images = std::vector<Image*>(images_.size());
    std::transform(images_.begin(), images_.end(), images.begin(),
        [](const auto& pair) { return pair.second; });

    if (index_ >= 0 && static_cast<size_t>(index_) < images.size()) {
        auto relativePos = charpos_ - Camera::getPos();
        images[index_]->draw(hdc, relativePos);
    }

    // ÇÃ·¡±× ÀÌ¹ÌÁö ·»´õ¸µ
    auto flagImages = std::vector<Image*>(flagImages_.size());
    std::transform(flagImages_.begin(), flagImages_.end(), flagImages.begin(),
        [](const auto& pair) { return pair.second; });

    if (flagIndex_ >= 0 && static_cast<size_t>(flagIndex_) < flagImages.size()) {
        auto flagPos = flagpos_ - Camera::getPos();
        flagImages[flagIndex_]->draw(hdc, flagPos);
    }

    // ½ºÅÈ ÀÌ¹ÌÁö ·»´õ¸µ

	auto statimage = ResourceHandler::findImage("stat_color.png"); // ½ºÅÈ UI ·»´õ¸µ
	if (statimage) {
		auto statPos = statpos_ - Camera::getPos();                            
        statimage->draw(hdc, statPos, statData[statIndex_].speedstat, -30);
		statimage->draw(hdc, statPos, statData[statIndex_].kickstat, -1);
		statimage->draw(hdc, statPos, statData[statIndex_].jumpstat, 27);
	}
    
    componentRender(hdc); // Animator³ª Collider ·»´õ¸µ
}