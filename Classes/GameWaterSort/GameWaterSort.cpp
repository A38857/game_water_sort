#include "Gui.h"
#include "Engine.h"
#include "Client.h"
#include "Function.h";
#include "PluginAd.h"
#include "SoundManager.h"
#include "GameWaterSort.h"
#include "TextureManager.h";

#ifdef PROJECT_WATER_SORT
const int	TURN_PER_SECOND		= 60;
const float SPEED_RATE			= 1;
const float SPEED_MOVE_TO		= SPEED_RATE * 2400;
const float SPEED_MOVE_BACK		= SPEED_RATE * 1800;
const float	TIME_PER_TURN		= 1.0 / TURN_PER_SECOND;
const float	SPEED_ROTATE_DROP_1 = SPEED_RATE * 10;
const float	SPEED_ROTATE_DROP_2 = SPEED_RATE * 16;
const float	SPEED_ROTATE_DROP_3 = SPEED_RATE * 21;
const float	SPEED_ROTATE_DROP_4 = SPEED_RATE * 28;
const float	SPEED_ROTATE_DROP_5 = SPEED_RATE * 44;
const float	SPEED_ROTATE_DROP_6 = SPEED_RATE * 25;
const int REWARD_UNDO			= 1;
const int REWARD_ADD_TUBE		= 2;
const int STAGE_NORMAL			= 0;
const int STAGE_SELECT			= 1;
const int STAGE_MOVE_TO			= 2;
const int STAGE_DROP			= 3;
const int STAGE_MOVE_BACK		= 4;
const int DROPLET_PER_WATER		= 22 / SPEED_RATE;

CGameWaterSort* pGame;

class CEffectWaterSort : public CEffect
{
public:
	CEffect* CreateTubeFirework()
	{
		// Particle
		for (int i = 0; i < 30; i++)
		{
			CParticle* p = NewParticle();
			p->Load(e::Format("gui/effect/firework_%i.png", e::RandomInt(1, 7)));
			p->SetScale(c::GUI_SCALE * e::RandomFloat(0.7, 1));
			p->AddAction(CSequence::Create(CAction::Delay(e::RandomFloat(1, 1.2)), CAction::FadeTo(0.2, 0)));
			p->AddAction(CSequence::Create(CAction::Delay(e::RandomFloat(1, 1.2)), CAction::ScaleTo(0.2, 0)));
			p->AddAction(CAction::RotateBy(0.8, e::RandomInt(-30, 30)));
			p->AddParticleAction(CParticleAction::Fire2D(c::GUI_SCALE * e::RandomInt(3400, 4200), CVec2(0, -1).Rotate(e::RandomInt(-8, 8)), c::GUI_SCALE * e::RandomInt(6500, 7500)));
			AddChild(p);
		}

		// ZDeep
		SetZDeep(30);
		SetScale(0.6);

		// Done
		return this;
	}
};

void CWindowWin::Load()
{
	// Window
	CWindow::Load(900, 1140, c::GUI_SCALE, true, false);

	// Gui
	SetZDeep(50);
	SetTitle("You Win");

	// Data
	CloseWhenTouchOutside = false;

	// Image Win
	ImageEmote.Load("gui/game/emote_win.png");
	ImageEmote.SetScale(c::GUI_SCALE * 1.5);
	ImageEmote.SetAnchorPointScale(0.5, 1);
	ImageEmote.SetPosition(GetWidth() / 2, c::GUI_SCALE * 555);
	AddChild(&ImageEmote);

	// Button Home
	ButtonHome.Load("gui/window/button_blue_3.png", 48, c::GUI_SCALE);
	ButtonHome.SetText("Home");
	ButtonHome.SetTextPos(0, -c::GUI_SCALE * 12);
	ButtonHome.SetPosition(ImageEmote.GetX(), ImageEmote.GetY() + ButtonHome.GetHeight() / 2 + c::GUI_SCALE * 60);
	AddChild(&ButtonHome);

	// Button Next
	ButtonNext.Load("gui/window/button_blue_3.png", 48, c::GUI_SCALE);
	ButtonNext.SetText("Next");
	ButtonNext.SetTextPos(0, -c::GUI_SCALE * 12);
	ButtonNext.SetPosition(ButtonHome.GetX(), ButtonHome.GetY() + ButtonHome.GetHeight() + c::GUI_SCALE * 60);
	AddChild(&ButtonNext);
}

void CWindowWin::Show()
{
	// Show
	CWindow::Show();

	// Gui
	ButtonNext.SetVisible(true);
	ImageEmote.Update("gui/game/emote_win.png");
}

void CWindowWin::OnClickButton(CButton* pButton)
{
	// Window
	CWindow::OnClickButton(pButton);

	// Button Home
	if (pButton == &ButtonHome) Game.pGame->Release();

	// Button Next
	if (pButton == &ButtonNext) pGame->BeginLevel();
}

CWaterData::CWaterData(int Color, bool IsLock)
{
	this->Color = Color;
	this->IsLock = IsLock;
}

void CGameData::Drop(int IndexDrop, int IndexReceive)
{
	// Backup
	CTubeData* pTubeDataDrop = &TubeDataList[IndexDrop];
	CTubeData* pTubeDataRecieve = &TubeDataList[IndexReceive];
	int ColorDrop = pTubeDataDrop->WaterDataList.back().Color;
	for (int i = int(pTubeDataDrop->WaterDataList.size()) - 1; i >= 0; i--)
	{
		if (pTubeDataDrop->WaterDataList[i].Color == ColorDrop && !pTubeDataDrop->WaterDataList[i].IsLock && pTubeDataRecieve->WaterDataList.size() < pTubeDataRecieve->Size)
		{
			pTubeDataRecieve->WaterDataList.push_back(CWaterData(ColorDrop, false));
			pTubeDataDrop->WaterDataList.pop_back();
		}
		else break;
	}

	// Unlock 
	for (int i = int(pTubeDataDrop->WaterDataList.size()) - 1; i >= 0; i--)
	{
		if (pTubeDataDrop->WaterDataList[i].Color == pTubeDataDrop->WaterDataList.back().Color && pTubeDataDrop->WaterDataList[i].IsLock)
		{
			pTubeDataDrop->WaterDataList[i].IsLock = false;
			for (int j = 0; j < pGame->GameDataList.size(); j++)
			{
				pGame->GameDataList[j].TubeDataList[IndexDrop].WaterDataList[i].IsLock = false;
			}
		}
		else break;
	}
}

