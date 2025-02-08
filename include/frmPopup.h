#ifndef FRMPOPUP_H
#define FRMPOPUP_H
#include <wx/wx.h>
#include <wx/statbmp.h> // Required for wxStaticBitmap
#include <wx/frame.h>
class ImagePopup : public wxFrame {
public:
    ImagePopup(const wxString& title, const wxString& imagePath);
};
#endif