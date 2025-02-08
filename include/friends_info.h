#ifndef FRIENDS_INFO_H
#define FRIENDS_INFO_H
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "public_variables.h"
#include "nemslib_webcam.h"
class IFriendsEventHandler {
public:
    virtual void OnDeleteButtonClicked(wxCommandEvent& event) = 0;
	virtual ~IFriendsEventHandler()=default;
};
class FriendsFrame : public wxPanel{
public:
    FriendsFrame(IFriendsEventHandler*,wxWindow*, wxWindowID, const wxPoint&, const wxSize&);
    ~FriendsFrame();
	std::vector<pub_lib::Friend> get_friends_set();
	void set_friend_set(const std::vector<pub_lib::Friend>&);
    void populateScrolledWindow();
    void addPerson(const wxString&, const wxBitmap&);
	void refresh_list();
	void onDeleteFriend(wxCommandEvent& event);//delete from list
private:
	IFriendsEventHandler* eventHandler_;
    wxScrolledWindow* scrolledWindow;
    std::vector<pub_lib::Friend> friends_set;
    std::map<int, wxButton*> del_buttons;//delete
	std::string curr_dir;
	nemslib_webcam nemslib_j;
};
#endif