#include "frmMain.h"
#include <unordered_map>
// Event table to bind menu events
// Event table binding menu events
wxBEGIN_EVENT_TABLE(frmMain, wxFrame)
    EVT_MENU(ID_MENU_WEBCAM_SETUP, frmMain::OnWebCamSetup)
    EVT_MENU(ID_MENU_FACE_RECOGNITION_SETUP, frmMain::OnFaceRecognitionSetup)
	EVT_MENU(ID_MENU_STRANGERS_CLEANUP, frmMain::OnStrangersCleanup)
	EVT_TIMER(wxID_ANY, frmMain::OnTimer) // Add this for the timer
    EVT_MENU(ID_MENU_EXIT, frmMain::OnExit)
wxEND_EVENT_TABLE()
void frmMain::start_webcam(){
	// --- Start webcam video stream ---
    if (webcam_index.length() >= 8) {
        webcam_index = webcam_index.substr(7, 1);
    } else {
        webcam_index = "0";
    }
    int webcamID = std::stoi(webcam_index);
    if (webcamID < 0 || webcamID > 5) {
        webcamID = 0; // Default to WebCam 0
    }
    if (!cap.open(webcamID)) {
        wxMessageBox("Error: Unable to open the webcam!", "Error", wxICON_ERROR);
    }
	stopCapture = false;
	m_captureThread = std::thread(&frmMain::capture_thread_webcam, this);
    // Start the timer to refresh video frames
    video_timer = new wxTimer(this);
    video_timer->Bind(wxEVT_TIMER, &frmMain::OnTimer, this);
    video_timer->Start(33); // Update at ~30fps
}
void frmMain::stop_webcam(){
	stopCapture = true;
	if(m_captureThread.joinable()){
		m_captureThread.join();
	}
	// Disable the timer to stop frame processing
    if (video_timer && video_timer->IsRunning()) {
        video_timer->Stop();
		video_timer->Unbind(wxEVT_TIMER,&frmMain::OnTimer,this);
		video_timer = nullptr; // Prevent double deletion
    }
    if (cap.isOpened()) {
        cap.release();
    }
}
frmMain::frmMain(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(900, 600)){
	/*
		get current work directory
	*/
	CurrDir = nemswebcam_j.getExecutableDirectory();
    // **Step 1: Create the Menu Bar**
    m_menubar3 = new wxMenuBar(0);
    // Create the "Setup" menu
    mnuSetup = new wxMenu();
    mnuSetup->Append(ID_MENU_WEBCAM_SETUP, _("WebCam setup"));           // Menu item for "WebCam setup"
    mnuSetup->Append(ID_MENU_FACE_RECOGNITION_SETUP, _("Friend list setup")); // Menu item for "Face recognition setup"
    mnuSetup->AppendSeparator();
	mnuSetup->Append(ID_MENU_STRANGERS_CLEANUP, _("Clean up all strangers"));
	mnuSetup->AppendSeparator();
	mnuSetup->Append(ID_MENU_EXIT, _("Exit\tCtrl+Q"));                  // Menu item for "Exit" with custom ID
    // Add the "Setup" menu to the menu bars
    m_menubar3->Append(mnuSetup, _("Setup"));
    // Attach the menu bar to the frame
    this->SetMenuBar(m_menubar3);
    // **Step 2: Create the Main Layout**
    wxBoxSizer* bSizerMain = new wxBoxSizer(wxHORIZONTAL);
	/*
	 * if there is not stranger , display friends 
	 */
	if(!std::filesystem::exists(CurrDir + "/friends.dat")){
		left_frame_title = "::: Friends :::";
	}
	// Add a PersonFrame to the main sizer
	htmWinListView_main = new PersonFrame(this,this, wxID_ANY, wxDefaultPosition, wxSize(200, 520), left_frame_title); // Parent is `this`
	bSizerMain->Add(htmWinListView_main, 2, wxEXPAND | wxALL, 5);
    // Add a static bitmap on the right
    wxBitmap placeholderBitmap(wxSize(590, 520)); // Placeholder for the bitmap
    bitmap_video_clip_main = new MyResizableBitmapPanel(this, placeholderBitmap);
    bSizerMain->Add(bitmap_video_clip_main, 8, wxEXPAND | wxALL, 5);
    // Apply the layout to the frame
    this->SetSizer(bSizerMain);
    this->Layout();
    // **Step 3: Create the Status Bar**
    frmMain_status = this->CreateStatusBar(2, wxSTB_SIZEGRIP, wxID_ANY); // Two fields in the status bar
    // Center the window on the screen
    this->Centre(wxBOTH);
	/*
	 *  Create folders 
	 */
	try {
		/*
			Create a folder for friend images
		 */
		if (!std::filesystem::exists(CurrDir + "/friends")) {
			std::filesystem::create_directory(CurrDir + "/friends");
		}
		if (!std::filesystem::exists(CurrDir + "/friends/faces")) {
			std::filesystem::create_directory(CurrDir + "/friends/faces");
		}
		if (!std::filesystem::exists(CurrDir + "/friends/faces_large")) {
			std::filesystem::create_directory(CurrDir + "/friends/faces_large");
		}
		if (!std::filesystem::exists(CurrDir + "/strangers")) {
			std::filesystem::create_directory(CurrDir + "/strangers");
		}
		if (!std::filesystem::exists(CurrDir + "/strangers/faces")) {
			std::filesystem::create_directory(CurrDir + "/strangers/faces");
		}
		if (!std::filesystem::exists(CurrDir + "/strangers/faces_large")) {
			std::filesystem::create_directory(CurrDir + "/strangers/faces_large");
		}
		faceImags_folder_friends = CurrDir + "/friends/faces";
		faceImags_large_folder_friends = CurrDir + "/friends/faces_large";
		faceImags_folder_strangers = CurrDir + "/strangers/faces";
		faceImags_large_folder_strangers = CurrDir + "strangers/faces_large";
	} catch (...) {}
	/*
	 * Load friends and strangers datasets 
	 */
	getMaxStrangersCount(stranger_count);
	/*
	 * check if friends list exists 
	*/
	if(std::filesystem::exists(CurrDir + "/friends.dat")){
		nemswebcam_j.ReadFriendsFromBinary(CurrDir + "/friends.dat",friends_info);
	}
	/*
	 * Display strangers 
	 */
	if(!friends_info.empty()){
		for(const auto& item : friends_info){
			//wxBitmap ConvertCvMatToWxBitmap(const cv::Mat& mat);
			wxBitmap wx_bitmap = nemswebcam_j.ConvertCvMatToWxBitmap(item.face_image);
			htmWinListView_main->Disable_Add_button = true;
			htmWinListView_main->addPerson(item.name, wx_bitmap, "", wx_bitmap);
		}
		current_display = CURRENT_DISPLAY::friends;
	}
	/*
	 * Face recogniton model 
	*/
	std::ofstream ofile(CurrDir + "/haarcascade_frontalface_default.xml", std::ios::out);
	if(!ofile.is_open()){
		ofile.open(CurrDir + "/haarcascade_frontalface_default.xml", std::ios::out);
	}
	ofile << pub_lib::facial_model;
	ofile.close();
	/*
	 * initialize face recognition model 
	*/
	if (!faceCascade.load(CurrDir + "/haarcascade_frontalface_default.xml")) {
		wxMessageBox("Error: Faile to open the facial recognition model file.", "Error", wxICON_ERROR);
	}
	/*
	 * Check webcam configuration file 
	 */
	// --- Load webcam configuration ---
	iniWebCam();
}
frmMain::~frmMain() {
	stop_webcam();
    // Destructor logic if needed
	// Clean up resources
    if (webcamSetupFrame) {
        webcamSetupFrame->Destroy();
	    webcamSetupFrame = nullptr; 
    }
	if (bitmap_video_clip_main) {
        delete bitmap_video_clip_main;
        bitmap_video_clip_main = nullptr; // Avoid dangling pointer
    }
}
/*
 * Sub forms event
 */
