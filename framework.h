#pragma once
// ReSharper disable All
#include<easyx.h>
#include<graphics.h>
#include<iostream>
#include<vector>
#include<map>
#include <iterator>
#include<queue>
#include<functional>
#include<set>
#include <cassert>
using namespace std;
#pragma region 基本结构
//二维向量
typedef struct
{
	int x;
	int y;
	
} Vector2;
//矩形，包含左上角，右下角，长宽等确定矩形的基本要素
typedef struct
{
	Vector2 origin;
	Vector2 end;
	Vector2 center;
	int width;
	int height;
} Rect;
//实例基类，框架内所有需要实例化的类都需要继承本类，分配唯一ID
class Instance
{
private:
	static int instanceId; //全局实例id位置
	static set<int> ids;  //已被占用的实例id
	int instance_id;  //当前实例id
	//分配一个与其他实例不冲突的ID
	static int AssignInstanceID()
	{
		do { if (instanceId == INT_MIN)instanceId == INT_MAX; instanceId--; } while (ExistID(instanceId));
		RegisterId(instanceId);
		return instanceId;
	}
	//检测是否存在ID
	static bool ExistID(int id)
	{
		return ids.find(id) != ids.end();
	}
	//注册一个ID
	static void RegisterId(int id)
	{
		ids.insert(id);
	}
public:
	//实例创建时分配ID
	Instance()
	{
		instance_id = AssignInstanceID();  //分配一个新的实例ID
	}  
	//实例销毁回收ID
	~Instance()
	{
		ids.erase(instance_id);
	}
	//获取当前实例的ID并返回
	int InstanceId()
	{
		return instance_id;
	}
	
};
int Instance::instanceId = INT_MAX;
set<int> Instance::ids;

//GUI接口，所有GUI组件继承该接口 
class GUI :public Instance
{
public:
	//负责GUI渲染
	virtual void OnGUI() = 0;
	//负责消息处理与事件响应
	virtual void OnEvent(ExMessage* message) = 0;
};

//画布类，负责画布生命维护，不实现具体逻辑
class Canvas :public Instance
{
private:
#pragma region 信息字段
	int width;//宽度
	int height;//高度
	int fps = 60;//帧率
	int frameStart = 0;//当前帧开始时间
	int frameTime;//每帧时长
	bool life = true;//是否存活
	int deltaTime;//上一帧消耗的时间 ms
	COLORREF bgc;//背景色
	ExMessage message;//消息临时内存
	HWND window;//窗口句柄
#pragma endregion
#pragma region 环境与队列
	//GUI注册环境
	map<int, GUI*> gui0;
	map<int, GUI*> gui1;
	map<int, GUI*> gui2;
	map<int, GUI*> gui3;
	map<int, map<int, GUI*>> envs{ { 0,gui0 }, {1,gui1}, {2,gui2},{3,gui3} };

	//GUI临时回收环境
	vector<GUI*> collection0;
	vector<GUI*> collection1;
	vector<GUI*> collection2;
	vector<GUI*> collection3;
	map<int, vector<GUI*>> cenvs{ {0,collection0},{1,collection1},{2,collection2},{3,collection3} };

	//渲染队列/消息队列
	queue <GUI* > renderQueue;
	queue<GUI*> eventQueue;

