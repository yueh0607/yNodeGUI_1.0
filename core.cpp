// ReSharper disable All
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<easyx.h>
#include<fstream>
#include<graphics.h>
#include<conio.h>
#include<list>
#include<functional>
#include <string>
#include<cmath>
#include "framework.h"
#include "node_menu.h"
#include "resource.h"
using namespace std;

#pragma region Defines

#define LINESIGN "Line:"
#define ENDSIGN "End"
#define FILENAME1 "course.data"
#define FILENAME2 "select.data"
#define BGFILENAME "Resources/background.jpg"
#define BUTTONCOLOR RGB(63, 92, 57)
#define LINEBOXCOLOR GREEN
#define BUTTONFONTCOLOR WHITE
#define GIRDFONTCOLOR BLACK
#define GIRDLINECOLOR BLACK
#define FONTNAME "宋体"

#pragma endregion

#pragma region 数据结构
typedef enum
{
	Pub = 0,//公共
	Nes = 1,//必修
	Sel = 2,  //选修
	NaN = 3
}nature;
string Index(nature nat)
{
	switch (nat)
	{
	case Pub:return "公共课";
	case Sel:return "选修课";
	case Nes:return "必修课";
	default:
		return "NULL";
	}
}
typedef struct
{
	long long id;   //课程编号
	char name[50];  //课程名称
	nature nat;   //课程性质
	float class_period; //总学时
	float teaching_time; //授课学时
	float experiental_time; //实验学时
	float credit;  //学分
	char term[50];

}Course;
#pragma endregion

#pragma region 全局变量_1
//课程顺序表
vector<Course*> courses;
int page1 = 0;
//网格编号，返回键编号，上下页键编号，排序按钮编号，总学分GUI编号
int gird_id, ret_id, next_id, last_id, id_sort, credit_sort, time_sort, alls_id;;
bool inputone = true;
int bg_id;
//当前Gird对应的课程顺序表指针
vector<Course*>* pointer_courses = &courses;
#pragma endregion

#pragma region 工具函数
#pragma region 课程顺序表操作
bool exist_course(long long id)
{
	for (auto& i : courses)
	{
		if (i->id == id) return true;
	}
	return false;
}
bool exist_course(long long id, vector<Course*>* pointer)
{
	for (auto& i : *pointer)
	{
		if (i->id == id) return true;
	}
	return false;
}
void remove_course(long long id, vector<Course*>* pointer)
{
	for (int i = 0; i < pointer->size(); i++)
	{
		if ((*pointer)[i]->id == id) pointer->erase(pointer->begin() + i);
	}
}
Course* find_course(long long id)
{
	for (auto& i : courses)
	{
		if (i->id == id) return i;
	}
	return NULL;
}
#pragma endregion
#pragma region 课程排序
//交换函数
void swap(Course** a, Course** b)
{
	Course* c = *a;
	*a = *b;
	*b = c;
}
//根据ID进行冒泡排序
void sortByid(vector<Course*>& nums)
{
	for (int i = 0; i < nums.size() - 2; i++)
	{
		for (int x = 0; x < nums.size() - i - 1; x++)
		{
			if (nums[x]->id > nums[x + 1]->id) swap(&nums[x], &nums[x + 1]);
		}
	}
}
void sortByCredit(vector<Course*>& nums)
{
	for (int i = 0; i < nums.size() - 2; i++)
	{
		for (int x = 0; x < nums.size() - i - 1; x++)
		{
			if (nums[x]->credit > nums[x + 1]->credit) swap(&nums[x], &nums[x + 1]);
		}
	}
}
void sortByTime(vector<Course*>& nums)
{
	for (int i = 0; i < nums.size() - 1; i++)
	{
		int minIndex = i;
		for (int a = i + 1; a < nums.size(); a++)
		{
			minIndex = nums[a]->class_period < nums[minIndex]->class_period ? a : minIndex;
		}
		if (minIndex != 0) swap(&nums[i], &nums[minIndex]);
	}
}
#pragma endregion
#pragma region 表格显示与刷新
string ftostr(double value, int fixed)
{
	string str = to_string(value);
	return str.substr(0, str.find('.') + fixed + 1);
}
void SetRow(Gird* gird, int r, Course* course)
{
	gird->SetUnit(r, 0, to_string(course->id).c_str());
	gird->SetUnit(r, 1, course->name);
	gird->SetUnit(r, 2, Index(course->nat).c_str());
	gird->SetUnit(r, 3, ftostr(course->class_period, 1).c_str());
	gird->SetUnit(r, 4, ftostr(course->teaching_time, 1).c_str());
	gird->SetUnit(r, 5, ftostr(course->experiental_time, 1).c_str());
	gird->SetUnit(r, 6, ftostr(course->credit, 1).c_str());
	gird->SetUnit(r, 7, course->term);
}
void SetRow(Gird* gird, int r)
{
	gird->SetUnit(r, 0, " ");
	gird->SetUnit(r, 1, " ");
	gird->SetUnit(r, 2, " ");
	gird->SetUnit(r, 3, " ");
	gird->SetUnit(r, 4, " ");
	gird->SetUnit(r, 5, " ");
	gird->SetUnit(r, 6, " ");
	gird->SetUnit(r, 7, " ");
}
void showPage(Canvas& canvas, int gird_id, int page, vector<Course*>& cs)
{
	assert(page >= 0);
	Gird* gird = (Gird*)canvas.Env(1).GetGUI(gird_id);
	int start = (page) * (gird->yCount - 1);

	for (int i = 0; i < gird->yCount - 1; i++)
	{
		if (i + start < cs.size())
		{
			SetRow(gird, i + 1, cs[i + start]);
		}
		else
		{
			SetRow(gird, i + 1);
		}
	}
}
void nextPage(Canvas& canvas)
{
	int c = ((Gird*)canvas.GetGUI(gird_id))->yCount - 1;
	int allpage = pointer_courses->size() / c;
	showPage(canvas, gird_id, page1 = min(allpage, page1 + 1), *pointer_courses);
}
void lastPage(Canvas& canvas)
{
	showPage(canvas, gird_id, page1 = max(0, page1 - 1), *pointer_courses);
}
#pragma endregion
#pragma endregion