void CWater::Set(int Color, int Value, bool IsLock)
{
	// Data
	this->Color = Color;
	this->Value = Value;
	this->IsLock = IsLock;

	// Image
	Image.Load(e::Format("game_water_sort/water_%i.png", Color));
	Image.SetZDeep(-2);
	Image.SetAnchorPointScale(0.5, 1);

	// Image Lock
	if (IsLock)
	{
		// Image Water Lock
		ImageWaterLock.Load("game_water_sort/water_lock.png");
		ImageWaterLock.SetZDeep(-1);
		ImageWaterLock.SetScale(10, 1);
		ImageWaterLock.SetAnchorPointScale(0.5, 1);

		// Image Lock
		ImageLock.Load("game_water_sort/lock.png");
		ImageLock.SetZDeep(0);
		ImageLock.SetAnchorPointScale(0.5, 0.5);
	}

	// Render
	Render();
}

void CWater::Render()
{
	Image.SetScale(10, float(Value) / DROPLET_PER_WATER);
}

void CTube::Load(int Size)
{
	// Create
	CNode::Create();

	// Data
	pTarget = 0;
	Stage = STAGE_NORMAL;
	this->Size = Size;
	Rotation = RotationDes = RotateDir = 0;
	RotateSpeed = MoveSpeed = 0;

	// Image Tube
	ImageTube.Load(e::Format("game_water_sort/tube_%i.png",Size));
	ImageTube.SetZDeep(1);
	ImageTube.SetScale(c::GUI_SCALE);
	ImageTube.SetAnchorPointScale(0.5, 0);
	AddChild(&ImageTube);

	// Button Tube
	ButtonTube.Load("game_water_sort/button_blank.png", "game_water_sort/button_blank.png", 0, c::GUI_SCALE);
	ButtonTube.TouchBoundary = false;
	ButtonTube.TouchEffect = false;
	ButtonTube.SoundEnable = false;
	ButtonTube.SetAnchorPointScale(1, 0);
	ButtonTube.SetPosition(0, ImageTube.GetHeight() / 2);
	ButtonTube.SetZDeep(0);
	AddChild(&ButtonTube);

	// ImageTube 4
	CImage ImageTube4;
	ImageTube4.Load(e::Format("game_water_sort/tube_%i.png", MAX_SIZE));
	ImageTube4.SetScale(c::GUI_SCALE);

	// ImageLine
	string File = "game_water_sort/water_1.png";
	ImageLine.Load(File);
	ImageLine.SetScale(c::GUI_SCALE * 0.08, (5 * ImageTube4.GetHeight() / 4) / ImageLine.GetHeight());
	ImageLine.SetPosition(CVec2(ButtonTube.GetX(), ButtonTube.GetY() - (ImageLine.GetHeight() - ButtonTube.GetHeight()) / 2 - 5 * c::GUI_SCALE));
	ImageLine.SetVisible(false);
	ImageLine.SetZDeep(-3);
	AddChild(&ImageLine);

	// Mask
	ClippingMask.create();
	ClippingMask.setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	ClippingMask.setZOrder(-1);
	DrawMask(Size);
	ImageTube.pNode->addChild(&ClippingMask);
}

void CTube::DrawMask(int Size)
{
	// Mask Image
	CImage MaskImage;
	MaskImage.Load(e::Format("game_water_sort/mask_%i.png", Size));
	auto MaskSize = MaskImage.pNode->getContentSize();
	MaskImage.pNode->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
	MaskImage.pNode->setPosition(cocos2d::Vec2(MaskSize.width / 2, MaskSize.height / 2));
	MaskImage.pNode->setRotation(180);

	// RenderTexture
	auto RenderTexture = cocos2d::RenderTexture::create(MaskSize.width, MaskSize.height);
	RenderTexture->begin();
	MaskImage.pNode->visit();
	RenderTexture->end();

	// Stencil Sprite
	auto StencilSprite = cocos2d::Sprite::createWithTexture(RenderTexture->getSprite()->getTexture());
	StencilSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE_TOP);
	StencilSprite->setBlendFunc(cocos2d::BlendFunc::ALPHA_PREMULTIPLIED);

	// Clipping Mask
	ClippingMask.setContentSize(MaskSize);
	ClippingMask.setStencil(StencilSprite);
	ClippingMask.setAlphaThreshold(0.1f);
	ClippingMask.setPosition(MaskSize.width, 2 * MaskSize.height);
}

void CTube::PredictData(int& PredictColor, int& PredictWaterCount)
{
	// Color
	PredictColor = 0;
	if (!WaterList.empty())
	{
		PredictColor = WaterList.back().Color;
	}
	else
	{
		for (int i = 0; i < pGame->pTubeList.size(); i++)
		{
			CTube* pTube = pGame->pTubeList[i];
			if (pTube->pTarget == this && !pTube->WaterList.empty() && !pTube->WaterList.back().IsLock) PredictColor = pTube->WaterList.back().Color;
		}
	}

	// Droplet Count
	PredictWaterCount = this->CountDroplet();
	for (int i = 0; i < pGame->pTubeList.size(); i++)
	{
		CTube* pTube = pGame->pTubeList[i];
		if (pTube->pTarget == this && PredictWaterCount < this->Size * DROPLET_PER_WATER) PredictWaterCount += pTube->CountDropletDrop();
	}
	if (PredictWaterCount > this->Size * DROPLET_PER_WATER) PredictWaterCount = this->Size * DROPLET_PER_WATER;
}

void CTube::CountDropletPreDrop(int& DropletCount, CTube *pTubeDrop)
{
	// Droplet Count
	DropletCount = this->CountDroplet();
	for (int i = 0; i < pGame->pTubeList.size(); i++)
	{
		CTube* pTube = pGame->pTubeList[i];
		if (pTube != pTubeDrop&& pTube->pTarget == this && DropletCount < this->Size * DROPLET_PER_WATER) DropletCount += pTube->CountDropletDrop();
	}
	if (DropletCount > this->Size * DROPLET_PER_WATER) DropletCount = this->Size * DROPLET_PER_WATER;
}