	//当前环境ID
	int envid = 0;
#pragma endregion
#pragma region 队列化GUI处理
	//渲染GUI并清空队列
	void RenderAll()
	{
		while (!renderQueue.empty())
		{
			renderQueue.front()->OnGUI();
			renderQueue.pop();
		}
	}
	//处理GUI消息并清空队列
	void BroadcastAll(ExMessage* message)
	{
		while (!eventQueue.empty())
		{
			eventQueue.front()->OnEvent(message);
			eventQueue.pop();
		}
	}
#pragma endregion
protected:
#pragma region 生命周期
	//开始时调用
	void (*OnStart)(Canvas& canvas);
	//GUI渲染时调用
	void (*OnGUI)(Canvas& canvas);
	//帧更新时调用
	void (*OnUpdate)(Canvas& canvas);
#pragma endregion
public:
#pragma region 画布属性-ReadOnly
	//中心点
	Vector2 Center() { return { width / 2,height / 2 }; }
	//宽度
	int Width() { return width; }
	//高度
	int Height() { return height; }
	//背景色
	COLORREF BackgroundColor() { return bgc; }
	//画布是否存活
	bool Life() { return life; }
	//每帧时间
	int FrameTime() { return frameTime; }
	//每秒帧数
	int FrameCount() { return fps; }
	//上一帧的时间，ms单位
	int DeltaTime() { return deltaTime; }
	HWND* Window()
	{
		return &window;
	}
#pragma endregion

#pragma region 构造与析构
	//传入xy长度，秒帧数，背景色
	Canvas(int xLen, int yLen, int frame = INT_MAX, COLORREF color = WHITE)
	{

		width = xLen;
		height = yLen;
		bgc = color;
		fps = frame;
		frameTime = 1000 / fps;
		deltaTime = 0;
	}
#pragma endregion

#pragma region GUI操作
	//切换环境并返回引用
	Canvas& Env(int env)
	{
		assert(env >= 0 && env < 4);
		envid = env;
		return *this;
	}
	//渲染某个已注册GUI
	void Draw(int id)
	{
		renderQueue.push(envs[envid][id]);
		eventQueue.push(envs[envid][id]);
	}
	//检查Canvas释放包含某个GUI
	bool ContainsKey(int id)
	{
		return envs[envid].find(id) != envs[envid].end();
	}
	//注册GUI到当前环境
	void Register(int id, GUI* gui)
	{
		envs[envid].insert({ id,gui });
	}
	//移除某个GUI的注册，但不释放内存
	void RemoveGUI(int id)
	{
		envs[envid].erase(id);
	}
	//从Canvas获取某个GUI并返回指针，如果不存在则报错
	GUI* GetGUI(int id)
	{
		return envs[envid][id];
	}
	//释放某个id的GUI所占用的内存并解除注册
	void ReleaseGUI(int id)
	{
		delete envs[envid][id];
		RemoveGUI(id);
	}
	//把GUI引用收集到Canvas内，作为一个Collection统一管理，失去单一管理权限
	void Collect(GUI* gui1, GUI* gui2 = nullptr, GUI* gui3 = nullptr, GUI* gui4 = nullptr)
	{
		cenvs[envid].push_back(gui1);
		if (gui2 != nullptr) cenvs[envid].push_back(gui2);
		else if (gui3 != nullptr) cenvs[envid].push_back(gui3);
		else if (gui4 != nullptr) cenvs[envid].push_back(gui4);
	}
#pragma endregion

#pragma region 批量释放资源

	//释放GUI所有的内存，并清除注册
	void ReleaseAllGUIS()
	{
		for (auto& i : envs[envid])
		{
			delete i.second;
		}
		envs[envid].clear();
	}
	//只从Canvas内移除注册
	void RemoveAllGUIS()
	{
		envs[envid].clear();
	}
	//只在Canvas内移除Collection的注册
	void RemoveAllCollections()
	{
		cenvs[envid].clear();
	}
	//释放所有Collection的内存，并清除注册
	void ReleaseAllCollections()
	{
		for (auto& i : cenvs[envid])
		{
			delete i;
		}
		cenvs[envid].clear();
	}
	//移除所有Collection和GUI的注册
	void RemoveAll()
	{
		RemoveAllGUIS();
		RemoveAllCollections();
	}
	//释放所有Coloection和GUI的内存
	void ReleaseAll()
	{
		ReleaseAllGUIS();
		ReleaseAllCollections();
	}

#pragma endregion

#pragma region 生命周期干涉操作
	//画布初始化
	void Show(void start(Canvas& canvas), void update(Canvas& canvas), void ongui(Canvas& canvas))
	{
		//生命周期
		OnStart = start;
		OnUpdate = update;
		OnGUI = ongui;

		//设置背景颜色
		window = initgraph(width, height);
		setbkcolor(bgc);
		cleardevice();

		//生命周期：Start
		OnStart(*this);

		while (life && IsWindow(window))
		{
			//帧开始计时
			frameStart = GetTickCount();
			//渲染与消息队列（将生命周期GUI和持久化渲染GUI添加到渲染队列和消息队列）
			OnGUI(*this);

			//清空画布开始渲染
			BeginBatchDraw();
			cleardevice();
			RenderAll();
			EndBatchDraw();

			//生命周期--消息分发
			if (peekmessage(&message))
			{
				BroadcastAll(&message);
				message = {};
			}
			else while (!eventQueue.empty()) eventQueue.pop();


			//生命周期--帧更新
			OnUpdate(*this);

			//帧数控制
			deltaTime = GetTickCount() - frameStart;
			if (frameTime - deltaTime > 0)
			{
				Sleep(frameTime - deltaTime);
			}
			//打印帧信息
			//cout << "Frame:" << count++ << "  " << "FrameTime:" << frameTime << "  " << "DeltaTime:" << deltaTime<<"  SleepTime:"<< frameTime - deltaTime << endl;
		}
		closegraph();
	}
	//关闭画布
	void Close()
	{
		life = false;
	}
#pragma endregion
};
#pragma endregion
#pragma region 矩形操作

