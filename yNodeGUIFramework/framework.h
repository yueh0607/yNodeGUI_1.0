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

#pragma region �����ṹ
//��ά����
typedef struct
{
	int x;
	int y;
	
} Vector2;
//���Σ��������Ͻǣ����½ǣ������ȷ�����εĻ���Ҫ��
typedef struct
{
	Vector2 origin;
	Vector2 end;
	Vector2 center;
	int width;
	int height;
} Rect;
//ʵ�����࣬�����������Ҫʵ�������඼��Ҫ�̳б��࣬����ΨһID
class Instance
{
private:
	static int instanceId; //ȫ��ʵ��idλ��
	static set<int> ids;  //�ѱ�ռ�õ�ʵ��id
	int instance_id;  //��ǰʵ��id
	//����һ��������ʵ������ͻ��ID
	static int AssignInstanceID()
	{
		do { if (instanceId == INT_MIN)instanceId == INT_MAX; instanceId--; } while (ExistID(instanceId));
		RegisterId(instanceId);
		return instanceId;
	}
	//����Ƿ����ID
	static bool ExistID(int id)
	{
		return ids.find(id) != ids.end();
	}
	//ע��һ��ID
	static void RegisterId(int id)
	{
		ids.insert(id);
	}
public:
	//ʵ������ʱ����ID
	Instance()
	{
		instance_id = AssignInstanceID();  //����һ���µ�ʵ��ID
	}  
	//ʵ�����ٻ���ID
	~Instance()
	{
		ids.erase(instance_id);
	}
	//��ȡ��ǰʵ����ID������
	int InstanceId()
	{
		return instance_id;
	}
	
};
int Instance::instanceId = INT_MAX;
set<int> Instance::ids;

//GUI�ӿڣ�����GUI����̳иýӿ� 
class GUI :public Instance
{
public:
	//����GUI��Ⱦ
	virtual void OnGUI() = 0;
	//������Ϣ�������¼���Ӧ
	virtual void OnEvent(ExMessage* message) = 0;
};

//�����࣬���𻭲�����ά������ʵ�־����߼�
class Canvas :public Instance
{
private:
#pragma region ��Ϣ�ֶ�
	int width;//���
	int height;//�߶�
	int fps = 60;//֡��
	int frameStart = 0;//��ǰ֡��ʼʱ��
	int frameTime;//ÿ֡ʱ��
	bool life = true;//�Ƿ���
	int deltaTime;//��һ֡���ĵ�ʱ�� ms
	COLORREF bgc;//����ɫ
	ExMessage message;//��Ϣ��ʱ�ڴ�
	HWND window;//���ھ��
#pragma endregion
#pragma region ���������
	//GUIע�ỷ��
	map<int, GUI*> gui0;
	map<int, GUI*> gui1;
	map<int, GUI*> gui2;
	map<int, GUI*> gui3;
	map<int, map<int, GUI*>> envs{ { 0,gui0 }, {1,gui1}, {2,gui2},{3,gui3} };

	//GUI��ʱ���ջ���
	vector<GUI*> collection0;
	vector<GUI*> collection1;
	vector<GUI*> collection2;
	vector<GUI*> collection3;
	map<int, vector<GUI*>> cenvs{ {0,collection0},{1,collection1},{2,collection2},{3,collection3} };

	//��Ⱦ����/��Ϣ����
	queue <GUI* > renderQueue;
	queue<GUI*> eventQueue;

