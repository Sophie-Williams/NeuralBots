#include "World.h"


void World::Draw(const Camera& camera)
{
	// Drawing Objects
	for (PhysObj* pPhysObj : m_PhysObjs)
	{
		pPhysObj->Draw(camera, m_dTime);
	}
}


bool World::CheckPointForSolid(const Vector2D& point, bool onlyStatic)
{
	for (PhysObj* pPhysObj : m_PhysObjs)
	{
		if (onlyStatic && !pPhysObj->IsStatic())
			continue;

		Vector2D p = pPhysObj->GetPosition();
		Vector2D s = Vector2D( pPhysObj->GetWidth(), pPhysObj->GetHeight() ) / 2.0f;

		if (point.x >(p.x - s.x) && point.x < (p.x + s.x)
			&& point.y > (p.y - s.y) && point.y < (p.y + s.y))
		{
			return true;
		}
	}
	return false;
}

// LINE/LINE
bool lineLine(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {

	// calculate the direction of the lines
	float uA = ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3)) / ((y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1));
	float uB = ((x2 - x1)*(y1 - y3) - (y2 - y1)*(x1 - x3)) / ((y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1));

	// if uA and uB are between 0-1, lines are colliding
	if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) {

		//// optionally, draw a circle where the lines meet
		//float intersectionX = x1 + (uA * (x2 - x1));
		//float intersectionY = y1 + (uA * (y2 - y1));

		return true;
	}
	return false;
}

// LINE/RECTANGLE
bool lineRect(float x1, float y1, float x2, float y2, float rx, float ry, float rw, float rh) {

	// check if the line has hit any of the rectangle's sides
	// uses the Line/Line function below
	if (lineLine(x1, y1, x2, y2, rx, ry, rx, ry + rh))
		return true;

	if (lineLine(x1, y1, x2, y2, rx + rw, ry, rx + rw, ry + rh))
		return true;

	if (lineLine(x1, y1, x2, y2, rx, ry, rx + rw, ry))
		return true;

	if (lineLine(x1, y1, x2, y2, rx, ry + rh, rx + rw, ry + rh))
		return true;

	return false;
}

bool World::CheckLineForCollision(const Vector2D& startpos, const Vector2D& endpos, bool onlyStatic)
{
	for (PhysObj* pPhysObj : m_PhysObjs)
	{
		if (onlyStatic && !pPhysObj->IsStatic())
			continue;

		Vector2D pos = pPhysObj->GetPosition();
		Vector2D size = Vector2D(pPhysObj->GetWidth(), pPhysObj->GetHeight());
		//if (GetDistanceToSphere(startpos, endpos, pPhysObj->GetPosition(), pPhysObj->GetRadius()) < pPhysObj->GetRadius())
			if (lineRect(startpos.x, startpos.y, endpos.x, endpos.y, pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y))
				return true;
	}
	return false;
}


