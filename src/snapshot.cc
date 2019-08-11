// Copyright (c) 2018-2019 Marco Wang <m.aesophor@gmail.com>
#include "snapshot.h"

extern "C"{
#include <unistd.h>
}
#include <fstream>

#include "client.h"
#include "window_manager.h"
#include "util.h"

using std::endl;
using std::array;
using std::string;
using std::ifstream;
using std::ofstream;

namespace wmderland {

const char Snapshot::kDelimiter_ = ' ';

Snapshot::Snapshot(const string& filename)
    : filename_(sys_utils::ToAbsPath(filename)) {}


bool Snapshot::FileExists() const {
  return access(filename_.c_str(), F_OK) != -1;
}

void Snapshot::Load() const {
  WindowManager* wm = WindowManager::GetInstance();
  ifstream fin(filename_);

  // 1. Client deserailization.
  int client_count = 0;
  fin >> client_count;

  for (int i = 0; i < client_count; i++) {
    Window window = None;
    int workspace_id = 0;
    bool is_floating = false;
    bool is_fullscreen = false;
    fin >> window >> workspace_id >> is_floating >> is_fullscreen;

    Client* client = new Client(wm->dpy_, window, wm->workspaces_[workspace_id]);
    client->set_floating(is_floating);
    client->set_fullscreen(is_fullscreen);
    Client::mapper_[window] = client;
  }
  

  // 2. Client Tree deserialization will call Client::mapper_[window],
  // so we have to restore all clients before doing this. See step 1.
  for (const auto workspace : wm->workspaces_) {
    string data;
    fin >> data;
    workspace->Deserialize(data);
  }


  // 3. Docks/Notifications deserialization.
  string line;

  std::getline(fin, line);
  if (line != "none") {
    for (const auto& token : string_utils::Split(line, ',')) {
      wm->docks_.push_back(static_cast<Window>(std::stoul(token)));
    }
  }
 
  std::getline(fin, line);
  if (line != "none") {
    for (const auto& token : string_utils::Split(line, ',')) {
      wm->notifications_.push_back(static_cast<Window>(std::stoul(token)));
    }
  }


  // 4. Delete snapshot file.
  //unlink(filename_.c_str());
  rename(filename_.c_str(), (filename_ + ".old").c_str());
}

void Snapshot::Save() const {
  WindowManager* wm = WindowManager::GetInstance();
  ofstream fout(filename_);

  // 1. Client::mapper_ serialization.
  fout << Client::mapper_.size() << endl;

  for (const auto& win_client_pair : Client::mapper_) {
    Window window = win_client_pair.first;
    Client* client = win_client_pair.second;

    fout << window << Snapshot::kDelimiter_
      << client->workspace()->id() << Snapshot::kDelimiter_
      << client->is_floating() << Snapshot::kDelimiter_
      << client->is_fullscreen() << endl;
  }


  // 2. Client Tree serialization.
  for (const auto workspace : wm->workspaces_) {
    fout << workspace->Serialize() << endl;
  }


  // 3. Docks/notifications serialization.
  if (wm->docks_.empty()) {
    fout << "none";
  } else {
    for (size_t i = 0; i < wm->docks_.size(); i++) {
      fout << wm->docks_[i];
      fout << ((i < wm->docks_.size() - 1) ? "," : "");
    }
  }
  fout << endl;

  if (wm->notifications_.empty()) {
    fout << "none";
  } else {
    for (size_t i = 0; i < wm->notifications_.size(); i++) {
      fout << wm->notifications_[i];
      fout << ((i < wm->notifications_.size() - 1) ? "," : "");
    }
  }
  fout << endl;
}

} // namespace wmderland