int CTube::CountDropletDrop()
{
	// Check
	if (WaterList.empty()) return 0;

	// Count
	int DropletCount = 0;
	int LastColor = WaterList.back().Color;
	for (int i = int(WaterList.size()) - 1; i >= 0; i--)
	{
		if (WaterList[i].Color == LastColor && !WaterList[i].IsLock) DropletCount += WaterList[i].Value;
		else break;
	}
	return DropletCount;
}

int CTube::CountDroplet()
{
	int DropletCount = 0;
	for (int i = 0; i < WaterList.size(); i++) DropletCount += WaterList[i].Value;
	return DropletCount;
}

void CTube::SetStage(int Stage)
{
	this->Stage = Stage;
	if (Stage == STAGE_NORMAL)
	{
		// Button
		ButtonTube.SetEnable(true);

		// ZDeep
		SetZDeep(0);
	}
	else if (Stage == STAGE_MOVE_TO)
	{
		// Button
		ButtonTube.SetEnable(false);

		// ZDeep
		SetZDeep(1);
	}
}

void CTube::SetPosition(float PosX, float PosY)
{
	// Data
	Pos = CVec2(PosX, PosY);

	// Gui
	CFrame::SetPosition(Pos);
}

void CTube::SetPosition(CVec2 Pos)
{
	SetPosition(Pos.x, Pos.y);
}

void CTube::ResetAnchorpoint()
{
	// Data
	float PreAC = ImageTube.GetAnchorPointScaleX();
	if (pTarget != nullptr)
	{
		if (Pos.x <= pTarget->Pos.x)
		{
			if (pTarget->Pos.x < ImageTube.GetHeight())
			{
				ImageTube.SetAnchorPointScale(0, 0);
				RotateDir = -1;
			}
			else
			{
				ImageTube.SetAnchorPointScale(1, 0);
				RotateDir = 1;
			}
		}
		else
		{
			if (abs(c::WINDOW_WIDTH - pTarget->Pos.x) < ImageTube.GetHeight())
			{
				ImageTube.SetAnchorPointScale(1, 0);
				RotateDir = 1;
			}
			else
			{
				ImageTube.SetAnchorPointScale(0, 0);
				RotateDir = -1;
			}
		}
		SetPosition(Pos.x + (ImageTube.GetAnchorPointScaleX() - PreAC) * ImageTube.GetWidth(), Pos.y);
		PosDes = Pos;
		PosRoot = PosRoot + CVec2((ImageTube.GetAnchorPointScaleX() - PreAC) * ImageTube.GetWidth(), 0);
	}
	if (Stage == STAGE_NORMAL)
	{
		ImageTube.SetAnchorPointScale(0.5, 0);
		SetPosition(Pos.x += (ImageTube.GetAnchorPointScaleX() - PreAC) * ImageTube.GetWidth(), Pos.y);
		PosDes = Pos;
		PosRoot = Pos;
	}
}

void CTube::Turn()
{
	// Data 
	float PrevRotation = Rotation;

	// Stage
	if (Stage == STAGE_MOVE_TO)
	{
		// Rotation
		if (Rotation == RotationDes && Pos == PosDes)
		{
			// Data
			RotationDes = RotateDir * 90;
			if (Size == MAX_SIZE) RotateSpeed = (WaterList.size() == 4) ? SPEED_ROTATE_DROP_4 : (WaterList.size() == 3) ? SPEED_ROTATE_DROP_3 : (WaterList.size() == 2) ? SPEED_ROTATE_DROP_2 : SPEED_ROTATE_DROP_1;
			else RotateSpeed = (WaterList.size() == 2) ? SPEED_ROTATE_DROP_5 : SPEED_ROTATE_DROP_6;
			SetStage(STAGE_DROP);

			// Gui
			pTarget->ImageLine.SetVisible(true);

			// Sound
			int DropletCount = 0, SoundDrop = 0;
			pTarget->CountDropletPreDrop(DropletCount, this);
			if ((CountDropletDrop() + DropletCount)/DROPLET_PER_WATER <= MAX_SIZE) SoundDrop = CountDropletDrop()/DROPLET_PER_WATER;
			else SoundDrop = MAX_SIZE - DropletCount /DROPLET_PER_WATER;
			pGame->SoundManager.Play(e::Format("game_water_sort/drop_water_%i", SoundDrop));
		}
	}
	if (Stage == STAGE_DROP)
	{
		// Speed
		if (Size == MAX_SIZE) RotateSpeed = (WaterList.size() == 4) ? SPEED_ROTATE_DROP_4 : (WaterList.size() == 3) ? SPEED_ROTATE_DROP_3 : (WaterList.size() == 2) ? SPEED_ROTATE_DROP_2 : SPEED_ROTATE_DROP_1;
		else RotateSpeed = (WaterList.size() == 2) ? SPEED_ROTATE_DROP_5 : SPEED_ROTATE_DROP_6;

		// Drop Water
		if ((IsDrop(pTarget) || pTarget->WaterList.empty()) && !WaterList.back().IsLock)
		{
			if (pTarget->WaterList.empty() || (pTarget->WaterList.back().Value == DROPLET_PER_WATER))
			{
				pTarget->AddWater(WaterList.back().Color, 0, false);
			}
			if (WaterList.back().Value > 0)
			{
				int WaterDrop = 1;
				WaterList.back().Value -= WaterDrop;
				pTarget->WaterList.back().Value += WaterDrop;
				pTarget->WaterList.back().Render();
				if (WaterList.back().Value == 0) RemoveWater();
			}
		}
		// Drop Finish
		else
		{
			if (pTarget->CheckFinish())
			{
				// Effect
				pGame->Game.EffectManager.AddEffect(pTarget->Pos.x, pTarget->Pos.y + ImageTube.GetHeight() - 80 * c::GUI_SCALE, ((new CEffectWaterSort())->CreateTubeFirework()));

				// Sound
				//pGameWaterSort->SoundManager.Vibrate();
			}

			// Move
			pTarget = 0;
			PosDes = PosRoot;
			MoveSpeed = SPEED_MOVE_BACK * c::GUI_SCALE;
			float Time = (PosDes - Pos).Length() / MoveSpeed;
			if (Time < 0.2)	Time = 0.2;
			else if (Time > 0.4)	Time = 0.4;
			Time = Time / SPEED_RATE;
			MoveSpeed = (PosDes - Pos).Length() / Time;

			// Rotate
			RotationDes = 0;
			RotateSpeed = abs(RotationDes - Rotation) / Time;

			// Stage
			SetStage(STAGE_MOVE_BACK);
		}
	}
	if (Stage == STAGE_MOVE_BACK && Rotation == RotationDes && Pos == PosRoot)
	{
		// Image Lock
		for (int i = 0; i < WaterList.size(); i++)
		{
			if (WaterList[i].IsLock) WaterList[i].ImageLock.SetVisible(true);
		}
		// Unlock
		for (int i = int(WaterList.size()) - 1; i >= 0; i--)
		{
			if (WaterList[i].Color == WaterList.back().Color && WaterList[i].IsLock)
			{
				// Data
				WaterList[i].IsLock = false;

				// Gui
				WaterList[i].ImageLock.AddAction(CAction::FadeTo(0.4, 0));
				WaterList[i].ImageWaterLock.AddAction(CAction::FadeTo(0.4, 0));
			}
			else break;
		}

		// Stage
		SetStage(STAGE_NORMAL);

		// Reset
		ResetAnchorpoint();
	}

	// Move
	if (Pos != PosDes)
	{
		CVec2 Dir = PosDes - Pos;
		float Distance = MoveSpeed * TIME_PER_TURN;
		if (Dir.Length() <= Distance)
		{
			SetPosition(PosDes);
		}
		else
		{
			Dir.Normalize(Distance);
			SetPosition(Pos + Dir);
		}
	}

	// Rotate
	if (Rotation != RotationDes)
	{
		if (abs(Rotation - RotationDes) <= RotateSpeed * TIME_PER_TURN)
		{
			Rotation = RotationDes;
		}
		else
		{
			if (Rotation < RotationDes) Rotation += RotateSpeed * TIME_PER_TURN;
			else Rotation -= RotateSpeed * TIME_PER_TURN;
		}
		SetRotation(Rotation);

		// Pos Bottom
		CVec2 Center = Pos + CVec2(-RotateDir * ImageTube.GetWidth() / 2, 481).Rotate(-PrevRotation);
		CVec2 PosS = Center + CVec2(0, 75 ).Rotate(-PrevRotation);
		CVec2 DifferPos = Center + CVec2(0, 75).Rotate(-Rotation) - PosS;

		// Water Rotate
		for (int i = 0; i < WaterList.size(); i++)
		{
			CVec2 cv;
			if (i > 0) cv = CVec2(WaterList[i - 1].Image.GetX(), WaterList[i - 1].Image.GetY()) - CVec2(0, WaterList[i - 1].Image.GetHeight()).Rotate(-Rotation);
			else cv = CVec2(WaterList[i].Image.GetX() + DifferPos.x, WaterList[i].Image.GetY() + DifferPos.y);
			WaterList[i].Image.SetRotation(-Rotation);
			WaterList[i].Image.SetPosition(cv);
			WaterList[i].Image.SetScale(WaterList[i].Image.GetScaleX(), Thick(Rotation, WaterList[i].Value));
			if (WaterList[i].ImageWaterLock.IsCreate())
			{
				WaterList[i].ImageWaterLock.SetRotation(-Rotation);
				WaterList[i].ImageWaterLock.SetPosition(WaterList[i].Image.GetX(), WaterList[i].Image.GetY());
				WaterList[i].ImageWaterLock.SetScale(WaterList[i].ImageWaterLock.GetScaleX(), Thick(Rotation, WaterList[i].Value));
			}
		}
	}
}

