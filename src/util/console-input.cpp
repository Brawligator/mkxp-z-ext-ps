#include "console-input.h"

#include <string>

#ifdef __WIN32__
#include <io.h>
#include <windows.h>
#else
#include <poll.h>
#include <unistd.h>
#endif

void (*debugOutputHandler)(const std::string &line) = nullptr;

static void rawWrite(const char *str, size_t len)
{
#ifdef __WIN32__
	DWORD written;
	WriteFile(GetStdHandle(STD_ERROR_HANDLE), str, (DWORD)len, &written, NULL);
#else
	write(STDERR_FILENO, str, len);
#endif
}

static void rawWrite(const char *str)
{
	rawWrite(str, strlen(str));
}

static void rawWriteLn(const std::string &line)
{
	rawWrite(line.c_str(), line.size());
	rawWrite("\n", 1);
}

static void rawPrompt()
{
	rawWrite(">> ", 3);
}

/* Returns true if stdin has data ready, with a timeout in ms.
 * Returns false on timeout or error. */
static bool stdinReady(int timeoutMs)
{
#ifdef __WIN32__
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	DWORD result = WaitForSingleObject(h, timeoutMs);
	return result == WAIT_OBJECT_0;
#else
	struct pollfd pfd;
	pfd.fd = STDIN_FILENO;
	pfd.events = POLLIN;
	pfd.revents = 0;
	int ret = poll(&pfd, 1, timeoutMs);
	return ret > 0 && (pfd.revents & POLLIN);
#endif
}

/* Read available bytes from stdin into buf.
 * Returns number of bytes read, 0 on EOF, -1 on error. */
static int stdinRead(char *buf, int size)
{
#ifdef __WIN32__
	HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
	DWORD bytesRead = 0;

	/* For console handles, use ReadConsoleA to get cooked input */
	if (GetConsoleMode(h, &bytesRead))
	{
		if (!ReadConsoleA(h, buf, size, &bytesRead, NULL))
			return -1;
		return (int)bytesRead;
	}

	/* For pipes/files, use ReadFile */
	if (!ReadFile(h, buf, size, &bytesRead, NULL))
		return -1;
	return (int)bytesRead;
#else
	return (int)read(STDIN_FILENO, buf, size);
#endif
}

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
	thread = SDL_CreateThread(consoleThreadFun, "console", this);
}

void ConsoleInput::stop()
{
	running = false;

	if (thread)
	{
		SDL_DetachThread(thread);
		thread = nullptr;
	}
}

bool ConsoleInput::poll(std::string &out)
{
	SDL_LockMutex(mutex);

	if (inputQueue.empty())
	{
		SDL_UnlockMutex(mutex);
		return false;
	}

	out = inputQueue.front();
	inputQueue.pop();

	SDL_UnlockMutex(mutex);
	return true;
}

void ConsoleInput::writeLine(const std::string &line)
{
	SDL_LockMutex(mutex);
	outputQueue.push(line);
	SDL_UnlockMutex(mutex);
}

void ConsoleInput::flushOutput()
{
	SDL_LockMutex(mutex);

	while (!outputQueue.empty())
	{
		rawWriteLn(outputQueue.front());
		outputQueue.pop();
	}

	SDL_UnlockMutex(mutex);
}

int ConsoleInput::consoleThreadFun(void *data)
{
	ConsoleInput *self = static_cast<ConsoleInput *>(data);
	std::string lineBuffer;

	rawPrompt();

	while (self->running)
	{
		/* Flush any pending output */
		self->flushOutput();

		/* Poll stdin with a short timeout so we can
		 * keep flushing output while waiting for input */
		if (!stdinReady(50))
			continue;

		char buf[4096];
		int n = stdinRead(buf, sizeof(buf) - 1);
		if (n <= 0)
			break;

		lineBuffer.append(buf, n);

		/* Extract complete lines */
		size_t pos;
		while ((pos = lineBuffer.find('\n')) != std::string::npos)
		{
			std::string line = lineBuffer.substr(0, pos);
			lineBuffer.erase(0, pos + 1);

			/* Strip trailing \r */
			if (!line.empty() && line.back() == '\r')
				line.pop_back();

			if (!line.empty())
			{
				SDL_LockMutex(self->mutex);
				self->inputQueue.push(line);
				SDL_UnlockMutex(self->mutex);
			}
			else
			{
				rawPrompt();
			}
		}
	}

	return 0;
}
