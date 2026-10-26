#include "common.hpp"

class Timer {
public:
	static void init();
	static void update();
	static void render();

	static double dt() {
		return dt_;
	}
	static float fdt() {
		return static_cast<float>(dt_);
	}

private:
	static LARGE_INTEGER prevCnt_;
	static LARGE_INTEGER currCnt_;
	static LARGE_INTEGER freq_;

	static double dt_;
	static double accT_;
	static std::int32_t fps_;
};