float CTube::Thick(float Rotation, int DropletCount)
{
	return (1 - ((1 - 0.3) * (abs(Rotation) / 90))) * (float(DropletCount) / DROPLET_PER_WATER);
}

bool CTube::IsFull()
{
	return (WaterList.size() == Size && WaterList.back().Value == DROPLET_PER_WATER);
}

bool CTube::IsDrop(CTube* pTube)
{
	if (!WaterList.empty() && !pTube->WaterList.empty() && (!pTube->IsFull())) return WaterList.back().Color == pTube->WaterList.back().Color;
	return false;
}

void CTube::AddWater(int Color, int Value, bool IsLock)
{
	// Check
	if (WaterList.size() >= Size) return;

	// Add
	CWater Water;
	Water.Set(Color, Value, IsLock);
	ClippingMask.addChild(Water.Image.pNode);
	Water.Image.SetPosition(0, ClippingMask.getContentSize().height - WaterList.size() * Water.Image.pNode->getContentSize().height);
	if (IsLock)
	{
		ClippingMask.addChild(Water.ImageWaterLock.pNode);
		Water.ImageWaterLock.SetPosition(Water.Image.GetX(), Water.Image.GetY());
		ClippingMask.addChild(Water.ImageLock.pNode);
		Water.ImageLock.SetPosition(Water.ImageWaterLock.GetX(), Water.ImageWaterLock.GetY() - Water.ImageWaterLock.GetHeight() / 2);
	}
	WaterList.push_back(Water);
}

void CTube::SetTarget(CTube* pTarget)
{
	// Data
	this->pTarget = pTarget;
	ResetAnchorpoint();
	PosDes = CVec2(pTarget->Pos.x + RotateDir * 5 * c::GUI_SCALE, pTarget->Pos.y - (pTarget->ImageLine.GetHeight() - pTarget->ImageTube.GetHeight()) + 5 * c::GUI_SCALE);
	MoveSpeed = SPEED_MOVE_TO * c::GUI_SCALE;

	// Rotation Des
	float Rot;
	if (Size == MAX_SIZE)
	{
		if (WaterList.size() == 1) RotationDes = RotateDir * 86;
		if (WaterList.size() == 2) RotationDes = RotateDir * 81;
		if (WaterList.size() == 3) RotationDes = RotateDir * 73;
		if (WaterList.size() == 4) RotationDes = RotateDir * 63;
	}
	else 
	{
		if (WaterList.size() == 1) RotationDes = RotateDir * 81;
		if (WaterList.size() == 2) RotationDes = RotateDir * 66;
	}

	// Speed
	float Time = (PosDes - Pos).Length() / MoveSpeed;
	if (Time < 0.2)	Time = 0.2;
	else if (Time > 0.3)	Time = 0.3;
	Time = Time / SPEED_RATE;
	MoveSpeed = (PosDes - Pos).Length() / Time;
	RotateSpeed = abs(RotationDes) / Time;
	SetStage(STAGE_MOVE_TO);

	// Gui
	for (int i = 0; i < WaterList.size(); i++)
	{
		if (WaterList[i].IsLock) WaterList[i].ImageLock.SetVisible(false);
	}
}

