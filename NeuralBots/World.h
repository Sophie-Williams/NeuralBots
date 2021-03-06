#pragma once
#include "Drawing.h"
#include "Bot.h"
#include <time.h>

#define WORLD_SIZE_X 3000
#define WORLD_SIZE_Y 3000
#define WALLS_THICKNESS 50
#define GRID_SCALE 100

#define BOTS_COUNT 20

class World
{
public:

	World()
		: m_dTime(1.0f)
	{
		PhysObj* pPhys = new PhysObj(Vector2D(WORLD_SIZE_X / 2, 0), WORLD_SIZE_X, WALLS_THICKNESS);
		pPhys->SetStatic();
		m_PhysObjs.push_back(pPhys);

		pPhys = new PhysObj(Vector2D(0, WORLD_SIZE_Y / 2), WALLS_THICKNESS, WORLD_SIZE_Y);
		pPhys->SetStatic();
		m_PhysObjs.push_back(pPhys);

		pPhys = new PhysObj(Vector2D(WORLD_SIZE_X / 2, WORLD_SIZE_Y), WORLD_SIZE_X, WALLS_THICKNESS);
		pPhys->SetStatic();
		m_PhysObjs.push_back(pPhys);
		
		pPhys = new PhysObj(Vector2D(WORLD_SIZE_X, WORLD_SIZE_Y / 2), WALLS_THICKNESS, WORLD_SIZE_Y);
		pPhys->SetStatic();
		m_PhysObjs.push_back(pPhys);

		LoadRoom();
	}

	void LoadRoom()
	{
		for (int i = 0; i < 40; i++)
		{
			float w = round(frand(GRID_SCALE, 500) / GRID_SCALE) * GRID_SCALE;
			float h = round(frand(GRID_SCALE, 500) / GRID_SCALE) * GRID_SCALE;

			PhysObj* pPhys = new PhysObj(
				Vector2D(
				round(frand(0, WORLD_SIZE_X) / GRID_SCALE) * GRID_SCALE + w / 2,
				round(frand(0, WORLD_SIZE_Y) / GRID_SCALE) * GRID_SCALE + h / 2),
				w, h);

			pPhys->SetStatic();
			m_PhysObjs.push_back(pPhys);
		}
	}

	// Size
	float width() { return WORLD_SIZE_X; }
	float height() { return WORLD_SIZE_Y; }

	void AddPhysObj(PhysObj* pObj)
	{
		m_PhysObjs.push_back(pObj);
	}

	void AddDanger(PhysObj* pObj)
	{
		m_Dangers.push_back(pObj);
	}

	// Removing functions
	void Remove(PhysObj* pPhysObj)
	{
		for (int i = 0; i < m_PhysObjs.size(); i++)
			if (m_PhysObjs[i] == pPhysObj)
				m_PhysObjs.erase(m_PhysObjs.begin() + i);
		pPhysObj->Remove();
	}

	void Remove(Bot* pBot);
	void RemoveD(PhysObj* pDanger);
	void RemoveSavedBot(Bot* pBot);

	void Draw(const Camera& camera);

	void AddBot();
	void AddBot(Bot* pBot1, Bot* pBot2);
	void Step();

	bool CheckPointForSolid(const Vector2D& point, bool onlyStatic = true);
	bool CheckLineForCollision(const Vector2D& startpos, const Vector2D& endpos, bool onlyStatic = true);
	const std::vector<Bot*>& GetBots() { return m_Bots; }
	const std::vector<PhysObj*>& GetDangers() { return m_Dangers; }
	const std::vector<Bot*>& GetSavedBots() { return m_SavedBots; }

private:

	bool Solve(PhysObj* pObj1, PhysObj* pObj2);
	bool HandlePhysObj(PhysObj* pObj);
	bool GetFitnessesBots(Bot*& pBot1, Bot*& pBot2);
	void ClearSavedNonAliveBots(Bot* pBot1, Bot* pBot2);

	// fields
	std::vector<Bot*> m_Bots;
	std::vector<Bot*> m_SavedBots;
	std::vector<PhysObj*> m_Dangers;
	std::vector<PhysObj*> m_PhysObjs;
	time_t m_LastTime;
	float m_dTime;
};