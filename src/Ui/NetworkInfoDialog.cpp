#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/popupwin.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <iostream>

#include "NetworkInfoDialog.hpp"

void NetworkInfoDialog::CancelButtonClick(wxCommandEvent &e)
{
    this->DestroyChildren();
    this->Destroy();
}
void NetworkInfoDialog::ConnectButtonClick(wxCommandEvent &e)
{
    unsigned long tempPort;
    try
    {
        std::stoi(portCtrl->GetLineText(0).ToStdString());
        tempPort = std::stoul(portCtrl->GetLineText(0).ToStdString());
    }
    catch (std::exception &e)
    {
        notifyText->SetLabel("Invalid port");
        return;
    }
    if (tempPort > 65535)
    {
        notifyText->SetLabel("Invalid port");
    }
    success = true;
    hostname = textCtrl->GetLineText(0).ToStdString();
    port = tempPort;

    this->DestroyChildren();
    this->Destroy();
}
void NetworkInfoDialog::CheckBoxClick(wxCommandEvent &e)
{
    if (portCheckBox->IsChecked())
    {
        portCtrl->ChangeValue(std::to_string(defaultPort));
        portCtrl->SetEditable(false);
        portCtrl->SetBackgroundColour(wxColor(200, 200, 200));
    }
    else
    {
        portCtrl->SetEditable(true);
        portCtrl->SetBackgroundColour(wxColor(255, 255, 255));
    }
}

NetworkInfoDialog::NetworkInfoDialog(wxWindow *parent, wxWindowID id, unsigned short defaultPort) : defaultPort(defaultPort), wxDialog(parent, id, "Connect", wxDefaultPosition, wxSize(400, 250))
{
    int firstRow = 30;
    int secondRow = 80;
    int thirdRow = 140;

    int firstColumn = 40;
    int secondColumn = 160;

    wxPanel *panel = new wxPanel(this);

    notifyText = new wxStaticText(panel, wxID_ANY, "", wxPoint(275, secondRow));

    wxStaticText *staticText = new wxStaticText(panel, wxID_ANY, "Hostname: ", wxPoint(firstColumn, firstRow));
    wxTextCtrl *tempTextCtrl = new wxTextCtrl(panel, wxID_ANY, "12345");
    textCtrl = new wxTextCtrl(panel, wxID_ANY, "127.0.0.1", wxPoint(secondColumn, firstRow), wxSize(200, tempTextCtrl->GetSize().y));
    tempTextCtrl->Destroy();

    wxStaticText *portText = new wxStaticText(panel, wxID_ANY, "Port: ", wxPoint(firstColumn, secondRow));
    portCtrl = new wxTextCtrl(panel, wxID_ANY, std::to_string(defaultPort), wxPoint(secondColumn, secondRow));
    // portCheckBox = new wxCheckBox(panel, wxID_ANY, "default port", wxPoint(secondColumn, 120));
    // portCheckBox->Bind(wxEVT_CHECKBOX, &NetworkInfoDialog::CheckBoxClick, this);
    // portCheckBox->SetValue(true);

    cancelButton = new wxButton(panel, wxID_ANY, "Cancel", wxPoint(100, thirdRow));
    cancelButton->Bind(wxEVT_BUTTON, &NetworkInfoDialog::CancelButtonClick, this);
    connectButton = new wxButton(panel, wxID_ANY, "Connect", wxPoint(200, thirdRow));
    connectButton->Bind(wxEVT_BUTTON, &NetworkInfoDialog::ConnectButtonClick, this);
}
