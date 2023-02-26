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

class SettingsDialog : public wxDialog
{
public:
    bool success = false;
    std::string name;

private:
    wxButton *cancelButton;
    wxButton *saveButton;

    wxTextCtrl *textCtrl;
    wxCheckBox *showConsoleCheckBox;

    void CancelButtonClick(wxCommandEvent &e);
    void SaveButtonClick(wxCommandEvent &e);

public:
    SettingsDialog(wxWindow *parent, wxWindowID id);
};
