#include "engine-precompiled-header.h"
#include "EnginePerformanceMonitor.h"
#include "../BaseEngineWidgetManager.h"
#include <imgui/addons/implot/implot.h>

longmarch::EnginePerformanceMonitor::EnginePerformanceMonitor()
{
	m_IsVisible = true;
	m_Size = ScaleSize({ 500, 400 });
}

void longmarch::EnginePerformanceMonitor::Render()
{
	// this is rendered in a table format
	//      col#1                         col#2
	//	|------------|-------------------------------------------|
	//	|FPS         |                                           |  // row#1
	//	|            |    graph widgets here occupy both rows    |
	//	|FRAME TIME  |                                           |  // row#2
	//	|--------------------------------------------------------|

	auto manager = ServiceLocator::GetSingleton<BaseEngineWidgetManager>(ENG_WIG_MAN_NAME);
	manager->PushWidgetStyle();

	static bool showFPS = false;
	static bool showFrameTime = false;
	float frameTime = FramerateController::GetInstance()->GetFrameTime();
	float frameRate = (1.0f / frameTime);
	ImGui::Begin("Engine Performance");
	ImGui::Columns(2);						// table has two columns
	ImGui::SetColumnWidth(-1, 220);			// width of the first column

											// column#1 | row#1 - starts
	if (ImGui::RadioButton("##FPS", showFPS == true))
	{
		showFPS = true;
		showFrameTime = false;
	}
	ImGui::SameLine();
	ImGui::Text("FRAME RATE: %.2f fps", frameRate);
	// column#1 | row#1 - ends

	ImGui::Text("");		// spacing between two adjacent rows

							// column#1 | row#2 - starts
	if (ImGui::RadioButton("##FrameTime", showFrameTime == true))
	{
		showFrameTime = true;
		showFPS = false;
	}
	ImGui::SameLine();
	ImGui::Text("FRAME TIME: %.2f ms", frameTime * 1e3);
	// column#1 | row#2 - ends

	ImGui::NextColumn();		// next column starts here

								// both graphs widgets occupy both rows (2nd column has only one row or rows are merged)
	{
		static ScrollingBuffer buffer;
		static float t = 0;
		t += ImGui::GetIO().DeltaTime;
		buffer.AddPoint(t, frameRate);
		if (showFPS) // render FPS
		{
			static float history = 10.0f;
			ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");

			static ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
			ImPlot::SetNextPlotLimitsX(t - history, t, ImGuiCond_Always);
			ImPlot::SetNextPlotLimitsY(0, 120);
			if (ImPlot::BeginPlot("##Scrolling", NULL, NULL, ImVec2(-1, 200), 0, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_LockMin)) {
				ImPlot::PlotLine("FPS", &buffer.Data[0].x, &buffer.Data[0].y, buffer.Data.size(), buffer.Offset, 2 * sizeof(float));
				ImPlot::EndPlot();
			}
		}
	}
	{
		static ScrollingBuffer buffer;
		static float t = 0;
		t += ImGui::GetIO().DeltaTime;
		buffer.AddPoint(t, frameTime * 1e3);
		if (showFrameTime) // render frame-time
		{
			static float history = 10.0f;
			ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");

			static ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
			ImPlot::SetNextPlotLimitsX(t - history, t, ImGuiCond_Always);
			ImPlot::SetNextPlotLimitsY(0, 60);
			if (ImPlot::BeginPlot("##Scrolling", NULL, NULL, ImVec2(-1, 200), 0, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_LockMin)) {
				ImPlot::PlotLine("FRAME TIME", &buffer.Data[0].x, &buffer.Data[0].y, buffer.Data.size(), buffer.Offset, 2 * sizeof(float));
				ImPlot::EndPlot();
			}
		}
	}
	ImGui::Columns(1); // resetting the number of columns to 1

	manager->CaptureMouseAndKeyboardOnHover();
	manager->PopWidgetStyle();
	ImGui::End();
}
