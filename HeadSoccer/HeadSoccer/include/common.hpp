#ifndef __common_hpp
#define __common_hpp

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <algorithm>	// min, max 함수의 오류 발생으로 인해 포함시킴

namespace Gdiplus {
	using std::min;
	using std::max;
}

#include "util.hpp"
#include "PointFloat.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <cstdint>
#include <cassert>

#include <filesystem>
namespace fs = std::filesystem;

#include <array>
#include <vector>
#include <map>

enum class ObjectType {
	Background,
	ImageStorage,
	Ball,
	TimeUI,
	InGameUI,
	Button,
	Player,

	GoalPost,

	EOE
};

enum class SceneType {
	Title,
	SelectCharacter,
	SelectStage,
	InGame,
	Result,

	EOE
};

enum class EventType {
	CreateObject,
	DestroyObject,
	ChangeScene,
	UpdateIndex,

	EOE
};

enum class StageNumber {
	None,
	Stage1,
	Stage2,
	Stage3,
	Stage4,

	EOE
};

#endif	// __common_hpp