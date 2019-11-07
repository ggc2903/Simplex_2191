#include "MyOctant.h"
using namespace Simplex;

//Static variables from header file
uint MyOctant::m_uMyOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

//Accessors
//Method to get all total number of MyOctants in the world
uint MyOctant::GetMyOctantCount()
{
	return m_uMyOctantCount;
}

//Method to return parent of MyOctant
MyOctant* MyOctant::GetParent()
{
	return m_pParent;
}

//Method to return the child MyOctant specified as a parameter
MyOctant* MyOctant::GetChild(uint a_nChild)
{
	//If the parameter is larger than the possible number of octants, stop right there criminal scum
	if (a_nChild > 7)
	{
		return nullptr;
	}

	return m_pChild[a_nChild];
}

//Method to return size of MyOctant
float MyOctant::GetSize()
{
	return m_fSize;
}

//Method to return center of MyOctant in global space
vector3 MyOctant::GetCenterGlobal()
{
	return m_v3Center;
}

//Method to return minimum value of MyOctant in global space
vector3 MyOctant::GetMinGlobal()
{
	return m_v3Min;
}

//Method to return maximum value of MyOctant in global space
vector3 MyOctant::GetMaxGlobal()
{
	return m_v3Max;
}

//Method to return whether or not this MyOctant is a leaf
bool MyOctant::IsLeaf()
{
	//If this octant has no children, then it is a leaf
	if (m_uChildren == 0)
	{
		return true;
	}
	return false;
}