	//��ǰ����ID
	int envid = 0;
#pragma endregion
#pragma region ���л�GUI����
	//��ȾGUI����ն���
	void RenderAll()
	{
		while (!renderQueue.empty())
		{
			renderQueue.front()->OnGUI();
			renderQueue.pop();
		}
	}
	//����GUI��Ϣ����ն���
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
#pragma region ��������
	//��ʼʱ����
	void (*OnStart)(Canvas& canvas);
	//GUI��Ⱦʱ����
	void (*OnGUI)(Canvas& canvas);
	//֡����ʱ����
	void (*OnUpdate)(Canvas& canvas);
#pragma endregion
public:
#pragma region ��������-ReadOnly
	//���ĵ�
	Vector2 Center() { return { width / 2,height / 2 }; }
	//���
	int Width() { return width; }
	//�߶�
	int Height() { return height; }
	//����ɫ
	COLORREF BackgroundColor() { return bgc; }
	//�����Ƿ���
	bool Life() { return life; }
	//ÿ֡ʱ��
	int FrameTime() { return frameTime; }
	//ÿ��֡��
	int FrameCount() { return fps; }
	//��һ֡��ʱ�䣬ms��λ
	int DeltaTime() { return deltaTime; }
	HWND* Window()
	{
		return &window;
	}
#pragma endregion

#pragma region ����������
	//����xy���ȣ���֡��������ɫ
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

#pragma region GUI����
	//�л���������������
	Canvas& Env(int env)
	{
		assert(env >= 0 && env < 4);
		envid = env;
		return *this;
	}
	//��Ⱦĳ����ע��GUI
	void Draw(int id)
	{
		renderQueue.push(envs[envid][id]);
		eventQueue.push(envs[envid][id]);
	}
	//���Canvas�ͷŰ���ĳ��GUI
	bool ContainsKey(int id)
	{
		return envs[envid].find(id) != envs[envid].end();
	}
	//ע��GUI����ǰ����
	void Register(int id, GUI* gui)
	{
		envs[envid].insert({ id,gui });
	}
	//�Ƴ�ĳ��GUI��ע�ᣬ�����ͷ��ڴ�
	void RemoveGUI(int id)
	{
		envs[envid].erase(id);
	}
	//��Canvas��ȡĳ��GUI������ָ�룬����������򱨴�
	GUI* GetGUI(int id)
	{
		return envs[envid][id];
	}
	//�ͷ�ĳ��id��GUI��ռ�õ��ڴ沢���ע��
	void ReleaseGUI(int id)
	{
		delete envs[envid][id];
		RemoveGUI(id);
	}
	//��GUI�����ռ���Canvas�ڣ���Ϊһ��Collectionͳһ����ʧȥ��һ����Ȩ��
	void Collect(GUI* gui1, GUI* gui2 = nullptr, GUI* gui3 = nullptr, GUI* gui4 = nullptr)
	{
		cenvs[envid].push_back(gui1);
		if (gui2 != nullptr) cenvs[envid].push_back(gui2);
		else if (gui3 != nullptr) cenvs[envid].push_back(gui3);
		else if (gui4 != nullptr) cenvs[envid].push_back(gui4);
	}
#pragma endregion

#pragma region �����ͷ���Դ

	//�ͷ�GUI���е��ڴ棬�����ע��
	void ReleaseAllGUIS()
	{
		for (auto& i : envs[envid])
		{
			delete i.second;
		}
		envs[envid].clear();
	}
	//ֻ��Canvas���Ƴ�ע��
	void RemoveAllGUIS()
	{
		envs[envid].clear();
	}
	//ֻ��Canvas���Ƴ�Collection��ע��
	void RemoveAllCollections()
	{
		cenvs[envid].clear();
	}
	//�ͷ�����Collection���ڴ棬�����ע��
	void ReleaseAllCollections()
	{
		for (auto& i : cenvs[envid])
		{
			delete i;
		}
		cenvs[envid].clear();
	}
	//�Ƴ�����Collection��GUI��ע��
	void RemoveAll()
	{
		RemoveAllGUIS();
		RemoveAllCollections();
	}
	//�ͷ�����Coloection��GUI���ڴ�
	void ReleaseAll()
	{
		ReleaseAllGUIS();
		ReleaseAllCollections();
	}

#pragma endregion

#pragma region �������ڸ������
	//������ʼ��
	void Show(void start(Canvas& canvas), void update(Canvas& canvas), void ongui(Canvas& canvas),bool showConsole=false)
	{
		//��������
		OnStart = start;
		OnUpdate = update;
		OnGUI = ongui;

		//���ñ�����ɫ
		window = showConsole?initgraph(width, height, EW_SHOWCONSOLE): initgraph(width, height);
		setbkcolor(bgc);
		cleardevice();

		//�������ڣ�Start
		OnStart(*this);

		while (life && IsWindow(window))
		{
			//֡��ʼ��ʱ
			frameStart = GetTickCount();
			//��Ⱦ����Ϣ���У�����������GUI�ͳ־û���ȾGUI��ӵ���Ⱦ���к���Ϣ���У�
			OnGUI(*this);

			//��ջ�����ʼ��Ⱦ
			BeginBatchDraw();
			cleardevice();
			RenderAll();
			EndBatchDraw();

			//��������--��Ϣ�ַ�
			if (peekmessage(&message))
			{
				BroadcastAll(&message);
				message = {};
			}
			else while (!eventQueue.empty()) eventQueue.pop();


			//��������--֡����
			OnUpdate(*this);

			//֡������
			deltaTime = GetTickCount() - frameStart;
			if (frameTime - deltaTime > 0)
			{
				Sleep(frameTime - deltaTime);
			}
			//��ӡ֡��Ϣ
			//cout << "Frame:" << count++ << "  " << "FrameTime:" << frameTime << "  " << "DeltaTime:" << deltaTime<<"  SleepTime:"<< frameTime - deltaTime << endl;
		}
		closegraph();
	}
	//�رջ���
	void Close()
	{
		life = false;
	}
#pragma endregion
};
#pragma endregion