bool CTube::CheckFinish()
{
	return IsFull() && Size == MAX_SIZE && WaterList[0].Color == WaterList[1].Color && WaterList[1].Color == WaterList[2].Color && WaterList[2].Color == WaterList[3].Color;
}

void CTube::RemoveWater()
{
	if (!WaterList.empty())
	{
		ClippingMask.removeChild(WaterList.back().Image.pNode);
		if (WaterList.back().ImageWaterLock.IsCreate())
		{
			ClippingMask.removeChild(WaterList.back().ImageWaterLock.pNode);
			ClippingMask.removeChild(WaterList.back().ImageLock.pNode);
		}
		WaterList.pop_back();
	}
}

void CTube::OnSelect()
{
	SetStage(STAGE_SELECT);
	PosDes = PosRoot + CVec2(0, -100 * c::GUI_SCALE);
	MoveSpeed = SPEED_MOVE_TO * c::GUI_SCALE;
}

void CTube::UnSelect()
{
	PosDes = PosRoot;
	MoveSpeed = SPEED_MOVE_TO * c::GUI_SCALE;
	SetStage(STAGE_NORMAL);
}

CGameWaterSort::~CGameWaterSort()
{
	ResetData();
}

void CGameWaterSort::Init()
{
	// Load
	CTextureManager::Load("game_water_sort/data-1.plist");
	CTextureManager::Load("game_water_sort/data-2.plist");

	// Create
	Create();

	// Data
	pGame = this;
	Level = 1;
	IsEnd = true;
	pSelectTube1 = 0;
	pSelectTube2 = 0;
	Reward = 0;
	UndoCount = 5;
	TurnCount = 0;
	TimeTurn = e::GetTime();

	// ImageBg
	CImage ImageBg;
	ImageBg.Load(c::IPAD ? "game_water_sort/bg_ipad.png" : "game_water_sort/bg_iphone.png");
	ImageBg.SetAnchorPoint(0, 0);
	ImageBg.SetScale(c::WINDOW_WIDTH / ImageBg.GetWidth(), c::WINDOW_HEIGHT / ImageBg.GetHeight());
	AddChild(&ImageBg);

	// Level
	string s;
	Level = Function.GetSetting("WaterSortLevel", s) ? stoi(s) : 1;

	// Text Level
	TextLevel.Create(c::GUI_SCALE * 60);
	TextLevel.SetPosition(c::WINDOW_WIDTH / 2, Gui.ButtonHome.GetY());
	AddChild(&TextLevel);

	// Button Restart
	ButtonRestart.Load("game_water_sort/button_restart.png", 40, c::GUI_SCALE);
	ButtonRestart.SetPosition(Gui.ButtonHome.GetX(), Gui.ButtonHome.GetY() + Gui.ButtonHome.GetHeight() / 2 + ButtonRestart.GetHeight() / 2 + c::GUI_SCALE * 30);
	AddChild(&ButtonRestart);

	// Button Add Tube
	ButtonAddTube.Load("game_water_sort/button_add_tube.png", "game_water_sort/button_add_tube_disable.png", 40, c::GUI_SCALE);
	ButtonAddTube.SetPosition(c::WINDOW_WIDTH / 2 + ButtonAddTube.GetWidth() / 2 + 30 * c::GUI_SCALE, TextLevel.GetY() + ButtonAddTube.GetHeight() / 2 + 110 * c::GUI_SCALE);
	AddChild(&ButtonAddTube);

	// Button Undo
	ButtonUndo.Load("game_water_sort/button_undo_no_ad.png", "game_water_sort/button_undo_no_ad_disable.png", 60, c::GUI_SCALE);
	ButtonUndo.Text.Recreate(60 * c::GUI_SCALE, ALIGN_CENTER, ALIGN_CENTER, TEXT_NORMAL, 0xffe323);
	ButtonUndo.SetPosition(c::WINDOW_WIDTH / 2 - ButtonUndo.GetWidth() / 2 - 30 * c::GUI_SCALE, ButtonAddTube.GetY());
	ButtonUndo.SetText(e::Format("%i", UndoCount));
	ButtonUndo.SetTextPos(-c::GUI_SCALE * 10, -c::GUI_SCALE * 6);
	AddChild(&ButtonUndo);

	// Button Undo Ad
	ButtonUndoAd.Load("game_water_sort/button_undo.png", "game_water_sort/button_undo_disable.png", 60, c::GUI_SCALE);
	ButtonUndoAd.SetPosition(ButtonUndo.GetX(), ButtonUndo.GetY());
	ButtonUndoAd.SetEnable(false);
	ButtonUndoAd.SetVisible(false);
	AddChild(&ButtonUndoAd);

	// Frame Map
	FrameMap.Create();
	AddChild(&FrameMap);

	// Window Win
	WindowWin.Load();
	AddChild(&WindowWin);

	// Sound
	SoundManager.Load("game_water_sort/drop_water_1");
	SoundManager.Load("game_water_sort/drop_water_2");
	SoundManager.Load("game_water_sort/drop_water_3");
	SoundManager.Load("game_water_sort/drop_water_4");

	// Begin
	BeginLevel();
}

