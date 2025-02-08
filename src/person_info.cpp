#include "person_info.h"
PersonFrame::PersonFrame(IEventHandler* eventHandler, wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size, const std::string& frame_title) : wxPanel(parent, id, pos, size), 
	eventHandler_(eventHandler), strTitle(frame_title), nemslib_j(){
    scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scrolledWindow->SetScrollRate(0, 20);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(scrolledWindow, 1, wxEXPAND | wxEXPAND, 5);
    SetSizer(mainSizer);
	CurrDir = nemslib_j.getExecutableDirectory();
}
PersonFrame::~PersonFrame() {
	try{
		friends_in_Main.clear();
		// Destructor logic if needed
		if(titleCtrl){
			delete titleCtrl;
			titleCtrl = nullptr;
		}
	}
	catch(...){}
}
void PersonFrame::set_frame_title(const std::string& str_title){
	if(str_title.empty()){
		return;
	}
	try{
		if(titleCtrl){
			wxString sTitle(str_title);
			titleCtrl->SetLabel(sTitle);//wxString::FromUTF8(str_title)
		}
	}
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors." << std::endl;
	}
}
void PersonFrame::set_current_list_type(size_t int_type){
	current_list_type_ = int_type;
}
void PersonFrame::populateScrolledWindow() {
	if(people.empty()){
		return;
	}
    scrolledWindow->DestroyChildren();
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);//wxVERTICAL
	titleCtrl = new wxStaticText(scrolledWindow, wxID_ANY, strTitle);//wxStaticText* 
	sizer->Add(titleCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    for (size_t i = 0; i < people.size(); ++i) {
		wxBoxSizer* personSizer = new wxBoxSizer(wxVERTICAL);//wxHORIZONTAL
		wxBitmap wx_bitmap = nemslib_j.ConvertCvMatToWxBitmap(people[i].face_img);
        wxImage image = wx_bitmap.ConvertToImage();
        image.Rescale(70, 70, wxIMAGE_QUALITY_HIGH);
        wxBitmap resizedBitmap = wxBitmap(image);
        wxStaticBitmap* imageCtrl = new wxStaticBitmap(scrolledWindow, wxID_ANY, resizedBitmap);
        personSizer->Add(imageCtrl, 0, wxEXPAND, 5);
        wxStaticText* nameCtrl = new wxStaticText(scrolledWindow, wxID_ANY, people[i].name);
        personSizer->Add(nameCtrl, 1, wxEXPAND, 5);
		wxStaticText* strDateTime = new wxStaticText(scrolledWindow, wxID_ANY, people[i].captured_date_time);
		personSizer->Add(strDateTime, 1, wxEXPAND, 5);
		wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
		wxButton* addButton = new wxButton(scrolledWindow, 1000 + i, "Add to friends");
		buttonSizer->Add(addButton, 1, wxEXPAND, 5);
		wxButton* viewButton = new wxButton(scrolledWindow, 2000 + i, "View");
		buttonSizer->Add(viewButton, 1, wxEXPAND, 5);
		personSizer->Add(buttonSizer, 1, wxEXPAND, 5);
		wxButton* deleteButton = new wxButton(scrolledWindow, 3000 + i, "Delete");
		personSizer->Add(deleteButton, 1, wxEXPAND, 5);
		add_buttons[1000 + i] = addButton;
		view_buttons[2000 + i] = viewButton;
        del_buttons[3000 + i] = deleteButton;
		addButton->Bind(wxEVT_BUTTON, &PersonFrame::onAddPerson, this);
		viewButton->Bind(wxEVT_BUTTON, &PersonFrame::onViewPerson, this);
        deleteButton->Bind(wxEVT_BUTTON, &PersonFrame::onDeletePerson, this);
        //personSizer->Add(deleteButton, 0, wxEXPAND, 5);
        sizer->Add(personSizer, 0, wxEXPAND | wxALL, 5);
		if(Disable_Add_button){
			addButton->Disable();
			deleteButton->Disable();
		}
    }
    scrolledWindow->SetSizer(sizer);
    scrolledWindow->Layout();
    scrolledWindow->FitInside();
    scrolledWindow->SetSize(scrolledWindow->GetBestSize());
    scrolledWindow->Refresh();
    scrolledWindow->Update();
}
void PersonFrame::addPerson(const std::string& name, const wxBitmap& image, const std::string& cap_time, const wxBitmap& img_large){
	if(name.empty()){
		return;
	}
    try{
		cv::Mat cv_image = nemslib_j.ConvertWxBitmapToCvMat(image);
		cv::Mat cv_image_large = nemslib_j.ConvertWxBitmapToCvMat(img_large);
		pub_lib::PersonBinInfo person(cv_image, cv_image_large, name, cap_time);
		people.push_back(person);
		populateScrolledWindow();
	}
	catch(...){}
}
void PersonFrame::set_people_list(const std::vector<pub_lib::PersonBinInfo>& p_list){
	if (!p_list.empty()) {
        people.clear(); // Clear the existing list of people
        for (const auto& item : p_list) { // Iterate over each PersonBinInfo in p_list
            pub_lib::PersonBinInfo person(
                 item.face_img,// Access the name of the current PersonBinInfo
				item.face_large_img,
				item.name,             // Access the small face image
                 item.captured_date_time // Access the capture time
            );
            people.push_back(person); // Add the person to the people vector
        }
    }
}
std::vector<pub_lib::PersonBinInfo> PersonFrame::get_people_list(){
	return people;
}
void PersonFrame::set_friends_in_main(const std::vector<pub_lib::Friend>& friends_from_main){
	friends_in_Main = friends_from_main;
}
void PersonFrame::clear_list(){
	if(!people.empty()){
		people.clear();
	}
}
void PersonFrame::refresh_list(){
	populateScrolledWindow();            // Re-populate the scrolled window
    this->Layout();                      // Re-layout the PersonFrame itself
    scrolledWindow->Layout();            // Re-layout the scrolled window
    scrolledWindow->Refresh();           // Force the scrolled window to redraw
    scrolledWindow->Update();     
}
void PersonFrame::onAddPerson(wxCommandEvent& event){//add to friends
	try{
		wxTextEntryDialog dialog(this,"Please input the person's name:", "Are you sure you want to add this person to your friend list?");
		if(dialog.ShowModal() == wxID_OK){
			wxString person_name_to_add = dialog.GetValue();
			int id = event.GetId() - 1000;
			if(id < 0){
				id = 0;
			}
			if(!people.empty()){
				auto person = people[id];
				std::string str_name_to_add;
				str_name_to_add = person_name_to_add;
				std::string str_msg;
				str_msg = person_name_to_add;
				nemslib_j.remove_space_for_filename(str_name_to_add);
				bool is_a_friend = false;
				std::string person_name;
				libface_j.person_exists(person.face_img,lib_face_recog::CURRENT_FaceType::face_friends,is_a_friend,person_name);
				if(is_a_friend){
					std::string strMsg = person_name + " is alreay in your friend's list";
					wxMessageBox(strMsg, "Info", wxOK);
					return;
				}
				cv::Mat face_image_to_add = person.face_img;
				pub_lib::Friend friend_to_add{
					str_name_to_add,
					face_image_to_add
				};
				friends_in_Main.push_back(friend_to_add);
				/*
				 * save to hard disk 
				 */
				nemslib_j.WriteFriendsToBinary(friends_in_Main, CurrDir + "/friends.dat");
				std::string out_face_path = CurrDir + "/friends/faces/" + str_name_to_add + ".jpg";
				cv::imwrite(out_face_path, face_image_to_add);
				// Create a custom event and post it to the parent window (frmMain)
				if (eventHandler_ != nullptr) {
					// Create a custom event and post it to the parent window (frmMain)
					wxCommandEvent addEvent(ADD_BUTTON_CLICKED, id);
					addEvent.SetEventObject(this);
					eventHandler_->OnAddButtonClicked(addEvent);
				}
			}
		}
	}//try
	catch(const std::exception& ex){
		std::cerr << ex.what() << std::endl;
	}
	catch(...){
		std::cerr << "Unknown errors" << std::endl;
	}
}
void PersonFrame::onViewPerson(wxCommandEvent& event){//view large image of the person
	int id = event.GetId() - 2000;
	if(id < 0){
		id = 0;
	}
	if (eventHandler_ != nullptr) {
		// Create a custom event and post it to the parent window (frmMain)
		wxCommandEvent viewEvent(VIEW_BUTTON_CLICKED, id);
		viewEvent.SetEventObject(this);
		eventHandler_->OnViewButtonClicked(viewEvent);
	}
}
void PersonFrame::onDeletePerson(wxCommandEvent& event) {// delete this person's info 
    int id = event.GetId();
    try{
		unsigned int id = event.GetId();
		id = id - 3000;
		if(id < 0){
			id = 0;
		}
		if (wxMessageBox("Are you sure you want to delete this person?",
					 "Confirmation",
					 wxYES_NO | wxICON_QUESTION) == wxNO) {
			return;
		} 
		if(current_list_type_ == 0){
			if(!people.empty()){
				people.erase(people.begin() + id);
				/*
				 * remove stranger's image from stranger folders 
				*/
				std::string stranger_name = people[id].name;
				std::string remove_face_path = CurrDir + "/strangers/faces/" + stranger_name + ".jpg";
				std::string remove_face_large_path = CurrDir + "/strangers/faces_large/" + stranger_name + ".jpg";
				if(std::filesystem::exists(remove_face_path)){
					std::filesystem::remove(remove_face_path);
				}
				if(std::filesystem::exists(remove_face_large_path)){
					std::filesystem::remove(remove_face_large_path);
				}
			}
			else{
				if(std::filesystem::exists(CurrDir + "/strangers.dat")){
					std::filesystem::remove(CurrDir + "/strangers.dat");
				}
			}
		}
		refresh_list();
		// Create a custom event and post it to the parent window (frmMain)
		if (eventHandler_ != nullptr) {
			// Create a custom event and post it to the parent window (frmMain)
			wxCommandEvent deleteEvent(DELETE_BUTTON_CLICKED, id);
			deleteEvent.SetEventObject(this);
			eventHandler_->OnDeleteButtonClicked(deleteEvent);
		}
	}
	catch(...){}
}