bool World::Solve(PhysObj* pObj1, PhysObj* pObj2)
{
	// Skipping if both of objects is static
	if (pObj1->IsStatic() && pObj2->IsStatic())
		return false;

	if (Bullet* pBull = dynamic_cast<Bullet*>(pObj1))
	{
		if (Bot* pBot = dynamic_cast<Bot*>(pObj2))
			if (pBot == pBull->GetOwner())
				return false;
	}
	else if (Bullet* pBull = dynamic_cast<Bullet*>(pObj2))
	{
		if (Bot* pBot = dynamic_cast<Bot*>(pObj1))
			if (pBot == pBull->GetOwner())
				return false;
	}

	Vector2D n = pObj1->GetPosition() - pObj2->GetPosition();

	// Calculate overlap on x axis
	float x_overlap = (pObj1->GetWidth() + pObj2->GetWidth()) / 2 - abs(n.x);

	Vector2D penetration = Vector2D(0, 0);

	// SAT test on x axis
	if (x_overlap > 0)
	{
		// Calculate overlap on y axis
		float y_overlap = (pObj1->GetHeight() + pObj2->GetHeight()) / 2 - abs(n.y);

		// SAT test on y axis
		if (y_overlap > 0)
		{
			// Find out which axis is axis of least penetration
			if (x_overlap < y_overlap)
			{
				// Point towards B knowing that n points from A to B
				char dir = n.x < 0 ? -1 : 1;
				//x_overlap -= 0.01;
				penetration = Vector2D(x_overlap * dir, 0);
			}
			else
			{
				// Point toward B knowing that n points from A to B
				char dir = n.y < 0 ? -1 : 1;
				//y_overlap -= 0.01;
				penetration = Vector2D(0, y_overlap * dir);
			}
		}
	}

	if (penetration == Vector2D(0, 0))
		return false;

	Vector2D dir = penetration.GetNormalize();

	if (!pObj1->IsStatic())
	{
		Vector2D pVel = pObj1->GetVelocity();
		pObj1->SetPosition(pObj1->GetPosition() + penetration);

		if (dir.x)
			pObj1->SetVelocity(Vector2D(0, fmin(pVel.y, pVel.y / 2.0f)));
		else if (dir.y)
			pObj1->SetVelocity(Vector2D(pVel.x, 0));

		// friction
		pObj1->SetVelocity(Vector2D(pObj1->GetVelocity().x / 1.1f, pObj1->GetVelocity().y));
	}

	if (!pObj2->IsStatic())
	{
		Vector2D pVel = pObj2->GetVelocity();
		pObj2->SetPosition(pObj2->GetPosition() - penetration);

		if (dir.x)
			pObj2->SetVelocity(Vector2D(0, fmin(pVel.y, pVel.y / 2.0f)));
		else if (dir.y)
			pObj2->SetVelocity(Vector2D(pVel.x, 0));

		// friction
		pObj1->SetVelocity(Vector2D(pObj1->GetVelocity().x / 1.1f, pObj1->GetVelocity().y));
	}

	return true;
}


bool World::HandlePhysObj(PhysObj* pObj)
{
	// Removing objects that is out of bounds
	Vector2D pos = pObj->GetPosition();
	if (pos.x < 0 || pos.x > WORLD_SIZE_X
		|| pos.y < 0 || pos.y > WORLD_SIZE_Y)
	{
		if (Bullet* pB = dynamic_cast<Bullet*>(pObj))
			RemoveD(pB);
		else if (Bot* pB = dynamic_cast<Bot*>(pObj))
			Remove(pB);
		else
			Remove(pObj);

		return false;
	}

	return true;
}


// Getting two best bots from simulation by their fitness
bool World::GetFitnessesBots(Bot*& pBot1, Bot*& pBot2)
{
	double fitness1 = -1;
	double fitness2 = -1;
	for (Bot* pBot : m_SavedBots)
	{
		if (pBot->GetBrain()->GetFitness() > fitness1)
		{
			fitness1 = pBot->GetBrain()->GetFitness();
			pBot1 = pBot;
		}
	}

	for (Bot* pBot : m_SavedBots)
	{
		if (pBot->GetBrain()->GetFitness() > fitness2 
			&& pBot != pBot1)
		{
			fitness2 = pBot->GetBrain()->GetFitness();
			pBot2 = pBot;
		}
	}

	if (pBot1 == NULL || pBot2 == NULL)
		return false;

	return true;
}


// Clear saved bots
void World::ClearSavedNonAliveBots(Bot* pBot1, Bot* pBot2)
{
	for (int i = 0; i < m_SavedBots.size(); i++)
	{
		Bot* pBot = m_SavedBots[i];
		if (pBot1 == pBot || pBot2 == pBot)
			continue;

		if (std::find(m_Bots.begin(), m_Bots.end(), pBot) == m_Bots.end())
		{
			pBot->Remove();
			m_SavedBots.erase(m_SavedBots.begin() + i);
			i--;
		}
	}
}


