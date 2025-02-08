#ifndef FRMWEBCAMSETUP_H
#define FRMWEBCAMSETUP_H
#include <wx/app.h> // Required for wxTheApp and CallAfter
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/timer.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/sizer.h> // REQUIRED: For wxBoxSizer
#include <wx/dcmemory.h>
#include <wx/msgdlg.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // For file system operations in C++17
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "nemslib_webcam.h"
class frmWebCamSetup : public wxFrame {
private:
    // Widgets
    wxStaticBitmap* bitmap_video_clip;
    wxStaticLine* m_staticline3;
    wxStaticText* lblWebCam;
    wxChoice* choice_webcam_index;
    wxStaticLine* m_staticline4;
    wxButton* btExit;
    // Dynamic webcam helper library
    nemslib_webcam nemswebcam_j;
    // Webcam configuration data
    std::string webcam_index;
    // Helper methods
    void PopulateWebcamChoice();
    wxBitmap GetPlaceholderBitmap();
    // Event handlers
	cv::VideoCapture cap;   // OpenCV video capture object
	wxTimer* video_timer = nullptr;   // Timer to refresh frames periodically
	void start_web_cam();
	void OnChoiceChanged(wxCommandEvent& event); 
	void OnTimer(wxTimerEvent& event); // Timer event handler for video frames
    void OnExitClicked(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
public:
    frmWebCamSetup(wxWindow* parent, wxWindowID id = wxID_ANY,
                   const wxString& title = _("::: WebCam Setup :::"),
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxSize(525, 395),
                   long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    ~frmWebCamSetup();
	enum {
		ID_WEBCAM_TIMER,
         ID_CHOICE_SETUP,
         ID_BT_EXIT
    };
};
#endif // FRMWEBCAMSETUP_H
