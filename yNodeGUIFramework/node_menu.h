#pragma once
// ReSharper disable All
#include<iostream>
#include<vector>
#include<cassert>
#include"framework.h"
#include<windows.h>
using namespace std;


class Menu;
class Node :public Instance
{
private:
	int lev = 0;//节点层级，根节点为0
public:
	Node* parent = nullptr; //父节点指针，根节点为nullptr
	void (*func)(Menu& munu, Canvas& canvas) = nullptr; //节点功能函数，循环执行
	void (*onceFunc)(Menu& munu) = nullptr; //节点功能辅助函数，只执行一次
	string tag; //节点名
	vector<Node*> childs; //子节点指针表
	bool funcNode = false;  //是否为单一功能节点

	//当前节点层级
	int Level() { return lev; }

	
#pragma region 节点构造
	//构造/析构
	Node()
	{
		tag = "RootNode";
		lev = 0;
	}
	//形参：父节点指针，节点名，节点功能函数(循环)，是否为单一功能节点，辅助函数(只执行一次)
	Node(Node* p, const char* t, void f(Menu& menu, Canvas& canvas) = nullptr, bool fnode = false, void once(Menu& menu) = nullptr)
	{
		assert(p != nullptr);
		parent = p;
		func = f;
		tag = t;
		funcNode = fnode;
		lev = p->lev + 1;
		onceFunc = once;
		parent->childs.push_back(this);
	}
	//DFS析构
	~Node()
	{
		if (childs.size() == 0) return;
		for (auto i : childs) delete i;
	}
	
#pragma endregion
#pragma region 节点切换
	//尝试跳转到下一节点
	Node* Next(int id, bool* result=NULL)
	{
		assert(id > 0);  
		if (id >= 1 && id <= childs.size())
		{
			if (result != NULL)*result = true;
			return childs[id - 1];
		}
		if (result != NULL)*result = false;
		return this;
	}
	//尝试返回上一节点
	Node* Last(bool* result = NULL)
	{
		if (parent != nullptr)
		{
			if (result != NULL)*result = true;
			return parent;
		}
		if (result != NULL)*result = false;
		return this;
	}
#pragma endregion
};

//节点菜单类，不负责具体逻辑，只负责维护节点树
class Menu :public Instance
{
	Node* root;  //根节点

	Menu() = default;
public:
	//当前节点,虽然可以被修改了，但是不管了....累
	Node* current;
	Canvas* canvas;//画布

	//构造/析构
	Menu(Node* first, Canvas* cv)
	{
		root = first;  //根节点
		canvas = cv;  //画布
		current = first;  //当前节点

	}
	~Menu()
	{
		//在根节点DFS回收整个N叉树
		delete root;
	}
#pragma region 节点跳转函数

	//节点函数，负责节点跳转，如果不允许跳转，则current不变，每次跳转都调用节点更新辅助函数
	void ToRoot()
	{
		current = root;
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}
	//节点函数，负责节点跳转，如果不允许跳转，则current不变，每次跳转都调用节点更新辅助函数
	void Last()
	{
		current = current->Last();
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}
	//节点函数，负责节点跳转，如果不允许跳转，则current不变，每次跳转都调用节点更新辅助函数
	void Next(int idx)
	{
		current = current->Next(idx);
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}

	/// <summary>
	/// 注册根节点组成的菜单
	/// </summary>
	/// <param name="root">根节点</param>
	/// <param name="xOffest">x偏移，在画布中心x的基础上偏移</param>
	/// <param name="yOffest">y偏移，在画布最顶端的基础上向下偏移</param>
	/// <param name="yStep">按钮生成的步长</param>
	/// <param name="width">按钮宽度</param>
	/// <param name="height">按钮高度</param>
	/// <param name="edgeWidth">线框边缘厚度</param>
	/// <param name="buttonColor">按钮背景颜色</param>
	/// <param name="fontColor">字体颜色</param>
	/// <param name="lineColor">线框颜色</param>
	/// <param name="fontName">字体名称</param>
	void RegisterMenuByRootNode( int xOffest, int yOffest, int yStep, int width, int height,int edgeWidth, COLORREF buttonColor, COLORREF fontColor, COLORREF lineColor, string fontName)
	{
		//N叉树节点层次遍历，注册按钮
		queue<Node*> que;
		que.push(root);
		while (!que.empty())
		{
			Node* node = que.front();
			que.pop();
			for (int i = 0; i < node->childs.size(); i++)
			{
				//计算Button位置和范围
				Vector2 center = { canvas->Center().x+xOffest ,yStep * i + yOffest };
				Rect rect = createRectbyCenter(center, width, height);
				//创建GUI组件
				Image* background = new Image(rect, buttonColor);
				Text* text = new Text(node->childs[i]->tag.c_str(), rect, fontName, fontColor);
				LineBox* edge = new LineBox(rect, lineColor, edgeWidth);
				Button* btn = new Button(background, text,edge);
				//添加按钮监听回调
				btn->AddListener([i,this]() {Next(i + 1); });
				//注册
				canvas->Env(0).Register(node->childs[i]->InstanceId(), btn);
				que.push(node->childs[i]);
			}
		}
	}
#pragma endregion
};
//输入组，负责一次性读取控制台输入
class InputGroup
{
private:
	COORD coord;
	HANDLE handle;
	struct KeyValuePair
	{
		string key;
		bool value;
		char* format;
		void* p;
	};
	vector<KeyValuePair> pool;
	bool nowarning = false;
public:
	InputGroup(int initY, bool warnning = true)
	{
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		coord.X = 0;
		coord.Y = initY;
		nowarning = !warnning;
	}
	void Add(const string& str, const char* formatter, void* pp)
	{

		pool.push_back({ str,true,(char*)formatter,pp });
	}
	void Add(const string& str)
	{
		pool.push_back({ str,false });
	}

	void Print()
	{
		for (auto& i : pool)
		{
			cout << i.key << endl;
		}
	}
	bool GetInput()
	{
		for (auto& i : pool)
		{
			if (i.value)
			{
				coord.X = i.key.length();
				SetConsoleCursorPosition(handle, coord);

				int r = scanf(i.format, i.p);
				if (r == 0 && !nowarning)
				{
					if (!nowarning)MessageBox(NULL, ("输入格式错误"), ("请检查您的输入并重新尝试"), MB_SYSTEMMODAL | MB_ICONSTOP);
					return false;
				}

			}
			coord.Y++;
		}
		return true;
	}
};

