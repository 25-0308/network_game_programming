#ifndef __character_info_hpp
#define __character_info_hpp

#include <array>
#include <string>

// 캐릭터 이미지 이름 앞부분과 국기 파일 (Protocol::Character 순서 = 이름순)
constexpr std::array<const char*, 4> characterNames{ "america", "argentina", "france", "korea" };
constexpr std::array<const char*, 4> characterFlags{ "a_flag.png", "ag_flag.png", "f_flag.png", "k_flag.png" };

class CharacterInfo
{
public:
	CharacterInfo() : imageName(), flagName() {}
	CharacterInfo(const CharacterInfo&) = default;
	CharacterInfo& operator=(const CharacterInfo&) = default;
	CharacterInfo(CharacterInfo&&) = default;
	CharacterInfo& operator=(CharacterInfo&&) = default;
	virtual ~CharacterInfo() = default;
	
	std::string getImage() const { return imageName; }
	std::string getFlag() const { return flagName; }
	
	void setImage(const std::string& img) { imageName = img; }
	void setFlag(const std::string& flg) { flagName = flg; }

private:
	std::string imageName;
	std::string flagName;
};

#endif	// __character_info_hpp
