# myGame
一些小游戏的实现

## 俄罗斯方块(Tetris)
v1和v2均是在linux环境下运行， 不支持windows（你可以在windows上安装wsl）


### v1版本
来源: 在 https://github.com/taylorconor/tinytetris.git 基础上修改得来

改进:
* 可读性更强, 修改了一些变量名和变量值
* 添加了下一个方块的显示
* 添加了游戏界面的相对(0,0)的偏移

操作:
* 使用左右方向键和ad进行方块的左右移动
* 使用上方向键或w进行方块的顺时针旋转
* 使用下方向键或s让方块直接到底
* q退出


![v1 版本 gif](img/tetris-v1.gif)


### v2版本
来源: 在v1的基础上修改得来

改进:
* 删除了curses.h，使用了printf(ANSI转义序列)代替curses的函数


操作:
* 使用左右方向键和ad进行方块的左右移动
* 使用上方向键或w进行方块的顺时针旋转
* 使用下方向键或s加速方块下落
* 使用Enter键让方块直接到底
* q或`Ctrl+C`退出
* 空格暂停


![v2 版本 gif](img/tetris-v2.gif)


## 2048

参考: https://github.com/mevdschee/2048.c.git



## 贪吃蛇(Snake)

操作:
* 使用方向键、`asdw`、`hjkl`进行上下左右移动
* 当按键方向与移动方向相同时，加速前进
* q或`Ctrl+C`退出
* 空格暂停

![v1 版本 Sname](img/snake-v1.gif)