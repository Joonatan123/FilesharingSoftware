#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/popupwin.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <iostream>

#pragma once

class NetworkInfoDialog : public wxDialog
{
public:
    bool success = false;
    unsigned short port;
    std::string hostname;

private:
    const unsigned short defaultPort;

    wxStaticText *notifyText;

    wxButton *cancelButton;
    wxButton *connectButton;

    wxTextCtrl *textCtrl;
    wxTextCtrl *portCtrl;

    wxCheckBox *portCheckBox;
    void CancelButtonClick(wxCommandEvent &e);
    void ConnectButtonClick(wxCommandEvent &e);
    void CheckBoxClick(wxCommandEvent &e);

public:
    NetworkInfoDialog(wxWindow *parent, wxWindowID id, unsigned short defaultPort);
};
