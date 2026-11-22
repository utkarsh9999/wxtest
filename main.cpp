#include <wx/wx.h>
#include <wx/sizer.h>
#include <opencv2/opencv.hpp>

// 1. Define a custom panel dedicated to rendering OpenCV matrices
class CameraPanel : public wxPanel {
    cv::VideoCapture cap;
    cv::Mat frame;
    wxImage image;
    wxBitmap bitmap;
    wxTimer* timer;

public:
    CameraPanel(wxWindow* parent) : wxPanel(parent) {
        // Open the default camera (0). Use an alternate index if needed.
        cap.open(0);
        if (!cap.isOpened()) {
            wxMessageBox("Error: Could not open the camera feed.", "Camera Error", wxOK | wxICON_ERROR);
        }

        // Bind the paint event to handle rendering operations
        Bind(wxEVT_PAINT, &CameraPanel::OnPaint, this);

        // Setup a fast continuous timer (~30 FPS -> 33ms interval)
        timer = new wxTimer(this);
        Bind(wxEVT_TIMER, &CameraPanel::OnTimer, this, timer->GetId());
        timer->Start(33);
    }

    ~CameraPanel() {
        timer->Stop();
        delete timer;
        if (cap.isOpened()) {
            cap.release();
        }
    }

private:
    void OnTimer(wxTimerEvent& event) {
        if (!cap.isOpened()) return;

        cap >> frame; // Pull a new matrix from the camera
        if (frame.empty()) return;

        // OpenCV natively reads BGR pixels; convert to standard RGB for wxWidgets
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // Allocate a raw data buffer block and copy OpenCV pixels into the canvas wrapper
        unsigned char* data = (unsigned char*)malloc(frame.rows * frame.cols * 3);
        memcpy(data, frame.data, frame.rows * frame.cols * 3);
        
        // Wrap the raw buffer in a wxImage object (wxWidgets claims buffer ownership)
        image.Create(frame.cols, frame.rows, data, false);
        bitmap = wxBitmap(image);

        // Request a screen paint update pass
        Refresh(false);
    }

    void OnPaint(wxPaintEvent& event) {
        wxPaintDC dc(this);
        if (bitmap.IsOk()) {
            // Blit the active frame directly onto the panel canvas surface
            dc.DrawBitmap(bitmap, 0, 0, false);
        }
    }
};

// 2. Main Window Application Frame
class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "OpenCV 5 + wxWidgets Stream", wxDefaultPosition, wxSize(660, 520)) {
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        CameraPanel* camPanel = new CameraPanel(this);
        
        sizer->Add(camPanel, 1, wxEXPAND | wxALL, 5);
        SetSizer(sizer);
        Layout();
    }
};

// 3. Application Entry point
class MyApp : public wxApp {
public:
    virtual bool OnInit() {
        MyFrame* frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);