void CGameWaterSort::BeginLevel()
{
	//Reset
	ResetAll();

	// Data
	IsEnd = false;
	IsHideLevel = false;
	CanUseBooster = true;
	UndoCount = 5;
	TimeLevelEnd = TimeFirework = 0;
	Reward = 0;
	if (Level < 1) Level = 1;
	int PriorityColorList[] = { 6, 2, 12, 4, 11, 10, 1, 7, 3, 5, 8, 9 };
	vector<int> ColorList;

	// Gui
	ButtonAddTube.SetEnable(true);
	TextLevel.SetText("Level %i", Level);
	InitButtonUndo();

	// Load
	string Line;
	CFile File;
	int TempLevel = (Level > 300) ? 101 + (Level - 101) % 200 : Level;
	File.Load(e::Format("game_water_sort/level/level_%s%i.txt", (TempLevel < 10) ? "00" : ((TempLevel < 100) ? "0" : ""), TempLevel));
	while (File.ReadLine(Line))
	{
		if (Line == "[tube_count]")
		{
			TubeCount = File.ReadInt();
		}
		if (Line == "[mode]")
		{
			IsHideLevel = File.ReadInt();
		}
		if (Line == "[tube]")
		{
			File.ReadLine(Line);
			vector<string> WordList;
			e::SplitWord(Line, WordList);
			CTube* pTube = new CTube();
			pTube->Load(MAX_SIZE);
			pTube->SetZDeep(0);
			pTube->ButtonTube.HandleEventCallback(this);
			for (int i = 0; i < WordList.size(); i++)
			{
				int index = -1;
				for (int j = 0; j < ColorList.size(); j++) 
				{
					if (stoi(WordList[i]) == ColorList[j]) 
					{
						index = j;
						break;
					}
				}
				if (index == -1)
				{
					ColorList.push_back(stoi(WordList[i]));
					index = ColorList.size() - 1;
				}
				pTube->AddWater(PriorityColorList[index], DROPLET_PER_WATER, IsHideLevel);
			}
			if (IsHideLevel)
			{
				pTube->WaterList.back().IsLock = false;
				pTube->WaterList.back().ImageWaterLock.SetVisible(false);
				pTube->WaterList.back().ImageLock.SetVisible(false);
			}
			FrameMap.AddChild(pTube);
			pTubeList.push_back(pTube);
		}
	}
	if (TubeCount > pTubeList.size())
	{
		int TubeEmpty = TubeCount - pTubeList.size();
		for (int i = 0; i < TubeEmpty; i++)
		{
			CTube* pTube = new CTube();
			pTube->Load(MAX_SIZE);
			pTube->SetZDeep(0);
			pTube->ButtonTube.HandleEventCallback(this);
			FrameMap.AddChild(pTube);
			pTubeList.push_back(pTube);
		}
	}

	// Position Tube
	InitTubePosition();

	// Backup
	CGameData GameData;
	for (int i = 0; i < pTubeList.size(); i++)
	{
		CTube* pTube = pTubeList[i];
		CTubeData TubeData;
		for (int j = 0; j < pTube->WaterList.size(); j++)
		{
			TubeData.Size = pTube->Size;
			TubeData.WaterDataList.push_back(CWaterData(pTube->WaterList[j].Color, pTube->WaterList[j].IsLock));
		}
		GameData.TubeDataList.push_back(TubeData);
	}
	GameDataList.push_back(GameData);

	// Sound
	SoundManager.Play("game_begin");
}

void CGameWaterSort::InitTubePosition()
{
	float CenterY = ((c::WINDOW_HEIGHT - c::MARGIN_BOTTOM) + (ButtonAddTube.GetY() + ButtonAddTube.GetHeight() / 2)) / 2;
	int CountCol = pTubeList.size();
	int HeightY = 0;
	int SpaceX = 0;
	if (pTubeList.size() > 5)
	{
		HeightY = -1;
		CountCol = (pTubeList.size() % 2) ? (pTubeList.size() / 2 + pTubeList.size() % 2) : (pTubeList.size() / 2);
	}
	for (int i = 0; i < pTubeList.size(); i++)
	{
		if (i == CountCol)
		{
			HeightY = 1;
			CountCol = pTubeList.size() - CountCol;
			SpaceX = 0;
		}
		float PosX = c::WINDOW_WIDTH / (2 * CountCol) + (SpaceX) * (c::WINDOW_WIDTH / ((CountCol)));
		float PosY = CenterY - pTubeList[i]->ImageTube.GetHeight() / 2 + HeightY * (pTubeList[i]->ImageTube.GetHeight() / 2 + 85 * c::GUI_SCALE);
		pTubeList[i]->SetPosition(CVec2(PosX, PosY));
		pTubeList[i]->PosDes = pTubeList[i]->PosRoot = pTubeList[i]->Pos;
		SpaceX++;
	}
}

void CGameWaterSort::InitButtonUndo()
{
	if (UndoCount > 0)
	{
		// Button Undo
		ButtonUndo.SetText(to_string(UndoCount));
		if (GameDataList.size() > 1)
		{
			ButtonUndo.SetEnable(true);
			ButtonUndo.SetTextColor(0xffe323);
		}
		else
		{
			ButtonUndo.SetEnable(false);
			ButtonUndo.SetTextColor(0xe4e4e4);
		}
		ButtonUndo.SetVisible(true);

		// Button Undo Ad
		ButtonUndoAd.SetEnable(false);
		ButtonUndoAd.SetVisible(false);
	}
	else
	{
		// Button Undo
		ButtonUndo.SetEnable(false);
		ButtonUndo.SetVisible(false);

		// Button Undo Ad
		ButtonUndoAd.SetEnable((GameDataList.size() > 1));
		ButtonUndoAd.SetVisible(true);
	}
}

void CGameWaterSort::SaveLevel()
{
	Game.UpdateLevel(Level);
	Function.SetSetting("WaterSortLevel", e::Format("%i", Level));
}

void CGameWaterSort::CheckEnd()
{
	// Check
	if (IsEnd) return;
	for (int i = 0; i < pTubeList.size(); i++)
	{
		if (!pTubeList[i]->WaterList.empty() && !pTubeList[i]->CheckFinish())
		{
			IsEnd = false;
			return;
		}
	}

	// Data
	IsEnd = true;
	TimeFirework = e::GetTime();
	TimeLevelEnd = e::GetTime();
}

void CGameWaterSort::UndoWater()
{
	// Data
	if (pSelectTube1)
	{
		pSelectTube1->SetPosition(pSelectTube1->PosRoot);
		pSelectTube1->PosDes = pSelectTube1->PosRoot = pSelectTube1->Pos;
		pSelectTube1->SetStage(STAGE_NORMAL);
		pSelectTube1 = 0;
	}

	// Undo
	if (GameDataList.size() > 1)
	{
		GameDataList.pop_back();
		UndoCount--;
	}
	for (int i = 0; i < pTubeList.size(); i++)
	{
		int Size = pTubeList[i]->WaterList.size();
		for (int j = 0; j < Size; j++)
		{
			pTubeList[i]->RemoveWater();
		}
	}
	for (int i = 0; i < pTubeList.size(); i++)
	{
		for (int j = 0; j < GameDataList.back().TubeDataList[i].WaterDataList.size(); j++)
		{
			pTubeList[i]->AddWater(GameDataList.back().TubeDataList[i].WaterDataList[j].Color, DROPLET_PER_WATER, GameDataList.back().TubeDataList[i].WaterDataList[j].IsLock);
		}
	}

	// Sound
	SoundManager.Play("booster");
}

