#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/popupwin.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/image.h>
#include <wx/event.h>
#include <iostream>
#include <map>
#include <memory>

#include <boost/algorithm/string.hpp>

#include "icons/Icons.h"

#include "NetworkInfoDialog.hpp"
#include "SettingsDialog.hpp"
#include "fss-class.hpp"
#include "fss-client.hpp"
#include "UiListSortUtility.hpp"

wxDEFINE_EVENT(evtIMPORTANT_MESSAGE, wxCommandEvent);
wxDEFINE_EVENT(evtREDRAW_TREE, wxCommandEvent);
// wxDEFINE_EVENT(evtREDRAW_LIST, wxCommandEvent);

class ClientUi : public wxFrame
{
    FssClient client;
    std::map<int, ClientSystem> idClientMap;
    wxPanel *panel;
    wxTreeCtrl *tree;
    wxListCtrl *list;
    wxImageList *imageList;
    std::map<wxTreeItemId, FilesystemEntry> treeEntryMap;
    std::mutex drawMutex;
    wxStatusBar *bottomBar;
    std::map<wxTreeItemId, int> treeSystemMap;
    std::map<wxTreeItemId, int> treeFolderMap;
    std::map<long, FilesystemEntry> listEntryMap;
    std::map<long, int> listSystemMap;
    std::pair<std::string, unsigned short> m_serverinfo;
    boost::thread clientThread;
    int sortDirection = 1;

    void CloseApp(wxCloseEvent &event);
    void HandlerRedraw(wxCommandEvent &event);
    void HandlerImportantMessage(wxCommandEvent &event);
    void ShowConnectionDialog();
    void RemoveFolder(std::string folder);
    void RemoveButtonClick(wxCommandEvent &e);
    void RefreshButtonClick(wxCommandEvent &e);
    void ConnectionButtonClick(wxCommandEvent &e);
    void DownloadButtonClick(wxCommandEvent &e);
    void AddButtonClick(wxCommandEvent &e);
    void ShowSettingsDialog();
    void SettingsButtonClick(wxCommandEvent &e);
    void ColumnClick(wxListEvent &e);
    void CollapseButtonClick(wxCommandEvent &e);
    void ExpandButtonClick(wxCommandEvent &e);
    void TreeClick(wxTreeEvent &event);
    void DisplayFolder(wxTreeItemId id);
    void SortByColumn(int column);
    static int NameSortCallback(wxIntPtr item1, wxIntPtr item2, long direction);
    static int DateSortCallback(wxIntPtr item1, wxIntPtr item2, long direction);
    static int ExtensionSortCallback(wxIntPtr item1, wxIntPtr item2, long direction);
    static int SizeSortCallback(wxIntPtr item1, wxIntPtr item2, long direction);
    void MakeTreeSingle(int systemId, wxTreeItemId root);

public:
    void MakeTree();
    void FilesUpdate(int clientId, ClientSystem system, bool save = true);
    void SaveState();
    void DisplayImportantMessage(std::string message);
    void StartApp();
    ClientUi();
};

class Main : public wxApp
{
public:
    Main();
    bool OnInit();
};

wxIMPLEMENT_APP(Main);