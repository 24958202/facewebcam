#ifndef PERSON_INFO_H
#define PERSON_INFO_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "public_variables.h"
#include "nemslib_webcam.h"
#include "lib_face_recog.h"
class IEventHandler {
public:
	virtual void OnAddButtonClicked(wxCommandEvent& event) = 0;
	virtual void OnViewButtonClicked(wxCommandEvent& event) = 0;
    virtual void OnDeleteButtonClicked(wxCommandEvent& event) = 0;
	virtual ~IEventHandler()=default;
};
class PersonFrame : public wxPanel {
public:
    PersonFrame(IEventHandler*,wxWindow*, wxWindowID, const wxPoint&, const wxSize&, const std::string&);
    ~PersonFrame();
	void set_frame_title(const std::string&);
	void set_current_list_type(size_t);
    void populateScrolledWindow();
    void addPerson(const std::string&, const wxBitmap&, const std::string&, const wxBitmap&);
	void set_people_list(const std::vector<pub_lib::PersonBinInfo>&);
	std::vector<pub_lib::PersonBinInfo> get_people_list();
	void set_friends_in_main(const std::vector<pub_lib::Friend>&);
	void clear_list();
	void refresh_list();
	void onAddPerson(wxCommandEvent& event);//add to friends
	void onViewPerson(wxCommandEvent& event);//view large images
	void onDeletePerson(wxCommandEvent& event);//delete from list
	bool Disable_Add_button = false;
private:
	enum {
		ADD_BUTTON_CLICKED = wxID_ANY + 1000,
		VIEW_BUTTON_CLICKED = wxID_ANY + 2000,
		DELETE_BUTTON_CLICKED = wxID_ANY + 3000
	};
	size_t current_list_type_ = 0;
	std::string CurrDir;
	wxStaticText* titleCtrl = nullptr;
	IEventHandler* eventHandler_;
    wxScrolledWindow* scrolledWindow;
    std::vector<pub_lib::Friend> friends_in_Main;
	std::vector<pub_lib::PersonBinInfo> people;
	std::map<int, wxButton*> add_buttons;//add to friends
	std::map<int, wxButton*> view_buttons;//view large images
    std::map<int, wxButton*> del_buttons;//delete
	std::string strTitle; 
	nemslib_webcam nemslib_j;
	lib_face_recog libface_j;
};
#endif