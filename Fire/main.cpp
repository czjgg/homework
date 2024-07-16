// 飞机大战
// EasyX
#include <iostream>
#include <graphics.h>//easyx
#include <vector>
#include <conio.h>
using namespace std;

constexpr auto swidth = 600;
constexpr auto sheight = 1100;

constexpr unsigned int SHP = 4;

constexpr auto hurttime = 1000;//ms

bool PointInRect(int x, int y, RECT& r)
{
	return (r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}

bool RectDuangRect(RECT &r1, RECT &r2)
{
	RECT r;
	r.left = r1.left - (r2.right - r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;

	return (r.left < r2.left&& r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

// 一个开始界面
void Welcome()
{
	LPCTSTR title = _T("飞机大战");
	LPCTSTR tplay = _T("开始游戏");
	LPCTSTR texit = _T("退出游戏");

	RECT tplayr, texitr;
	BeginBatchDraw();
	setbkcolor(WHITE);
	cleardevice();
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 5, title);

	settextstyle(40, 0, _T("黑体"));
	tplayr.left = swidth / 2 - textwidth(tplay) / 2;
	tplayr.right = tplayr.left + textwidth(tplay);
	tplayr.top = sheight / 5*2.5;
	tplayr.bottom = tplayr.top + textheight(tplay);

	texitr.left = swidth / 2 - textwidth(texit) / 2;
	texitr.right = texitr.left + textwidth(texit);
	texitr.top = sheight / 5 * 3;
	texitr.bottom = texitr.top + textheight(texit);

	outtextxy(tplayr.left, tplayr.top, tplay);
	outtextxy(texitr.left, texitr.top, texit);

	EndBatchDraw();

	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_MOUSE);
		if (mess.lbutton)
		{
			if (PointInRect(mess.x, mess.y, tplayr))
			{
				return;
			}
			else if (PointInRect(mess.x, mess.y, texitr))
			{
				exit(0);
			}
		}
	}
}

void Over(unsigned long long &kill)
{
	printf_s("o");
	TCHAR* str = new TCHAR[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kill);

	settextcolor(RED);
	outtextxy(swidth / 2 - textwidth(str) / 2, sheight / 5, str);

	// 键盘事件 （按Enter返回）
	LPCTSTR info = _T("按Enter返回");
	settextstyle(20, 0, _T("黑体"));
	outtextxy(swidth - textwidth(info), sheight - textheight(info), info);

	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_KEY);
		if (mess.vkcode == 0x0D)
		{
			return;
		}
	}
}

// 背景、敌机、英雄、子弹

class BK
{
public:
	BK(IMAGE &img)
		:img(img), y(-sheight)
	{

	}
	void Show()
	{
		if (y == 0) { y = -sheight; }
		y += 4;
		putimage(0, y, &img);
	}

private:
	IMAGE& img;
	int y;

};

class Hero
{
public:
	Hero(IMAGE& img)
		:img(img), HP(SHP)
	{
		rect.left = swidth / 2 - img.getwidth() / 2;
		rect.top = sheight - img.getheight();
		rect.right = rect.left + img.getwidth();
		rect.bottom = sheight;
	}
	void Show()
	{
		setlinecolor(RED);
		setlinestyle(PS_SOLID, 4);
		putimage(rect.left, rect.top, &img);
		line(rect.left, rect.top - 5, rect.left + (img.getwidth() / SHP * HP), rect.top - 5);
	}
	void Control()
	{
		ExMessage mess;
		if (peekmessage(&mess, EM_MOUSE))
		{
			rect.left = mess.x - img.getwidth() / 2;
			rect.top = mess.y - img.getheight() / 2;
			rect.right = rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}
	}

	void setHP(int times) {
		if (times == 0) return;
		HP *= times;
	}

	bool hurt()
	{
		HP--;
		return (HP == 0) ? false : true;
	}

	RECT& GetRect() { return rect; }

	unsigned int speed;

private:
	IMAGE& img;
	RECT rect;


	unsigned int HP;

};

class Enemy
{
public:
	Enemy(IMAGE& img, int x, IMAGE*& boom,int type = 1)
		:img(img), isdie(false), boomsum(0),EnemyType(type)
	{
		selfboom[0] = boom[0];
		selfboom[1] = boom[1];
		selfboom[2] = boom[2];
		rect.left = x;
		rect.right = rect.left + img.getwidth();
		rect.top = -img.getheight();
		rect.bottom = 0;
	}
	bool Show()
	{

		if (isdie)
		{
			if (boomsum == 3)
			{
				return false;
			}
			putimage(rect.left, rect.top, selfboom + boomsum);
			boomsum++;

			return true;
		}

		if (rect.top >= sheight)
		{
			return false;
		}
		rect.top += 4;
		rect.bottom += 4;
		if (EnemyType == 3) {
			if (direction == 0) { // 左右移动
				rect.left += 2; // 向右移动
				if (rect.left > swidth - img.getwidth()) {
					direction = 1; // 到达右边界，改变方向
				}
			}
			else {
				rect.left -= 2; // 向左移动
				if (rect.left < 0) {
					direction = 0; // 到达左边界，改变方向
				}
			}
		}

		rect.right = rect.left + img.getwidth();
		putimage(rect.left, rect.top, &img);

		return true;
	}

