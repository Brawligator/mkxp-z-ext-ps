#include "console-input.h"

#include <iostream>
#include <string>

void (*debugOutputHandler)(const std::string &line) = nullptr;

ConsoleInput::ConsoleInput()
    : thread(nullptr),
      mutex(SDL_CreateMutex()),
      running(false)
{}

ConsoleInput::~ConsoleInput()
{
	stop();
	SDL_DestroyMutex(mutex);
}

void ConsoleInput::start()
{
	if (running)
		return;

	running = true;
	thread = SDL_CreateThread(readerThreadFun, "console", this);
}

void ConsoleInput::stop()
{
	running = false;

	/* The reader thread is blocked on stdin, so we detach it
	 * rather than waiting. It will exit when stdin closes
	 * or the process terminates. */
	if (thread)
	{
		SDL_DetachThread(thread);
		thread = nullptr;
	}
}

bool ConsoleInput::poll(std::string &out)
{
	SDL_LockMutex(mutex);

	if (queue.empty())
	{
		SDL_UnlockMutex(mutex);
		return false;
	}

	out = queue.front();
	queue.pop();

	SDL_UnlockMutex(mutex);
	return true;
}

void ConsoleInput::writeLine(const std::string &line)
{
	SDL_LockMutex(mutex);

	/* Move cursor to column 0, clear the line, print output,
	 * then reprint the prompt */
	std::cout << "\r\033[K" << line << std::endl;
	std::cout << ">> " << std::flush;

	SDL_UnlockMutex(mutex);
}

int ConsoleInput::readerThreadFun(void *data)
{
	ConsoleInput *self = static_cast<ConsoleInput *>(data);
	std::string line;

	std::cout << ">> " << std::flush;

	while (self->running && std::getline(std::cin, line))
	{
		if (line.empty())
		{
			std::cout << ">> " << std::flush;
			continue;
		}

		SDL_LockMutex(self->mutex);
		self->queue.push(line);
		SDL_UnlockMutex(self->mutex);
	}

	return 0;
}