#pragma region 数据持久化
//保存课程vector
void save_courses(vector<Course*>& courses, string fileName)
{
	//创建文件输出流
	ofstream fout;
	fout.open(fileName);
	//逐个写入课程文件
	for (int i = 0; i < courses.size(); i++)
	{
		//添加行号
		fout << LINESIGN << i + 1 << " " << courses[i]->id << " " << courses[i]->name << " " << courses[i]->nat << " " << courses[i]->class_period << " " << courses[i]->teaching_time << " ";
		fout << courses[i]->experiental_time << " " << courses[i]->credit << " " << courses[i]->term << endl;
	}
	//尾标识
	fout << ENDSIGN;
	//关闭文件流，将buffer写入文件
	fout.close();
}
//读取课程vector
void read_courses(vector<Course*>& courses, string fileName)
{
	ifstream fin;
	fin.open(fileName);
	if (!fin) save_courses(courses, fileName);
	string sign;
	if (!fin.eof())
	{
		while (fin >> sign, sign != ENDSIGN)
		{
			Course* course = new Course();
			int nat;
			fin >> course->id >> course->name >> nat >> course->class_period >> course->teaching_time >> course->experiental_time >> course->credit >> course->term;
			course->nat = (nature)nat;
			courses.push_back(course);
		}
	}
	fin.close();
}

#pragma endregion

