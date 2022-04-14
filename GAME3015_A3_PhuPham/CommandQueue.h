#pragma once

#include "Command.h"
#include "GameInfo.h"

class CommandQueue
{
public:
	void push(const Command& command)
	{
		mQueue.push(command);
	}

	Command pop()
	{
		Command	temp = mQueue.front();

		mQueue.pop();

		return temp;
	}

	bool isEmpty()	const
	{
		return mQueue.empty();
	}

private:
	std::queue<Command>	mQueue;
};

