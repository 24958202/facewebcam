#include "friends_info.h"
FriendsFrame::FriendsFrame(IFriendsEventHandler* eventHandler, wxWindow* parent, wxWindowID id, const wxPoint& pos,
	const wxSize& size) : wxPanel(parent, id, pos, size), nemslib_j(), eventHandler_(eventHandler){
    scrolledWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    scrolledWindow->SetScrollRate(0, 20);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(scrolledWindow, 1, wxEXPAND | wxALL, 5);//wxEXPAND | wxEXPAND
    SetSizer(mainSizer);
	/*
	 * initialize title 
	*/
	scrolledWindow->DestroyChildren();
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);//wxVERTICAL
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* ImageTitle = new wxStaticText(scrolledWindow, wxID_ANY, "Picture                ");
	buttonSizer->Add(ImageTitle, 1, wxEXPAND, 5);
	wxStaticText* NameTitle = new wxStaticText(scrolledWindow, wxID_ANY, "Name");
	buttonSizer->Add(NameTitle, 1, wxEXPAND, 5);
	sizer->Add(buttonSizer, 0, wxEXPAND, 5);
	scrolledWindow->SetSizer(sizer);
    scrolledWindow->Layout();
    scrolledWindow->FitInside();
    scrolledWindow->SetSize(scrolledWindow->GetBestSize());
    scrolledWindow->Refresh();
    scrolledWindow->Update();
	curr_dir = nemslib_j.getExecutableDirectory();
}
FriendsFrame::~FriendsFrame() {
	try{
		// Destructor logic if needed
		if(eventHandler_){
			eventHandler_ = nullptr;
		}
//		for(auto& pair : del_buttons){
//			if(pair.second){
//				delete pair.second;
//				pair.second = nullptr;
//			}
//		}
		del_buttons.clear();
	}
	catch(...){}
}
std::vector<pub_lib::Friend> FriendsFrame::get_friends_set(){
	return friends_set;
}
void FriendsFrame::set_friend_set(const std::vector<pub_lib::Friend>& input_friends){
	try{
		friends_set.clear();
		friends_set.insert(friends_set.end(),input_friends.begin(),input_friends.end());
	}
	catch(...){}
}
void FriendsFrame::populateScrolledWindow() {
	del_buttons.clear();
    scrolledWindow->DestroyChildren();
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);//wxVERTICAL
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* ImageTitle = new wxStaticText(scrolledWindow, wxID_ANY, "Picture                ");
	buttonSizer->Add(ImageTitle, 0, wxEXPAND, 5);
	wxStaticText* NameTitle = new wxStaticText(scrolledWindow, wxID_ANY, "Name");
	buttonSizer->Add(NameTitle, 0, wxEXPAND, 5);
	sizer->Add(buttonSizer, 0, wxEXPAND, 5);
    for (size_t i = 0; i < friends_set.size(); ++i) {
	    wxBoxSizer* personSizer = new wxBoxSizer(wxHORIZONTAL);//wxHORIZONTAL
	    wxBitmap bt_image = nemslib_j.ConvertCvMatToWxBitmap(friends_set[i].face_image);;
		wxImage image = bt_image.ConvertToImage();
	    image.Rescale(70, 70, wxIMAGE_QUALITY_HIGH);
        wxBitmap resizedBitmap = wxBitmap(image);
        wxStaticBitmap* imageCtrl = new wxStaticBitmap(scrolledWindow, wxID_ANY, resizedBitmap);
        personSizer->Add(imageCtrl, 1, wxEXPAND, 5);
        wxStaticText* nameCtrl = new wxStaticText(scrolledWindow, wxID_ANY, friends_set[i].name);
        personSizer->Add(nameCtrl, 1, wxEXPAND, 5);
		wxButton* deleteButton = new wxButton(scrolledWindow, 1000 + i, "Delete");
		del_buttons[1000 + i] = deleteButton;
		personSizer->Add(deleteButton, 1, wxEXPAND, 5);
        deleteButton->Bind(wxEVT_BUTTON, &FriendsFrame::onDeleteFriend, this);
        sizer->Add(personSizer, 1, wxEXPAND | wxALL, 5);
    }
    scrolledWindow->SetSizer(sizer);
    scrolledWindow->Layout();
    scrolledWindow->FitInside();
    scrolledWindow->SetSize(scrolledWindow->GetBestSize());
    scrolledWindow->Refresh();
    scrolledWindow->Update();
}
void FriendsFrame::addPerson(const wxString& name, const wxBitmap& image){
	if(name.empty()){
		return;
	}
	try{
		cv::Mat cv_image = nemslib_j.ConvertWxBitmapToCvMat(image);
		pub_lib::Friend person(std::string(name.ToStdString()),cv_image);
		friends_set.push_back(person);
		populateScrolledWindow();
	}
	catch(...){}
}
void FriendsFrame::refresh_list(){
	populateScrolledWindow();            // Re-populate the scrolled window
    this->Layout();                      // Re-layout the PersonFrame itself
    scrolledWindow->Layout();            // Re-layout the scrolled window
    scrolledWindow->Refresh();           // Force the scrolled window to redraw
    scrolledWindow->Update();     
}
void FriendsFrame::onDeleteFriend(wxCommandEvent& event) {// delete this person's info 
	int result = wxMessageBox("Are you sure you want to delete this person?",
                          "Confirm",
                          wxYES_NO | wxICON_QUESTION);
    if(result == wxNO){
		return;
	}
    int id = event.GetId();
    if (del_buttons.find(id) != del_buttons.end()) {
        // Remove person from vector
        for (size_t i = 0; i < friends_set.size(); ++i) {
            if (1000 + i == id) {
				std::string friend_name = friends_set[i].name;
				nemslib_j.remove_space_for_filename(friend_name);
				std::string remove_face_path = curr_dir + "/friends/faces/" + friend_name + ".jpg";
				std::string remove_face_large_path = curr_dir + "/friends/faces_large/" + friend_name + ".jpg";
				if(std::filesystem::exists(remove_face_path)){
					std::filesystem::remove(remove_face_path);
				}
				if(std::filesystem::exists(remove_face_large_path)){
					std::filesystem::remove(remove_face_large_path);
				}
				friends_set.erase(friends_set.begin() + i);
				if(!friends_set.empty()){
					nemslib_j.WriteFriendsToBinary(friends_set, curr_dir + "/friends.dat");
				}
			    break;
            }
        }
		try{
			refresh_list();
			// Create a custom event and post it to the parent window (frmMain)
			if (eventHandler_ != nullptr) {
                // Create a custom event and post it to the parent window (frmMain)
                wxCommandEvent deleteEvent(wxID_ANY + 1, id);
                deleteEvent.SetEventObject(this);
                eventHandler_->OnDeleteButtonClicked(deleteEvent);
            }
		}
		catch(...){}
    } else {
        wxLogError("Invalid button ID passed to onDeletePerson: %d", id);
    }
}