#pragma region 功能实现
#pragma region 课程信息录入
//录入课程信息
void input(Menu& menu, Canvas& canvas)
{
	//弹出提示窗口，清空控制台，输入缓冲区
	MessageBox(NULL, ("请在控制台内录入课程"), ("提示"), MB_SYSTEMMODAL | MB_ICONINFORMATION);
	rewind(stdin);
	system("cls");

	cout << "请输入录入的课程信息!注意课程性质仅为(公共课=0|必修课=1|选修课=2),请勿以空格分割字符串" << endl;
	//创建临时输入缓冲区
	Course* course = (Course*)malloc(sizeof(Course));
	//创建输入组
	InputGroup ipg{ 1 };
	int buffer = 0;
	ipg.Add("课程编号(int):", "%lld", &(course->id));
	ipg.Add("课程名称(str):", "%s", &(course->name));
	ipg.Add("课程性质(int):", "%d", &(course->nat));
	ipg.Add("总 学 时(flo):", "%f", &(course->class_period));
	ipg.Add("授课学时(flo):", "%f", &(course->teaching_time));
	ipg.Add("实验学时(flo):", "%f", &(course->experiental_time));
	ipg.Add("学    分(flo):", "%f", &(course->credit));
	ipg.Add("开课学期(str):", "%s", &(course->term));
	ipg.Add("确认1/0)(int):", "%d", &buffer);
	//打印输入组
	ipg.Print();
	//修正枚举范围
	if (course->nat > NaN || course->nat < Pub) course->nat = NaN;
	//读取输入
	if (ipg.GetInput() && buffer)
	{
		system("cls");
		if (exist_course(course->id))
		{
			cout << "已存在相同编号的课程！请检查重试！";
		}
		else
		{
			inputone = true;
			courses.push_back(course);
			save_courses(courses, FILENAME1);
			cout << "录入课程成功!请回到GUI界面继续操作";
		}
	}
	else
	{
		system("cls");
		cout << "录入失败！";
	}
	//返回上级菜单
	menu.Last();
}
//删除课程
void del_course(Menu& menu, Canvas& canvas)
{
	//清屏，清空输入缓冲区并弹出提示
	system("cls");
	rewind(stdin);
	MessageBox(NULL, ("请在控制台内输入删除的课程编号"), ("提示"), MB_SYSTEMMODAL | MB_ICONINFORMATION);
	//读取输入
	cout << "课程编号:";
	long long id;
	int x = scanf("%lld", &id);
	//搜索课程结果
	bool result = true;


	system("cls");

	//进入if则说明格式错误
	if (x == 0)
	{
		MessageBox(NULL, ("请检查您的输入并重新尝试"), ("输入格式错误"), MB_SYSTEMMODAL | MB_ICONSTOP);
		cout << "输入格式错误!";

	}
	else for (int i = 0; i < courses.size(); i++)
	{
		//删除课程并保存
		if (courses[i]->id == id)
		{
			free(*(courses.begin() + i));
			courses.erase(courses.begin() + i);
			cout << "移除课程成功!";
			result = false;
			save_courses(courses, FILENAME1);
			break;
		}
	}
	//如果搜索不到
	if (result)
	{
		cout << "找不到此课程，请检查输入重试！";
	}
	//返回上级菜单
	menu.Last();
}
//修改课程
void set_course(Menu& menu, Canvas& canvas)
{
	system("cls");
	rewind(stdin);
	MessageBox(NULL, ("请在控制台内输入要修改的课程编号"), ("提示"), MB_SYSTEMMODAL | MB_ICONINFORMATION);
	cout << "课程编号:";
	long long id;
	int x = scanf("%lld", &id);
	bool result = true;
	system("cls");
	Course* temp;
	if (x == 0)
	{
		MessageBox(NULL, ("请检查您的输入并重新尝试"), ("输入格式错误"), MB_SYSTEMMODAL | MB_ICONSTOP);
		cout << "输入格式错误!";

	}
	else for (int i = 0; i < courses.size(); i++)
	{
		if (courses[i]->id == id)
		{
			int size = courses.size();
			input(menu, canvas);
			if (size != courses.size())
			{
				free(*(courses.begin() + i));
				courses.erase(courses.begin() + i);
				save_courses(courses, FILENAME1);
			}
			result = false;
			break;
		}
	}
	if (result)
	{
		cout << "找不到此课程，请检查输入重试！";
	}
	menu.Last();
}
#pragma endregion
#pragma region 课程信息浏览
//浏览表格
void browsing(Menu& menu, Canvas& canvas)
{
	pointer_courses = &courses;
	canvas.Env(1).Draw(gird_id);
	canvas.Env(1).Draw(ret_id);
	canvas.Env(1).Draw(last_id);
	canvas.Env(1).Draw(next_id);
	canvas.Env(1).Draw(id_sort);
	canvas.Env(1).Draw(credit_sort);
	canvas.Env(1).Draw(time_sort);
}
//刷新表格，只执行一次
void fresh(Menu& menu)
{
	pointer_courses = &courses;
	page1 = 0;
	lastPage(*menu.canvas);
}
#pragma endregion
#pragma region 课程信息查询
vector<Course*> query_courses;
//读取查询条件
void query(Menu& menu, Canvas& canvas)
{
	MessageBox(NULL, ("请在控制台输入筛选条件"), ("提示"), MB_SYSTEMMODAL | MB_ICONINFORMATION);
	rewind(stdin);
	system("cls");
	query_courses.clear();
	//MessageBox(NULL,"提示", "请在控制台中录入课程信息!",MB_HELP|MB_OK);
	cout << "请输入筛选条件：" << endl;

	Course* course = (Course*)malloc(sizeof(Course));

	if (course != NULL)
	{
		InputGroup ipg{ 1 ,false };
		ipg.Add("课程编号(int):", "%lld", &(course->id));
		ipg.Add("课程名称(str):", "%s", &(course->name));
		ipg.Add("课程性质(int):", "%d", &(course->nat));
		ipg.Add("总 学 时(flo):", "%f", &(course->class_period));
		ipg.Add("授课学时(flo):", "%f", &(course->teaching_time));
		ipg.Add("实验学时(flo):", "%f", &(course->experiental_time));
		ipg.Add("学    分(flo):", "%f", &(course->credit));
		ipg.Add("开课学期(str):", "%s", &(course->term));
		ipg.Print();

		if (course->nat > NaN || course->nat < Pub) course->nat = NaN;

		ipg.GetInput();


		for (auto& i : courses)
		{
			if (i->id == course->id || !strcmp(course->name, i->name) || i->nat == course->nat || abs(i->class_period - course->class_period) < 0.1f
				|| abs(i->experiental_time - course->experiental_time) < 0.1f || abs(i->teaching_time - course->teaching_time) < 0.1f ||
				abs(course->credit - i->credit) < 0.1f || !strcmp(i->term, course->term))
			{
				query_courses.push_back(i);
			}
		}
	}
}
//展示查询界面UI
void queryshow(Menu& menu, Canvas& canvas)
{
	canvas.Env(1).Draw(gird_id);
	canvas.Env(1).Draw(ret_id);
	canvas.Env(1).Draw(last_id);
	canvas.Env(1).Draw(next_id);
}
//刷新表格
void fresh_query(Menu& menu)
{
	query(menu, *menu.canvas);
	pointer_courses = &query_courses;
	page1 = 0;
	lastPage(*menu.canvas);

}
#pragma endregion
#pragma region 学生选课
vector<Course*> select_courses;
//选课读取
void choose(Menu& menu, Canvas& canvas)
{
	rewind(stdin);
	system("cls");
	MessageBox(NULL, ("请在控制台输入选课编号,选课总学分不得低于30"), ("提示"), MB_SYSTEMMODAL | MB_ICONINFORMATION);
	long long id;
	InputGroup ipg = { 0 };
	ipg.Add("输入正数选课,负数移除");
	ipg.Add("选课编号:", "%lld", &id);
	ipg.Print();
	ipg.GetInput();
	if (exist_course(abs(id)))
	{
		if (id >= 0)
		{
			system("cls");
			if (!exist_course(id, &select_courses))
			{
				cout << "选课成功";
				select_courses.push_back(find_course(id));
				save_courses(select_courses, FILENAME2);
			}
			else
			{

				cout << "已经选择过该课程";
			}
		}
		else
		{
			system("cls");
			if (!exist_course(id, &select_courses))
			{
				remove_course(id, &select_courses);
				cout << "移除成功";
				save_courses(select_courses, FILENAME2);
			}
			else
			{
				cout << "您没有选择过该课程";
			}
		}
	}
	else
	{
		system("cls");
		cout << "不存在该课程！";
	}
	menu.Last();
}
//已选课程课查看
void view_selected(Menu& menu, Canvas& canvas)
{
	canvas.Env(1).Draw(gird_id);
	canvas.Env(1).Draw(ret_id);
	canvas.Env(1).Draw(last_id);
	canvas.Env(1).Draw(next_id);
	canvas.Env(1).Draw(alls_id);
}
//刷新表格
void fresh_selected(Menu& menu)
{
	pointer_courses = &select_courses;
	page1 = 0;
	float allscore = 0;
	for (auto& i : select_courses)
	{
		allscore += i->credit;
	}
	((Text*)menu.canvas->Env(1).GetGUI(alls_id))->SetText("总学分:" + ftostr(allscore, 1));
	lastPage(*menu.canvas);
}
#pragma endregion
#pragma region 退出与返回
//退出系统
void exit_system(Menu& menu, Canvas& canvas)
{
	save_courses(courses, FILENAME1);
	save_courses(select_courses, FILENAME2);
	//清空控制台
	system("cls");
	printf("感谢您的使用！期待下次见面");
	//关闭画布渲染
	canvas.Close();
	//回收菜单管理器以及所有根节点
	delete& menu;
	//以安全结束code退出应用程序
	exit(0);
}
//返回上层菜单
void last_menu(Menu& menu, Canvas& canvas)
{
	menu.Last();
	menu.Last();
}
#pragma endregion
#pragma endregion

