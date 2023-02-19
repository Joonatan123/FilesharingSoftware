#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/popupwin.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <iostream>

#include "SettingsDialog.hpp"

void SettingsDialog::CancelButtonClick(wxCommandEvent &e)
{
    this->DestroyChildren();
    this->Destroy();
}
void SettingsDialog::SaveButtonClick(wxCommandEvent &e)
{
    name = textCtrl->GetLineText(0).ToStdString();
    success = true;
    this->DestroyChildren();
    this->Destroy();
}

SettingsDialog::SettingsDialog(wxWindow *parent, wxWindowID id) : wxDialog(parent, id, "Settings", wxDefaultPosition, wxSize(500, 170))
{
    wxPanel *panel = new wxPanel(this);

    wxStaticText *staticText = new wxStaticText(panel, wxID_ANY, "Name of this PC: ", wxPoint(20, 20));
    wxTextCtrl *tempTextCtrl = new wxTextCtrl(panel, wxID_ANY, "12345");
    textCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(200, 20), wxSize(200, tempTextCtrl->GetSize().y));
    tempTextCtrl->Destroy();

    cancelButton = new wxButton(panel, wxID_ANY, "Cancel", wxPoint(20, 70));
    cancelButton->Bind(wxEVT_BUTTON, &SettingsDialog::CancelButtonClick, this);
    saveButton = new wxButton(panel, wxID_ANY, "Save", wxPoint(120, 70));
    saveButton->Bind(wxEVT_BUTTON, &SettingsDialog::SaveButtonClick, this);
}