void frmMain::iniWebCam(){
	try{
		std::string webConfigFile = CurrDir + "/webCamConfig.dat";
		if (std::filesystem::exists(webConfigFile)) {
			webcam_index = nemswebcam_j.ReadBinaryOne_from_std(webConfigFile);
			if(webcam_index.empty()){
				wxMessageBox("Error: Faile to open the webcam's configuration file!Please reselect your webcam's index.", "Error", wxICON_ERROR);
				webcam_index = "WebCam 0"; // Default value
			}
			else{
				// --- Start webcam video stream ---
				webcam_index = webcam_index.substr(7,1);
			}
			/*
			 * Start webcam 
			 */
			start_webcam();
		} else {
			webcam_index = "WebCam 0"; // Default value
			//Open the webcam setup window
			wxCommandEvent dummyEvent;
			OnWebCamSetup(dummyEvent);
		}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void frmMain::getMaxStrangersCount(unsigned int stranger_id_count){
	try{
		if(std::filesystem::exists(CurrDir + "/stranger_id.txt")){
			std::fstream ifile(CurrDir + "/stranger_id.txt");
			if (!ifile) {
				stranger_id_count = 0;
				return;
			}
			std::string line;
			if (std::getline(ifile, line)){
				if(!line.empty()){
					stranger_id_count = std::stoul(line);
				}
			}
			ifile.close();   
		}
		else{
			stranger_id_count = 0;
		}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void frmMain::setMaxStrangersCount(unsigned int stranger_id_count){
	try{
		std::fstream ofile(CurrDir + "/stranger_id.txt", std::ios::out);
		if(!ofile.is_open()){
			ofile.open(CurrDir + "/stranger_id.txt", std::ios::out);
		}
		ofile << std::to_string(stranger_id_count);
		ofile.close();
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void frmMain::OnAddButtonClicked(wxCommandEvent& event){//add strangers to friend's list
	htmWinListView_main->refresh_list();
	// Ensure the main layout re-adjusts
	this->Layout();
	this->Refresh();
	wxMessageBox("Successfully add to your friend list.", "Info", wxOK);
}
void frmMain::OnViewButtonClicked(wxCommandEvent& event){//
	std::lock_guard<std::mutex> lock(frameMutex);
	unsigned int id = event.GetId();
	if(id < 0){
		id = 0;
	}
	if(current_display == CURRENT_DISPLAY::friends){
		if(friends_info.empty() || id > friends_info.size()){
			return;
		}
		std::string view_name = friends_info[id].name;
		if(view_name.empty()){
			return;
		}
		nemswebcam_j.remove_space_for_filename(view_name);
		try{
			 // Trigger the popup with the image
			wxString filePath = CurrDir + "/friends/faces_large/" + view_name + ".jpg";
			if(wxFileExists(filePath)){
				ImagePopup* popup = new ImagePopup("Image Viewer", filePath);
				popup->Show(true);  // Show the popup window
			}
		}
		catch(...){}
	}
	else if(current_display == CURRENT_DISPLAY::strangers){
		if(id > strangers_info.size()){
			return;
		}
		try{
			if(!friends_info.empty() && !strangers_info.empty()){
				std::string view_name = strangers_info[id].name;
				if(view_name.empty()){
					return;
				}
				nemswebcam_j.remove_space_for_filename(view_name);
				wxString filePath = CurrDir + "/strangers/faces_large/" + view_name + ".jpg";
				if(wxFileExists(filePath)){
					 // Trigger the popup with the image
					ImagePopup* popup = new ImagePopup("Image Viewer", filePath);
					popup->Show(true);  // Show the popup window
				}
			}
		}
		catch(...){}
	}
}
void frmMain::OnDeleteButtonClicked(wxCommandEvent& event){
	try{
		htmWinListView_main->set_friends_in_main(friends_info);
		if(current_display == CURRENT_DISPLAY::strangers){
			htmWinListView_main->set_current_list_type(0);
		}
		if(current_display == CURRENT_DISPLAY::friends){
			htmWinListView_main->set_current_list_type(1);
		}
		htmWinListView_main->refresh_list();
		// Ensure the main layout re-adjusts
		this->Layout();
		this->Refresh();
	}
	catch(...){}
}
// **Event Handlers**
void frmMain::OnWebCamSetup(wxCommandEvent& WXUNUSED(event)) {
	stop_webcam();//stop the main webcam catch;
	// Check if the WebCam Setup window is already open
    if (webcamSetupFrame) {
        webcamSetupFrame->Raise(); // Bring the existing window to the top
        return;
    }
    // Create and show the webcam setup frame
    webcamSetupFrame = new frmWebCamSetup(
        this,                        // Parent is frmMain
        wxID_ANY,                    // Use default ID
        _(":::WebCam Setup:::"),           // Frame title
        wxDefaultPosition,           // Default position
        wxSize(525, 395),            // Frame size (adjust as necessary)
        //wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL // Window style
		wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) | wxFRAME_FLOAT_ON_PARENT//No resize
    );
    webcamSetupFrame->Show(true);    // Show the frame
	// Bring the window to the top
    webcamSetupFrame->Raise();
	// Bind the close event to reset the pointer when the window is closed
    webcamSetupFrame->Bind(wxEVT_CLOSE_WINDOW, &frmMain::OnWebCamSetup_Closed, this);
	frmMain_status->SetStatusText(":::Webcam setup:::",0);
}
void frmMain::OnWebCamSetup_Closed(wxCloseEvent& event){
	// Reset the pointer to allow reopening the window
    webcamSetupFrame = nullptr;
	start_webcam();
	// Proceed with the default close behavior
    event.Skip();
}
void frmMain::OnFaceRecognitionSetup(wxCommandEvent& WXUNUSED(event)) {
	stop_webcam();//stop the main webcam catch;
	// Check if the WebCam Setup window is already open
    if (faceSetupFrame) {
        faceSetupFrame->Raise(); // Bring the existing window to the top
        return;
    }
    // Create and show the webcam setup frame
    faceSetupFrame = new frmFaceDetectionSetup(
        this,                        // Parent is frmMain
        wxID_ANY,                    // Use default ID
        _(":::Friend list setup:::"),           // Frame title
        wxDefaultPosition,           // Default position
        wxSize(420, 660),            // Frame size (adjust as necessary)
        //wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL // Window style
		wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) | wxFRAME_FLOAT_ON_PARENT//No resize
    );
    faceSetupFrame->Show(true);    // Show the frame
	faceSetupFrame->Raise();
	// Bind the close event to reset the pointer when the window is closed
    faceSetupFrame->Bind(wxEVT_CLOSE_WINDOW, &frmMain::OnFaceRecognitionSetup_Closed, this);
	frmMain_status->SetStatusText(":::Friend list setup:::", 0);
}
void frmMain::OnFaceRecognitionSetup_Closed(wxCloseEvent& event){
	faceSetupFrame->Unbind(wxEVT_CLOSE_WINDOW,&frmMain::OnFaceRecognitionSetup_Closed,this);
	// Reset the pointer to allow reopening the window
    faceSetupFrame = nullptr;
//	/*
//	 * reload people list for main camera 
//	*/
	if(!friends_info.empty()){
		friends_info.clear();
	}
	if(std::filesystem::exists(CurrDir + "/friends.dat")){
		nemswebcam_j.ReadFriendsFromBinary(CurrDir + "/friends.dat",friends_info);
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	start_webcam();
	// Proceed with the default close behavior
    event.Skip();
}
void frmMain::OnStrangersCleanup(wxCommandEvent& event){
	if (wxMessageBox("Are you sure you want to delete all strangers?",
                 "Confirmation",
					 wxYES_NO | wxICON_QUESTION) == wxNO) {
		return;
	} 
	try{
		if(!strangers_info.empty()){
			strangers_info.clear();
			current_display = CURRENT_DISPLAY::friends;
			htmWinListView_main->set_frame_title("::: Friends :::");
			htmWinListView_main->clear_list();
			if(!friends_info.empty()){
				for(const auto& item : friends_info){
					//wxBitmap ConvertCvMatToWxBitmap(const cv::Mat& mat);
					wxBitmap wx_bitmap = nemswebcam_j.ConvertCvMatToWxBitmap(item.face_image);
					htmWinListView_main->Disable_Add_button = true;
					htmWinListView_main->addPerson(item.name, wx_bitmap, "", wx_bitmap);
				}
				current_display = CURRENT_DISPLAY::friends;
			}
			htmWinListView_main->refresh_list();
			// Ensure the main layout re-adjusts
			this->Layout();
			this->Refresh();
		}
		stranger_count = 0;
		setMaxStrangersCount(stranger_count);
		std::string face_folder = CurrDir + "/strangers/faces";
		try{
			if(std::filesystem::exists(face_folder) && std::filesystem::is_directory(face_folder)){
				for(const auto& entry : std::filesystem::directory_iterator(face_folder)){
					std::filesystem::permissions(entry.path(),std::filesystem::perms::owner_all,std::filesystem::perm_options::add);
					std::filesystem::remove_all(entry.path());
				}
			}
		}
		catch(...){}
		std::string face_large_folder = CurrDir + "/strangers/faces_large";
		try{
			if(std::filesystem::exists(face_large_folder) && std::filesystem::is_directory(face_large_folder)){
				for(const auto& entry : std::filesystem::directory_iterator(face_large_folder)){
					std::filesystem::permissions(entry.path(),std::filesystem::perms::owner_all,std::filesystem::perm_options::add);
					std::filesystem::remove_all(entry.path());
				}
			}
		}
		catch(...){}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
	wxMessageBox("Successfully removed all strangers.", "Info", wxOK);
}
void frmMain::process_detection_webcam(
const cv::Mat& frame,
const std::vector<std::pair<cv::Mat,std::string>>& friends_detected,
const std::vector<cv::Mat>& strangers_detected
){
	   
}
std::string frmMain::checkExistingFace(const cv::Mat& img_input, const lib_face_recog::CURRENT_FaceType& face_type) {
	if(img_input.empty()){
		return "friends_info_empty";
	}
	std::string friends_face_folder = CurrDir + "/friends/faces";
	if (!std::filesystem::exists(friends_face_folder) || !std::filesystem::is_directory(friends_face_folder)) {
        return "friends_info_empty";
    }
	std::string strangers_face_folder = CurrDir + "/strangers/faces";
	if (!std::filesystem::exists(strangers_face_folder) || !std::filesystem::is_directory(strangers_face_folder)) {
        return "strangers_info_empty";
    }
	std::string stranger_id;
	try{
		if(face_type == lib_face_recog::CURRENT_FaceType::face_friends){
			bool is_a_friend = false;
			std::string person_name;
			libface_j.person_exists(img_input,lib_face_recog::CURRENT_FaceType::face_friends,is_a_friend,person_name);
			if(is_a_friend){
				return "f" + person_name;
			}
			else{
				if(!friends_info.empty()){
					bool is_a_stranger = false;
					std::string person_name;
					libface_j.person_exists(img_input,lib_face_recog::CURRENT_FaceType::face_strangers,is_a_stranger,person_name);
					if(is_a_stranger){
						return "1stranger" + person_name;
					}
					else{
						getMaxStrangersCount(stranger_count);
						stranger_count++;
						stranger_id = (stranger_count < 9) ? "0" + std::to_string(stranger_count + 1) : std::to_string(stranger_count + 1);
						setMaxStrangersCount(stranger_count);
						return "0stranger" + stranger_id;
					}
				}
				else{
					return "friends_info_empty";
				}
			}
		}//if
	}//try
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
	return "cant_be_recognized";
}
void frmMain::onFacesDetected(const std::vector<cv::Rect>& faces, cv::Mat& frame) {
    if (faces.empty()) return;
	try{
		for(size_t i = 0; i < faces.size(); ++i){
			cv::Rect faceROI = faces[i]; // Save the first detected face
			cv::Mat faceImage = frame(faceROI).clone();
			cv::resize(faceImage, faceImage, cv::Size(80, 80));  
			std::string str_return = checkExistingFace(faceImage,lib_face_recog::CURRENT_FaceType::face_friends);
			if(!str_return.empty()){
				if(str_return == "friends_info_empty"){
					current_display = CURRENT_DISPLAY::friends;
					libface_j.mark_on_the_person(frame,faces,"Friend's list is empty");
				}
				else if(str_return == "cant_be_recognized"){
					current_display = CURRENT_DISPLAY::strangers;
					libface_j.mark_on_the_person(frame,faces,str_return);
				}
				else{
					// Get the current date and time
					time_t now = time(0);
					tm* local_time = localtime(&now);
					char time_buffer[100];
					strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time);
					std::string str_time = std::string(time_buffer);
					if(str_return.substr(0,1) == "f"){//friends
						pub_lib::PersonBinInfo a_friend;
						a_friend.face_img = faceImage;
						a_friend.face_large_img = frame;
						str_return = str_return.substr(1);
						a_friend.name = str_return;
						a_friend.captured_date_time = str_time;
						/*
						 * save to disk remove_space_for_filename
						*/
						nemswebcam_j.remove_space_for_filename(str_return);
						cv::imwrite(CurrDir + "/friends/faces_large/" + str_return + ".jpg" , frame);
						libface_j.mark_on_the_person(frame,faces,str_return);
						current_display = CURRENT_DISPLAY::friends;
						htmWinListView_main->set_frame_title("::: Friends :::");
					}
					else if(str_return.substr(0,2) == "1s"){
						current_display = CURRENT_DISPLAY::strangers;
						htmWinListView_main->set_frame_title("::: Strangers :::");
						libface_j.mark_on_the_person(frame,faces,str_return.substr(1));
						return;
					}
					else if(str_return.substr(0,2) == "0s"){
						if(friends_info.empty()){
							return;
						}
						std::string str_return_stranger = str_return.substr(1);
						if(str_return_stranger == "stranger"){
							return;
						}
						cv::imwrite(CurrDir + "/strangers/faces/" + str_return_stranger + ".jpg" , faceImage);
						cv::imwrite(CurrDir + "/strangers/faces_large/" + str_return_stranger + ".jpg" , frame);
						htmWinListView_main->set_frame_title("::: Strangers :::");
						libface_j.mark_on_the_person(frame,faces,str_return_stranger);
						/*
						 * add to list 
						*/
						current_display = CURRENT_DISPLAY::strangers;
						htmWinListView_main->set_frame_title("::: Strangers :::");
						libface_j.get_strangers_and_display(strangers_info);
						htmWinListView_main->clear_list();
						htmWinListView_main->set_people_list(strangers_info);
						htmWinListView_main->refresh_list();
						libface_j.mark_on_the_person(frame,faces,str_return_stranger);
						current_display = CURRENT_DISPLAY::strangers;
						htmWinListView_main->set_frame_title("::: Strangers :::");
						htmWinListView_main->refresh_list();
						// Ensure the main layout re-adjusts
						this->Layout();
						this->Refresh();
					}
				}
			}//if return string is not empty
			else{
				libface_j.mark_on_the_person(frame,faces,"");
			}
		}//for
	}//try
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
	htmWinListView_main->refresh_list();
	// Ensure the main layout re-adjusts
	this->Layout();
	this->Refresh();
}
void frmMain::OnTimer(wxTimerEvent& event) {
	cv::Mat uiFrame;
	{
		std::lock_guard<std::mutex> lock(frameMutex);
		if(frame.empty()){
			return;
		}
		uiFrame = frame.clone();
		//faceReg
		try{
			cv::cvtColor(uiFrame, uiFrame, cv::COLOR_BGR2RGB);
			cv::Mat low_quality_frame = uiFrame / (256 / cvMatlevels) * (256 / cvMatlevels);
			// Resize the frame to fit the wxStaticBitmap
			cv::Mat resized_frame;
			cv::resize(low_quality_frame, resized_frame, cv::Size(640, 320));//0.5, 0.5
			libface_j.AddDateTimeOverlay(resized_frame);
			/*
			 * Facial detection 
			 */
			cv::Mat gray;  
			cv::cvtColor(resized_frame, gray, cv::COLOR_BGR2GRAY);  
			std::vector<cv::Rect> faces;  
			faceCascade.detectMultiScale(gray, faces, 1.1, 10, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(50, 50));  
			if (!faces.empty()) {  
				onFacesDetected(faces, resized_frame);  
			}  
			wxImage image(resized_frame.cols, resized_frame.rows, resized_frame.data, true);
			wxBitmap bitmap(image);
			// Update the static bitmap and refresh
			bitmap_video_clip_main->SetBitmap(bitmap);
			Refresh();
		}
		catch (const std::exception& ex) {
			std::cerr << "Exception in OnTimer: " << ex.what() << std::endl;
		} catch (...) {
			std::cerr << "Unknown error in OnTimer." << std::endl;
		}	
	}
}
void frmMain::capture_thread_webcam(){
	try{
		while(!stopCapture){
			cv::Mat tempFrame;
			cap >> tempFrame;
			if(tempFrame.empty()) continue;
			{
				std::lock_guard<std::mutex> lock(frameMutex);
				frame = tempFrame.clone();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void frmMain::OnExit(wxCommandEvent& WXUNUSED(event)) {
    Close(true); // Close the frame when "Exit" is selected
	wxTheApp->ExitMainLoop(); // Terminate the main event loop, effectively shutting down the program
}