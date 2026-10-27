#ifndef __character_info_hpp
#define __character_info_hpp

#include <string>

class CharacterInfo
{
public:
	CharacterInfo() : imageName(), flagName(), speedStat_(0), kickStat_(0), jumpStat_(0) {}
	CharacterInfo(const CharacterInfo&) = default;
	CharacterInfo& operator=(const CharacterInfo&) = default;
	CharacterInfo(CharacterInfo&&) = default;
	CharacterInfo& operator=(CharacterInfo&&) = default;
	virtual ~CharacterInfo() = default;
	
	std::string getImage() const { return imageName; }
	std::string getFlag() const { return flagName; }
	int getSpeedStat() const { return speedStat_; }
	int getJumpStat() const { return jumpStat_; }
	int getKickStat() const { return kickStat_; }
	
	void setImage(const std::string& img) { imageName = img; }
	void setFlag(const std::string& flg) { flagName = flg; }
	void setStats(int speed, int kick, int jump) {
		speedStat_ = speed;
		kickStat_ = kick;
		jumpStat_ = jump;
	}

private:
	std::string imageName;
	std::string flagName;
	int speedStat_;
	int kickStat_;
	int jumpStat_;
};

#endif	// __character_info_hpp

