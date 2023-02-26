#include "ui-client.hpp"

void ClientUi::CloseApp(wxCloseEvent &event)
{
    LOG_INFO(main_app) << "closing";
    SaveState();
    event.Skip();
}
void ClientUi::HandlerRedraw(wxCommandEvent &event)
{
    LOG_INFO(main_app) << "Redraw tree" << std::endl;
    MakeTree();
}
void ClientUi::HandlerImportantMessage(wxCommandEvent &event)
{
    bottomBar->SetStatusText(event.GetString());
}
void ClientUi::ShowConnectionDialog()
{
    NetworkInfoDialog dialog(this, wxID_ANY, 12345);

    dialog.ShowModal();

    if (dialog.success)
    {
        std::cout << "Connecting to: " << dialog.hostname << "     port: " << dialog.port << std::endl;
        client.connect(dialog.hostname, dialog.port);
        m_serverinfo = std::pair<std::string, unsigned short>(dialog.hostname, dialog.port);
        SaveState();
    }
}
void ClientUi::RemoveFolder(std::string folder)
{
    client.RemoveFolder(folder);
    MakeTree();
    SaveState();
}
void ClientUi::RemoveButtonClick(wxCommandEvent &e)
{
    wxTreeItemId item = tree->GetFocusedItem();
    // LOG_DEBUG(main_app) << "remove buttonnn " << ((treeEntryMap.count(item) == 0 || treeSystemMap[item] != 0 || treeEntryMap[item].id != 0) ? "true" : "false");
    if (treeEntryMap.count(item) == 0 || treeSystemMap[item] != 0 || treeEntryMap[item].id != 0)
    {
        LOG_ERROR(main_app) << Log::Important(true) << "invalid selection for folder deletion";
        return;
    }
    LOG_INFO(main_app) << Log::Important(true) << "removing folder: " << treeEntryMap[item].name;
    RemoveFolder(treeEntryMap[item].name);
}
void ClientUi::RefreshButtonClick(wxCommandEvent &e)
{
    LOG_DEBUG(main_app) << "Refresh button";
    client.MakeFolders();
}
void ClientUi::ConnectionButtonClick(wxCommandEvent &e)
{
    ShowConnectionDialog();
}
void ClientUi::DownloadButtonClick(wxCommandEvent &e)
{
    wxDirDialog dialog(this, "Select download destination");

    if (dialog.ShowModal() == wxID_CANCEL)
        return;
    if (list->GetSelectedItemCount() == 0)
    {
        std::cout << "Cannot download: no files selected\n";
        return;
    }
    std::cout << dialog.GetPath() << std::endl;
    std::cout << "download count: " << list->GetSelectedItemCount() << "\n";
    int index = list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (!idClientMap[listSystemMap[index]].online)
    {
        std::cout << "Cannot download: peer offline\n";
        return;
    }
    std::vector<int> intVec;
    std::vector<FilesystemEntry> fsVec;
    while (index != -1)
    {
        if (listEntryMap[index].isDirectory)
        {
            LOG_ERROR(main_app) << "cannot download: cannot download directory";
            index = list->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            continue;
        }
        intVec.push_back(listSystemMap[index]);
        fsVec.push_back(listEntryMap[index]);
        std::cout << listEntryMap[index].name << " from id: " << listSystemMap[index] << std::endl;
        index = list->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    }
    if (fsVec.size() != 0)
        client.DownloadFiles(fsVec, intVec[0], dialog.GetPath().ToStdString());
}
void ClientUi::AddButtonClick(wxCommandEvent &e)
{
    wxDirDialog dialog(this);

    if (dialog.ShowModal() == wxID_CANCEL)
        return;
    std::cout << dialog.GetPath() << std::endl;
    std::vector<std::string> path{dialog.GetPath().ToStdString() + "/"}; // does not work on windows
    client.MakeFolders(path);
}
void ClientUi::ShowSettingsDialog()
{
    SettingsDialog dialog(this, wxID_ANY);
    dialog.ShowModal();
    if (dialog.success && dialog.name != "")
    {
        idClientMap[0].client_name = dialog.name;
        MakeTree();
        client.SetName(dialog.name);
    }
}
void ClientUi::SettingsButtonClick(wxCommandEvent &e)
{
    ShowSettingsDialog();
}
void ClientUi::ColumnClick(wxListEvent &e)
{
    SortByColumn(e.GetColumn());
}
void ClientUi::CollapseButtonClick(wxCommandEvent &e)
{
    tree->CollapseAll();
}

