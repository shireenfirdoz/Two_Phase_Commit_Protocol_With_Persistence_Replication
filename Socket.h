#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <string>

#include <sys/poll.h>
#include <sys/select.h>

#define NAGLE_ON	0
#define NAGLE_OFF	1

class Socket {
protected:
	int fd_;
	bool is_initialized_;

private:
	int nagle_;

public:
	Socket();
	virtual ~Socket();

	int Send(char *buffer, int size, int flags = 0);
	int Recv(char *buffer, int size, int flags = 0);

	int Send(void *buffer, int size);
	int Recv(void *buffer, int size);

	int NagleOn(bool on_off);
	bool IsNagleOn();

	void Close();

	bool getIsInitialized();

	int writeInt(int number);
	int readInt();
};


#endif // end of #ifndef __SOCKET_H__