//通过四个边界坐标创建矩形
Rect createRectbyPoint(int left, int top, int right, int bottom)
{
	Rect rect;
	rect.center = { (right + left) / 2,(top + bottom) / 2 };
	rect.origin = { left,top };
	rect.end = { right,bottom };
	rect.width = right - left;
	rect.height = bottom - top;
	return rect;
}
//通过中心点和长宽创建矩形
Rect createRectbyCenter(int x, int y, int width, int height)
{
	Rect rect;
	rect.center = { x,y };
	rect.origin = { x - width / 2,y - height / 2 };
	rect.end = { x + width / 2,y + height / 2 };
	rect.width = width;
	rect.height = height;
	return rect;
}
//通过中心点和长宽创建矩形创建矩形
Rect createRectbyCenter(Vector2 center, int width, int height)
{
	Rect rect;
	rect.center = center;
	rect.origin = { center.x - width / 2,center.y - height / 2 };
	rect.end = { center.x + width / 2,center.y + height / 2 };
	rect.width = width;
	rect.height = height;
	return rect;
}
//移动矩形位置到新的矩形并返回，不改变原矩形
Rect moveRect(Vector2 offset, Rect rect)
{
	return createRectbyPoint(rect.origin.x + offset.x, rect.origin.y + offset.y, rect.end.x + offset.x, rect.end.y + offset.y);
}
//判断某位置是否在矩形内
bool inRect(int x, int y, const Rect* rect)
{
	if (x >= rect->origin.x && x <= rect->end.x && y >= rect->origin.y && y <= rect->end.y)
	{
		return true;
	}
	return false;
}
#pragma endregion

#pragma region GUI组件

//可以缩放的方形边框
class LineBox :public GUI
{
	Rect rect;  //原矩形大小，也是响应事件的矩形大小
	Rect temp;  //缩放后矩形大小
	COLORREF color;  //线框颜色
	bool state = false;  //当前是否缩放

public:

	void OnGUI() override
	{
		setlinecolor(color);  //设置线颜色
		if (!state)   
			rectangle(rect.origin.x, rect.origin.y, rect.end.x, rect.end.y);  //绘制原线框
		else rectangle(temp.origin.x, temp.origin.y, temp.end.x, temp.end.y);  //绘制放大后线框

	}
	void OnEvent(ExMessage* message) override
	{
		if (inRect(message->x, message->y, &rect))  //如果鼠标在原线框范围内，则修改状态
		{
			state = true;
		}
		else
		{
			state = false;
		}
	}
	//形参：原线框矩形，线框颜色，缩放长度
	LineBox(Rect rct, COLORREF c, int s = 10)
	{
		rect = rct;
		color = c;
		temp = createRectbyCenter(rect.center, rect.width + s, rect.height + s);
	}
};
//允许加载纯色和图片的矩形
class Image : public GUI
{
	IMAGE img;  //图片数据
	bool pureColor = false;   //是否为纯色图片
	COLORREF color;  //如果是纯色图片，颜色是什么
public:
	Rect rect;  //图片的矩形
	void OnGUI() override
	{
		if (!pureColor)putimage(rect.origin.x, rect.origin.y, &img);  //如果不是纯色，则渲染到屏幕的是图片
		else  //如果是纯色
		{
			setfillcolor(color);  //设置填充颜色
			fillrectangle(rect.origin.x, rect.origin.y, rect.end.x, rect.end.y);  //填充矩形并渲染到屏幕
		}
	}
	void OnEvent(ExMessage* message) override{}
	Image(Rect rct, string path)
	{
		color = NULL;
		pureColor = false;
		rect = rct;
		loadimage(&img, path.c_str(), rect.width, rect.height);
	}
	Image(Rect rct,int res,const char* resType = "IMAGE")
	{

		pureColor = true;
		rect = rct;
		
		loadimage(&img, resType, MAKEINTRESOURCE(res), rect.width, rect.height);
	}
	Image(Rect rct, COLORREF c)
	{
		img = NULL;
		pureColor = true;
		rect = rct;
		color = c;
	}
};
//显示文字的box
class Text : public GUI
{
	string style;  //字体名称
	COLORREF color;  //字体颜色
	RECT rr;   //系统使用的矩形
public:
	Rect rect;  //框架使用的矩形
	string text;  //文本框的内容
	bool center = true;  //是否水平居中显示
	void OnGUI()  override
	{
		setbkmode(TRANSPARENT);  //文本背景透明
		settextcolor(color);  //设置文本颜色
		settextstyle(16, 0, style.c_str());  //设置字体
		if (center)drawtext(text.c_str(), &rr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);  //渲染文字
		else drawtext(text.c_str(), &rr, DT_VCENTER | DT_SINGLELINE);
	}
	void OnEvent(ExMessage* message)override {}
	void SetText(string str, string sty = "宋体", COLORREF col = -1)
	{
		if (col != -1)  color = col;
		if (sty != "宋体") style = sty;
		text = str;
	}
	Text(string txt, Rect rct, string st = "宋体", const COLORREF c = BLACK, bool hcenter = true)
	{
		color = c;
		text = txt;
		rect = rct;
		style = st;
		center = hcenter;
		rr = { rect.origin.x,rect.origin.y,rect.end.x,rect.end.y };
	}
	Text(string txt, Rect rct, bool hcenter = true)
	{
		color = BLACK;
		text = txt;
		rect = rct;
		style = "宋体";
		center = hcenter;
		rr = { rect.origin.x,rect.origin.y,rect.end.x,rect.end.y };
	}
};
//能点击的按钮
class Button : public GUI
{
	Image* a = nullptr;  //按钮的图片指针
	Text* t = nullptr;  //按钮文字指针
	LineBox* box = nullptr;  //按钮线框指针
	vector<function<void(void)>> onclicks;  //消息列表

	void OnGUI() override
	{
		//分别调用子对象的渲染函数
		if (a != nullptr)a->OnGUI();  
		if (t != nullptr)t->OnGUI();
		if (box != nullptr)box->OnGUI();
	}

	void OnEvent(ExMessage* message) override
	{
		//处理子对象消息
		if (a != nullptr)a->OnEvent(message);
		if (t != nullptr)t->OnEvent(message);
		if (box != nullptr) box->OnEvent(message);
		
		//处理按下消息
		if (a!=NULL&&message->message==WM_LBUTTONDOWN&& inRect(message->x, message->y, &(a->rect)))
		{	
			for (auto& i : onclicks) i();
		}
	}

public:
	//添加监听事件
	void AddListener(function<void(void)> onclick)
	{
		onclicks.push_back(onclick);
	}
	//移除监听事件，效率较低
	void RemoveListener(function<void(void)> onclick)
	{
		for (auto i = onclicks.begin(); i != onclicks.end(); i++)
		{
			bool eq = *(onclick.target<void(*)(void)>()) == *(i->target<void(*)(void)>());
			if (eq)
			{
				onclicks.erase(i);
			}
		}
	}
	//根据id移除监听
	void RemoveListener(int id)
	{
		onclicks.erase(onclicks.begin() + id);
	}
	//移除全部监听
	void RemoveAllListener()
	{
		onclicks.clear();
	}
	//形参：图片指针，文本指针，线框指针，点击范围以图片指针范围为准
	Button(Image* img, Text* txt = nullptr, LineBox* edge = nullptr)
	{
		a = img;
		t = txt;
		box = edge;
	}
};
//可自定义的网格
class Gird : public GUI
{
private:
	Text* (**units);  //网格的文本指针 二维数组
	Rect rect;  //矩形范围
	COLORREF color;  //网格线颜色
	COLORREF fontcolor;  //网格文本颜色
	string dstyle;  //网格字体名称
public:


	int xCount;  //列数
	int yCount;  //行数
	Rect unitRect;  //单元格大小，左上角为0，0
#pragma region 生命
	//初始化网格文本
	void initUnits()
	{
		units = new Text * *[yCount];
		for (int i = 0; i < yCount; i++)
			units[i] = new Text * [xCount] {0};

		for (int y = 0; y < yCount; y++)for (int x = 0; x < xCount; x++)
			units[y][x] = new Text("", moveRect({ rect.origin.x + x * unitRect.width,rect.origin.y + unitRect.height * y }, unitRect), dstyle, fontcolor);

	}
	//rct指总网格大小，xy是网格数量
	Gird(Rect rct, int xC, int yC, COLORREF c = BLACK, string style = "宋体", COLORREF fontc = BLACK)
	{
		rect = rct;
		xCount = xC;
		yCount = yC;
		dstyle = style;
		unitRect = createRectbyPoint(0, 0, rct.width / xC, rct.height / yC);
		color = c;
		fontcolor = fontc;
		initUnits();
	}
	//leftTop为左上角位置，xC,yC指网格数量，width，height指网格宽高
	Gird(Vector2 leftTop, int xC, int yC, int width, int height, COLORREF c = BLACK, string style = "宋体", COLORREF fontc = BLACK)
	{
		xCount = xC;
		yCount = yC;
		rect = createRectbyPoint(leftTop.x, leftTop.y, xC * width, yC * height);
		dstyle = style;
		unitRect = createRectbyPoint(0, 0, width, height);
		color = c;
		fontcolor = fontc;
		initUnits();
	}
	//leftTop为左上角位置，xC,yC指网格数量，width，height指网格宽高
	Gird(const int left, int top, int xC, int yC, int width, int height, COLORREF c = BLACK, string style = "宋体", COLORREF fontc = BLACK)
	{
		xCount = xC;
		yCount = yC;
		rect = createRectbyPoint(left, top, xC * width, yC * height);
		dstyle = style;
		unitRect = createRectbyPoint(0, 0, width, height);
		color = c;
		fontcolor = fontc;
		initUnits();
	}
	~Gird()
	{
		for (int x = 0; x < xCount; x++)for (int y = 0; y < yCount; y++) delete units[x][y];
		for (int i = 0; i < xCount; i++)
		{
			delete(units[i]);
		}
		delete units;
	}
	//仅在GUI渲染时回调
	void OnGUI() override
	{
		setlinecolor(color);
		//绘制网格线
		for (int x = 0; x < xCount + 1; x++)
		{
			line(rect.origin.x + x * unitRect.width, rect.origin.y, rect.origin.x + x * unitRect.width, rect.end.y);
		}
		for (int y = 0; y < yCount + 1; y++)
		{
			line(rect.origin.x, rect.origin.y + y * unitRect.height, rect.end.x, rect.origin.y + y * unitRect.height);
		}
		//绘制文字
		for (int y = 0; y < yCount; y++)
		{
			for (int x = 0; x < xCount; x++)
			{
				units[y][x]->OnGUI();
			}
		}
	}
	//仅在处理事件时回调
	void OnEvent(ExMessage* message) override
	{
		//处理子对象消息
		for (int y = 0; y < yCount; y++)
		{
			for (int x = 0; x < xCount; x++)
			{
				units[y][x]->OnEvent(message);
			}
		}

	}
#pragma endregion

#pragma region 单元格操作

	//设置单元格文字，返回Text指针，需要自行处理
	void SetUnit(int x, int y, string text, const COLORREF color = BLACK)
	{
		assert(x < yCount&& y < xCount);
		units[x][y]->SetText(text);
	}
#pragma endregion

};
#pragma endregion