void CGameWaterSort::OnClickButton(CButton* pButton)
{
	// Button Home
	if (pButton == &Gui.ButtonHome)
	{
#ifdef TEST_MODE
		Level--;
		if (Level < 1)Level = 1;
		SaveLevel();
		BeginLevel();
#else
		Release();
		ShowAdInterstitial();
#endif
	}

	// Button Setting
	if (pButton == &Gui.ButtonSetting)
	{
		Level++;
		SaveLevel();
		BeginLevel();
	}

	// Button Restart
	if (pButton == &ButtonRestart)
	{
		BeginLevel();
		ShowAdInterstitial();
	}

	// Playing
	if (!IsEnd)
	{
		// Button Undo Ad
		if (pButton == &ButtonUndoAd && CanUseBooster == true)
		{
			Reward = REWARD_UNDO;
			PluginAd.Show(TAG_REWARDED);
		}

		// Button Undo
		if (pButton == &ButtonUndo && CanUseBooster == true && UndoCount > 0)
		{
			UndoWater();
			ButtonUndo.SetText(e::Format("%i", UndoCount));
			if (UndoCount == 0) InitButtonUndo();
		}

		// Button Add Tube
		if (pButton == &ButtonAddTube && CanUseBooster == true && (pTubeList.size() < TubeCount + 1 || pTubeList.back()->Size < MAX_SIZE))
		{
			Reward = REWARD_ADD_TUBE;
			PluginAd.Show(TAG_REWARDED);
		}

		// Button Tube
		for (int i = 0; i < pTubeList.size(); i++)
		{
			if (&pTubeList[i]->ButtonTube == pButton)
			{
				bool IsReceived = false;
				for (int j = 0; j < pTubeList.size(); j++)
				{
					if (j != i && pTubeList[j]->pTarget == pTubeList[i])
					{
						IsReceived = true;
						break;
					}
				}
				if (pSelectTube1 == 0 && !pTubeList[i]->WaterList.empty() && pTubeList[i]->Stage == STAGE_NORMAL && !IsReceived)
				{
					// Data
					pSelectTube1 = pTubeList[i];

					// Gui
					pTubeList[i]->OnSelect();

					// Sound
					//SoundManager.Haptic();
					break;
				}
				if (pSelectTube1 != 0)
				{
					// Data
					pSelectTube2 = pTubeList[i];
					int PredictColor = 0, PredictWaterCount = 0;
					pSelectTube2->PredictData(PredictColor, PredictWaterCount);
					if (pSelectTube1 == pSelectTube2 || (IsReceived && ((PredictWaterCount / DROPLET_PER_WATER) >= pSelectTube2->Size || (pSelectTube1->WaterList.back().Color != PredictColor && PredictColor != 0))))
					{
						// Data
						pSelectTube1->UnSelect();
						pSelectTube1 = 0;

						// Sound
						//SoundManager.Haptic();
						break;
					}
					else if (((PredictWaterCount / DROPLET_PER_WATER) >= pSelectTube2->Size || (pSelectTube1->WaterList.back().Color != PredictColor && PredictColor != 0))
						&& !pSelectTube2->WaterList.empty() && pSelectTube2->Stage == STAGE_NORMAL)
					{
						// Data
						pSelectTube1->UnSelect();
						pSelectTube1 = pTubeList[i];
						pTubeList[i]->OnSelect();

						// Sound
						//SoundManager.Haptic();
						break;
					}
					else
					{
						// Data
						pSelectTube2 = pTubeList[i];
						if (pSelectTube1->IsDrop(pSelectTube2) || pSelectTube2->WaterList.empty()) SetDrop();
						pSelectTube1 = 0;

						// Sound
						//SoundManager.Haptic();
						break;
					}
				}
			}
		}
	}
}

void CGameWaterSort::OnKeyDown(EventKeyboard::KeyCode Key)
{
	if (Key == EventKeyboard::KeyCode::KEY_0)
	{
		//RestartGame();
		Level = 1;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_1)
	{
		Level++;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_2)
	{
		Level--;
		if (Level < 1) Level = 1;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_3)
	{
		Level += 10;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_4)
	{
		Level -= 10;
		if (Level < 1) Level = 1;
		SaveLevel();
		BeginLevel();
	}
	else if (Key == EventKeyboard::KeyCode::KEY_5)
	{
		WindowWin.Show();
	}
}

void CGameWaterSort::OnUpdate(float DeltaTime)
{
	// PlayTime
	if (!PluginAd.IsShow && !WindowWin.GetVisible()) PluginAd.PlayTime += DeltaTime;

	// Data
	bigint TimeThis = e::GetTime();
	int Turn = (TimeThis - TimeTurn) * TURN_PER_SECOND / 1000;
	if (TurnCount < Turn - TURN_PER_SECOND) TurnCount = Turn - TURN_PER_SECOND;
	while (TurnCount < Turn)
	{
		// Tube Turn
		for (int i = 0; i < pTubeList.size(); i++)
		{
			pTubeList[i]->Turn();

			// ImageLine
			bool IsLine = false;
			for (int k = 0; k < pTubeList.size(); k++)
			{
				if (k != i && pTubeList[k]->pTarget == pTubeList[i] && pTubeList[k]->Stage == STAGE_DROP)
				{
					IsLine = true;
					break;
				}
			}
			if (pTubeList[i]->Stage == STAGE_NORMAL)
			{
				if (pTubeList[i]->ImageLine.GetVisible() != IsLine) pTubeList[i]->ImageLine.SetVisible(IsLine);
			}
		}

		// Data
		TurnCount++;
	}

	// Booster
	if (CanUseBooster == false)
	{
		CanUseBooster = true;
		for (int i = 0; i < pTubeList.size(); i++)
		{
			if (pTubeList[i]->Stage != STAGE_NORMAL && pTubeList[i]->Stage != STAGE_SELECT)
			{
				CanUseBooster = false;
				break;
			}
		}
	}

	// Button Undo
	if (ButtonUndo.GetEnable() && GameDataList.size() > 1) InitButtonUndo();
	else InitButtonUndo();

	// Check End
	CheckEnd();

	// End
	if (IsEnd)
	{
		if (TimeFirework > 0 && e::GetTime() - TimeFirework > 500)
		{
			// Data
			TimeFirework = 0;

			// Effect
			Game.EffectManager.NewEffect(c::WINDOW_WIDTH/2, 0.8*c::WINDOW_HEIGHT)->CreateFirework();

			// Sound
			SoundManager.Play("game_win");
		}
		if (TimeLevelEnd > 0 && e::GetTime() - TimeLevelEnd > 1800)
		{
			// Data
			TimeLevelEnd = 0;

			// Window
			WindowWin.Show();

			// Show Ads
			ShowAdInterstitial();

			// Level
			Level++;
			SaveLevel();
		}
	}
}

