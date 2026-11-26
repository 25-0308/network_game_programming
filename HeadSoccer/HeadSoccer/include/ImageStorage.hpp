#ifndef __IMAGE_STORAGE_HPP__
#define __IMAGE_STORAGE_HPP__

#include "PointFloat.hpp"
#include "Object.hpp"
#include <map>
#include <string>
#include <vector>

class ImageStorage : public Object
{
public:
    ImageStorage(const std::string& leftOrRight) : Object("", PointFloat( ), PointFloat( ) ), index_( 0 ) {
        initializeImages( leftOrRight );
    }
    ImageStorage(const ImageStorage&) = delete;
    ImageStorage& operator=(const ImageStorage&) = delete;
    ImageStorage(ImageStorage&&) = delete;
    ImageStorage& operator=(ImageStorage&&) = delete;
    virtual ~ImageStorage() = default;

    void update() override;
    void update(std::int32_t index);
    void render(HDC hdc) override;

    void addImage(const std::string& key, Image* image) {
        images_[key] = image;
    }
    void addFlagImage(const std::string& key, Image* image) {
        flagImages_[key] = image;
    }
    std::int32_t getIndex() const { return index_; }
    void setIndex(std::int32_t index) { index_ = (index % 4 + 4) % 4; }
    void updateIndices(std::int32_t delta) {
        setIndex(index_ + delta);
    }
    void setimagepos(PointFloat charpos, PointFloat flagpos, PointFloat statpos) {
        charpos_ = charpos;
        flagpos_ = flagpos;
        statpos_ = statpos;
    }

private:
    void initializeImages(const std::string& leftOrRight);
    PointFloat charpos_;
    PointFloat flagpos_;
    PointFloat statpos_;
    std::map<std::string, Image*> images_;    // 캐릭터 이미지
    std::map<std::string, Image*> flagImages_; // 국기 이미지
    std::int32_t index_;	// 캐릭터, 국기, 스탯이 같은 번호 (Protocol::Character)
};

#endif // __IMAGE_STORAGE_HPP__