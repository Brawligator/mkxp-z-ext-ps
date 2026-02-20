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

	/* Returns the next queued command from the user, or false if none */
	bool poll(std::string &out);

	/* Queue a line for the console thread to print.
	 * Safe to call from any thread. */
	void writeLine(const std::string &line);

private:
	static int consoleThreadFun(void *data);

	void flushOutput();

	SDL_Thread *thread;
	SDL_mutex *mutex;
	std::queue<std::string> inputQueue;
	std::queue<std::string> outputQueue;
	bool running;
};

/* When set, Debug() routes output here instead of stderr directly. */
extern void (*debugOutputHandler)(const std::string &line);

#endif // CONSOLE_INPUT_H
