#include "frmWebCamSetup.h"
#include <wx/sizer.h>      // Fix for wxBoxSizer (Add these imports)
#include <wx/log.h>        // Optional debug logging
#include <stdexcept>       // For handling file and exception logic
// Event table
wxBEGIN_EVENT_TABLE(frmWebCamSetup, wxFrame)
    EVT_BUTTON(frmWebCamSetup::ID_BT_EXIT, frmWebCamSetup::OnExitClicked) // Bind Exit button to OnExitClicked
	EVT_CHOICE(wxID_ANY, frmWebCamSetup::OnChoiceChanged)
	EVT_TIMER(wxID_ANY, frmWebCamSetup::OnTimer) // Add this for the timer
wxEND_EVENT_TABLE()
void frmWebCamSetup::start_web_cam(){
	// --- Start webcam video stream ---
    if (webcam_index.length() >= 8) {
        webcam_index = webcam_index.substr(7, 1);
    } else {
        webcam_index = "0";
    }
    int webcamID = std::stoi(webcam_index);
    if (webcamID < 0 || webcamID >= choice_webcam_index->GetCount()) {
        webcamID = 0; // Default to WebCam 0
    }
    choice_webcam_index->SetSelection(webcamID);
    if (!cap.open(webcamID)) {
        wxMessageBox("Error: Unable to open the webcam!", "Error", wxICON_ERROR);
    }
    // Start the timer to refresh video frames
    video_timer = new wxTimer(this);
    video_timer->Bind(wxEVT_TIMER, &frmWebCamSetup::OnTimer, this);
    video_timer->Start(33); // Update at ~30fps
}
// Constructor
frmWebCamSetup::frmWebCamSetup(wxWindow* parent, wxWindowID id,
                               const wxString& title, const wxPoint& pos,
                               const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style), nemswebcam_j() {
    SetSizeHints(wxDefaultSize, wxDefaultSize);
    // --- Create layout and UI elements ---
    wxBoxSizer* bSizer4 = new wxBoxSizer(wxVERTICAL);
    // Placeholder for video clip
    bitmap_video_clip = new wxStaticBitmap(this, wxID_ANY, GetPlaceholderBitmap(),
                                           wxDefaultPosition, wxSize(300, 200), 0);
    bSizer4->Add(bitmap_video_clip, 0, wxALIGN_CENTER | wxALL, 5);
    // Horizontal separator line
    m_staticline3 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    bSizer4->Add(m_staticline3, 0, wxEXPAND | wxALL, 5);
    // Webcam selection label and dropdown
    wxBoxSizer* bSizer5 = new wxBoxSizer(wxHORIZONTAL);
    lblWebCam = new wxStaticText(this, wxID_ANY,
                                 _("Is your webcam working? If not, please reselect your webcam:"),
                                 wxDefaultPosition, wxDefaultSize, 0);
    lblWebCam->Wrap(-1);
    bSizer5->Add(lblWebCam, 0, wxEXPAND | wxALL, 5);
    choice_webcam_index = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, 0);
    bSizer5->Add(choice_webcam_index, 0, wxEXPAND | wxALL, 5);
    bSizer4->Add(bSizer5, 0, wxEXPAND | wxALL, 5);
    // Another horizontal separator
    m_staticline4 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    bSizer4->Add(m_staticline4, 0, wxEXPAND | wxALL, 5);
	//Button layout (confirm and exit)
	wxBoxSizer* bSizer6 = new wxBoxSizer(wxHORIZONTAL);
	// Add a spacer to push the button to the right
	bSizer6->AddStretchSpacer(1); ////// Flexible space
	btExit = new wxButton(this, wxID_ANY, _("Exit"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer6->Add(btExit, 0, wxEXPAND | wxALL, 5);//////wxALIGN_CENTER_VERTICAL | wxALL
	//bSizer4->Add(btExit, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 5); ///////// No padding on bottom
	bSizer4->Add(bSizer6, 0, wxEXPAND | wxALL, 5);
    // --- Finalize layout ---
    SetSizer(bSizer4);
	Fit(); // Automatically adjust window size to fit the contents
	Layout();
    Centre(wxBOTH);
    // --- Load webcam configuration ---
    std::string webConfigFile = nemswebcam_j.getExecutableDirectory() + "/webCamConfig.dat";
    if (std::filesystem::exists(webConfigFile)) {
        webcam_index = nemswebcam_j.ReadBinaryOne_from_std(webConfigFile);
        if (webcam_index.empty()) {
            wxMessageBox("Error: Failed to open the webcam's configuration file! Please reselect your webcam:", "Error", wxICON_ERROR);
            webcam_index = "WebCam 0"; // Default value
        }
    } else {
        webcam_index = "WebCam 0"; // Default value
    }
    PopulateWebcamChoice();
	start_web_cam();
    // --- Bind events ---
    btExit->Bind(wxEVT_BUTTON, &frmWebCamSetup::OnExitClicked, this);
}
// Destructor
frmWebCamSetup::~frmWebCamSetup() {
	int selectedIndex = choice_webcam_index->GetSelection();
    if (selectedIndex != wxNOT_FOUND) {
        wxString selectedItem = choice_webcam_index->GetString(selectedIndex);
        webcam_index = std::string(selectedItem.mb_str());
        std::string workDir = nemswebcam_j.getExecutableDirectory() + "/webCamConfig.dat";
        nemswebcam_j.WriteBinaryOne_from_std(webcam_index, workDir);
    } else {
        wxMessageBox(_("No webcam selected."), _("Warning"), wxOK | wxICON_WARNING);
    }
	try{
		// Nothing special needed as wxWidgets will handle cleanup
		 // Stop the timer and release webcam resources
		if (video_timer && video_timer->IsRunning()) {
			video_timer->Stop();
			video_timer = nullptr;
		}
		if (cap.isOpened()) {
			cap.release();
		}
	}
	catch(...){}
}
void frmWebCamSetup::OnChoiceChanged(wxCommandEvent& event) {
	// Disable the timer to stop frame processing
    if (video_timer && video_timer->IsRunning()) {
        video_timer->Stop();
    }
	// Disable the dropdown to prevent further interactions during switching
    choice_webcam_index->Disable();
	// Display a black image to clear the current frame
    cv::Mat black_frame = cv::Mat::zeros(200, 300, CV_8UC3); // Create a black image (300x200)
    wxImage black_image(black_frame.cols, black_frame.rows, black_frame.data, true);
    wxBitmap black_bitmap(black_image);
    bitmap_video_clip->SetBitmap(black_bitmap); // Set the black bitmap
    bitmap_video_clip->Refresh();           
    // Get the newly selected webcam index from the dropdown
    int selectedIndex = choice_webcam_index->GetSelection();
    if (selectedIndex == wxNOT_FOUND) {
        wxMessageBox("No webcam selected.", "Error", wxICON_ERROR);
		choice_webcam_index->Enable();
        return;
    }
    // Extract the webcam index as an integer
    wxString selectedWebcam = choice_webcam_index->GetString(selectedIndex);
	selectedWebcam = selectedWebcam.substr(7,1);
    int newWebcamIndex = wxAtoi(selectedWebcam); // Convert string to integer
    // Perform webcam switching in a separate thread
    std::thread webcam_thread([this, newWebcamIndex]() {
        // Stop the current webcam feed
        if (cap.isOpened()) {
            cap.release();
        }
        // Reinitialize the webcam with the new index
        bool success = cap.open(newWebcamIndex);
        // Re-enable the dropdown and restart the timer on the main thread
        wxTheApp->CallAfter([this]() {
            choice_webcam_index->Enable(); // Re-enable the dropdown
            if (video_timer) {
                video_timer->Start(33); // Restart the timer (~30 FPS)
            }
        });
    });
    // Detach the thread to allow it to run independently
    webcam_thread.detach();
}
void frmWebCamSetup::OnTimer(wxTimerEvent& event) {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        return;
    }
    try {
        // Convert BGR to RGB for wxWidgets
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        // Resize the frame to fit the wxStaticBitmap
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(300, 200));
        // Convert to wxBitmap
        wxImage image(resized_frame.cols, resized_frame.rows, resized_frame.data, true);
        wxBitmap bitmap(image);
        // Update the static bitmap and refresh
        bitmap_video_clip->SetBitmap(bitmap);
        Refresh();
    } catch (const std::exception& e) {
        wxLogMessage("Error: %s", e.what());
    }
	catch(...){}
}
// Event Handlers
void frmWebCamSetup::OnExitClicked(wxCommandEvent& event) {
    Close();
}
// Populate choices
void frmWebCamSetup::PopulateWebcamChoice() {
    wxArrayString choices;
    choices.Add("WebCam 0");
    choices.Add("WebCam 1");
    choices.Add("WebCam 2");
	choices.Add("WebCam 3");
	choices.Add("WebCam 4");
	choices.Add("WebCam 5");
    choice_webcam_index->Append(choices);
    if (choices.GetCount() > 0) {
        choice_webcam_index->SetSelection(0);
    }
}
// Placeholder bitmap for no video signal
wxBitmap frmWebCamSetup::GetPlaceholderBitmap() {
    wxBitmap bmp(300, 200);
    wxMemoryDC dc(bmp);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SetTextForeground(*wxBLACK);
    //dc.SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxNORMAL, wxFONTWEIGHT_NORMAL));
    dc.DrawText("No video signal", 100, 90);
    dc.SelectObject(wxNullBitmap);
	dc.SetFont(wxFontInfo(10).Family(wxFONTFAMILY_DEFAULT).Style(wxFONTSTYLE_NORMAL).Weight(wxFONTWEIGHT_NORMAL));
    return bmp;
}
