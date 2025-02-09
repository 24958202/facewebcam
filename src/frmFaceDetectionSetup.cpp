#include "frmFaceDetectionSetup.h"
#include <stdexcept>       // For handling file and exception logic
///////////////////////////////////////////////////////////////////////////
// Event table
wxBEGIN_EVENT_TABLE(frmFaceDetectionSetup, wxFrame)
    EVT_BUTTON(frmFaceDetectionSetup::ID_BT_Add, frmFaceDetectionSetup::OnAddClicked) // Bind Open button to OnOpenClicked
    EVT_BUTTON(frmFaceDetectionSetup::ID_BT_EXIT, frmFaceDetectionSetup::OnExitClicked) // Bind Exit button to OnExitClicked
	EVT_TIMER(wxID_ANY, frmFaceDetectionSetup::OnTimer) // Add this for the timer
wxEND_EVENT_TABLE()
frmFaceDetectionSetup::frmFaceDetectionSetup( 
wxWindow* parent, 
wxWindowID id, 
const wxString& title, 
const wxPoint& pos, 
const wxSize& size, 
long style, 
const std::vector<pub_lib::Friend>& friends_main
) : wxFrame(parent, id, title, pos, size, style), nemswebcam_j(), friends_dataset(friends_main)
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer(wxVERTICAL);
	lblInfo = new wxStaticText(this, wxID_ANY, _("Click \"add\" button to add you to the friend list."), wxDefaultPosition, wxDefaultSize, 0);
	lblInfo->Wrap( -1 );
	bSizerMain->Add( lblInfo, 0, wxALL, 5 );
	bitWebCam = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(300,200), 0);
	bSizerMain->Add(bitWebCam, 0, wxALIGN_CENTER|wxALL, 5);
	lblName = new wxStaticText(this, wxID_ANY, _("Please input your name:"), wxDefaultPosition, wxDefaultSize, 0);
	lblName->Wrap(-1);
	bSizerMain->Add(lblName, 0, wxALL, 5);
	wxBoxSizer* btsize_name;
	btsize_name = new wxBoxSizer( wxHORIZONTAL );
	txtName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 300,-1 ), 0 );
	btsize_name->Add(txtName, 0, wxALL, 5);
	btAdd = new wxButton( this, wxID_ANY, _("add"), wxDefaultPosition, wxDefaultSize, 0 );
	btsize_name->Add(btAdd, 0, wxALL, 5);
	bSizerMain->Add(btsize_name, 1, wxEXPAND, 5);
	// Add a PersonFrame to the main sizer
	listPeopleView = new FriendsFrame(this,this, wxID_ANY, wxDefaultPosition, wxSize(420, 300)); // Parent is `this`
	bSizerMain->Add(listPeopleView, 2, wxEXPAND | wxALL, 5);
	btExit = new wxButton( this, wxID_ANY, _("Exit"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizerMain->Add( btExit, 0, wxALIGN_RIGHT|wxALL, 5 );
	this->SetSizer( bSizerMain );
	this->Layout();
	this->Centre( wxBOTH );
	// --- Bind events ---
    btAdd->Bind(wxEVT_BUTTON, &frmFaceDetectionSetup::OnAddClicked, this);
    btExit->Bind(wxEVT_BUTTON, &frmFaceDetectionSetup::OnExitClicked, this);
	/*
	 * start webcam 
	 */
	 // --- Load webcam configuration ---
	curr_dir = nemswebcam_j.getExecutableDirectory();
    std::string webConfigFile = curr_dir + "/webCamConfig.dat";
    if (std::filesystem::exists(webConfigFile)) {
        webcam_index = nemswebcam_j.ReadBinaryOne_from_std(webConfigFile);
        if (webcam_index.empty()) {
            wxMessageBox("Error: Failed to open the webcam's configuration file! Please reselect your webcam:", "Error", wxICON_ERROR);
            webcam_index = "WebCam 0"; // Default value
        }
		if (webcam_index.length() >= 8) {
			webcam_index = webcam_index.substr(7, 1);
		} else {
			webcam_index = "0";
		}
		int webcamID = std::stoi(webcam_index);
		if (webcamID < 0) {
			webcamID = 0; // Default to WebCam 0
		}
		if (!cap.open(webcamID)) {
			wxMessageBox("Error: Unable to open the webcam!", "Error", wxICON_ERROR);
		}
		if (!faceCascade.load(curr_dir + "/haarcascade_frontalface_default.xml")) {
			std::cerr << "Error loading cascade file for face detection!" << std::endl;
		}
		// Start the timer to refresh video frames
		video_timer = new wxTimer(this);
		video_timer->Bind(wxEVT_TIMER, &frmFaceDetectionSetup::OnTimer, this);
		video_timer->Start(33); // Update at ~30fps
    } else {
		wxMessageBox("Please set up your webcam first.", "Info", wxOK);
        Close();
    }
	friends_folder_path = curr_dir + "/friends/faces";
	/*
	 *  load friends list if exists 
	 */
	if(std::filesystem::exists(curr_dir + "/friends.dat")){
		nemswebcam_j.ReadFriendsFromBinary(curr_dir + "/friends.dat",friends_dataset);
		if(!friends_dataset.empty()){
			listPeopleView->set_friend_set(friends_dataset);//add to friends_dataset
			listPeopleView->refresh_list();
			// Ensure the main layout re-adjusts
			this->Layout();
			this->Refresh();
		}
	}
}
frmFaceDetectionSetup::~frmFaceDetectionSetup()
{
	try{
		friends_dataset.clear();
		 // Stop the timer and release webcam resources
		if (video_timer){
			if(video_timer->IsRunning()){
				video_timer->Stop();
			}
			video_timer->Unbind(wxEVT_TIMER,&frmFaceDetectionSetup::OnTimer,this);
			video_timer = nullptr;
		}
		if (cap.isOpened()) {
			cap.release();
		}
	}
	catch(...){}
}
bool frmFaceDetectionSetup::person_exists(const std::string& person_name){
	std::string remove_space_name = person_name;
	nemswebcam_j.remove_space_for_filename(remove_space_name);
	for(const auto& item : friends_dataset){
		if(item.name == remove_space_name){
			return true;
		}
	}
	return false;
}
void frmFaceDetectionSetup::OnDeleteButtonClicked(wxCommandEvent& event){
	listPeopleView->refresh_list();
	// Ensure the main layout re-adjusts
    this->Layout();
    this->Refresh();
}
void frmFaceDetectionSetup::OnAddClicked(wxCommandEvent& event) {
	if (txtName->GetValue().IsEmpty()) {
		// The wxTextCtrl is empty
		wxMessageBox("Please input your name.", "Info", wxOK);
		return;
	} 
	try{
		std::string str_input_person;
		str_input_person = txtName->GetValue();
		if(person_exists(str_input_person)){
			wxLogMessage("You are already in the list.");
			txtName->Clear();
			return;
		}
		if(faceImage.empty()){
			wxLogMessage("Catpured image errors");
			return;
		}
		std::string name_to_save = str_input_person;
		nemswebcam_j.remove_space_for_filename(name_to_save);
		str_input_person_face_path = friends_folder_path + "/" + name_to_save + ".jpg";
		cv::imwrite(str_input_person_face_path, faceImage);
		/*
		 *  write to dat file
		 */
		 pub_lib::Friend pInfo;
		 pInfo.face_image = faceImage;
		 pInfo.name = str_input_person;
		 friends_dataset.push_back(pInfo);
		 listPeopleView->set_friend_set(friends_dataset);//add to friends_dataset
		 nemswebcam_j.WriteFriendsToBinary(friends_dataset, curr_dir + "/friends.dat");
		 //friends_dataset
		 txtName->Clear();
		 wxBitmap bitmap(str_input_person_face_path, wxBITMAP_TYPE_JPEG);
		 listPeopleView->refresh_list();
		 // Ensure the main layout re-adjusts
		 this->Layout();
		 this->Refresh();
		 txtName->Clear();
		 wxMessageBox("Successfully added you to the friend list.", "Info", wxOK);
	}
	catch(...){}
}
bool frmFaceDetectionSetup::checkExistingFace(const std::string& faces_folder_path, const cv::Mat& img_input){
	if (!std::filesystem::exists(faces_folder_path) || !std::filesystem::is_directory(faces_folder_path)) {
        return false;
    }
    cv::Mat img_gray;
    cv::cvtColor(img_input, img_gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(img_gray, img_gray, cv::Size(5, 5), 0);
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create(pub_lib::MAX_FEATURES);
    std::vector<cv::KeyPoint> keypoints_input;
    cv::Mat descriptors_input;
    detector->detectAndCompute(img_gray, cv::noArray(), keypoints_input, descriptors_input);
    if (descriptors_input.empty()) {
        std::cerr << "Input image has no descriptors." << std::endl;
        return false;
    }
    for (const auto& entry : std::filesystem::directory_iterator(faces_folder_path)) {
        if (entry.is_regular_file()) {
            cv::Mat existing_face = cv::imread(entry.path().string());
            if (existing_face.empty()) continue;
            cv::Mat existing_face_gray;
            cv::cvtColor(existing_face, existing_face_gray, cv::COLOR_BGR2GRAY);
            std::vector<cv::KeyPoint> keypoints_existing;
            cv::Mat descriptors_existing;
            detector->detectAndCompute(existing_face_gray, cv::noArray(), keypoints_existing, descriptors_existing);
            if (descriptors_existing.empty()) {
                std::cerr << "Existing face has no descriptors." << std::endl;
                continue;
            }
            cv::BFMatcher matcher(cv::NORM_L2);
            std::vector<std::vector<cv::DMatch>> knnMatches;
            matcher.knnMatch(descriptors_input, descriptors_existing, knnMatches, 2);
            std::vector<cv::DMatch> goodMatches;
            for (const auto& match : knnMatches) {
                if (match.size() > 1 && match[0].distance < pub_lib::RATIO_THRESH * match[1].distance) {
                    goodMatches.push_back(match[0]);
                }
            }
            if (goodMatches.size() > pub_lib::DE_THRESHOLD) {
                return true;
            }
        }
    }
    return false;
}
void frmFaceDetectionSetup::onFacesDetected(
		const std::vector<cv::Rect>& faces, 
		cv::Mat& input_frame
	){
	if (faces.empty()) return;
	std::string str_input_person; 
	str_input_person = txtName->GetValue();
	if(str_input_person.empty()){
		return;
	}
	/**/
	for (size_t i = 0; i < faces.size(); ++i) {
        cv::rectangle(input_frame, faces[i], cv::Scalar(0, 255, 0), 2);
        cv::putText(input_frame, str_input_person, cv::Point(faces[i].x, faces[i].y - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
    }
    cv::Rect faceROI = faces[0]; // Save the first detected face
	faceImage = input_frame(faceROI).clone();
	cv::Mat process_face_image = input_frame(faceROI).clone();
    if (checkExistingFace(friends_folder_path, process_face_image)) {
		std::cout << "The face image already exists." << std::endl;
        return;
    }
}
void frmFaceDetectionSetup::OnTimer(wxTimerEvent& event){
	cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        return;
    }
    try {
        // Convert BGR to RGB for wxWidgets
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
		cv::Mat low_quality_frame = frame / (256 / cvMatlevels) * (256 / cvMatlevels);
        // Resize the frame to fit the wxStaticBitmap
        cv::Mat resized_frame;
        cv::resize(low_quality_frame, resized_frame, cv::Size(300, 200));
		cv::Mat gray;  
		cv::cvtColor(resized_frame, gray, cv::COLOR_BGR2GRAY);  
		std::vector<cv::Rect> faces;  
		faceCascade.detectMultiScale(gray, faces, 1.1, 10, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));  
		if (!faces.empty()) {  
			onFacesDetected(faces, resized_frame);  
		}  
		else{
			std::cerr << "faces are empty!" << std::endl;
		}
        // Convert to wxBitmap
        wxImage image(resized_frame.cols, resized_frame.rows, resized_frame.data, true);
        wxBitmap bitmap(image);
        // Update the static bitmap and refresh
        bitWebCam->SetBitmap(bitmap);
        Refresh();
    } catch (const std::exception& e) {
        wxLogMessage("Error: %s", e.what());
    }
	catch(...){}
}
void frmFaceDetectionSetup::OnExitClicked(wxCommandEvent& event) {
	Close();
}