void World::Step()
{
	float m_dTime = (clock() - m_LastTime);
	m_LastTime = clock();

	if (m_Bots.size() < BOTS_COUNT)
	{
		if (frand(0, 1) < 0.1f)
		{
			Bot* pBot1 = NULL;
			Bot* pBot2 = NULL;
			GetFitnessesBots(pBot1, pBot2);
			AddBot(pBot1, pBot2);

			std::cout << "Reproduced bot created\n";
			// Clear
			ClearSavedNonAliveBots(pBot1, pBot2);
		}
		else
			AddBot();
	}

	// Handling Cell Creature
	for (Bot* pBot : m_Bots)
	{
		pBot->Step(this, m_dTime);
	}


	// Solving Phys Objects
	for (int i = 0; i < m_PhysObjs.size(); i++)
	{
		PhysObj* pObj1 = m_PhysObjs[i];
		pObj1->RunPhys(m_dTime);

		if (!HandlePhysObj(pObj1))
			continue;

		for (int j = (i + 1); j < m_PhysObjs.size(); j++)
		{
			PhysObj* pObj2 = m_PhysObjs[j];
			assert(pObj1 || pObj2);

			//if (pObj1 == NULL)
			//	break;

			if (!Solve(pObj1, pObj2))
				continue;

			// Handling Bullet
			Bot* pBotToKill = NULL;
			Bullet* pBullet = NULL;

			if (Bullet* pBull = dynamic_cast<Bullet*>(pObj1))
			{
				pBullet = pBull;
				if (Bot* pBot = dynamic_cast<Bot*>(pObj2))
					pBotToKill = pBot;
			}
			else if (Bullet* pBull = dynamic_cast<Bullet*>(pObj2))
			{
				pBullet = pBull;
				if (Bot* pBot = dynamic_cast<Bot*>(pObj1))
					pBotToKill = pBot;
			}

			if (pBullet)
			{
				if (pBotToKill)
				{
					if (pBullet->GetOwner() != pBotToKill)
					{
						// Adding fittness to owner of the bullet
						if (std::find(m_Bots.begin(), m_Bots.end(), pBullet->GetOwner()) != m_Bots.end())
							pBullet->GetOwner()->GetBrain()->AddFitness(1);

						Remove(pBotToKill);
						RemoveD(pBullet);
						break;
					}
				}
				else
				{
					RemoveD(pBullet);
				}
			}
		}
	}
}


void World::AddBot()
{
	while (true)
	{
		Vector2D pos = Vector2D((0.5 + frand(-0.5, 0.5)) * WORLD_SIZE_X, (0.5 + frand(-0.5, 0.5)) * WORLD_SIZE_Y);
		if (!CheckPointForSolid(pos))
		{
			Bot* pBot = new Bot(pos);
			m_Bots.push_back(pBot);
			m_PhysObjs.push_back(pBot);
			m_SavedBots.push_back(pBot);
			break;
		}
	}
}


void World::AddBot(Bot* pBot1, Bot* pBot2)
{
	assert(pBot1, pBot2);
	while (true)
	{
		Vector2D pos = Vector2D((0.5 + frand(-0.5, 0.5)) * WORLD_SIZE_X, (0.5 + frand(-0.5, 0.5)) * WORLD_SIZE_Y);
		if (!CheckPointForSolid(pos))
		{
			Bot* pBot = new Bot(pos, pBot1, pBot2);
			m_Bots.push_back(pBot);
			m_PhysObjs.push_back(pBot);
			m_SavedBots.push_back(pBot);
			break;
		}
	}
}


void World::Remove(Bot* pBot)
{
	m_PhysObjs.erase(std::remove(m_PhysObjs.begin(), m_PhysObjs.end(), pBot), m_PhysObjs.end());
	m_Bots.erase(std::remove(m_Bots.begin(), m_Bots.end(), pBot), m_Bots.end());
	//pBot->Remove();
}


void World::RemoveSavedBot(Bot* pBot)
{
	m_SavedBots.erase(std::remove(m_SavedBots.begin(), m_SavedBots.end(), pBot), m_SavedBots.end());
	pBot->Remove();
}


void World::RemoveD(PhysObj* pDanger)
{
	m_PhysObjs.erase(std::remove(m_PhysObjs.begin(), m_PhysObjs.end(), pDanger), m_PhysObjs.end());
	m_Dangers.erase(std::remove(m_Dangers.begin(), m_Dangers.end(), pDanger), m_Dangers.end());
	pDanger->Remove();
}