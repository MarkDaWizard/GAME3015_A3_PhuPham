#pragma once

#include "GameInfo.h"
#include "Category.h"

class Command
{
public:
	Command();

	std::function<void(class SceneNode&, float)> action;
	Category category;
};