void CGameWaterSort::SetDrop()
{
	// Backup
	int IndexDrop = -1, IndexReceive = -1;
	for (int i = 0; i < pTubeList.size(); i++)
	{
		CTube* pTube = pTubeList[i];
		if (pTube == pSelectTube1) IndexDrop = i;
		else if (pTube == pSelectTube2) IndexReceive = i;
	}
	CGameData GameData;
	GameData = GameDataList.back();
	GameData.Drop(IndexDrop, IndexReceive);
	GameDataList.push_back(GameData);

	// Data
	CanUseBooster = false;
	pSelectTube1->SetTarget(pSelectTube2);

	// Gui
	pSelectTube2->ImageLine.Update(e::Format("game_water_sort/water_%i.png", pSelectTube1->WaterList.back().Color));
}

void CGameWaterSort::ShowAdInterstitial()
{
	if (Level >= Client.AdLevel && PluginAd.PlayTime >= Client.AdTime)
	{
		if (IsUse) PluginAd.Show(TAG_INTERSTITIAL);
		else Game.IsShowAd = true;
	}
}

void CGameWaterSort::OnAdShowComplete(int Tag, bool IsReward)
{
	if (Tag == TAG_REWARDED)
	{
		if (IsReward)
		{
			if (Reward == REWARD_UNDO)
			{
				// Undo
				UndoCount = 5;
				UndoWater();
				InitButtonUndo();
			}
			else if (Reward == REWARD_ADD_TUBE)
			{
				// Sound
				SoundManager.Play("booster");

				// UnSelect
				if (pSelectTube1)
				{
					pSelectTube1->SetPosition(pSelectTube1->PosRoot);
					pSelectTube1->PosDes = pSelectTube1->PosRoot = pSelectTube1->Pos;
					pSelectTube1->SetStage(STAGE_NORMAL);
					pSelectTube1 = 0;
				}

				// Add Tube
				if (pTubeList.size() == TubeCount)
				{
					// Add Tube
					CTube* pTube = new CTube();
					pTube->Load(2);
					pTube->SetZDeep(0);
					pTube->ButtonTube.HandleEventCallback(this);
					FrameMap.AddChild(pTube);
					pTubeList.push_back(pTube);
					InitTubePosition();
					CImage ImageTube4;
					ImageTube4.Load(e::Format("game_water_sort/tube_%i.png", MAX_SIZE));
					ImageTube4.SetScale(c::GUI_SCALE);
					pTube->SetPosition(pTube->GetX(), pTube->GetY() + abs(ImageTube4.GetHeight() - pTube->ImageTube.GetHeight()));
					pTube->ButtonTube.SetPosition(0, pTube->ButtonTube.GetY() - abs(ImageTube4.GetHeight() - pTube->ImageTube.GetHeight())/2);
					pTube->ImageLine.SetPosition(CVec2(pTube->ButtonTube.GetX(), pTube->ButtonTube.GetY() - (pTube->ImageLine.GetHeight() - pTube->ButtonTube.GetHeight()) / 2 - 5 * c::GUI_SCALE));
					pTube->PosDes = pTube->PosRoot = pTube->Pos;

					// Backup
					CTubeData TubeData;
					TubeData.Size = pTube->Size;
					for (int i = 0; i < GameDataList.size(); i++)
					{
						GameDataList[i].TubeDataList.push_back(TubeData);
					}
				}
				else if (pTubeList.size() > TubeCount && pTubeList.back()->Size < MAX_SIZE)
				{
					// Add Tube
					CTube* pTube = pTubeList.back();
					float MaxHeightTube = pTube->ImageTube.GetHeight();
					pTube->Size = MAX_SIZE;
					pTube->ImageTube.Update(e::Format("game_water_sort/tube_%i.png", pTube->Size));
					pTube->SetPosition(CVec2(pTube->GetX(), pTube->GetY() - abs(MaxHeightTube - pTube->ImageTube.GetHeight())));
					pTube->PosDes = pTube->PosRoot = pTube->Pos;
					pTube->ButtonTube.SetPosition(0, pTube->ButtonTube.GetY() + abs(MaxHeightTube - pTube->ImageTube.GetHeight()));
					pTube->ImageLine.SetPosition(CVec2(pTube->ButtonTube.GetX(), pTube->ButtonTube.GetY() - (pTube->ImageLine.GetHeight() - pTube->ButtonTube.GetHeight()) / 2 - 5 * c::GUI_SCALE));
					pTube->DrawMask(pTube->Size);

					// Set Pos Water
					for (int i = 0; i < pTube->WaterList.size(); i++)
					{
						pTube->WaterList[i].Image.SetPosition(0, pTube->ClippingMask.getContentSize().height - i * pTube->WaterList[i].Image.pNode->getContentSize().height);
					}

					// Backup
					for (int i = 0; i < GameDataList.size(); i++)
					{
						GameDataList[i].TubeDataList.back().Size = MAX_SIZE;
					}
				}
				if (pTubeList.size() == TubeCount + 1 && pTubeList.back()->Size == 4)
				{
					// Gui
					ButtonAddTube.SetEnable(false);
				}
			}
		}
	}
}

void CGameWaterSort::ResetData()
{
	// Data
	pSelectTube1 = 0;
	pSelectTube2 = 0;
	GameDataList.clear();

	// Tube
	for (int i = 0; i < pTubeList.size(); i++) delete pTubeList[i];
	pTubeList.clear();
}

void CGameWaterSort::ResetAll()
{
	// Window
	if (WindowWin.IsCreate()) WindowWin.SetVisible(false);

	// Tube List
	FrameMap.RemoveAllChild();

	// Reset
	ResetData();
}
#endif
