#pragma once
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <string>

// allocate on the heap!
class Listener {
public:
	Listener();
	~Listener();
private:
	QLocalServer server;
};

class ListenerRegister {
	friend class Listener;
public:
	ListenerRegister(const char* command, const char* (*callback)(const QStringList& args));
private:
	struct Signal {
		const char* command;
		const char*(*callback)(const QStringList& args);
	};
	inline static QList<Signal> registered; // c++17 internal initialization \\ use forward declaration for older cpp standarts: QList<ListenerRegister::Signal> ListenerRegister::registered;
};

extern Listener* api;

#define COMMAND(command) \
	static const char* listener_callback_##command(const QStringList& args); \
	ListenerRegister listener_signal_##command(#command, listener_callback_##command); \
	static const char* listener_callback_##command(const QStringList& args)

#define NORETURN return nullptr

#define RETURN_UINT64(uint64) \
	std::string s = std::to_string(uint64); \
	char* result = new char[s.size() + 1]; \
	std::memcpy(result, s.c_str(), s.size() + 1); \
	return result

#define RETURN_STR(QStr) \
	QByteArray arr = QStr.toUtf8(); \
	char* result = new char[arr.size() + 1]; \
	std::memcpy(result, arr.constData(), arr.size()); \
	result[arr.size()] = '\0'; \
	return result

#define RETURN_BOOL(b) \
	std::string s = std::to_string(b); \
	char* result = new char[s.size() + 1]; \
	std::memcpy(result, s.c_str(), s.size() + 1); \
	return result

