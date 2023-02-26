# yNodeGUI
1.定位：针对于简易C++游戏开发或者xxx系统期末大作业开发

2.目标人群：主要针对有面向对象基础的同学，要求掌握一定easyx和C++的基本语法

3.使用方式:仅拷贝node_menu.h和framework.h到项目内，建议使用Microsoft VisualStudio,并在easyx官网（https://easyx.cn/  ）   .安装easyx

4.打包方式:将VS切换到Release分支，在项目Release设置内，代码生成选择MT，即可脱离C++和easyx环境运行
# 功能
1.多样的GUI组件
包含按钮，缩放框，网格，文本，图片等多种组件

2.极低学习成本
框架内实现了NodeGUI，菜单管理方便，同时简单的注册机制容易上手

4.高效的菜单管理器
将菜单抽象为N叉树，能高效处置多级深度菜单
# API使用

1.Main函数内创建图形界面

	//创建画布
	Canvas canvas = { 1000,600,90, RGB(255,255,255) }; 
	//展示GUI并阻塞程序
	canvas.Show(OnStart, OnUpdate, OnGUI,true); 

2.在Start内定义菜单节点

  	//创建根节点和节点菜单管理器
	Node* root = new Node();
	menu = new Menu(root, &canvas);

	//主菜单节点
	Node* m1 = new Node(root, "课程信息录入");
	Node* m2 = new Node(root, "课程信息浏览", browsing, true, fresh);
	Node* m3 = new Node(root, "课程信息查询", queryshow, true, fresh_query);
	Node* m4 = new Node(root, "学生自由选课");
	Node* m6 = new Node(root, "安全退出系统", exit_system, true);

	//主菜单选项1  一级菜单节点
	Node* x1 = new Node(m1, "录入课程信息", input, true);
	Node* x2 = new Node(m1, "修改课程信息", set_course, true);
	Node* x3 = new Node(m1, "删除课程信息", del_course, true);
	Node* x4 = new Node(m1, "返回", last_menu, true);

	//主菜单选项4  一级菜单节点
	Node* n1 = new Node(m4, "自由选课", choose, true);
	Node* n2 = new Node(m4, "已选课程查看", view_selected, true, fresh_selected);
	Node* n3 = new Node(m4, "返回", last_menu, true);
  
3. 注册和渲染 /这些内容可以在不同的位置实现，具体请看Core.cpp文件内的示例

	//注册重绘机制
	
	int id = node->childs[i]->InstanceId();
	
	canvas.Env(0).Register(id, btn);
	
	canvas.Env(0).Draw(id);


# demo展示 课程管理信息系统
![XU46_ WNAT(@N9X7@Q96~DP(1)](https://user-images.githubusercontent.com/102401735/210708153-57b217cd-1c39-4807-a5a6-3491b8a3aa7c.png)
![`)A$IB%W9_S(ZG` EIVYS0T(1)](https://user-images.githubusercontent.com/102401735/210708171-f9381628-22e6-4f81-b43f-cf506db0c33f.png)
![image](https://user-images.githubusercontent.com/102401735/210708204-9b2827bd-805b-4eec-ae7c-e2bae25f54c8.png)

# 缺陷
1.没有实现输入框和更多的GUI组件，具备一定局限性

2.采用 Env-Id 注册重绘机制，不适合过于复杂的项目，不过对小游戏来讲完全不是问题，甚至更方便

3.没有实现重绘消息剔除，只能驾驭小游戏或者性能要求低的项目
