#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/statusbr.h>
#include <memory> // For std::unique_ptr
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <ctime>
#include <atomic>//for stopCapture;
#include "public_variables.h"
#include "frmWebCamSetup.h"
#include "frmFaceDetectionSetup.h"
#include "person_info.h"
#include "MyResizableBitmapPanel.h"
#include "frmPopup.h"
#include "lib_face_recog.h"
class frmMain : public wxFrame, public IEventHandler {
private:
	std::atomic<bool> stopCapture;//Flag to stop the capture thread
	std::mutex frameMutex;
	cv::CascadeClassifier faceCascade;  
	cv::Mat frame;
	cv::VideoCapture cap;   // OpenCV video capture object
	std::thread m_captureThread; //background thread for video capture
	wxTimer* video_timer = nullptr;   // Timer to refresh frames periodically
	enum CURRENT_DISPLAY{
		friends,
		strangers
	};
	CURRENT_DISPLAY current_display;
	unsigned int cvMatlevels = 4; // Levels per channel (e.g., 4, 8, 16)
	std::string CurrDir;
	std::string left_frame_title;
	unsigned int stranger_count = 0;
	/*
	 * friends dataset
	 */
	std::vector<pub_lib::Friend> friends_info;
	/*
	 * strangers dataset
	 */
	std::vector<pub_lib::PersonBinInfo> strangers_info;
	/*
	 * friends
	 */
    std::string faceImags_folder_friends;
	std::string faceImags_large_folder_friends;
	/*
	 * stranger 
	 */
	std::string faceImags_folder_strangers;
	std::string faceImags_large_folder_strangers;
	/*
	 *  file process library
	 */
    nemslib_webcam nemswebcam_j;
	/*
	 * facial recognition library
	*/
	lib_face_recog libface_j;
    std::string webcam_index;
    wxMenuBar* m_menubar3;
    wxMenu* mnuSetup;
    PersonFrame* htmWinListView_main;//wxListCtrl
    MyResizableBitmapPanel* bitmap_video_clip_main = nullptr;
    wxStatusBar* frmMain_status;
	/*
	 * Sub forms events definition 
	 */
	void OnAddButtonClicked(wxCommandEvent& event) override;// IEventHandler implementation
	void OnViewButtonClicked(wxCommandEvent& event) override;// IEventHandler implementation
	void OnDeleteButtonClicked(wxCommandEvent& event) override;// IEventHandler implementation
	/*
	 * Sub forms definition
	*/
    frmWebCamSetup* webcamSetupFrame = nullptr;
    frmFaceDetectionSetup* faceSetupFrame = nullptr;
	void getMaxStrangersCount(unsigned int);
	void setMaxStrangersCount(unsigned int);
    void OnWebCamSetup(wxCommandEvent& event);   // Open the sub-frame
	void OnWebCamSetup_Closed(wxCloseEvent& event);   // Open the sub-frame
    void OnFaceRecognitionSetup(wxCommandEvent& event);
	void OnFaceRecognitionSetup_Closed(wxCloseEvent& event);
	void OnStrangersCleanup(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);             // Exit the app
	void mark_on_the_person(cv::Mat&, const std::vector<cv::Rect>&, const std::string&);
	std::string checkExistingFace(const cv::Mat&, const lib_face_recog::CURRENT_FaceType&);
	void onFacesDetected(const std::vector<cv::Rect>&, cv::Mat&);
	void OnTimer(wxTimerEvent& event);
	void start_webcam();
	void stop_webcam();
	void capture_thread_webcam();
	void process_detection_webcam(const cv::Mat&, const std::vector<std::pair<cv::Mat,std::string>>&, const std::vector<cv::Mat>&);
    wxDECLARE_EVENT_TABLE();
public:
    frmMain(const wxString& title);
    ~frmMain();
    enum {
        ID_MENU_WEBCAM_SETUP = wxID_HIGHEST + 1,
        ID_MENU_FACE_RECOGNITION_SETUP,
		ID_MENU_STRANGERS_CLEANUP,
        ID_MENU_EXIT
    };
};
#endif // FRMMAIN_H
