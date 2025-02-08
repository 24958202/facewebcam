#ifndef FRMFACEDETECTIONSETUP_H
#define FRMFACEDETECTIONSETUP_H
#include <wx/app.h> // Required for wxTheApp and CallAfter
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/msgdlg.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // For file system operations in C++17
#include <thread>
#include <mutex>
#include <opencv2/opencv.hpp>
#include "public_variables.h"
#include "nemslib_webcam.h"
#include "friends_info.h"
class frmFaceDetectionSetup : public wxFrame, public IFriendsEventHandler
{
	private:
		 // Dynamic webcam helper library
		cv::Mat faceImage;//current face image
		std::string curr_dir;
	    std::string friends_folder_path;
		std::string str_input_person_face_path;
		nemslib_webcam nemswebcam_j;
		wxStaticText* lblInfo;
		wxStaticBitmap* bitWebCam;
		wxStaticText* lblName;
		wxTextCtrl* txtName;
		wxButton* btAdd;
		FriendsFrame* listPeopleView;
		wxButton* btExit;
		std::string webcam_index;
		std::vector<pub_lib::Friend> friends_dataset;
		bool person_exists(const std::string&);
		void remove_space_for_filename(std::string&);
		void OnDeleteButtonClicked(wxCommandEvent& event) override;
		void OnAddClicked(wxCommandEvent& event);
		void OnExitClicked(wxCommandEvent& event);
		unsigned int cvMatlevels = 4; // Levels per channel (e.g., 4, 8, 16)
		cv::CascadeClassifier faceCascade;
		cv::VideoCapture cap;   // OpenCV video capture object
		wxTimer* video_timer = nullptr;   // Timer to refresh frames periodically
		bool checkExistingFace(const std::string&, const cv::Mat&);
		void onFacesDetected(const std::vector<cv::Rect>&, cv::Mat&);
		void OnTimer(wxTimerEvent& event);
		wxDECLARE_EVENT_TABLE();
	public:
		frmFaceDetectionSetup(
		wxWindow* parent, wxWindowID id = wxID_ANY, 
		const wxString& title = _(":::Friend list setup:::"), 
		const wxPoint& pos = wxDefaultPosition, 
		const wxSize& size = wxSize( 517,395 ), 
		long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL, 
		const std::vector<pub_lib::Friend>& friends_main = {}
		);
		~frmFaceDetectionSetup();
		enum {
			ID_BT_Add,
			ID_BT_EXIT
		};
};
#endif