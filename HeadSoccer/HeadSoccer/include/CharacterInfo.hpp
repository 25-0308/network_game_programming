#include <string>

class CharacterInfo
{
public:
	CharacterInfo() : imageName(), flagName(), score(0), speedStat_(0), jumpStat_(0), kickStat_(0) {}
	CharacterInfo(const std::string& img, const std::string& flg, int scr, int speed, int jump, int kick, int player_num)
		: imageName(img), flagName(flg), score(scr), speedStat_(speed), jumpStat_(jump), kickStat_(kick) {
	}
	CharacterInfo(const CharacterInfo&) = default;
	CharacterInfo& operator=(const CharacterInfo&) = default;
	CharacterInfo(CharacterInfo&&) = default;
	CharacterInfo& operator=(CharacterInfo&&) = default;
	virtual ~CharacterInfo() = default;
	
	std::string getImage() const { return imageName; }
	std::string getFlag() const { return flagName; }
	int getScore() const { return score; }
	int getSpeedStat() const { return speedStat_; }
	int getJumpStat() const { return jumpStat_; }
	int getKickStat() const { return kickStat_; }
	
	void setImage(const std::string& img) { imageName = img; }
	void setFlag(const std::string& flg) { flagName = flg; }
	void setScore(int scr) { score = scr; }
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
	int score;
};