void ClientUi::ExpandButtonClick(wxCommandEvent &e)
{
    tree->ExpandAll();
}
void ClientUi::TreeClick(wxTreeEvent &event)
{
    wxTreeItemId item = event.GetItem();
    if (treeEntryMap.count(item) == 0)
        return;
    DisplayFolder(item);
}
void ClientUi::DisplayFolder(wxTreeItemId id)
{
    FilesystemEntry parent = treeEntryMap[id];
    parent.Print();
    // std::cout << "parent: ";
    // parent.Print();
    //  auto a = list->InsertItem(0, parent.name);
    if (parent.id == -1 || !parent.isDirectory)
        return;
    list->DeleteAllItems();
    // list->ClearAll();
    int counter = 0;
    listEntryMap.clear();
    for (FilesystemEntry entry : idClientMap[treeSystemMap[id]].folders[treeFolderMap[id]])
    {
        if (entry.parent_directory == parent.id)
        {
            // entry.Print();
            long listIndex = list->InsertItem(counter, entry.name);
            entry.Print();
            listEntryMap[listIndex] = entry;
            listSystemMap[listIndex] = treeSystemMap[id];
            /*list->SetItemData(listIndex, reinterpret_cast<wxIntPtr>(&(listEntryMap[listIndex])));
            if (!entry.isDirectory)
            {
                std::string extension = boost::filesystem::path(entry.name).extension().string();
                list->SetItem(listIndex, 1, entry.MakeTimeString());
                list->SetItem(listIndex, 2, extension);
                list->SetItem(listIndex, 3, entry.MakeFileSizeString());
            }*/

            counter++;
        }
    }
    sortDirection = 1;
    SortByColumn(0);
    // list->SetItem(a, 1, "0");
}
void ClientUi::SortByColumn(int column)
{
    std::vector<FilesystemEntry> temp;
    for (std::pair<long, FilesystemEntry> entry : listEntryMap)
    {
        temp.push_back(entry.second);
    }
    temp = SortList(temp, sortDirection, column);

    list->DeleteAllItems();
    // list->ClearAll();
    int counter = 0;
    listEntryMap.clear();
    for (FilesystemEntry entry : temp)
    {
        // entry.Print();
        long listIndex = list->InsertItem(counter, entry.name);
        entry.Print();
        listEntryMap[listIndex] = entry;
        // listSystemMap[listIndex] = treeSystemMap[id];
        list->SetItemData(listIndex, reinterpret_cast<wxIntPtr>(&(listEntryMap[listIndex])));
        if (!entry.isDirectory)
        {
            std::string extension = boost::filesystem::path(entry.name).extension().string();
            list->SetItem(listIndex, 1, entry.MakeTimeString());
            list->SetItem(listIndex, 2, extension);
            list->SetItem(listIndex, 3, entry.MakeFileSizeString());
        }

        counter++;
    }
    sortDirection = -sortDirection;
    return;
}
int ClientUi::NameSortCallback(wxIntPtr item1, wxIntPtr item2, long direction)
{
    FilesystemEntry *entry1 = reinterpret_cast<FilesystemEntry *>(item1);
    FilesystemEntry *entry2 = reinterpret_cast<FilesystemEntry *>(item2);
    std::cout << direction << std::endl;
    if (entry1->isDirectory && !entry2->isDirectory)
        return -1;
    if (entry2->isDirectory && !entry1->isDirectory)
        return 1;
    std::cout << direction << std::endl;
    if (boost::algorithm::to_lower_copy(entry1->name) < boost::algorithm::to_lower_copy(entry2->name))
        return 1 * direction;
    else
        return -1 * direction;
}
int ClientUi::DateSortCallback(wxIntPtr item1, wxIntPtr item2, long direction)
{
    FilesystemEntry *entry1 = reinterpret_cast<FilesystemEntry *>(item1);
    FilesystemEntry *entry2 = reinterpret_cast<FilesystemEntry *>(item2);
    if (entry1->isDirectory && !entry2->isDirectory)
        return -1;
    if (entry2->isDirectory && !entry1->isDirectory)
        return 1;
    if (entry2->isDirectory && entry1->isDirectory)
        return 0;
    return entry1->last_modified == entry2->last_modified ? 0 : (entry1->last_modified > entry2->last_modified ? -1 : 1) * direction;
}
int ClientUi::ExtensionSortCallback(wxIntPtr item1, wxIntPtr item2, long direction)
{
    FilesystemEntry *entry1 = reinterpret_cast<FilesystemEntry *>(item1);
    FilesystemEntry *entry2 = reinterpret_cast<FilesystemEntry *>(item2);
    if (entry1->isDirectory && !entry2->isDirectory)
        return -1;
    if (entry2->isDirectory && !entry1->isDirectory)
        return 1;
    if (entry2->isDirectory && entry1->isDirectory)
        return 0;
    return strcmp(boost::algorithm::to_lower_copy(boost::filesystem::path(entry1->name).extension().string()).c_str(),
                  boost::algorithm::to_lower_copy((boost::filesystem::path(entry2->name).extension().string())).c_str()) *
           direction;
}
int ClientUi::SizeSortCallback(wxIntPtr item1, wxIntPtr item2, long direction)
{
    FilesystemEntry *entry1 = reinterpret_cast<FilesystemEntry *>(item1);
    FilesystemEntry *entry2 = reinterpret_cast<FilesystemEntry *>(item2);
    if (entry1->isDirectory && !entry2->isDirectory)
        return -1;
    if (entry2->isDirectory && !entry1->isDirectory)
        return 1;
    if (entry2->isDirectory && entry1->isDirectory)
        return 0;
    return entry1->size == entry2->size ? 0 : (entry1->size > entry2->size ? -1 : 1) * direction;
}
void ClientUi::MakeTreeSingle(int systemId, wxTreeItemId root)
{
    // stores treeId of what should be the parent for the next entry
    std::vector<wxTreeItemId> treeId;
    int imageId = -1;
    if (systemId == 0)
        imageId = -1;
    else if (idClientMap[systemId].online)
        imageId = 0;
    else if (!idClientMap[systemId].online)
        imageId = 1;
    std::string name = idClientMap[systemId].client_name;
    if (systemId == 0)
        name = "This PC (" + idClientMap[0].client_name + ")";
    treeId.push_back(tree->AppendItem(root, name, imageId, -1, NULL));
    std::vector<std::vector<FilesystemEntry>> &folders = idClientMap[systemId].folders;
    int count = 0; // which folder
    for (std::vector<FilesystemEntry> &folder : folders)
    {
        for (int i = 0; i < folder.size(); i++)
        {
            wxTreeItemId id = tree->AppendItem(treeId[folder[i].depth], folder[i].name, -1, -1, NULL);
            treeEntryMap[id] = folder[i];
            treeSystemMap[id] = systemId;
            treeFolderMap[id] = count;
            if ((folder[i].depth + 1) == treeId.size())
            {
                treeId.push_back(id);
            }
            treeId[folder[i].depth + 1] = id;
        }
        count++;
    }
}
void ClientUi::MakeTree()
{
    drawMutex.lock();
    tree->DeleteAllItems();
    wxTreeItemId root = tree->AppendItem(tree->GetRootItem(), "root", -1, -1, NULL);
    for (auto it = idClientMap.begin(); it != idClientMap.end(); it++)
    {
        MakeTreeSingle(it->first, root);
    }
    drawMutex.unlock();
}
void ClientUi::FilesUpdate(int clientId, ClientSystem system, bool save)
{
    std::cout << "FIlesupdate\n";
    drawMutex.lock();
    idClientMap[clientId] = system;

    // MakeTree();
    drawMutex.unlock();
    if (save)
        SaveState();

    wxCommandEvent *event = new wxCommandEvent(evtREDRAW_TREE); // No specific id

    wxQueueEvent(this, event); // to ourselves
}
void ClientUi::SaveState()
{
    std::ofstream ofs("ClientSave");

    {
        boost::archive::text_oarchive oa(ofs);
        // write class instance to archive
        oa << m_serverinfo;
        client.SaveState(oa);
        // archive and stream closed when destructors are called
    }
}
void ClientUi::DisplayImportantMessage(std::string message)
{
    // Bind(myEVT_MYEVENT,&ClientUi::OnMyEvent);
    // MyEvent ev(this);

    wxCommandEvent *event = new wxCommandEvent(evtIMPORTANT_MESSAGE); // No specific id

    // Add any data; sometimes the only information needed at the destination is the arrival of the event itself
    event->SetString(message);
    // Then post the event
    wxQueueEvent(this, event); // to ourselves
}
void ClientUi::StartApp()
{
    // boost::this_thread::sleep_for(boost::chrono::milliseconds(200));
    LOG_INFO(main_app) << "StartApp";
    bool loadSuccess = true;
    bool fileExists = boost::filesystem::exists("ClientSave");
    Log::SetMessageCallback(boost::bind(&ClientUi::DisplayImportantMessage, this, boost::placeholders::_1));
    if (fileExists)
    {
        LOG_INFO(main_app) << "save file found";
        std::ifstream ifs("ClientSave");

        {
            try
            {
                boost::archive::text_iarchive ia(ifs);
                ia >> m_serverinfo;
                client.start(true, ia);
                LOG_INFO(main_app) << Log::Important() << "connecting hostname: " << m_serverinfo.first << " port: " << m_serverinfo.second;
                client.connect(m_serverinfo.first, m_serverinfo.second);
            }
            catch (std::exception e)
            {
                loadSuccess = false;
                LOG_ERROR(main_app) << Log::Important() << "save file corrupted or load error";
            }
            // write class instance to archive
            // write class instance to archive
        }
    }
    if (!fileExists || !loadSuccess)
    {
        m_serverinfo.first = "127.0.0.1";
        m_serverinfo.second = 12345;
        if (!fileExists)
            LOG_INFO(main_app) << "save not found";
        std::stringstream temp("");
        boost::archive::text_iarchive temp2(temp, boost::archive::archive_flags::no_header);
        client.start(false, temp2);

        ShowSettingsDialog();
        ShowConnectionDialog();
    }
}
ClientUi::ClientUi()
    : wxFrame(NULL, wxID_ANY, "Filesharing App", wxDefaultPosition, wxSize(1200, 800)),
      client(boost::bind(&ClientUi::FilesUpdate, this, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
{
    // boost::function<void(int , ClientSystem)> client123123 =(boost::bind(&ClientUi::FilesUpdate, this,boost::placeholders::_1,boost::placeholders::_2));
    // client.start();
    idClientMap[0] = ClientSystem("This PC");
    wxInitAllImageHandlers();
    wxBitmap bitmapDownload = wxBITMAP_PNG_FROM_DATA(Download);             //(wxImage(_T("icons/Download.png")));
    wxBitmap bitmapCollapseFolder = wxBITMAP_PNG_FROM_DATA(CollapseFolder); //(wxImage(_T("icons/CollapseFolder.png")));
    wxBitmap bitmapExpandFolder = wxBITMAP_PNG_FROM_DATA(ExpandFolder);
    wxBitmap bitmapRemoveFolder = wxBITMAP_PNG_FROM_DATA(RemoveFolder);
    wxBitmap bitmapAddFolder = wxBITMAP_PNG_FROM_DATA(Add);
    wxBitmap bitmapCloud = wxBITMAP_PNG_FROM_DATA(Connect);
    wxBitmap bitmapSettings = wxBITMAP_PNG_FROM_DATA(Settings);
    wxBitmap bitmapRefresh = wxBITMAP_PNG_FROM_DATA(Refresh);

    wxBitmap bitmapOnline = wxBITMAP_PNG_FROM_DATA(Online);
    wxBitmap bitmapOffline = wxBITMAP_PNG_FROM_DATA(Offline);
    // wxBitmap bitmapClose = wxBITMAP_PNG_FROM_DATA(CollapseFolder);//.Rescale(72, 72, wxIMAGE_QUALITY_BICUBIC)); // Resize(wxSize(72,72),wxPoint(0,0)));
    //  wxBitmap bitmap(wxBITMAP(wxIMAGE(("icons/reload"));
    int border = 5;
    // wxWindow::SetWindowStyle(wxBORDER_RAISED);
    Centre();
    wxFrame::FromDIP(wxSize(400, 100));
    SetMinSize(wxSize(400, 300));
    panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(400, 100));

    // wxBitmap bitmapClose2(wxImage(_T("icons/Close.png")).Rescale(24, 24, wxIMAGE_QUALITY_BICUBIC));
    imageList = new wxImageList();
    imageList->Create(24, 24);
    imageList->Add(bitmapOnline);
    imageList->Add(bitmapOffline);

    // window->Show();
    // wxDialog * dialog = new wxDialog(panel, wxID_ANY);

    // wxDialog dialog(this, wxID_ANY, "asdasd");
    // dialog.ShowModal();

    // wxFrame* frame2 = new wxFrame(this, wxID_ANY, "title", wxDefaultPosition, wxSize(800, 600));
    // wxPopupWindow *pop = new wxPopupWindow(frame2);

    // frame2->Show();

    // panel->SetBackgroundColour(wxColor(200, 200, 200));
    panel->SetMinSize(wxSize(400, 200));

    // wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    // wxSP_BORDER | wxSP_LIVE_UPDATE | wxSP_THIN_SASH);

    wxBoxSizer *verticalSizer = new wxBoxSizer(wxVERTICAL);
    // wxBoxSizer *menuSizer = new wxBoxSizer(wxHORIZONTAL);

    wxPanel *topBar = new wxPanel(panel);
    wxPanel *middlePanel = new wxPanel(panel);
    // wxPanel *bottomBar = new wxPanel(panel);
    bottomBar = new wxStatusBar(panel, wxID_ANY, wxSTB_DEFAULT_STYLE);
    /// topBar->SetBackgroundColour(wxColor(240, 240, 240)); // 100, 100, 200));
    topBar->SetMinSize(wxSize(120, 120));
    wxBitmapButton *downloadButton = new wxBitmapButton(topBar, wxID_ANY, bitmapDownload, wxDefaultPosition, wxDefaultSize, wxBU_TOP);
    downloadButton->Bind(wxEVT_BUTTON, &ClientUi::DownloadButtonClick, this);
    downloadButton->SetToolTip("Download selected files");
    wxBitmapButton *addButton = new wxBitmapButton(topBar, wxID_ANY, bitmapAddFolder, wxDefaultPosition, wxDefaultSize, wxBU_TOP);
    addButton->SetToolTip("Add shared folder");
    addButton->Bind(wxEVT_BUTTON, &ClientUi::AddButtonClick, this);
    wxBitmapButton *connectionButton = new wxBitmapButton(topBar, wxID_ANY, bitmapCloud, wxDefaultPosition, wxDefaultSize, wxBU_TOP);
    connectionButton->Bind(wxEVT_BUTTON, &ClientUi::ConnectionButtonClick, this);
    wxBitmapButton *settingsButton = new wxBitmapButton(topBar, wxID_ANY, bitmapSettings, wxDefaultPosition, wxDefaultSize, wxBU_TOP);
    settingsButton->Bind(wxEVT_BUTTON, &ClientUi::SettingsButtonClick, this);

    wxBitmapButton *refreshButton = new wxBitmapButton(topBar, wxID_ANY, bitmapRefresh, wxDefaultPosition, wxDefaultSize, wxBU_TOP);
    refreshButton->Bind(wxEVT_BUTTON, &ClientUi::RefreshButtonClick, this);

    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    buttonSizer->Add(downloadButton, 0, wxALL | wxALIGN_CENTER, 10);
    buttonSizer->Add(addButton, 0, wxALL | wxALIGN_CENTER, 10);
    buttonSizer->Add(connectionButton, 0, wxALL | wxALIGN_CENTER, 10);
    buttonSizer->Add(refreshButton, 0, wxALL | wxALIGN_CENTER, 10);
    buttonSizer->Add(settingsButton, 0, wxALL | wxALIGN_CENTER, 10);

    buttonSizer->AddStretchSpacer();
    topBar->SetSizer(buttonSizer);

    /// middlePanel->SetBackgroundColour(wxColor(100, 255, 100));

    bottomBar->SetStatusText("");
    /*bottomBar->SetBackgroundColour(wxColor(200, 100, 100));
    bottomBar->SetMinSize(wxSize(50, 50));
    bottomBar->SetWindowStyle(wxBORDER_THEME);*/

    verticalSizer->Add(topBar, 0, wxEXPAND | wxALL, border);
    verticalSizer->Add(middlePanel, 1, wxEXPAND | wxLEFT | wxRIGHT, border);
    verticalSizer->Add(bottomBar, 0, wxEXPAND | wxALL, border);

    panel->SetSizerAndFit(verticalSizer, 0);

    //
    wxBoxSizer *middleSizer = new wxBoxSizer(wxVERTICAL);
    wxSplitterWindow *middleSplitter = new wxSplitterWindow(middlePanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                            wxSP_BORDER | wxSP_LIVE_UPDATE | wxSP_THIN_SASH);

    middleSizer->Add(middleSplitter, 1, wxEXPAND, 0);

    wxPanel *middleLeftPanel = new wxPanel(middleSplitter);
    wxPanel *middleRightContent = new wxPanel(middleSplitter);

    middleLeftPanel->SetMinSize(wxSize(200, 200));

    wxPanel *middleLeftButtons = new wxPanel(middleLeftPanel);
    /// middleLeftButtons->SetBackgroundColour(wxColor(230, 230, 230));
    // wxPanel *tree = new wxPanel(middleLeftPanel);
    tree = new wxTreeCtrl(middleLeftPanel, wxID_ANY, wxDefaultPosition, wxSize(200, 200), wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxVSCROLL, wxDefaultValidator, "lol"); // wxTR_ROW_LINES
    tree->Bind(wxEVT_TREE_ITEM_ACTIVATED, &ClientUi::TreeClick, this);
    //tree->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_DEFAULT); #does not exist on windows
    tree->SetImageList(imageList);
    // std::vector<std::string> folders({"/home/user/Projects/filesharingsoftware/UITesting/", "/home/user/Projects/filesharingsoftware/UITesting/"});
    // idClientMap[0].MakeFolders(folders);

    list = new wxListCtrl(middleRightContent, -1, wxDefaultPosition, wxSize(50, 50), wxLC_REPORT | wxLC_HRULES, wxDefaultValidator, "asd");
    list->Bind(wxEVT_LIST_COL_CLICK, &ClientUi::ColumnClick, this);
    wxBoxSizer *listSizer = new wxBoxSizer(wxHORIZONTAL);
    listSizer->Add(list, 1, wxEXPAND, 0);
    long column = list->InsertColumn(0, _("Name"), 0, 100);
    list->SetColumnWidth(column, 400);
    column = list->InsertColumn(1, _("Last modified"));
    list->SetColumnWidth(column, 200);
    column = list->InsertColumn(2, _("Extension"));
    list->SetColumnWidth(column, 120);
    column = list->InsertColumn(3, _("Size"));
    list->SetColumnWidth(column, 120);

    // auto a = list->InsertItem(0, "0");
    // list->SetItem(a, 1, "0");
    // list->InsertItem(1, "1");
    middleRightContent->SetSizer(listSizer);

    wxBoxSizer *middleLeftSizer = new wxBoxSizer(wxVERTICAL);

    middleLeftButtons->SetMinSize(wxSize(40, 40));

    // tree->SetBackgroundColour(wxColor(255, 255, 100));

    middleLeftSizer->Add(middleLeftButtons, 0, wxEXPAND);
    middleLeftSizer->Add(tree, 1, wxEXPAND);
    middleLeftPanel->SetSizer(middleLeftSizer);
    // tree->SetMinSize(wxSize(100, 100));

    int leftMenuButtonSize = 40;
    wxBitmapButton *buttonCollapse = new wxBitmapButton(middleLeftButtons, wxID_ANY, bitmapCollapseFolder, wxDefaultPosition, wxSize(leftMenuButtonSize, leftMenuButtonSize), wxBU_TOP);
    buttonCollapse->Bind(wxEVT_BUTTON, &ClientUi::CollapseButtonClick, this);
    buttonCollapse->SetToolTip("Collapse folders");
    wxBitmapButton *buttonExpand = new wxBitmapButton(middleLeftButtons, wxID_ANY, bitmapExpandFolder, wxPoint(leftMenuButtonSize, 0), wxSize(leftMenuButtonSize, leftMenuButtonSize), wxBU_TOP);
    buttonExpand->Bind(wxEVT_BUTTON, &ClientUi::ExpandButtonClick, this);
    buttonExpand->SetToolTip("Expand folders");
    wxBitmapButton *buttonRemove = new wxBitmapButton(middleLeftButtons, wxID_ANY, bitmapRemoveFolder, wxPoint(leftMenuButtonSize * 3, 0), wxSize(leftMenuButtonSize, leftMenuButtonSize), wxBU_TOP);
    buttonRemove->Bind(wxEVT_BUTTON, &ClientUi::RemoveButtonClick, this);
    buttonRemove->SetToolTip("Remove shared folder");

    /// middleRightContent->SetBackgroundColour(wxColor(255, 100, 255));
    middleRightContent->SetMinSize(wxSize(200, 0));

    middleSplitter->SplitVertically(middleLeftPanel, middleRightContent);
    middleSplitter->SetSashPosition(300);
    middleSplitter->SetMinimumPaneSize(40);

    middlePanel->SetSizer(middleSizer);

    // auto clientStartF = boost::bind(&FssClient::start, &client);
    // clientThread = boost::thread(clientStartF);

    // auto upload_function = boost::bind(&ClientUi::StartApp, this);
    // boost::thread asd{upload_function};

    this->Bind(wxEVT_CLOSE_WINDOW, &ClientUi::CloseApp, this);
    Bind(evtIMPORTANT_MESSAGE, &ClientUi::HandlerImportantMessage, this);
    Bind(evtREDRAW_TREE, &ClientUi::HandlerRedraw, this);
}

Main::Main()
{
    Log::init();
}
bool Main::OnInit()
{
    ClientUi *clientUi = new ClientUi();
    clientUi->Show(true);
    clientUi->StartApp();

    // auto run_function = boost::bind(&ClientUi::StartApp, boost::placeholders::_1);
    // boost::thread m_downloadthread =boost::thread(run_function, &clientUi);

    // auto upload_function = boost::bind(&DownloadManager::Download, this);
    // m_downloadthread = boost::thread{upload_function};

    return true;
}