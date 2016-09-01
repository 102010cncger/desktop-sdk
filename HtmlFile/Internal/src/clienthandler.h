﻿#ifndef _HTMLFILE_CLIENT_HANDLER_
#define _HTMLFILE_CLIENT_HANDLER_

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_helpers.h"

#include "include/base/cef_bind.h"
#include "include/cef_frame.h"
#include "include/wrapper/cef_closure_task.h"
#include "cefclient/browser/client_handler.h"
#include "cefclient/common/client_switches.h"
#include "include/cef_app.h"

#include "cefclient/renderer/client_renderer.h"
#include "cefclient/browser/main_message_loop.h"

#include "../../../core/DesktopEditor/common/File.h"
#include <string>
#include <vector>

#include "cefclient/browser/root_window_manager.h"

class CHtmlClientHandler : public client::ClientHandler
{
private:
    std::wstring m_sTempFile;
    std::wstring m_sCachePath;

public:
    client::RootWindowManager* m_pManager;

public:
    CHtmlClientHandler(Delegate* delegate, std::string sUrl, client::RootWindowManager* pManager = NULL) : client::ClientHandler(delegate, false, sUrl)
    {
        m_pManager = pManager;
    }

    void Init(const std::vector<std::wstring>& arSdks, const std::vector<std::wstring>& arFiles, const std::wstring& sDestinationFile)
    {
        m_sCachePath = sDestinationFile;
        std::wstring sUniquePath = NSFile::CFileBinary::CreateTempFileWithUniqueName(NSFile::CFileBinary::GetTempPath(), L"HTML");

        // под линуксом предыдущая функция создает файл!!!
        if (NSFile::CFileBinary::Exists(sUniquePath))
            NSFile::CFileBinary::Remove(sUniquePath);

        m_sTempFile = sUniquePath + L".html";
        NSFile::CFileBinary oFileBinary;
        oFileBinary.CreateFileW(m_sTempFile);

        std::wstring sFilesHTML = L"";
        for (std::vector<std::wstring>::const_iterator iter = arFiles.begin(); iter != arFiles.end(); )
        {
            sFilesHTML += L"\"";
            sFilesHTML += *iter;
            sFilesHTML += L"\"";

            iter++;
            if (iter != arFiles.end())
                sFilesHTML += L",";
        }

        std::wstring sSdkPathHTML;
        for (std::vector<std::wstring>::const_iterator i = arSdks.begin(); i != arSdks.end(); i++)
        {
            sSdkPathHTML += (L"<script src=\"" + *i + L"\" type=\"text/javascript\"></script>");
        }

        std::wstring sHtmlContent = L"\
<!DOCTYPE html>\
<html>\
<head>\
<title>OnlyOffice: HtmlFile</title>\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\
<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge,chrome=IE8\" />\
</head>\
<body>\
<div id=\"editor_sdk\" style=\"width:100%;height:100%;\">\
</div>" + sSdkPathHTML + L"\
<script>\
window.onload = function ()\
{\
    window.Native.InitSDK(\"editor_sdk\");\
    window.Native.OpenEmptyDocument();\
    window.Native.SetDestinationDocumentPath(\"" + sDestinationFile + L"\");\
    \
    window.iframes_convert = [" + sFilesHTML + L"];\
    \
    var _current_iframe_convert = 0;\
    \
    window.on_load_iframe = function ()\
    {\
        window.Native.AddHtml(\"pasteFrame\");\
        \
        ++_current_iframe_convert;\
        if (_current_iframe_convert < window.iframes_convert.length)\
        {\
            window.add_iframe_convert();\
        }\
        else\
        {\
            window.Native.SaveDocument();\
            window.Native.Exit();\
        }\
    };\
    \
    window.add_iframe_convert = function()\
    {\
        if (_current_iframe_convert >= window.iframes_convert.length)\
            return;\
        \
        var ifr = document.createElement(\"iframe\");\
        ifr.name = \"pasteFrame\";\
        ifr.id = \"pasteFrame\";\
        ifr.style.position = 'absolute';\
        ifr.style.top = '-100px';\
        ifr.style.left = '0px';\
        ifr.style.width = '10000px';\
        ifr.style.height = '100px';\
        ifr.style.overflow = 'hidden';\
        ifr.style.zIndex = -1000;\
        ifr.onload = window.on_load_iframe;\
        ifr.src = window.iframes_convert[_current_iframe_convert];\
        \
        document.body.appendChild(ifr);\
    };\
    \
    window.add_iframe_convert();\
};\
</script>\
</body>\
</html>";

                // for debug:
                // setTimeout(window.add_iframe_convert, 40000);\
                // for release:
                // window.add_iframe_convert();\

        oFileBinary.WriteStringUTF8(sHtmlContent, true);
        oFileBinary.CloseFile();

#if 0
        NSFile::CFileBinary oFileBinaryTest;
        oFileBinaryTest.CreateFileW(L"D://123.html");
        oFileBinaryTest.WriteStringUTF8(sHtmlContent, true);
        oFileBinaryTest.CloseFile();
#endif
    }

    void RemoveTemp()
    {
        if (!m_sTempFile.empty())
            NSFile::CFileBinary::Remove(m_sTempFile);
        m_sTempFile = L"";
    }

    virtual ~CHtmlClientHandler()
    {
        RemoveTemp();
    }

    std::wstring GetUrl()
    {
        if (0 == m_sTempFile.find(L"/"))
            return L"file://" + m_sTempFile;
        else
            return L"file:///" + m_sTempFile;

        return m_sTempFile;
    }

    std::wstring GetCachePath()
    {
        return m_sCachePath;
    }

    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      const CefString& target_url,
                                      const CefString& target_frame_name,
                                      const CefPopupFeatures& popupFeatures,
                                      CefWindowInfo& windowInfo,
                                      CefRefPtr<CefClient>& client,
                                      CefBrowserSettings& settings,
                                      bool* no_javascript_access)
    {
        CEF_REQUIRE_IO_THREAD();
        return true;
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                          CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message)
    {
        CEF_REQUIRE_UI_THREAD();

        // Check for messages from the client renderer.
        std::string message_name = message->GetName();
        if (message_name == "Exit")
        {
            if (m_pManager)
                m_pManager->CloseAllWindows(false);
            return true;
        }

        return false;
    }    

    virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser,
                                  const CefString& message,
                                  const CefString& source,
                                  int line) OVERRIDE
    {
        return false;
    }

    virtual bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
                               const CefKeyEvent& event,
                               CefEventHandle os_event,
                               bool* is_keyboard_shortcut) OVERRIDE
    {
        return false;
    }

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                           TerminationStatus status) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        // TODO: exit with error
    }

public:
    IMPLEMENT_REFCOUNTING(CHtmlClientHandler);
    DISALLOW_COPY_AND_ASSIGN(CHtmlClientHandler);
};


#endif // _HTMLFILE_CLIENT_HANDLER_
