#pragma once

#include "Game.h";
#include "Window.h";

#ifdef PROJECT_WATER_SORT
const int MAX_SIZE = 4;

class CWindowWin : public CWindow
{
public:
	void	Load();
	void	Show();
	void	OnClickButton(CButton* pButton);

public:
	CImage	ImageEmote;
	CButton ButtonNext, ButtonHome;
};

class CWaterData
{
public:
	CWaterData(int Color, bool IsLock);

public:
	int		Color;
	bool	IsLock;
};

class CTubeData
{
public:
	int		Size;
	vector<CWaterData> WaterDataList;
};

class CGameData
{
public:
	void	Drop(int IndexDrop, int IndexReceive);

public:
	vector<CTubeData> TubeDataList;
};

class CWater : public CNode
{
public:
	void	Set(int Color, int Value, bool IsLock);
	void	Render();

public:
	int		Color, Value;
	bool	IsLock;
	CImage	Image, ImageWaterLock, ImageLock;
};

class CTube : public CFrame
{
public:
	void	Turn();
	void	Load(int Size = MAX_SIZE);
	bool	IsFull();
	bool	IsDrop(CTube* pTube);
	void	DrawMask(int Size);
	void	AddWater(int Color, int Value, bool IsLock);
	void	UnSelect();
	void	OnSelect();
	bool	CheckFinish();
	void	RemoveWater();
	void	SetStage(int Stage);
	void	SetTarget(CTube* pTarget);
	void	SetPosition(CVec2 Pos);
	void	SetPosition(float PosX, float PosY);
	void	PredictData(int& PredictColor, int& PredictWaterCount);
	void	CountDropletPreDrop(int& PredictWaterCount, CTube* pTubeDrop);
	void	ResetAnchorpoint();
	int		CountDroplet();
	int		CountDropletDrop();
	float	Thick(float Rotation, int DropletCount);

public:
	int		Size, RotateDir, Stage;
	float	Rotation, RotationDes, RotateSpeed, MoveSpeed;
	CVec2	PosRoot, PosDes, Pos;
	CTube* pTarget;
	CImage	ImageLine, ImageTube;
	CButton	ButtonTube;
	ClippingNode ClippingMask;
	vector<CWater> WaterList;
};

class CGameWaterSort : public IGame
{
public:
	~CGameWaterSort();

	void	Init();
	void	SetDrop();
	void	ResetAll();
	void	CheckEnd();
	void	ResetData();
	void	SaveLevel();
	void	UndoWater();
	void	BeginLevel();
	void	InitButtonUndo();
	void	InitTubePosition();
	void	ShowAdInterstitial();
	void	OnUpdate(float DeltaTime);
	void	OnKeyDown(EventKeyboard::KeyCode Key);
	void	OnClickButton(CButton* pButton);
	void	OnAdShowComplete(int Tag, bool IsReward);

public:
	int		Level, Reward, TubeCount, UndoCount, TurnCount;
	bool	IsEnd, IsHideLevel, CanUseBooster;
	bigint	TimeLevelEnd, TimeFirework, TimeTurn;
	CTube*	pSelectTube1, * pSelectTube2;
	CText	TextLevel;
	CFrame	FrameMap;
	CButton	ButtonUndo, ButtonUndoAd, ButtonRestart, ButtonAddTube;
	CWindowWin	WindowWin;
	vector<CTube*> pTubeList;
	vector<CGameData> GameDataList;
};
#endif
