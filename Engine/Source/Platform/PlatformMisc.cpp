#include "PlatformMisc.h"
#pragma warning(push)
#pragma warning(disable : 4091)
#include <windows.h>
#include <Shobjidl_core.h>
#include <shlobj_core.h>
#include <Psapi.h>
#pragma warning(pop)
#pragma warning(disable : 4996)

namespace gear {
    namespace PlatformMisc {
        bool OpenFileDialog(const std::string& filter, std::string& file) {
            OPENFILENAMEA ofn;
            CHAR sz_file[260] = { 0 };
            CHAR current_dir[256] = { 0 };
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = sz_file;
            ofn.nMaxFile = sizeof(sz_file);
            if (GetCurrentDirectoryA(256, current_dir))
                ofn.lpstrInitialDir = current_dir;
            ofn.lpstrFilter = filter.c_str();
            ofn.nFilterIndex = 1;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn) == TRUE) {
                file = ofn.lpstrFile;
                return true;
            }
            return false;
        }

        bool OpenDirectoryDialog(const std::string dialog_title, const std::string& default_dir, std::string& dir) {
            std::wstring w_dialog_title, w_default_dir;
            w_dialog_title.assign(dialog_title.begin(), dialog_title.end());
            w_default_dir.assign(default_dir.begin(), default_dir.end());

            bool ret = false;
            IFileOpenDialog* pfd;
            if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
                DWORD dwFlags = 0;
                pfd->GetOptions(&dwFlags);
                pfd->SetOptions(dwFlags | FOS_PICKFOLDERS);

                if (!w_dialog_title.empty()) {
                    pfd->SetTitle(w_dialog_title.c_str());
                }

                if (!w_default_dir.empty()) {
                    IShellItem* default_path_item;
                    if (SUCCEEDED(::SHCreateItemFromParsingName(w_default_dir.c_str(), nullptr, IID_PPV_ARGS(&default_path_item)))) {
                        pfd->SetFolder(default_path_item);
                    }
                }

                if (SUCCEEDED(pfd->Show(NULL))) {
                    IShellItem* pfd_result;
                    if (SUCCEEDED(pfd->GetResult(&pfd_result))) {
                        PWSTR p_file_path = nullptr;
                        if (SUCCEEDED(pfd_result->GetDisplayName(SIGDN_FILESYSPATH, &p_file_path))) {
                            ret = true;
                            std::wstring w_dir(p_file_path);
                            dir.assign(w_dir.begin(), w_dir.end());
                            ::CoTaskMemFree(p_file_path);
                        }
                    }
                }
            }

            return ret;
        }
    }
}