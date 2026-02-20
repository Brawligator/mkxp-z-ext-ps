#ifndef CONSOLE_INPUT_H
#define CONSOLE_INPUT_H

#include <SDL_mutex.h>
#include <SDL_thread.h>

#include <queue>
#include <string>

class ConsoleInput
{
public:
	ConsoleInput();
	~ConsoleInput();

	void start();
	void stop();

	/* Returns the next queued command, or empty string if none */
	bool poll(std::string &out);

private:
	static int readerThreadFun(void *data);

	SDL_Thread *thread;
	SDL_mutex *mutex;
	std::queue<std::string> queue;
	bool running;
};

#endif // CONSOLE_INPUT_H
