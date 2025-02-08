#include "frmMain.h"
#include <wx/wx.h>
#include "author_info.h"
#include "facewebcam.xpm"
// Implement the application class
//dc.SetFont(wxFontInfo(10).Family(wxFONTFAMILY_DEFAULT).Style(wxFONTSTYLE_NORMAL).Weight(wxFONTWEIGHT_NORMAL));
class FaceWebCam : public wxApp {
public:
    virtual bool OnInit() override; // wxWidgets application initialization
};
// Declare the application entry point
wxIMPLEMENT_APP(FaceWebCam);
bool FaceWebCam::OnInit() {
	std::string strauthor = GetAuthorInfo();
	wxSizerFlags::DisableConsistencyChecks();
	 // Register all default image handlers
    wxInitAllImageHandlers(); // Includes handlers for JPEG, PNG, BMP, etc.
    // Create the main application frame
    frmMain* mainFrame = new frmMain("::: Face Recognition Webcam :::");
	wxIcon appIcon(facewebcam_xpm); // Create an icon with the XPM data
    mainFrame->SetIcon(appIcon); // Set the icon for the main window
    mainFrame->Show(true); // Make the frame visible
    return true; // wxWidgets will now start its event loop
}