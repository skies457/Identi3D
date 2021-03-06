//
// File: SettingsWindow.cc
// =======================
//

#include <src/system/SettingsWindow.h>
#include <src/ext_resources/resource.h>
#include <src/utils/OptionTree.h>

namespace Identi3D
{

#if defined(_DEBUG)
	const wchar_t *external_resources = L"ext_resources_d.dll";
#else
	const wchar_t *external_resources = L"ext_resources.dll";
#endif // defined(_DEBUG)

	const wchar_t *available_resolution[] = {L"640x480x16@60",
											 L"640x480x32@60",
											 L"800x600x16@60",
											 L"800x600x32@60",
											 L"1024x768x16@60",
											 L"1024x768x32@60",
											 L"1280x1024x16@60",
											 L"1280x1024x32@60",
											 L"1280x720x16@60",
											 L"1280x720x32@60",
											 L"1366x768x16@60",
											 L"1366x768x32@60"};
	const UINT resolution_count = 12;
	const UINT default_resolution_index = 9;	// 1280x720x32@60


	const wchar_t *available_render_plugin[] = {L"Direct3D9",
												L"OpenGL"};
	const UINT render_plugin_count = 2;
	const UINT default_render_plugin_index = 0;

	SettingsWindowResult SettingsWindow::show(OptionTree &tree)
	{
		SettingsWindowResult retval;
		HMODULE hExtRes = LoadLibrary(external_resources);
		if(hExtRes == NULL) {
			MessageBoxA(NULL, E_SYSTEM_LOAD_RESOURCES_FAILURE, "Error", MB_ICONERROR | MB_OK);
			return SettingsWindowResult_Cancelled;
		}

		retval = (SettingsWindowResult)DialogBoxParam(
			hExtRes, 
			MAKEINTRESOURCE(IDD_CONFDIALOG), 
			NULL, 
			DlgProc, 
			(LPARAM)&tree);

		FreeLibrary(hExtRes);
		return retval;
	}

	INT_PTR SettingsWindow::DlgProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		static OptionTree *tree = NULL;
		static HWND plugin = NULL, adapter = NULL, 
			fscreen = NULL, windowed = NULL, resolution = NULL,
			software = NULL;
		static bool modified = false;

		int i, def;
		bool found;
		std::wstring result;

		switch(Msg)
		{
		case WM_INITDIALOG:
			tree = (OptionTree *)lParam;

			plugin = GetDlgItem(hDlg, IDC_PLUGIN);
			adapter = GetDlgItem(hDlg, IDC_ADAPTER);
			fscreen = GetDlgItem(hDlg, IDC_FULLSCREEN);
			windowed = GetDlgItem(hDlg, IDC_WINDOWED);
			resolution = GetDlgItem(hDlg, IDC_RESOLUTION);
			software = GetDlgItem(hDlg, IDC_SOFTWARE);

			result = tree->getValue(L"System.DefaultRenderDevice");
			for(i = 0, def = default_render_plugin_index, found = false; 
				i < render_plugin_count; i++) {
					SendMessage(plugin, CB_ADDSTRING, 0, (LPARAM)available_render_plugin[i]);
					if(!found && result.length() && result == available_render_plugin[i]) {
						def = i;
						found = true;
					}
			}
			SendMessage(plugin, CB_SETCURSEL, def, 0);
			
			SendMessage(adapter, CB_ADDSTRING, 0, (LPARAM)L"Default");
			SendMessage(adapter, CB_ADDSTRING, 0, (LPARAM)L"Secondary (if any)");
			result = tree->getValue(L"Renderer.UseSecondaryAdapter");
			if(result == L"true")
				SendMessage(adapter, CB_SETCURSEL, 1, 0);
			else
				SendMessage(adapter, CB_SETCURSEL, 0, 0);
			
			result = tree->getValue(L"Graphics.Windowed");
			if(result == L"false")
				SendMessage(fscreen, BM_SETCHECK, 1, 0);
			else
				SendMessage(windowed, BM_SETCHECK, 1, 0);
			
			result = tree->getValue(L"Graphics.ScreenResolution");
			for(i = 0, def = default_resolution_index, found = false; 
				i < resolution_count; i++) {
					SendMessage(resolution, CB_ADDSTRING, 0, (LPARAM)available_resolution[i]);
					if(!found && result.length() && result == available_resolution[i]) {
						def = i;
						found = true;
					}
			}
			SendMessage(resolution, CB_SETCURSEL, def, 0);

			result = tree->getValue(L"Renderer.UseRefDevice");
			if(result == L"true")
				SendMessage(software, BM_SETCHECK, 1, 0);

			return TRUE;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
				i = SendMessage(plugin, CB_GETCURSEL, 0, 0);
				if(i >= 0 && i < render_plugin_count)
					tree->addElement(L"System.DefaultRenderDevice", available_render_plugin[i]);

				i = SendMessage(adapter, CB_GETCURSEL, 0, 0);
				if(i == 1)
					tree->addElement(L"Renderer.UseSecondaryAdapter", L"true");
				else
					tree->addElement(L"Renderer.UseSecondaryAdapter", L"false");

				i = SendMessage(fscreen, BM_GETCHECK, 0, 0);
				if(i == BST_CHECKED)
					tree->addElement(L"Graphics.Windowed", L"false");
				else
					tree->addElement(L"Graphics.Windowed", L"true");

				i = SendMessage(resolution, CB_GETCURSEL, 0, 0);
				if(i >= 0 && i < resolution_count)
					tree->addElement(L"Graphics.ScreenResolution", available_resolution[i]);

				i = SendMessage(software, BM_GETCHECK, 0, 0);
				if(i == BST_CHECKED)
					tree->addElement(L"Renderer.UseRefDevice", L"true");
				else
					tree->addElement(L"Renderer.UseRefDevice", L"false");

				EndDialog(hDlg,
					modified
						? SettingsWindowResult_Modified
						: SettingsWindowResult_NoModification);
				return TRUE;
			case IDCANCEL:
				EndDialog(hDlg, SettingsWindowResult_Cancelled);
				return TRUE;
			case IDC_FULLSCREEN:
				SendMessage(windowed, BM_SETCHECK, 0, 0);
				modified = true;
				return TRUE;
			case IDC_WINDOWED:
				SendMessage(fscreen, BM_SETCHECK, 0, 0);
				modified = true;
				return TRUE;
			case IDC_PLUGIN:
				switch(HIWORD(wParam))
				{
				case CBN_SELCHANGE:
					modified = true;
					return TRUE;
				}
				break;
			case IDC_ADAPTER:
				switch(HIWORD(wParam))
				{
				case CBN_SELCHANGE:
					modified = true;
					return TRUE;
				}
				break;
			case IDC_RESOLUTION:
				switch(HIWORD(wParam))
				{
				case CBN_SELCHANGE:
					modified = true;
					return TRUE;
				}
			case IDC_SOFTWARE:
				modified = true;
				return TRUE;
			}
			break;
		}

		return FALSE;
	}

}