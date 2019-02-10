#ifndef WMDERLAND_WM_HPP_
#define WMDERLAND_WM_HPP_

#include "properties.hpp"
#include "workspace.hpp"
#include "config.hpp"
#include "cookie.hpp"
#include "util.hpp"
extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
}
#include <memory>
#include <vector>

#define MOUSE_LEFT_BTN 1
#define MOUSE_MID_BTN 2
#define MOUSE_RIGHT_BTN 3

#define NORMAL_CURSOR 0
#define MOVE_CURSOR 1
#define RESIZE_CURSOR 3

#define WM_STATE_WITHDRAWN 0
#define WM_STATE_NORMAL 1
#define WM_STATE_ICONIC 3

class WindowManager {
public:
    static std::unique_ptr<WindowManager> GetInstance();
    virtual ~WindowManager();
    void Run();

private:
    static WindowManager* instance_;
    WindowManager(Display* dpy);
    void InitWorkspaces(int count);
    void InitProperties();
    void InitXEvents();
    void InitCursors();
 
    // XEvent handlers
    void OnMapRequest(const XMapRequestEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XButtonEvent& e);
    void OnClientMessage(const XClientMessageEvent& e);
    static int OnXError(Display* dpy, XErrorEvent* e);

    // Workspace manipulation
    void GotoWorkspace(int next);
    void MoveWindowToWorkspace(Window window, int next); 

    // Client manipulation
    void Center(Window w);
    void ToggleFloating(Window w);
    void ToggleFullscreen(Window w);
    void KillClient(Window w);

    // Window type
    bool IsWindowOfType(Window, Atom property_atom, Atom target_atom);
    bool IsDock(Window w);
    bool IsDialog(Window w);
    bool IsSplash(Window w);
    bool IsNotification(Window w);
    bool IsFullscreen(Window w);

    // Docks, bars and notifications
    inline void MapDocks() const;
    inline void UnmapDocks() const;
    inline void RaiseNotifications() const;

    // Window position and size
    Area CalculateTilingArea();
    void ResizeWindowFromCookie(Window w, const Area& cookie_area);

    // Misc
    void UpdateClientList();


    Display* dpy_;
    Window root_window_;
    Window wmcheckwin_;
    Cursor cursors_[4];

    std::unique_ptr<Properties> prop_;
    std::unique_ptr<Config> config_;
    std::unique_ptr<Cookie> cookie_;

    // The floating windows vector contain windows that should not be tiled but
    // must be kept on the top, e.g., dock, notifications, etc.
    std::vector<Window> docks_;
    std::vector<Window> notifications_;

    // Workspaces contain clients, where a client is a window that can be tiled
    // by the window manager.
    Workspace* workspaces_[WORKSPACE_COUNT];
    short current_;

    // Window move, resize event cache.
    XButtonEvent btn_pressed_event_;
};

#endif
