#include "console-input.h"

#include <string>
#include <cstring>

#ifdef __WIN32__
#include <io.h>
#include <windows.h>
#else
#include <poll.h>
#include <unistd.h>
#include <termios.h>
#endif

void (*debugOutputHandler)(const std::string &line) = nullptr;

static void rawWrite(const char *str, size_t len)
{
#ifdef __WIN32__
	DWORD written;
	HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
	WriteFile(h, str, (DWORD)len, &written, NULL);
#else
	ssize_t r;
	while (len > 0)
	{
		r = write(STDERR_FILENO, str, len);
		if (r <= 0) break;
		str += r;
		len -= r;
	}
#endif
}

static void rawWrite(const std::string &s)
{
	rawWrite(s.c_str(), s.size());
}

ConsoleInput::ConsoleInput()
    : thread(nullptr),
      mutex(SDL_CreateMutex()),
      running(false)
#ifndef __WIN32__
      , rawModeSet(false)
#endif
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

int ConsoleInput::consoleThreadFun(void *data)
{
	ConsoleInput *self = static_cast<ConsoleInput *>(data);

#ifndef __WIN32__
	/* Put terminal in raw mode: no echo, non-canonical */
	struct termios oldTerm, newTerm;

	if (tcgetattr(STDIN_FILENO, &oldTerm) == 0)
	{
		newTerm = oldTerm;
		newTerm.c_lflag &= ~((unsigned)ECHO | (unsigned)ICANON);
		newTerm.c_cc[VMIN] = 0;
		newTerm.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &newTerm);
		self->rawModeSet = true;
	}
#endif

	rawWrite(">> ");

	while (self->running)
	{
		/* Flush pending output, clearing current input line first */
		SDL_LockMutex(self->mutex);
		bool hasOutput = !self->outputQueue.empty();

		if (hasOutput)
		{
			/* Clear the current prompt + input */
			rawWrite("\r\033[K");

			while (!self->outputQueue.empty())
			{
				rawWrite(self->outputQueue.front());
				rawWrite("\n", 1);
				self->outputQueue.pop();
			}
		}

		SDL_UnlockMutex(self->mutex);

		if (hasOutput)
		{
			/* Redraw prompt and current input buffer */
			rawWrite(">> ");
			SDL_LockMutex(self->mutex);
			rawWrite(self->inputLine);
			SDL_UnlockMutex(self->mutex);
		}

		/* Poll stdin for input */
#ifdef __WIN32__
		HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
		DWORD result = WaitForSingleObject(h, 50);
		if (result != WAIT_OBJECT_0)
			continue;

		INPUT_RECORD ir;
		DWORD eventsRead;
		if (!PeekConsoleInput(h, &ir, 1, &eventsRead) || eventsRead == 0)
			continue;
		if (!ReadConsoleInput(h, &ir, 1, &eventsRead))
			continue;
		if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown)
			continue;

		char c = ir.Event.KeyEvent.uChar.AsciiChar;
#else
		struct pollfd pfd;
		pfd.fd = STDIN_FILENO;
		pfd.events = POLLIN;
		pfd.revents = 0;

		if (poll(&pfd, 1, 50) <= 0 || !(pfd.revents & POLLIN))
			continue;

		char c;
		if (read(STDIN_FILENO, &c, 1) != 1)
			break;
#endif

		if (c == '\n' || c == '\r')
		{
			rawWrite("\n", 1);

			SDL_LockMutex(self->mutex);
			if (!self->inputLine.empty())
			{
				self->inputQueue.push(self->inputLine);
				self->inputLine.clear();
			}
			SDL_UnlockMutex(self->mutex);

			rawWrite(">> ");
		}
		else if (c == 127 || c == 8)
		{
			/* Backspace */
			SDL_LockMutex(self->mutex);
			if (!self->inputLine.empty())
			{
				self->inputLine.pop_back();
				rawWrite("\b \b", 3);
			}
			SDL_UnlockMutex(self->mutex);
		}
		else if (c == 3)
		{
			/* Ctrl+C: clear current input */
			SDL_LockMutex(self->mutex);
			rawWrite("\r\033[K");
			self->inputLine.clear();
			SDL_UnlockMutex(self->mutex);
			rawWrite(">> ");
		}
		else if (c >= 32)
		{
			/* Printable character */
			SDL_LockMutex(self->mutex);
			self->inputLine += c;
			SDL_UnlockMutex(self->mutex);
			rawWrite(&c, 1);
		}
	}

#ifndef __WIN32__
	/* Restore terminal */
	if (self->rawModeSet)
		tcsetattr(STDIN_FILENO, TCSANOW, &oldTerm);
#endif

	return 0;
}