#pragma region ���β���

//ͨ���ĸ��߽����괴������
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
//ͨ�����ĵ�ͳ���������
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
//ͨ�����ĵ�ͳ��������δ�������
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
//�ƶ�����λ�õ��µľ��β����أ����ı�ԭ����
Rect moveRect(Vector2 offset, Rect rect)
{
	return createRectbyPoint(rect.origin.x + offset.x, rect.origin.y + offset.y, rect.end.x + offset.x, rect.end.y + offset.y);
}
//�ж�ĳλ���Ƿ��ھ�����
bool inRect(int x, int y, const Rect* rect)
{
	if (x >= rect->origin.x && x <= rect->end.x && y >= rect->origin.y && y <= rect->end.y)
	{
		return true;
	}
	return false;
}
#pragma endregion

#pragma region GUI���

//�������ŵķ��α߿�
class LineBox :public GUI
{
	Rect rect;  //ԭ���δ�С��Ҳ����Ӧ�¼��ľ��δ�С
	Rect temp;  //���ź���δ�С
	COLORREF color;  //�߿���ɫ
	bool state = false;  //��ǰ�Ƿ�����

public:

	void OnGUI() override
	{
		setlinecolor(color);  //��������ɫ
		if (!state)   
			rectangle(rect.origin.x, rect.origin.y, rect.end.x, rect.end.y);  //����ԭ�߿�
		else rectangle(temp.origin.x, temp.origin.y, temp.end.x, temp.end.y);  //���ƷŴ���߿�

	}
	void OnEvent(ExMessage* message) override
	{
		if (inRect(message->x, message->y, &rect))  //��������ԭ�߿�Χ�ڣ����޸�״̬
		{
			state = true;
		}
		else
		{
			state = false;
		}
	}
	//�βΣ�ԭ�߿���Σ��߿���ɫ�����ų���
	LineBox(Rect rct, COLORREF c, int s = 10)
	{
		rect = rct;
		color = c;
		temp = createRectbyCenter(rect.center, rect.width + s, rect.height + s);
	}
};
//������ش�ɫ��ͼƬ�ľ���
class Image : public GUI
{
	IMAGE img;  //ͼƬ����
	bool pureColor = false;   //�Ƿ�Ϊ��ɫͼƬ
	COLORREF color;  //����Ǵ�ɫͼƬ����ɫ��ʲô
public:
	Rect rect;  //ͼƬ�ľ���
	void OnGUI() override
	{
		if (!pureColor)putimage(rect.origin.x, rect.origin.y, &img);  //������Ǵ�ɫ������Ⱦ����Ļ����ͼƬ
		else  //����Ǵ�ɫ
		{
			setfillcolor(color);  //���������ɫ
			fillrectangle(rect.origin.x, rect.origin.y, rect.end.x, rect.end.y);  //�����β���Ⱦ����Ļ
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
	Image(Rect rct, COLORREF c)
	{
		img = NULL;
		pureColor = true;
		rect = rct;
		color = c;
	}
};
//��ʾ���ֵ�box
class Text : public GUI
{
	string style;  //��������
	COLORREF color;  //������ɫ
	RECT rr;   //ϵͳʹ�õľ���
public:
	Rect rect;  //���ʹ�õľ���
	string text;  //�ı��������
	bool center = true;  //�Ƿ�ˮƽ������ʾ
	void OnGUI()  override
	{
		setbkmode(TRANSPARENT);  //�ı�����͸��
		settextcolor(color);  //�����ı���ɫ
		settextstyle(16, 0, style.c_str());  //��������
		if (center)drawtext(text.c_str(), &rr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);  //��Ⱦ����
		else drawtext(text.c_str(), &rr, DT_VCENTER | DT_SINGLELINE);
	}
	void OnEvent(ExMessage* message)override {}
	void SetText(string str, string sty = "����", COLORREF col = -1)
	{
		if (col != -1)  color = col;
		if (sty != "����") style = sty;
		text = str;
	}
	Text(string txt, Rect rct, string st = "����", const COLORREF c = BLACK, bool hcenter = true)
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
		style = "����";
		center = hcenter;
		rr = { rect.origin.x,rect.origin.y,rect.end.x,rect.end.y };
	}
};
//�ܵ���İ�ť
class Button : public GUI
{
	Image* a = nullptr;  //��ť��ͼƬָ��
	Text* t = nullptr;  //��ť����ָ��
	LineBox* box = nullptr;  //��ť�߿�ָ��
	vector<function<void(void)>> onclicks;  //��Ϣ�б�

	void OnGUI() override
	{
		//�ֱ�����Ӷ������Ⱦ����
		if (a != nullptr)a->OnGUI();  
		if (t != nullptr)t->OnGUI();
		if (box != nullptr)box->OnGUI();
	}
	void OnEvent(ExMessage* message) override
	{
		//�����Ӷ�����Ϣ
		if (a != nullptr)a->OnEvent(message);
		if (t != nullptr)t->OnEvent(message);
		if (box != nullptr) box->OnEvent(message);
		
		//��������Ϣ
		if (a!=NULL&&message->message==WM_LBUTTONDOWN&& inRect(message->x, message->y, &(a->rect)))
		{	
			for (auto& i : onclicks) i();
		}
	}

public:
	//��Ӽ����¼�
	void AddListener(function<void(void)> onclick)
	{
		onclicks.push_back(onclick);
	}
	//�Ƴ������¼���Ч�ʽϵ�
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
	//����id�Ƴ�����
	void RemoveListener(int id)
	{
		onclicks.erase(onclicks.begin() + id);
	}
	//�Ƴ�ȫ������
	void RemoveAllListener()
	{
		onclicks.clear();
	}
	//�βΣ�ͼƬָ�룬�ı�ָ�룬�߿�ָ�룬�����Χ��ͼƬָ�뷶ΧΪ׼
	Button(Image* img, Text* txt = nullptr, LineBox* edge = nullptr)
	{
		a = img;
		t = txt;
		box = edge;
	}
	Button(Rect rct,COLORREF imgColor, string txt, COLORREF fColor, COLORREF edgeColor)
	{
		Image* img = new Image(rct, imgColor);
		Text* t = new Text(txt, rct,"����",fColor, true);
		LineBox* lb = new LineBox(rct, edgeColor);
		a = img;
		t = t;
		box = lb;
	}
	~Button()
	{
		delete a;
		delete t;
		delete box;
	}
};
//���Զ��������
class Gird : public GUI
{
private:
	Text* (**units);  //������ı�ָ�� ��ά����
	Rect rect;  //���η�Χ
	COLORREF color;  //��������ɫ
	COLORREF fontcolor;  //�����ı���ɫ
	string dstyle;  //������������
public:


	int xCount;  //����
	int yCount;  //����
	Rect unitRect;  //��Ԫ���С�����Ͻ�Ϊ0��0
#pragma region ����
	//��ʼ�������ı�
	void initUnits()
	{
		units = new Text * *[yCount];
		for (int i = 0; i < yCount; i++)
			units[i] = new Text * [xCount] {0};

		for (int y = 0; y < yCount; y++)for (int x = 0; x < xCount; x++)
			units[y][x] = new Text("", moveRect({ rect.origin.x + x * unitRect.width,rect.origin.y + unitRect.height * y }, unitRect), dstyle, fontcolor);

	}
	//rctָ�������С��xy����������
	Gird(Rect rct, int xC, int yC, COLORREF c = BLACK, string style = "����", COLORREF fontc = BLACK)
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
	//leftTopΪ���Ͻ�λ�ã�xC,yCָ����������width��heightָ������
	Gird(Vector2 leftTop, int xC, int yC, int width, int height, COLORREF c = BLACK, string style = "����", COLORREF fontc = BLACK)
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
	//leftTopΪ���Ͻ�λ�ã�xC,yCָ����������width��heightָ������
	Gird(const int left, int top, int xC, int yC, int width, int height, COLORREF c = BLACK, string style = "����", COLORREF fontc = BLACK)
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
	//����GUI��Ⱦʱ�ص�
	void OnGUI() override
	{
		setlinecolor(color);
		//����������
		for (int x = 0; x < xCount + 1; x++)
		{
			line(rect.origin.x + x * unitRect.width, rect.origin.y, rect.origin.x + x * unitRect.width, rect.end.y);
		}
		for (int y = 0; y < yCount + 1; y++)
		{
			line(rect.origin.x, rect.origin.y + y * unitRect.height, rect.end.x, rect.origin.y + y * unitRect.height);
		}
		//��������
		for (int y = 0; y < yCount; y++)
		{
			for (int x = 0; x < xCount; x++)
			{
				units[y][x]->OnGUI();
			}
		}
	}
	//���ڴ����¼�ʱ�ص�
	void OnEvent(ExMessage* message) override
	{
		//�����Ӷ�����Ϣ
		for (int y = 0; y < yCount; y++)
		{
			for (int x = 0; x < xCount; x++)
			{
				units[y][x]->OnEvent(message);
			}
		}

	}
#pragma endregion

#pragma region ��Ԫ�����

	//���õ�Ԫ�����֣�����Textָ�룬��Ҫ���д���
	void SetUnit(int x, int y, string text, const COLORREF color = BLACK)
	{
		assert(x < yCount&& y < xCount);
		units[x][y]->SetText(text);
	}
#pragma endregion

};
#pragma endregion