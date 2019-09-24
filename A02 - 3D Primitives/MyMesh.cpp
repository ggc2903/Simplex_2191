#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------
	std::vector<vector3> basePoints; //List holding vector3 positions of all base circle points
	double triSize = 2 * PI / a_nSubdivisions; //Divides radius of circle up by number of subdivisions

	vector3 baseFirstPoint = vector3(0, -a_fHeight / 2, 0); //Store first point of triangle to be the base
	vector3 baseSecondPoint = vector3(a_fRadius, -a_fHeight / 2, 0); //Store second point of triangle
	vector3 topPoint = vector3(0, a_fHeight / 2, 0);

	for (int i = 0; i <= a_nSubdivisions; i++) //Loopg goes through and created a triangle based on the number of subdivisions
	{
		//Get the angle of the subdivison
		double angle = triSize * i;

		//Calculate the x and Z values of the new thirdPoint
		float baseNewX = a_fRadius * cos(angle);
		float baseNewZ = a_fRadius * sin(angle);

		//Use AddTri function to generate a triangle based on the generated thirdPoint
		AddTri(baseFirstPoint, baseSecondPoint, vector3(baseNewX, -a_fHeight / 2, baseNewZ));
		basePoints.push_back(baseSecondPoint);

		//Set the secondPoint equal to the newly generated point, so the next triangle will be created from the correct location.
		baseSecondPoint = vector3(baseNewX, -a_fHeight / 2, baseNewZ);
	}

	//Generate outer triangles that connect the topPoint to the basePoints
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddTri(topPoint, basePoints[0], basePoints[i]);
		}
		else
		{
			AddTri(topPoint, basePoints[i + 1], basePoints[i]);
		}
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------
	std::vector<vector3> basePoints; //List holding vector3 positions of all base circle points
	std::vector<vector3> topPoints; //List holding vector3 positions of all top circle points
	double triSize = 2 * PI / a_nSubdivisions; //Divides radius of circle up by number of subdivisions


	vector3 baseFirstPoint = vector3(0, -a_fHeight/2, 0); //Store first point of triangle to be the base
	vector3 baseSecondPoint = vector3(a_fRadius, -a_fHeight/2, 0); //Store second point of triangle

	for (int i = 0; i <= a_nSubdivisions; i++) //Loopg goes through and created a triangle based on the number of subdivisions
	{
		//Get the angle of the subdivison
		double angle = triSize * i;

		//Calculate the x and Z values of the new thirdPoint
		float baseNewX = a_fRadius * cos(angle);
		float baseNewZ = a_fRadius * sin(angle);

		//Use AddTri function to generate a triangle based on the generated thirdPoint
		AddTri(baseFirstPoint, baseSecondPoint, vector3(baseNewX, -a_fHeight/2, baseNewZ));
		basePoints.push_back(baseSecondPoint);

		//Set the secondPoint equal to the newly generated point, so the next triangle will be created from the correct location.
		baseSecondPoint = vector3(baseNewX, -a_fHeight/2, baseNewZ);
	}

	vector3 topFirstPoint = vector3(0, a_fHeight / 2, 0); //Store first point of triangle to be the base
	vector3 topSecondPoint = vector3(a_fRadius, a_fHeight / 2, 0); //Store second point of triangle

	for (int i = 0; i <= a_nSubdivisions; i++) //Loopg goes through and created a triangle based on the number of subdivisions
	{
		//Get the angle of the subdivison
		double angle = triSize * i;

		//Calculate the x and Z values of the new thirdPoint
		float topNewX = a_fRadius * cos(angle);
		float topNewZ = a_fRadius * sin(angle);

		//Use AddTri function to generate a triangle based on the generated thirdPoint
		AddTri(vector3(topNewX, a_fHeight / 2, topNewZ), topSecondPoint, topFirstPoint);
		topPoints.push_back(topSecondPoint);

		//Set the secondPoint equal to the newly generated point, so the next triangle will be created from the correct location.
		topSecondPoint = vector3(topNewX, a_fHeight / 2, topNewZ);
	}

	//Generate outer planes that connect topPoints to basePoints
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddQuad(topPoints[i], topPoints[0], basePoints[i], basePoints[0]);
		} 
		else 
		{
			AddQuad(topPoints[i], topPoints[i + 1], basePoints[i], basePoints[i + 1]);
		}
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------
	std::vector<vector3> outerBasePoints; //List holding vector3 positions of all outer base circle points
	std::vector<vector3> outerTopPoints; //List holding vector3 positions of all outer top circle points

	std::vector<vector3> innerBasePoints; //List holding vector3 positions of all inner base circle points
	std::vector<vector3> innerTopPoints; //List holding vector3 positions of all inner top circle points

	double triSize = 2 * PI / a_nSubdivisions; //Divides radius of circle up by number of subdivisions


	vector3 baseFirstPoint = vector3(0, -a_fHeight / 2, 0); //Store first point of triangle to be the base
	vector3 outerBaseSecondPoint = vector3(a_fOuterRadius, -a_fHeight / 2, 0); //Store second point of triangle for outer ring
	vector3 innerBaseSecondPoint = vector3(a_fInnerRadius, -a_fHeight / 2, 0); //Store second point of triangle for inner ring

	for (int i = 0; i <= a_nSubdivisions; i++) //Loopg goes through and created a triangle based on the number of subdivisions
	{
		//Get the angle of the subdivison
		double angle = triSize * i;

		//Calculate the x and Z values of the new outer thirdPoint
		float outerBaseNewX = a_fOuterRadius * cos(angle);
		float outerBaseNewZ = a_fOuterRadius * sin(angle);

		//Calculate the x and Z values of the new inner thirdPoint
		float innerBaseNewX = a_fInnerRadius * cos(angle);
		float innerBaseNewZ = a_fInnerRadius * sin(angle);

		outerBasePoints.push_back(outerBaseSecondPoint);
		innerBasePoints.push_back(innerBaseSecondPoint);

		//Set the secondPoint equal to the newly generated point, so the next triangle will be created from the correct location.
		outerBaseSecondPoint = vector3(outerBaseNewX, -a_fHeight / 2, outerBaseNewZ);
		innerBaseSecondPoint = vector3(innerBaseNewX, -a_fHeight / 2, innerBaseNewZ);
	}

	vector3 topFirstPoint = vector3(0, a_fHeight / 2, 0); //Store first point of triangle to be the base
	vector3 outerTopSecondPoint = vector3(a_fOuterRadius, a_fHeight / 2, 0); //Store second point of triangle
	vector3 innerTopSecondPoint = vector3(a_fInnerRadius, a_fHeight / 2, 0); //Store second point of triangle

	for (int i = 0; i <= a_nSubdivisions; i++) //Loopg goes through and created a triangle based on the number of subdivisions
	{
		//Get the angle of the subdivison
		double angle = triSize * i;

		//Calculate the x and Z values of the new thirdPoint
		float outerTopNewX = a_fOuterRadius * cos(angle);
		float outerTopNewZ = a_fOuterRadius * sin(angle);

		float innerTopNewX = a_fInnerRadius * cos(angle);
		float innerTopNewZ = a_fInnerRadius * sin(angle);

		outerTopPoints.push_back(outerTopSecondPoint);
		innerTopPoints.push_back(innerTopSecondPoint);

		//Set the secondPoint equal to the newly generated point, so the next triangle will be created from the correct location.
		outerTopSecondPoint = vector3(outerTopNewX, a_fHeight / 2, outerTopNewZ);
		innerTopSecondPoint = vector3(innerTopNewX, a_fHeight / 2, innerTopNewZ);
	}

	//Generate planes for top circle
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddQuad(innerTopPoints[i], innerTopPoints[0], outerTopPoints[i], outerTopPoints[0]);
		}
		else
		{
			AddQuad(innerTopPoints[i], innerTopPoints[i + 1], outerTopPoints[i], outerTopPoints[i + 1]);
		}
	}

	//Generate planes for bottom circle
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddQuad(outerBasePoints[i], outerBasePoints[0], innerBasePoints[i], innerBasePoints[0]);
		}
		else
		{
			AddQuad(outerBasePoints[i], outerBasePoints[i + 1], innerBasePoints[i], innerBasePoints[i + 1]);
		}
	}

	//Generate outer planes that connect outerTopPoints to outerBasePoints
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddQuad(outerTopPoints[i], outerTopPoints[0], outerBasePoints[i], outerBasePoints[0]);
		}
		else
		{
			AddQuad(outerTopPoints[i], outerTopPoints[i + 1], outerBasePoints[i], outerBasePoints[i + 1]);
		}
	}

	//Generate inner planes that connect innerTopPoints to innerBasePoints
	for (int i = 0; i <= a_nSubdivisions; i++)
	{
		if (i == a_nSubdivisions)
		{
			AddQuad(outerTopPoints[0], outerTopPoints[i], outerBasePoints[0], outerBasePoints[i]);
		}
		else
		{
			AddQuad(outerTopPoints[i + 1], outerTopPoints[i], outerBasePoints[i + 1], outerBasePoints[i]);
		}
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}