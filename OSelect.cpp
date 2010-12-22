#include "OSelect.h"
#include <iostream>
#include <string.h>
#include <errno.h>

/**
 * @desc No argument constructor
 * Useless...
 */
OSelect::OSelect() {
	FD_ZERO(&set);
}

/**
 * @desc OSelect constructor
 * This constructor also setup max_fd.
 * @param int fds[] File descriptor that you want to watch for activity
 */
OSelect::OSelect(const int fds[]) {

	int				pi = NULL;
	int				max_fd = -1;

	OSelect();
	for (pi = 0; fds[pi] >= 0; pi++) {
		if (fds[pi] > max_fd)
			max_fd = fds[pi];

		this->fds.push_front(fds[pi]);
	}

	this->max_fd = max_fd;
}

/**
 * @desc Add a file descriptor to monitor
 * @param const int &fd File descriptor
 * @return void
 */
void OSelect::add(const int &fd) {
	if (fd > max_fd)
		max_fd = fd;
	return fds.push_front(fd);
}

/**
 * @desc Remove a file from set
 * @param const int &fd File descriptor to remove
 * @return void
 */
void OSelect::remove(const int &fd) {
	return fds.remove(fd);
}

/**
 * @desc How many file descriptors are we keeping on track ?
 * @return size_t Number of file descriptors in current set
 */
size_t OSelect::count(void) {
	return fds.size();
}

/**
 * @desc Check to see if one of our file descriptor
 * is available for reading, in time period
 * @param const struct timeval *time Timeout
 * @return list<int> File descriptors available for reading
 */
list<int> OSelect::can_read(const struct timeval *time) {

	list<int>::iterator	it;
	list<int>			readable;
	struct timeval 		timeout;
	int					code = 0;

again:
	/* Timeout could be modified by select() */
	memcpy(&timeout, time, sizeof(timeout));

	FD_ZERO(&set);
	for (it = fds.begin(); it != fds.end(); it++)
		FD_SET(*it, &set);

	code = select(max_fd+1, &set, NULL, NULL, &timeout);
	if (code < 0) {
		if (errno != EINTR) {
			throw("Select error");
		}
		goto again;
	}

	for (it = fds.begin(); it != fds.end(); it++) {
		if (FD_ISSET(*it, &set))
			readable.push_front(*it);
	}

	return readable;
}

/**
 * @param const int seconds Timeout in seconds
 */
list<int> OSelect::can_read(const int seconds) {
	struct timeval timeout = {seconds, 0};
	return can_read(&timeout);
}

list<int> OSelect::can_write(const struct timeval *time) {

	list<int>::iterator	it;
	list<int>			writable;
	struct timeval 		timeout;
	int					code = 0;

again:
	/* Timeout could be modified by select() */
	memcpy(&timeout, time, sizeof(timeout));

	FD_ZERO(&set);
	for (it = fds.begin(); it != fds.end(); it++)
		FD_SET(*it, &set);

	code = select(max_fd+1, NULL, &set, NULL, &timeout);
	if (code < 0) {
		if (errno != EINTR) {
			throw("Select error");
		}
		goto again;
	}

	for (it = fds.begin(); it != fds.end(); it++) {
		if (FD_ISSET(*it, &set))
			writable.push_front(*it);
	}

	return writable;
}

list<int> OSelect::can_write(const int seconds) {
	struct timeval timeout = {seconds, 0};
	return can_write(&timeout);
}
