#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <csignal>

extern volatile sig_atomic_t g_stop;
void handle_signal(int);

#endif