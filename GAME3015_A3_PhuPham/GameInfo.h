#pragma once

#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/d3dApp.h"
#include "FrameResource.h"
#include <map>
#include <vector>
#include <functional>
#include <queue>

using Microsoft::WRL::ComPtr;
const int gNumFrameResources = 3;
using namespace DirectX;

/// <summary>
/// Action enums for character game objects
/// </summary>
enum Action
{
	MoveLeft,
	MoveRight,
	MoveFront,
	MoveBack,
	Fire,
	Max
};

enum class State_ID
{
	None,
	Title,
	Menu,
	Game,
	Pause
};

