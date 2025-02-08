#ifndef MYRESIZABLEBITMAPPANEL_H
#define MYRESIZABLEBITMAPPANEL_H
#include <wx/wx.h>
#include <wx/statbmp.h>
#include <wx/frame.h>
class MyResizableBitmapPanel : public wxPanel
{
public:
    MyResizableBitmapPanel(wxWindow* parent, const wxBitmap& bitmap)
        : wxPanel(parent, wxID_ANY), m_bitmap(bitmap) {
        Bind(wxEVT_PAINT, &MyResizableBitmapPanel::OnPaint, this);         // Handle the painting process
        Bind(wxEVT_SIZE, &MyResizableBitmapPanel::OnSize, this);           // Handle resizing of the panel
    }
    void SetBitmap(const wxBitmap& bitmap) {
        m_bitmap = bitmap;
        this->Refresh(); // Redraw the panel
    }
private:
    wxBitmap m_bitmap;
    void OnPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);
        if (m_bitmap.IsOk()) {
            // Get the size of the panel
            wxSize size = GetClientSize();
            // Scale the bitmap to fit the panel
            wxImage scaledImage = m_bitmap.ConvertToImage();
            scaledImage.Rescale(size.GetWidth(), size.GetHeight(), wxIMAGE_QUALITY_HIGH);
            wxBitmap scaledBitmap(scaledImage);
            // Draw the scaled bitmap
            dc.DrawBitmap(scaledBitmap, 0, 0, false);
		    dc.SetFont(wxFontInfo(10).Family(wxFONTFAMILY_DEFAULT).Style(wxFONTSTYLE_NORMAL).Weight(wxFONTWEIGHT_NORMAL));
        }
    }
    void OnSize(wxSizeEvent& event) {
        this->Refresh(); // Trigger a repaint whenever the panel is resized
        event.Skip(); // Allow further processing of the event
    }
};
#endif