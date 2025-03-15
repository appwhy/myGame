
#include <termios.h>  // struct termios, tcgetattr, tcsetattr
#include <stdlib.h>   // rand, srand, exit, FD_ZERO, FD_SET, select
#include <unistd.h>   // STDIN_FILENO, read

int getch(unsigned int timeout_ms=0) {
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
    return getch(0);
}


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