//Method to return whether this MyOctant has more entities than the parameter
bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	uint count = 0;
	uint entityCount = m_pEntityMngr->GetEntityCount();

	for (uint i = 0; i < entityCount; i++)
	{
		if (IsColliding(i))
		{
			count++;
		}
		if (count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}

//Private methods

//Init() Allocates member fields
void MyOctant::Init()
{
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_uID = m_uMyOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

//Release() Deallocates member fields
void MyOctant::Release()
{
	if (m_uLevel == 0)
	{
		//If the level is the root, kill all of the branches first
		KillBranches();
	}

	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

//ConstructList() Creates the list of all leafs that contain objects
void MyOctant::ConstructList()
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}

//Public Methods

//Constructors and Rule of Threes
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//Calling Init() to initialize values
	Init();

	m_uMyOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uMyOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	//Create a list to hold all Min/Max values of MyOctant
	std::vector<vector3> lMinMax;

	//Create variable 
	//uint entityCount

	for (uint i = 0; i < m_pEntityMngr->GetEntityCount(); i++)
	{
		MyEntity* theEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* theRigidBody = theEntity->GetRigidBody();
		lMinMax.push_back(theRigidBody->GetMinGlobal());
		lMinMax.push_back(theRigidBody->GetMaxGlobal());
	}

	MyRigidBody* theRigidBody = new MyRigidBody(lMinMax);
	vector3 halfWidth = theRigidBody->GetHalfWidth();
	float halfWidthX = halfWidth.x;

	for (int i = 1; i < 3; i++)
	{
		if (halfWidthX < halfWidth[i])
		{
			halfWidthX = halfWidth[i];
		}
	}

	vector3 v3Center = theRigidBody->GetCenterLocal();

	lMinMax.clear();
	SafeDelete(theRigidBody);

	//Set min/max
	m_fSize = halfWidthX * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(halfWidthX));
	m_v3Max = m_v3Center + (vector3(halfWidthX));

	//Increment Octant count as an Octant was added! Yay! (I hope)
	m_uMyOctantCount++;
	ConstructTree(m_uMaxLevel);
}
MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	//Calling Init() to initialize values
	Init();

	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//Set min/max
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	//Increment Octant count as an Octant was added
	m_uMyOctantCount++;
}
//Copy Constructor
MyOctant::MyOctant(MyOctant const& other)
{
	//Basically doing what Init() does, but just copying values over from the passed in MyOctant instead of initializing default values

	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot, other.m_pRoot;
	m_pParent = other.m_pParent;
	m_lChild, other.m_lChild;

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
}
//Copy Assignment Operator and Swap Method
MyOctant& MyOctant::operator=(MyOctant const& other)
{
	//Stop immediately and do nothing if they already are the same thing
	if (this == &other)
	{
		return *this;
	}

	Release();
	Init();
	MyOctant anOctant(other);
	Swap(anOctant);
}
void MyOctant::Swap(MyOctant& other)
{
	//The same thing as the copy constructor, but using std::swap?
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_lChild, other.m_lChild);

	for (uint i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

//Destructor
MyOctant::~MyOctant()
{
	Release();
};

//Display Methods (Recursively displaying wireframes)
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint leafCount = m_lChild.size();

	for (uint i = 0; i < leafCount; i++)
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

//Other Methods
bool MyOctant::IsColliding(uint a_uRBIndex)
{
	//When the parameter is larger than the existing count of entities, there cant be a collision
	if (a_uRBIndex >= m_pEntityMngr->GetEntityCount())
	{
		return false;
	}

	//Get all global values so things can be compared
	MyEntity* theEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* theRigidBody = theEntity->GetRigidBody();
	vector3 minGlobal = theRigidBody->GetMinGlobal();
	vector3 maxGlobal = theRigidBody->GetMaxGlobal();

	//Check X values
	if (m_v3Max.x < minGlobal.x)
	{
		return false;
	}
	if (m_v3Min.x > maxGlobal.x)
	{
		return false;
	}
	//Check Y values
	if (m_v3Max.y < minGlobal.y)
	{
		return false;
	}
	if (m_v3Min.y > maxGlobal.y)
	{
		return false;
	}
	//Check Z values
	if (m_v3Max.z < minGlobal.z)
	{
		return false;
	}
	if (m_v3Min.z > maxGlobal.z)
	{
		return false;
	}

	//Return true after everything else has be passed over
	return true;
}

void MyOctant::ClearEntityList()
{
	//Recursively clear entity lists
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}

void MyOctant::Subdivide()
{
	//Don't subdivide if you've reached the max level
	if (m_uLevel >= m_uMaxLevel)
	{
		return;
	}

	//Don't subdivide if it's already has children
	if (m_uChildren != 0)
	{
		return;
	}

	m_uChildren = 8;

	float size = m_fSize / 4.0f;
	float doubledSize = size * 2.0f;
	vector3 center = m_v3Center;

	//Building the 8 Octant Children

	//Bottom
	//------Back
	//----------Left
	center.x -= size;
	center.y -= size;
	center.z -= size;
	m_pChild[0] = new MyOctant(center, doubledSize);
	//----------Right
	center.x += doubledSize;
	m_pChild[1] = new MyOctant(center, doubledSize);
	//------Front
	//----------Right
	center.z += doubledSize;
	m_pChild[2] = new MyOctant(center, doubledSize);
	//----------Left
	center.x -= doubledSize;
	m_pChild[3] = new MyOctant(center, doubledSize);

	//Top
	//------Front
	//----------Left
	center.y += doubledSize;
	m_pChild[4] = new MyOctant(center, doubledSize);
	//----------Right
	center.x += doubledSize;
	m_pChild[5] = new MyOctant(center, doubledSize);
	//------Back
	//----------Right
	center.z -= doubledSize;
	m_pChild[6] = new MyOctant(center, doubledSize);
	//----------Left
	center.x -= doubledSize;
	m_pChild[7] = new MyOctant(center, doubledSize);

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

void MyOctant::KillBranches()
{
	//Recursively delete all children off of the branch this is called on
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	//Since all children are deleted, this octant no longer has children
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//This should only be called on the root
	if (m_uLevel != 0)
	{
		return;
	}

	m_uMaxLevel = a_nMaxLevel;

	m_uMyOctantCount = 1;

	m_EntityList.clear();

	//Subdivide if the tree contains more than the ideal count
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	AssignIDtoEntity();

	ConstructList();
}

void MyOctant::AssignIDtoEntity()
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0)
	{
		uint entityCount = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < entityCount; i++)
		{
			m_EntityList.push_back(i);
			m_pEntityMngr->AddDimension(i, m_uID);
		}
	}
}