	void Isdie()
	{
		isdie = true;
	}

	void MoveTowardsHero(Hero& player) {
		if (player.GetRect().top < rect.top) {
			rect.top += 2; // 向玩家位置靠近
		}
		if (player.GetRect().left < rect.left) {
			rect.left -= 2; // 向玩家位置靠近
		}
		else {
			rect.left += 2;
		}
	}


	RECT& GetRect() { return rect; }
	int getEnemyType() { return EnemyType; }

private:
	IMAGE& img;
	RECT rect;
	IMAGE selfboom[3];
	int direction = 0;
	int EnemyType;

	bool isdie;
	int boomsum;
};

class Bullet
{
public:
	Bullet(IMAGE& img, RECT pr)
		:img(img)
	{
		rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.top = pr.top - img.getheight();
		rect.bottom = rect.top + img.getheight();
	}
	bool Show()
	{
		if (rect.bottom <= 0)
		{
			return false;
		}
		rect.top -= 3;
		rect.bottom -= 3;
		putimage(rect.left, rect.top, &img);

		return true;
	}
	RECT& GetRect() { return rect; }

protected:
	IMAGE& img;
	RECT rect;
};

class EBullet : public Bullet
{
public:
	EBullet(IMAGE& img, RECT pr)
		:Bullet(img, pr)
	{
		rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.top = pr.bottom;
		rect.bottom = rect.top + img.getheight();
	}
	bool Show()
	{
		if (rect.top >= sheight)
		{
			return false;
		}
		rect.top += 5;
		rect.bottom += 5;
		putimage(rect.left, rect.top, &img);

		return true;
	}
};


bool AddEnemy(vector<Enemy*>& es, IMAGE& enemyimg, IMAGE* boom,int type)
{
	Enemy* e = new Enemy(enemyimg, abs(rand()) % (swidth - enemyimg.getwidth()), boom, type);
	for (auto& i : es)
	{
		if (RectDuangRect(i->GetRect(), e->GetRect()))
		{
			delete e;
			return false;
		}
	}
	es.push_back(e);
	return true;

}

void DrawScoreBoard(unsigned long long kill)
{
	TCHAR str[128];
	_stprintf_s(str, 128, _T("击杀数：%llu"), kill);

	settextcolor(BLACK);
	settextstyle(20, 0, _T("黑体"));
	outtextxy(10, 10, str); // 在窗口左上角绘制
}

void ChooseHero(IMAGE& hero1, IMAGE& hero2, IMAGE& hero3,int& plainType)
{
	LPCTSTR title = _T("选择战机");
	RECT hero1Rect, hero2Rect, hero3Rect;

	BeginBatchDraw();
	setbkcolor(WHITE);
	cleardevice();
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 5, title);

	// 绘制战机选项
	hero1Rect.left = swidth / 4 - hero1.getwidth() / 2;
	hero1Rect.top = sheight / 2 - hero1.getheight() / 2;
	hero1Rect.right = hero1Rect.left + hero1.getwidth();
	hero1Rect.bottom = hero1Rect.top + hero1.getheight();

	hero2Rect.left = swidth / 2 - hero2.getwidth() / 2;
	hero2Rect.top = hero1Rect.top;
	hero2Rect.right = hero2Rect.left + hero2.getwidth();
	hero2Rect.bottom = hero2Rect.top + hero2.getheight();

	hero3Rect.left = swidth * 3 / 4 - hero3.getwidth() / 2;
	hero3Rect.top = hero1Rect.top;
	hero3Rect.right = hero3Rect.left + hero3.getwidth();
	hero3Rect.bottom = hero3Rect.top + hero3.getheight();

	putimage(hero1Rect.left, hero1Rect.top, &hero1);
	putimage(hero2Rect.left, hero2Rect.top, &hero2);
	putimage(hero3Rect.left, hero3Rect.top, &hero3);

	EndBatchDraw();

	// 等待选择
	while (true)
	{
		ExMessage mess;
		getmessage(&mess, EM_MOUSE);
		if (mess.lbutton)
		{
			if (PointInRect(mess.x, mess.y, hero1Rect))
			{
				// 选择战机1
				plainType = 1;
				return;
			}
			else if (PointInRect(mess.x, mess.y, hero2Rect))
			{
				// 选择战机2
				plainType = 2;
				return;
			}
			else if (PointInRect(mess.x, mess.y, hero3Rect))
			{
				// 选择战机3
				plainType = 3;
				return;
			}
		}
	}
}