#pragma region 全局变量_2
//菜单节点管理器
Menu* menu;
#pragma endregion

#pragma region 画布生命周期

//生命周期-Start | Canvas 消息 | Priority=0
void OnStart(Canvas& canvas)
{
#pragma region 窗体信息初始化
	//设置窗口标题
	SetWindowText(GetHWnd(), "课程信息管理系统");
	//读取全部课程信息
	read_courses(courses, FILENAME1);
	//读取选课信息
	read_courses(select_courses, FILENAME2);
#pragma endregion	
#pragma region Menu节点注册
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
#pragma endregion
#pragma region 节点菜单GUI注册

	menu->RegisterMenuByRootNode( 0, 100, 100, 300, 50,10, BUTTONCOLOR, BUTTONFONTCOLOR, LINEBOXCOLOR, FONTNAME);

#pragma endregion
#pragma region 背景注册
	Image* background = new Image(createRectbyPoint(0, 0, 1000, 600),BGFILENAME);
	bg_id = background->InstanceId();
	canvas.Env(1).Register(background->InstanceId(), background);
#pragma endregion
#pragma region Gird网格注册
	//网格对象初始化
	Gird* gird = new Gird(createRectbyPoint(50, 50, 900, 500), 8, 11, GIRDLINECOLOR, FONTNAME, GIRDFONTCOLOR);
	canvas.Env(1).Register(gird->InstanceId(), gird);
	gird_id = gird->InstanceId();
#pragma endregion
#pragma region 表头初始化
	gird->SetUnit(0, 0, "课程编号");
	gird->SetUnit(0, 1, "课程名称");
	gird->SetUnit(0, 2, "课程性质");
	gird->SetUnit(0, 3, "总学时");
	gird->SetUnit(0, 4, "授课学时");
	gird->SetUnit(0, 5, "实验学时");
	gird->SetUnit(0, 6, "学分");
	gird->SetUnit(0, 7, "开课学期");
#pragma endregion
#pragma region 表功能按钮注册
	//返回按钮注册
	Rect rct = createRectbyCenter({ 100,550 }, 50, 20);
	Image* bg = new Image(rct, BUTTONCOLOR);
	Text* txt = new Text("返回", rct, FONTNAME, BUTTONFONTCOLOR);
	LineBox* lb = new LineBox(rct, LINEBOXCOLOR);
	Button* ret = new Button(bg, txt, lb);
	ret->AddListener([&]() {menu->Last(); page1 = 0; });
	ret_id = ret->InstanceId();
	canvas.Env(1).Register(ret_id, ret);

	//上页按钮注册
	Rect rc2 = createRectbyCenter({ 200,550 }, 50, 20);
	Image* bg2 = new Image(rc2, BUTTONCOLOR);
	Text* txt2 = new Text("上页", rc2, FONTNAME, BUTTONFONTCOLOR);
	LineBox* lb2 = new LineBox(rc2, LINEBOXCOLOR);
	Button* ret2 = new Button(bg2, txt2, lb2);
	ret2->AddListener([&]() {lastPage(canvas); });
	last_id = ret2->InstanceId();
	canvas.Env(1).Register(last_id, ret2);

	//下页按钮注册
	Rect rc3 = createRectbyCenter({ 300,550 }, 50, 20);
	Image* bg3 = new Image(rc3, BUTTONCOLOR);
	Text* txt3 = new Text("下页", rc3, FONTNAME, BUTTONFONTCOLOR);
	LineBox* lb3 = new LineBox(rc3, LINEBOXCOLOR);
	Button* ret3 = new Button(bg3, txt3, lb3);
	ret3->AddListener([&]() {nextPage(canvas); });
	next_id = ret3->InstanceId();
	canvas.Env(1).Register(next_id, ret3);
#pragma endregion
#pragma region 表排序按钮注册
	//ID排序按钮注册
	Rect rc4 = createRectbyCenter({ 400,550 }, 80, 20);
	Image* bg4 = new Image(rc4, BUTTONCOLOR);
	Text* txt4 = new Text("编号排序", rc4, FONTNAME, WHITE);
	LineBox* lb4 = new LineBox(rc4, LINEBOXCOLOR);
	Button* ret4 = new Button(bg4, txt4, lb4);
	ret4->AddListener([&]() {sortByid(courses); page1 = 0; lastPage(canvas); });
	id_sort = ret4->InstanceId();
	canvas.Env(1).Register(id_sort, ret4);

	//总时排序按钮注册
	Rect rc5 = createRectbyCenter({ 500,550 }, 80, 20);
	Image* bg5 = new Image(rc5, BUTTONCOLOR);
	Text* txt5 = new Text("总时排序", rc5, FONTNAME, WHITE);
	LineBox* lb5 = new LineBox(rc5, LINEBOXCOLOR);
	Button* ret5 = new Button(bg5, txt5, lb5);
	ret5->AddListener([&]() {sortByTime(courses); page1 = 0; lastPage(canvas); });
	time_sort = ret5->InstanceId();
	canvas.Env(1).Register(time_sort, ret5);

	//学分排序按钮注册
	Rect rc6 = createRectbyCenter({ 600,550 }, 80, 20);
	Image* bg6 = new Image(rc6, BUTTONCOLOR);
	Text* txt6 = new Text("学分排序", rc6, FONTNAME, WHITE);
	LineBox* lb6 = new LineBox(rc6, LINEBOXCOLOR);
	Button* ret6 = new Button(bg6, txt6, lb6);
	ret6->AddListener([&]() {sortByCredit(courses); page1 = 0; lastPage(canvas); });
	credit_sort = ret6->InstanceId();
	canvas.Env(1).Register(credit_sort, ret6);
#pragma endregion
#pragma region 选课信息显示UI注册
	//已选课程总学分注册
	Text* all_credit = new Text("总学分:", createRectbyPoint(50, 10, 200, 40), false);
	alls_id = all_credit->InstanceId();
	canvas.Env(1).Register(all_credit->InstanceId(), all_credit);
#pragma endregion
}
//生命周期-OnGUI | Canvas 消息 | Priority=1
void OnGUI(Canvas& canvas)
{
	canvas.Env(1).Draw(bg_id); //渲染背景
	//功能节点自行决定实现逻辑
	if (menu->current->funcNode)
	{
		if (menu->current->func != nullptr) menu->current->func(*menu, canvas);
	}
	//非功能节点按位置分布自动渲染,在Env0内进行操作
	if (!menu->current->funcNode) for (int i = 0; i < (menu->current)->childs.size(); i++)
	{
		canvas.Env(0).Draw(menu->current->childs[i]->InstanceId());//渲染节点菜单GUI
	}
}

//生命周期-OnUpdate | Canvas 消息 | Priority=2
void OnUpdate(Canvas& canvas)
{

}
#pragma endregion

int main()
{
	Canvas canvas = { 1000,600,90, RGB(255,255,255) }; //创建1000*600的窗户，背景色为白色
	canvas.Show(OnStart, OnUpdate, OnGUI,true); //初始化GUI画布并启动生命周期，阻塞程序,显示小黑窗
	return 0;
}