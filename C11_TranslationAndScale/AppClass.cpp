#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	//m_pMesh->GenerateSphere(1.0f, 5, C_WHITE);

	meshListCount = 46;
	for (int i = 0; i < meshListCount; i++)
	{
		MyMesh* m_pMesh = new MyMesh();
		MeshList.push_back(m_pMesh);
		m_pMesh->GenerateCube(1.0f, C_BLACK);
	}

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	
	matrix4 m4Scale = glm::scale(IDENTITY_M4, vector3(2.0f,2.0f,2.0f));

	static float overallXValue = 0.0f;
	static float overallYValue = 0.0f;
	overallXValue += 0.1f;

	static float xValue = 0.0f;
	static float yValue = 0.0f;

	matrix4 m4Translate;
	matrix4 m4Model;

	//matrix4 m4Model = m4Translate * m4Scale;

	for (int i = 0; i < meshListCount; i++)
	{
		xValue = 0.0f;
		if (i < 2)
		{
			yValue = 5.0f;
			if (i < 1)
			{
				xValue += -3.0f;
			}
			else 
			{
				xValue += 3.0f;
			}
		}
		else if (i >= 2 && i < 4)
		{
			yValue = 4.0f;
			if (i < 3)
			{
				xValue += -2.0f;
			}
			else
			{
				xValue += 2.0f;
			}
		}
		else if (i >= 4 && i < 11)
		{
			yValue = 3.0f;
			xValue += 1.0f * i - 7.0f;
		}
		else if (i >= 11 && i < 18)
		{
			yValue = 2.0f;
			xValue += 1.0f * i - 15.0f;
			if (i > 12)
			{
				xValue += 1.0f;
			}
			if (i > 15)
			{
				xValue += 1.0f;
			}
		}
		else if (i >= 18 && i < 29)
		{
			yValue = 1.0f;
			xValue += 1.0f * i - 23.0f;
		}
		else if (i >= 29 && i < 38)
		{
			yValue = 0.0f;
			xValue += 1.0f * i - 34.0f;
			if (i > 29)
			{
				xValue += 1.0f;
			}
			if (i > 36)
			{
				xValue += 1.0f;
			}
		}
		else if (i >= 38 && i < 42)
		{
			yValue = -1.0f;
			xValue += 1.0f * i - 43.0f;
			if (i > 38)
			{
				xValue += 1.0f;
			}
			if (i > 39)
			{
				xValue += 5.0f;
			}
			if (i > 40)
			{
				xValue += 1.0f;
			}
		}
		else if (i >= 42)
		{
			yValue = -2.0f;
			xValue += 1.0f * i - 44.0f;
			if (i > 43)
			{
				xValue += 1.0f;
			}
		}

		m4Translate = glm::translate(IDENTITY_M4, vector3(overallXValue + xValue, yValue, -3.0f));

		m4Model = m4Scale * m4Translate;
	
		MeshList[i]->Render(m4Projection, m4View, m4Model);
	}

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}