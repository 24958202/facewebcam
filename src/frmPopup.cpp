#include "frmPopup.h"
// Define the constructor for the ImagePopup class
ImagePopup::ImagePopup(const wxString& title, const wxString& imagePath)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)) {
    // Load the image into wxImage
    wxImage image;
    if (!image.LoadFile(imagePath)) {
        wxMessageBox("Could not load image!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    // Optional: Rescale the image to fit the window
    wxSize clientSize = GetClientSize();
    image.Rescale(clientSize.GetWidth(), clientSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
    // Display the image using wxStaticBitmap
    wxStaticBitmap* imageCtrl = new wxStaticBitmap(this, wxID_ANY, wxBitmap(image));
}
