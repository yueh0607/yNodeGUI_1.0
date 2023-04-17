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
	int lev = 0;//�ڵ�㼶�����ڵ�Ϊ0
public:
	Node* parent = nullptr; //���ڵ�ָ�룬���ڵ�Ϊnullptr
	void (*func)(Menu& munu, Canvas& canvas) = nullptr; //�ڵ㹦�ܺ�����ѭ��ִ��
	void (*onceFunc)(Menu& munu) = nullptr; //�ڵ㹦�ܸ���������ִֻ��һ��
	string tag; //�ڵ���
	vector<Node*> childs; //�ӽڵ�ָ���
	bool funcNode = false;  //�Ƿ�Ϊ��һ���ܽڵ�

	//��ǰ�ڵ�㼶
	int Level() { return lev; }

	
#pragma region �ڵ㹹��
	//����/����
	Node()
	{
		tag = "RootNode";
		lev = 0;
	}
	//�βΣ����ڵ�ָ�룬�ڵ������ڵ㹦�ܺ���(ѭ��)���Ƿ�Ϊ��һ���ܽڵ㣬��������(ִֻ��һ��)
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
	//DFS����
	~Node()
	{
		if (childs.size() == 0) return;
		for (auto i : childs) delete i;
	}
	
#pragma endregion
#pragma region �ڵ��л�
	//������ת����һ�ڵ�
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
	//���Է�����һ�ڵ�
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

//�ڵ�˵��࣬����������߼���ֻ����ά���ڵ���
class Menu :public Instance
{
	Node* root;  //���ڵ�

	Menu() = default;
public:
	//��ǰ�ڵ�,��Ȼ���Ա��޸��ˣ����ǲ�����....��
	Node* current;
	Canvas* canvas;//����

	//����/����
	Menu(Node* first, Canvas* cv)
	{
		root = first;  //���ڵ�
		canvas = cv;  //����
		current = first;  //��ǰ�ڵ�

	}
	~Menu()
	{
		//�ڸ��ڵ�DFS��������N����
		delete root;
	}
#pragma region �ڵ���ת����

	//�ڵ㺯��������ڵ���ת�������������ת����current���䣬ÿ����ת�����ýڵ���¸�������
	void ToRoot()
	{
		current = root;
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}
	//�ڵ㺯��������ڵ���ת�������������ת����current���䣬ÿ����ת�����ýڵ���¸�������
	void Last()
	{
		current = current->Last();
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}
	//�ڵ㺯��������ڵ���ת�������������ת����current���䣬ÿ����ת�����ýڵ���¸�������
	void Next(int idx)
	{
		current = current->Next(idx);
		if (current->onceFunc != nullptr) current->onceFunc(*this);
	}

	/// <summary>
	/// ע����ڵ���ɵĲ˵�
	/// </summary>
	/// <param name="root">���ڵ�</param>
	/// <param name="xOffest">xƫ�ƣ��ڻ�������x�Ļ�����ƫ��</param>
	/// <param name="yOffest">yƫ�ƣ��ڻ�����˵Ļ���������ƫ��</param>
	/// <param name="yStep">��ť���ɵĲ���</param>
	/// <param name="width">��ť���</param>
	/// <param name="height">��ť�߶�</param>
	/// <param name="edgeWidth">�߿��Ե���</param>
	/// <param name="buttonColor">��ť������ɫ</param>
	/// <param name="fontColor">������ɫ</param>
	/// <param name="lineColor">�߿���ɫ</param>
	/// <param name="fontName">��������</param>
	void RegisterMenuByRootNode( int xOffest, int yOffest, int yStep, int width, int height,int edgeWidth, COLORREF buttonColor, COLORREF fontColor, COLORREF lineColor, string fontName)
	{
		//N�����ڵ��α�����ע�ᰴť
		queue<Node*> que;
		que.push(root);
		while (!que.empty())
		{
			Node* node = que.front();
			que.pop();
			for (int i = 0; i < node->childs.size(); i++)
			{
				//����Buttonλ�úͷ�Χ
				Vector2 center = { canvas->Center().x+xOffest ,yStep * i + yOffest };
				Rect rect = createRectbyCenter(center, width, height);
				//����GUI���
				Image* background = new Image(rect, buttonColor);
				Text* text = new Text(node->childs[i]->tag.c_str(), rect, fontName, fontColor);
				LineBox* edge = new LineBox(rect, lineColor, edgeWidth);
				Button* btn = new Button(background, text,edge);
				//��Ӱ�ť�����ص�
				btn->AddListener([i,this]() {Next(i + 1); });
				//ע��
				canvas->Env(0).Register(node->childs[i]->InstanceId(), btn);
				que.push(node->childs[i]);
			}
		}
	}
#pragma endregion
};
//�����飬����һ���Զ�ȡ����̨����
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
					if (!nowarning)MessageBox(NULL, ("�����ʽ����"), ("�����������벢���³���"), MB_SYSTEMMODAL | MB_ICONSTOP);
					return false;
				}

			}
			coord.Y++;
		}
		return true;
	}
};

