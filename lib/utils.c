#include <stdlib.h>   // rand, srand, exit, FD_ZERO, FD_SET, select
#include <unistd.h>   // STDIN_FILENO, read
#include <sys/time.h>
#include <stdbool.h>



#ifdef _WIN32  // Windows 版本
#include <windows.h>
#include <conio.h>    // 用于 kbhit()

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

// Windows 终端（cmd.exe 和 PowerShell）默认不支持 ANSI 转义序列，
// 但 Windows 10 及更新版本已经支持 ANSI 转义序列，需要 手动启用。
void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut == INVALID_HANDLE_VALUE) return;
    // 获取当前模式
    if (!GetConsoleMode(hOut, &dwMode)) return;
    // 启用 ANSI 处理（ENABLE_VIRTUAL_TERMINAL_PROCESSING）
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static DWORD oldMode;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (enable){  // 在windows下要手动启用ANSI转义序列, 为了兼容linux, 就在这里启用了
		enableANSI();
	}
    if (enable && !enabled) {
        SetConsoleMode(hStdin, oldMode);  // 恢复原模式
        enabled = true;
    }
    else if (!enable && enabled) {
        GetConsoleMode(hStdin, &oldMode);
        DWORD newMode = oldMode;
        newMode &= ~ENABLE_LINE_INPUT;  // 禁用行缓冲
        newMode &= ~ENABLE_ECHO_INPUT;  // 禁用回显
        SetConsoleMode(hStdin, newMode);
        enabled = false;
    }
}

int getch(){
	if(kbhit()){
		return _getch();
	}
	return -1;
}

#else  // Linux / macOS 版本
#include <termios.h>  // struct termios, tcgetattr, tcsetattr

// 禁用缓冲输入时，用户输入的每个字符都会立即传递给程序，而不需要按Enter(用于实时交互)
void setBufferedInput(bool enable)
{
	static bool enabled = true;
	static struct termios old;
	struct termios new_;

	if (enable && !enabled)
	{
		// restore the former settings
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		// set the new state
		enabled = true;
	}
	else if (!enable && enabled)
	{
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO, &new_);
		// we want to keep the old setting to restore them at the end
		old = new_;
		// disable canonical mode (buffered i/o) and local echo
		new_.c_lflag &= (~ICANON & ~ECHO);
		// set the new_ settings immediately
		tcsetattr(STDIN_FILENO, TCSANOW, &new_);
		// set the new state
		enabled = false;
	}
}

int getch_with_timeout(unsigned int timeout_ms) {
    struct timeval timeout;
    fd_set fds;
    int ret;
    char c;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    timeout.tv_sec = timeout_ms / 1000;   // 秒部分
    timeout.tv_usec = (timeout_ms % 1000) * 1000;  // 毫秒转换为微秒

    ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);

    if (ret > 0) {
        if (read(STDIN_FILENO, &c, 1) > 0) {
            return (unsigned char)c;  // 返回读取的字符
        }
    }
    return -1; // 超时或错误
}

int getch(){
    return getch_with_timeout(0);
}

#endif