bool Play(int plainType)
{
	setbkcolor(WHITE);
	cleardevice();
	bool is_play = true;

	IMAGE heroimg, enemyimg, bkimg, bulletimg;
	IMAGE eboom[3];

	loadimage(&heroimg, _T("../../images/me1.png"));
	loadimage(&enemyimg, _T("../../images/enemy1.png"));
	loadimage(&bkimg, _T("../../images/bk2.png"), swidth, sheight * 2);
	loadimage(&bulletimg, _T("../../images/bullet1.png"));

	loadimage(&eboom[0], _T("../../images/enemy1_down2.png"));
	loadimage(&eboom[1], _T("../../images/enemy1_down3.png"));
	loadimage(&eboom[2], _T("../../images/enemy1_down4.png"));

	BK bk = BK(bkimg);
	Hero hp = Hero(heroimg);

	vector<Enemy*> es;
	vector<Bullet*> bs;
	vector<EBullet*> ebs;
	int bsing = 0;

	clock_t hurtlast = clock();

	unsigned long long kill = 0;

	for (int i = 0; i < 5; i++)
	{
		AddEnemy(es, enemyimg, eboom, i%3+1);
	}
	unsigned int speed = plainType;
	hp.setHP(plainType);
	while (is_play)
	{
		bsing++;



		
		if (bsing %  speed == 0)
		{
			bs.push_back(new Bullet(bulletimg, hp.GetRect()));
		}
		if (bsing == 60)
		{
			bsing = 0;
			for (auto& i : es)
			{
				ebs.push_back(new EBullet(bulletimg, i->GetRect()));
			}
		}

		BeginBatchDraw();

		bk.Show();
		Sleep(2);
		flushmessage();
		Sleep(2);
		hp.Control();
		// 绘制计分板
		DrawScoreBoard(kill);
		if (_kbhit())
		{
			char v = _getch();
			if (v == 0x20)
			{
				Sleep(500);
				while (true)
				{
					if (_kbhit())
					{
						v = _getch();
						if (v == 0x20)
						{
							break;
						}
					}
					Sleep(16);
				}
			}
		}
		hp.Show();

		auto bsit = bs.begin();
		while (bsit != bs.end())
		{
			if (!(*bsit)->Show())
			{
				bsit = bs.erase(bsit);
			}
			else
			{
				bsit++;
			}
		}

		auto ebsit = ebs.begin();
		while (ebsit != ebs.end())
		{
			if (!(*ebsit)->Show())
			{
				ebsit = ebs.erase(ebsit);
			}
			else
			{
				if (RectDuangRect((*ebsit)->GetRect(), hp.GetRect()))
				{
					if (clock() - hurtlast >= hurttime)
					{
						is_play = hp.hurt();
						hurtlast = clock();
					}
				}
				ebsit++;
			}
			
		}

		auto it = es.begin();
		
		while (it != es.end())
		{
			if ((*it)->getEnemyType() == 2) {
				(*it)->MoveTowardsHero(hp);
			}
			if (RectDuangRect((*it)->GetRect(), hp.GetRect()))
			{
				if (clock() - hurtlast >= hurttime)
				{
					is_play = hp.hurt();
					hurtlast = clock();
				}
			}
			auto bit = bs.begin();
			while (bit != bs.end())
			{
				if (RectDuangRect((*bit)->GetRect(), (*it)->GetRect()))
				{
					(*it)->Isdie();
					delete (*bit);
					bs.erase(bit);

					kill++;

					break;
				}
				bit++;
			}
			if (!(*it)->Show())
			{
				delete (*it);
				es.erase(it);
				it = es.begin();
			}
			it++;
		}
		for (int i = 0; i < 5 - es.size(); i++)
		{
			AddEnemy(es, enemyimg, eboom, i%3+1);
		}

		EndBatchDraw();
	}
	printf_s("e");
	Over(kill);

	return true;
}

int main()
{
	// easyx初始化
	initgraph(swidth, sheight, EW_NOMINIMIZE | EW_SHOWCONSOLE);

	IMAGE hero1, hero2, hero3;
	loadimage(&hero1, _T("../../images/me1.png"));
	loadimage(&hero2, _T("../../images/me1.png"));
	loadimage(&hero3, _T("../../images/me1.png"));
	bool is_live = true;

	int plainType = 1;
	while (is_live)
	{
		Welcome();
		ChooseHero(hero1, hero2, hero3,plainType);
		// Play
		is_live = Play(plainType);
	}

	return 0;
}