/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * RDP Settings
 *
 * Copyright 2009-2011 Jay Sorg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "certificate.h"
#include "capabilities.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <ctype.h>

#include <winpr/crt.h>
#include <winpr/file.h>
#include <winpr/path.h>
#include <winpr/sysinfo.h>
#include <winpr/registry.h>

#include <freerdp/settings.h>
#include <freerdp/build-config.h>
#include <ctype.h>

#include "settings.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

static const char client_dll[] = "C:\\Windows\\System32\\mstscax.dll";

#define REG_QUERY_DWORD_VALUE(_key, _subkey, _type, _value, _size, _result)                    \
	_size = sizeof(DWORD);                                                                     \
	if (RegQueryValueEx(_key, _subkey, NULL, &_type, (BYTE*)&_value, &_size) == ERROR_SUCCESS) \
	_result = _value

#define REG_QUERY_BOOL_VALUE(_key, _subkey, _type, _value, _size, _result)                     \
	_size = sizeof(DWORD);                                                                     \
	if (RegQueryValueEx(_key, _subkey, NULL, &_type, (BYTE*)&_value, &_size) == ERROR_SUCCESS) \
	_result = _value ? TRUE : FALSE

#define SERVER_KEY "Software\\" FREERDP_VENDOR_STRING "\\" FREERDP_PRODUCT_STRING "\\Server"
#define CLIENT_KEY "Software\\" FREERDP_VENDOR_STRING "\\" FREERDP_PRODUCT_STRING "\\Client"
#define BITMAP_CACHE_KEY CLIENT_KEY "\\BitmapCacheV2"
#define GLYPH_CACHE_KEY CLIENT_KEY "\\GlyphCache"
#define POINTER_CACHE_KEY CLIENT_KEY "\\PointerCache"

static void settings_client_load_hkey_local_machine(rdpSettings* settings)
{
	HKEY hKey;
	LONG status;
	DWORD dwType;
	DWORD dwSize;
	DWORD dwValue;
	status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, CLIENT_KEY, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (status == ERROR_SUCCESS)
	{
		REG_QUERY_DWORD_VALUE(hKey, _T("DesktopWidth"), dwType, dwValue, dwSize,
		                      settings->DesktopWidth);
		REG_QUERY_DWORD_VALUE(hKey, _T("DesktopHeight"), dwType, dwValue, dwSize,
		                      settings->DesktopHeight);
		REG_QUERY_BOOL_VALUE(hKey, _T("Fullscreen"), dwType, dwValue, dwSize, settings->Fullscreen);
		REG_QUERY_DWORD_VALUE(hKey, _T("ColorDepth"), dwType, dwValue, dwSize,
		                      settings->ColorDepth);
		REG_QUERY_DWORD_VALUE(hKey, _T("KeyboardType"), dwType, dwValue, dwSize,
		                      settings->KeyboardType);
		REG_QUERY_DWORD_VALUE(hKey, _T("KeyboardSubType"), dwType, dwValue, dwSize,
		                      settings->KeyboardSubType);
		REG_QUERY_DWORD_VALUE(hKey, _T("KeyboardFunctionKeys"), dwType, dwValue, dwSize,
		                      settings->KeyboardFunctionKey);
		REG_QUERY_DWORD_VALUE(hKey, _T("KeyboardLayout"), dwType, dwValue, dwSize,
		                      settings->KeyboardLayout);
		REG_QUERY_BOOL_VALUE(hKey, _T("ExtSecurity"), dwType, dwValue, dwSize,
		                     settings->ExtSecurity);
		REG_QUERY_BOOL_VALUE(hKey, _T("NlaSecurity"), dwType, dwValue, dwSize,
		                     settings->NlaSecurity);
		REG_QUERY_BOOL_VALUE(hKey, _T("TlsSecurity"), dwType, dwValue, dwSize,
		                     settings->TlsSecurity);
		REG_QUERY_BOOL_VALUE(hKey, _T("RdpSecurity"), dwType, dwValue, dwSize,
		                     settings->RdpSecurity);
		REG_QUERY_BOOL_VALUE(hKey, _T("MstscCookieMode"), dwType, dwValue, dwSize,
		                     settings->MstscCookieMode);
		REG_QUERY_DWORD_VALUE(hKey, _T("CookieMaxLength"), dwType, dwValue, dwSize,
		                      settings->CookieMaxLength);
		REG_QUERY_BOOL_VALUE(hKey, _T("BitmapCache"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheEnabled);
		REG_QUERY_BOOL_VALUE(hKey, _T("OffscreenBitmapCache"), dwType, dwValue, dwSize,
		                     settings->OffscreenSupportLevel);
		REG_QUERY_DWORD_VALUE(hKey, _T("OffscreenBitmapCacheSize"), dwType, dwValue, dwSize,
		                      settings->OffscreenCacheSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("OffscreenBitmapCacheEntries"), dwType, dwValue, dwSize,
		                      settings->OffscreenCacheEntries);
		RegCloseKey(hKey);
	}

	status =
	    RegOpenKeyExA(HKEY_LOCAL_MACHINE, BITMAP_CACHE_KEY, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (status == ERROR_SUCCESS)
	{
		REG_QUERY_DWORD_VALUE(hKey, _T("NumCells"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2NumCells);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cell0NumEntries"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2CellInfo[0].numEntries);
		REG_QUERY_BOOL_VALUE(hKey, _T("Cell0Persistent"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheV2CellInfo[0].persistent);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cell1NumEntries"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2CellInfo[1].numEntries);
		REG_QUERY_BOOL_VALUE(hKey, _T("Cell1Persistent"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheV2CellInfo[1].persistent);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cell2NumEntries"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2CellInfo[2].numEntries);
		REG_QUERY_BOOL_VALUE(hKey, _T("Cell2Persistent"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheV2CellInfo[2].persistent);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cell3NumEntries"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2CellInfo[3].numEntries);
		REG_QUERY_BOOL_VALUE(hKey, _T("Cell3Persistent"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheV2CellInfo[3].persistent);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cell4NumEntries"), dwType, dwValue, dwSize,
		                      settings->BitmapCacheV2CellInfo[4].numEntries);
		REG_QUERY_BOOL_VALUE(hKey, _T("Cell4Persistent"), dwType, dwValue, dwSize,
		                     settings->BitmapCacheV2CellInfo[4].persistent);
		REG_QUERY_BOOL_VALUE(hKey, _T("AllowCacheWaitingList"), dwType, dwValue, dwSize,
		                     settings->AllowCacheWaitingList);
		RegCloseKey(hKey);
	}

	status =
	    RegOpenKeyExA(HKEY_LOCAL_MACHINE, GLYPH_CACHE_KEY, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (status == ERROR_SUCCESS)
	{
		REG_QUERY_DWORD_VALUE(hKey, _T("SupportLevel"), dwType, dwValue, dwSize,
		                      settings->GlyphSupportLevel);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache0NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[0].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache0MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[0].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache1NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[1].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache1MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[1].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache2NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[2].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache2MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[2].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache3NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[3].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache3MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[3].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache4NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[4].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache4MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[4].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache5NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[5].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache5MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[5].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache6NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[6].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache6MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[6].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache7NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[7].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache7MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[7].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache8NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[8].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache8MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[8].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache9NumEntries"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[9].cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("Cache9MaxCellSize"), dwType, dwValue, dwSize,
		                      settings->GlyphCache[9].cacheMaximumCellSize);
		REG_QUERY_DWORD_VALUE(hKey, _T("FragCacheNumEntries"), dwType, dwValue, dwSize,
		                      settings->FragCache->cacheEntries);
		REG_QUERY_DWORD_VALUE(hKey, _T("FragCacheMaxCellSize"), dwType, dwValue, dwSize,
		                      settings->FragCache->cacheMaximumCellSize);
		RegCloseKey(hKey);
	}

	status =
	    RegOpenKeyExA(HKEY_LOCAL_MACHINE, POINTER_CACHE_KEY, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (status == ERROR_SUCCESS)
	{
		REG_QUERY_BOOL_VALUE(hKey, _T("LargePointer"), dwType, dwValue, dwSize,
		                     settings->LargePointerFlag);
		REG_QUERY_BOOL_VALUE(hKey, _T("ColorPointer"), dwType, dwValue, dwSize,
		                     settings->ColorPointerFlag);
		REG_QUERY_DWORD_VALUE(hKey, _T("PointerCacheSize"), dwType, dwValue, dwSize,
		                      settings->PointerCacheSize);
		RegCloseKey(hKey);
	}
}

static void settings_server_load_hkey_local_machine(rdpSettings* settings)
{
	HKEY hKey;
	LONG status;
	DWORD dwType;
	DWORD dwSize;
	DWORD dwValue;
	status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, SERVER_KEY, 0, KEY_READ | KEY_WOW64_64KEY, &hKey);

	if (status != ERROR_SUCCESS)
		return;

	REG_QUERY_BOOL_VALUE(hKey, _T("ExtSecurity"), dwType, dwValue, dwSize, settings->ExtSecurity);
	REG_QUERY_BOOL_VALUE(hKey, _T("NlaSecurity"), dwType, dwValue, dwSize, settings->NlaSecurity);
	REG_QUERY_BOOL_VALUE(hKey, _T("TlsSecurity"), dwType, dwValue, dwSize, settings->TlsSecurity);
	REG_QUERY_BOOL_VALUE(hKey, _T("RdpSecurity"), dwType, dwValue, dwSize, settings->RdpSecurity);
	RegCloseKey(hKey);
}

static void settings_load_hkey_local_machine(rdpSettings* settings)
{
	if (settings->ServerMode)
		settings_server_load_hkey_local_machine(settings);
	else
		settings_client_load_hkey_local_machine(settings);
}

static BOOL settings_get_computer_name(rdpSettings* settings)
{
	CHAR computerName[256];
	DWORD nSize = sizeof(computerName);

	if (!GetComputerNameExA(ComputerNameNetBIOS, computerName, &nSize))
		return FALSE;

	if (nSize > MAX_COMPUTERNAME_LENGTH)
		computerName[MAX_COMPUTERNAME_LENGTH] = '\0';

	return freerdp_settings_set_string(settings, FreeRDP_ComputerName, computerName);
}

BOOL freerdp_settings_set_default_order_support(rdpSettings* settings)
{
	if (!settings)
		return FALSE;

	ZeroMemory(settings->OrderSupport, 32);
	settings->OrderSupport[NEG_DSTBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_PATBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_SCRBLT_INDEX] = TRUE;
	settings->OrderSupport[NEG_OPAQUE_RECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIDSTBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIPATBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTISCRBLT_INDEX] = FALSE;
	settings->OrderSupport[NEG_MULTIOPAQUERECT_INDEX] = TRUE;
	settings->OrderSupport[NEG_MULTI_DRAWNINEGRID_INDEX] = FALSE;
	settings->OrderSupport[NEG_LINETO_INDEX] = TRUE;
	settings->OrderSupport[NEG_POLYLINE_INDEX] = TRUE;
	settings->OrderSupport[NEG_MEMBLT_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEMBLT_V2_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_MEM3BLT_V2_INDEX] = settings->BitmapCacheEnabled;
	settings->OrderSupport[NEG_SAVEBITMAP_INDEX] = FALSE;
	settings->OrderSupport[NEG_GLYPH_INDEX_INDEX] =
	    settings->GlyphSupportLevel != GLYPH_SUPPORT_NONE;
	settings->OrderSupport[NEG_FAST_INDEX_INDEX] =
	    settings->GlyphSupportLevel != GLYPH_SUPPORT_NONE;
	settings->OrderSupport[NEG_FAST_GLYPH_INDEX] =
	    settings->GlyphSupportLevel != GLYPH_SUPPORT_NONE;
	settings->OrderSupport[NEG_POLYGON_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_POLYGON_CB_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_SC_INDEX] = FALSE;
	settings->OrderSupport[NEG_ELLIPSE_CB_INDEX] = FALSE;
	return TRUE;
}

rdpSettings* freerdp_settings_new(DWORD flags)
{
	char* base;
	rdpSettings* settings;
	settings = (rdpSettings*)calloc(1, sizeof(rdpSettings));

	if (!settings)
		return NULL;

	settings->HiDefRemoteApp = FALSE;
	settings->RemoteApplicationSupportMask =
	    RAIL_LEVEL_SUPPORTED | RAIL_LEVEL_DOCKED_LANGBAR_SUPPORTED |
	    RAIL_LEVEL_SHELL_INTEGRATION_SUPPORTED | RAIL_LEVEL_LANGUAGE_IME_SYNC_SUPPORTED |
	    RAIL_LEVEL_SERVER_TO_CLIENT_IME_SYNC_SUPPORTED | RAIL_LEVEL_HIDE_MINIMIZED_APPS_SUPPORTED |
	    RAIL_LEVEL_WINDOW_CLOAKING_SUPPORTED | RAIL_LEVEL_HANDSHAKE_EX_SUPPORTED;
	settings->SupportHeartbeatPdu = TRUE;
	settings->ServerMode = (flags & FREERDP_SETTINGS_SERVER_MODE) ? TRUE : FALSE;
	settings->WaitForOutputBufferFlush = TRUE;
	settings->MaxTimeInCheckLoop = 100;
	settings->DesktopWidth = 1024;
	settings->DesktopHeight = 768;
	settings->Workarea = FALSE;
	settings->Fullscreen = FALSE;
	settings->GrabKeyboard = TRUE;
	settings->Decorations = TRUE;
	settings->RdpVersion = RDP_VERSION_10_7;
	settings->ColorDepth = 16;
	settings->ExtSecurity = FALSE;
	settings->NlaSecurity = TRUE;
	settings->TlsSecurity = TRUE;
	settings->RdpSecurity = TRUE;
	settings->NegotiateSecurityLayer = TRUE;
	settings->RestrictedAdminModeRequired = FALSE;
	settings->MstscCookieMode = FALSE;
	settings->CookieMaxLength = DEFAULT_COOKIE_MAX_LENGTH;
	settings->ClientBuild = 18363; /* Windows 10, Version 1909 */
	settings->KeyboardType = 4;
	settings->KeyboardSubType = 0;
	settings->KeyboardFunctionKey = 12;
	settings->KeyboardLayout = 0;
	settings->KeyboardHook = KEYBOARD_HOOK_FULLSCREEN_ONLY;
	settings->UseRdpSecurityLayer = FALSE;
	settings->SaltedChecksum = TRUE;
	settings->ServerPort = 3389;
	settings->GatewayPort = 443;
	settings->DesktopResize = TRUE;
	settings->ToggleFullscreen = TRUE;
	settings->DesktopPosX = UINT32_MAX;
	settings->DesktopPosY = UINT32_MAX;
	settings->SoftwareGdi = TRUE;
	settings->UnmapButtons = FALSE;
	settings->PerformanceFlags = PERF_FLAG_NONE;
	settings->AllowFontSmoothing = TRUE;
	settings->AllowDesktopComposition = FALSE;
	settings->DisableWallpaper = FALSE;
	settings->DisableFullWindowDrag = TRUE;
	settings->DisableMenuAnims = TRUE;
	settings->DisableThemes = FALSE;
	settings->ConnectionType = CONNECTION_TYPE_LAN;
	settings->NetworkAutoDetect = TRUE;
	settings->EncryptionMethods = ENCRYPTION_METHOD_NONE;
	settings->EncryptionLevel = ENCRYPTION_LEVEL_NONE;
	settings->FIPSMode = FALSE;
	settings->CompressionEnabled = TRUE;
	settings->LogonNotify = TRUE;
	settings->BrushSupportLevel = BRUSH_COLOR_FULL;
	settings->CompressionLevel = PACKET_COMPR_TYPE_RDP61;
	settings->Authentication = TRUE;
	settings->AuthenticationOnly = FALSE;
	settings->CredentialsFromStdin = FALSE;
	settings->DisableCredentialsDelegation = FALSE;
	settings->AuthenticationLevel = 2;
	settings->ChannelCount = 0;
	settings->ChannelDefArraySize = 32;
	settings->ChannelDefArray =
	    (CHANNEL_DEF*)calloc(settings->ChannelDefArraySize, sizeof(CHANNEL_DEF));

	if (!settings->ChannelDefArray)
		goto out_fail;

	settings->SupportMonitorLayoutPdu = FALSE;
	settings->MonitorCount = 0;
	settings->MonitorDefArraySize = 32;
	settings->MonitorDefArray =
	    (rdpMonitor*)calloc(settings->MonitorDefArraySize, sizeof(rdpMonitor));

	if (!settings->MonitorDefArray)
		goto out_fail;

	settings->MonitorLocalShiftX = 0;
	settings->MonitorLocalShiftY = 0;
	settings->MonitorIds = (UINT32*)calloc(16, sizeof(UINT32));

	if (!settings->MonitorIds)
		goto out_fail;

	if (!settings_get_computer_name(settings))
		goto out_fail;

	settings->ReceivedCapabilities = calloc(1, 32);

	if (!settings->ReceivedCapabilities)
		goto out_fail;

	settings->ClientProductId = calloc(1, 32);

	if (!settings->ClientProductId)
		goto out_fail;

	settings->ClientHostname = calloc(1, 32);

	if (!settings->ClientHostname)
		goto out_fail;

	gethostname(settings->ClientHostname, 31);
	settings->ClientHostname[31] = 0;
	settings->ColorPointerFlag = TRUE;
	settings->LargePointerFlag = (LARGE_POINTER_FLAG_96x96 | LARGE_POINTER_FLAG_384x384);
	settings->PointerCacheSize = 20;
	settings->SoundBeepsEnabled = TRUE;
	settings->DrawGdiPlusEnabled = FALSE;
	settings->DrawAllowSkipAlpha = TRUE;
	settings->DrawAllowColorSubsampling = TRUE;
	settings->DrawAllowDynamicColorFidelity = TRUE;
	settings->FrameMarkerCommandEnabled = TRUE;
	settings->SurfaceFrameMarkerEnabled = TRUE;
	settings->AllowCacheWaitingList = TRUE;
	settings->BitmapCacheV2NumCells = 5;
	settings->BitmapCacheV2CellInfo =
	    (BITMAP_CACHE_V2_CELL_INFO*)calloc(6, sizeof(BITMAP_CACHE_V2_CELL_INFO));

	if (!settings->BitmapCacheV2CellInfo)
		goto out_fail;

	settings->BitmapCacheV2CellInfo[0].numEntries = 600;
	settings->BitmapCacheV2CellInfo[0].persistent = FALSE;
	settings->BitmapCacheV2CellInfo[1].numEntries = 600;
	settings->BitmapCacheV2CellInfo[1].persistent = FALSE;
	settings->BitmapCacheV2CellInfo[2].numEntries = 2048;
	settings->BitmapCacheV2CellInfo[2].persistent = FALSE;
	settings->BitmapCacheV2CellInfo[3].numEntries = 4096;
	settings->BitmapCacheV2CellInfo[3].persistent = FALSE;
	settings->BitmapCacheV2CellInfo[4].numEntries = 2048;
	settings->BitmapCacheV2CellInfo[4].persistent = FALSE;
	settings->NoBitmapCompressionHeader = TRUE;
	settings->RefreshRect = TRUE;
	settings->SuppressOutput = TRUE;
	settings->GlyphSupportLevel = GLYPH_SUPPORT_NONE;
	settings->GlyphCache = calloc(10, sizeof(GLYPH_CACHE_DEFINITION));

	if (!settings->GlyphCache)
		goto out_fail;

	settings->FragCache = calloc(1, sizeof(GLYPH_CACHE_DEFINITION));

	if (!settings->FragCache)
		goto out_fail;

	settings->GlyphCache[0].cacheEntries = 254;
	settings->GlyphCache[0].cacheMaximumCellSize = 4;
	settings->GlyphCache[1].cacheEntries = 254;
	settings->GlyphCache[1].cacheMaximumCellSize = 4;
	settings->GlyphCache[2].cacheEntries = 254;
	settings->GlyphCache[2].cacheMaximumCellSize = 8;
	settings->GlyphCache[3].cacheEntries = 254;
	settings->GlyphCache[3].cacheMaximumCellSize = 8;
	settings->GlyphCache[4].cacheEntries = 254;
	settings->GlyphCache[4].cacheMaximumCellSize = 16;
	settings->GlyphCache[5].cacheEntries = 254;
	settings->GlyphCache[5].cacheMaximumCellSize = 32;
	settings->GlyphCache[6].cacheEntries = 254;
	settings->GlyphCache[6].cacheMaximumCellSize = 64;
	settings->GlyphCache[7].cacheEntries = 254;
	settings->GlyphCache[7].cacheMaximumCellSize = 128;
	settings->GlyphCache[8].cacheEntries = 254;
	settings->GlyphCache[8].cacheMaximumCellSize = 256;
	settings->GlyphCache[9].cacheEntries = 64;
	settings->GlyphCache[9].cacheMaximumCellSize = 256;
	settings->FragCache->cacheEntries = 256;
	settings->FragCache->cacheMaximumCellSize = 256;
	settings->OffscreenSupportLevel = FALSE;
	settings->OffscreenCacheSize = 7680;
	settings->OffscreenCacheEntries = 2000;
	settings->DrawNineGridCacheSize = 2560;
	settings->DrawNineGridCacheEntries = 256;
	settings->ClientDir = _strdup(client_dll);

	if (!settings->ClientDir)
		goto out_fail;

	settings->RemoteWndSupportLevel = WINDOW_LEVEL_SUPPORTED | WINDOW_LEVEL_SUPPORTED_EX;
	settings->RemoteAppNumIconCaches = 3;
	settings->RemoteAppNumIconCacheEntries = 12;
	settings->VirtualChannelChunkSize = CHANNEL_CHUNK_LENGTH;
	settings->MultifragMaxRequestSize = (flags & FREERDP_SETTINGS_SERVER_MODE) ? 0 : 0xFFFF;
	settings->GatewayUseSameCredentials = FALSE;
	settings->GatewayBypassLocal = FALSE;
	settings->GatewayRpcTransport = TRUE;
	settings->GatewayHttpTransport = TRUE;
	settings->GatewayUdpTransport = TRUE;
	settings->FastPathInput = TRUE;
	settings->FastPathOutput = TRUE;
	settings->LongCredentialsSupported = TRUE;
	settings->FrameAcknowledge = 2;
	settings->MouseMotion = TRUE;
	settings->NSCodecColorLossLevel = 3;
	settings->NSCodecAllowSubsampling = TRUE;
	settings->NSCodecAllowDynamicColorFidelity = TRUE;
	settings->AutoReconnectionEnabled = FALSE;
	settings->AutoReconnectMaxRetries = 20;
	settings->GfxThinClient = TRUE;
	settings->GfxSmallCache = FALSE;
	settings->GfxProgressive = FALSE;
	settings->GfxProgressiveV2 = FALSE;
	settings->GfxH264 = FALSE;
	settings->GfxAVC444 = FALSE;
	settings->GfxSendQoeAck = FALSE;
	settings->ClientAutoReconnectCookie =
	    (ARC_CS_PRIVATE_PACKET*)calloc(1, sizeof(ARC_CS_PRIVATE_PACKET));

	if (!settings->ClientAutoReconnectCookie)
		goto out_fail;

	settings->ServerAutoReconnectCookie =
	    (ARC_SC_PRIVATE_PACKET*)calloc(1, sizeof(ARC_SC_PRIVATE_PACKET));

	if (!settings->ServerAutoReconnectCookie)
		goto out_fail;

	settings->ClientTimeZone = (LPTIME_ZONE_INFORMATION)calloc(1, sizeof(TIME_ZONE_INFORMATION));

	if (!settings->ClientTimeZone)
		goto out_fail;

	settings->DeviceArraySize = 16;
	settings->DeviceArray =
	    (RDPDR_DEVICE**)calloc(settings->DeviceArraySize, sizeof(RDPDR_DEVICE*));

	if (!settings->DeviceArray)
		goto out_fail;

	settings->StaticChannelArraySize = 16;
	settings->StaticChannelArray =
	    (ADDIN_ARGV**)calloc(settings->StaticChannelArraySize, sizeof(ADDIN_ARGV*));

	if (!settings->StaticChannelArray)
		goto out_fail;

	settings->DynamicChannelArraySize = 16;
	settings->DynamicChannelArray =
	    (ADDIN_ARGV**)calloc(settings->DynamicChannelArraySize, sizeof(ADDIN_ARGV*));

	if (!settings->DynamicChannelArray)
		goto out_fail;

	settings->TcpKeepAlive = TRUE;
	settings->TcpKeepAliveRetries = 3;
	settings->TcpKeepAliveDelay = 5;
	settings->TcpKeepAliveInterval = 2;
	settings->TcpAckTimeout = 9000;

	if (!settings->ServerMode)
	{
		settings->RedirectClipboard = TRUE;
		/* these values are used only by the client part */
		settings->HomePath = GetKnownPath(KNOWN_PATH_HOME);

		if (!settings->HomePath)
			goto out_fail;

		/* For default FreeRDP continue using same config directory
		 * as in old releases.
		 * Custom builds use <Vendor>/<Product> as config folder. */
		if (_stricmp(FREERDP_VENDOR_STRING, FREERDP_PRODUCT_STRING))
		{
			base = GetKnownSubPath(KNOWN_PATH_XDG_CONFIG_HOME, FREERDP_VENDOR_STRING);

			if (base)
			{
				settings->ConfigPath = GetCombinedPath(base, FREERDP_PRODUCT_STRING);
			}

			free(base);
		}
		else
		{
			size_t i;
			char product[sizeof(FREERDP_PRODUCT_STRING)];
			memset(product, 0, sizeof(product));

			for (i = 0; i < sizeof(product); i++)
				product[i] = tolower(FREERDP_PRODUCT_STRING[i]);

			settings->ConfigPath = GetKnownSubPath(KNOWN_PATH_XDG_CONFIG_HOME, product);
		}

		if (!settings->ConfigPath)
			goto out_fail;
	}

	settings_load_hkey_local_machine(settings);

	settings->ActionScript = _strdup("~/.config/freerdp/action.sh");
	settings->XSelectionAtom = NULL;
	settings->SmartcardLogon = FALSE;
	settings->TlsSecLevel = 1;
	settings->OrderSupport = calloc(1, 32);

	if (!settings->OrderSupport)
		goto out_fail;

	if (!freerdp_settings_set_default_order_support(settings))
		goto out_fail;

	return settings;
out_fail:
	freerdp_settings_free(settings);
	return NULL;
}

static void freerdp_settings_free_internal(rdpSettings* settings)
{
	free(settings->ChannelDefArray);
	free(settings->MonitorDefArray);
	free(settings->MonitorIds);
	free(settings->ReceivedCapabilities);
	free(settings->OrderSupport);
	free(settings->ServerRandom);
	free(settings->ClientRandom);
	free(settings->ServerCertificate);
	certificate_free(settings->RdpServerCertificate);
	free(settings->ClientAutoReconnectCookie);
	free(settings->ServerAutoReconnectCookie);
	free(settings->ClientTimeZone);
	free(settings->BitmapCacheV2CellInfo);
	free(settings->GlyphCache);
	free(settings->FragCache);
	key_free(settings->RdpServerRsaKey);
	free(settings->LoadBalanceInfo);
	free(settings->RedirectionPassword);
	free(settings->RedirectionTsvUrl);

	freerdp_target_net_addresses_free(settings);
	freerdp_device_collection_free(settings);
	freerdp_static_channel_collection_free(settings);
	freerdp_dynamic_channel_collection_free(settings);

	/* Extensions */
	free(settings->ActionScript);
	settings->ActionScript = NULL;
	free(settings->XSelectionAtom);
	settings->XSelectionAtom = NULL;

	/* Free all strings, set other pointers NULL */
	freerdp_settings_free_keys(settings, TRUE);
}

void freerdp_settings_free(rdpSettings* settings)
{
	if (!settings)
		return;

	freerdp_settings_free_internal(settings);
	free(settings);
}

static BOOL freerdp_settings_int_buffer_copy(rdpSettings* _settings, const rdpSettings* settings)
{
	BOOL rc = FALSE;
	UINT32 index;

	if (!_settings || !settings)
		return FALSE;

	if (settings->LoadBalanceInfo && settings->LoadBalanceInfoLength)
	{
		_settings->LoadBalanceInfo = (BYTE*)calloc(1, settings->LoadBalanceInfoLength + 2);

		if (!_settings->LoadBalanceInfo)
			goto out_fail;

		CopyMemory(_settings->LoadBalanceInfo, settings->LoadBalanceInfo,
		           settings->LoadBalanceInfoLength);
		_settings->LoadBalanceInfoLength = settings->LoadBalanceInfoLength;
	}

	if (_settings->ServerRandomLength)
	{
		_settings->ServerRandom = (BYTE*)malloc(_settings->ServerRandomLength);

		if (!_settings->ServerRandom)
			goto out_fail;

		CopyMemory(_settings->ServerRandom, settings->ServerRandom, settings->ServerRandomLength);
		_settings->ServerRandomLength = settings->ServerRandomLength;
	}

	if (_settings->ClientRandomLength)
	{
		_settings->ClientRandom = (BYTE*)malloc(_settings->ClientRandomLength);

		if (!_settings->ClientRandom)
			goto out_fail;

		CopyMemory(_settings->ClientRandom, settings->ClientRandom, settings->ClientRandomLength);
		_settings->ClientRandomLength = settings->ClientRandomLength;
	}

	if (settings->ServerCertificateLength)
	{
		_settings->ServerCertificate = (BYTE*)malloc(settings->ServerCertificateLength);

		if (!_settings->ServerCertificate)
			goto out_fail;

		CopyMemory(_settings->ServerCertificate, settings->ServerCertificate,
		           settings->ServerCertificateLength);
		_settings->ServerCertificateLength = settings->ServerCertificateLength;
	}

	if (settings->RdpServerCertificate)
	{
		_settings->RdpServerCertificate = certificate_clone(settings->RdpServerCertificate);

		if (!_settings->RdpServerCertificate)
			goto out_fail;
	}

	if (settings->RdpServerRsaKey)
	{
		_settings->RdpServerRsaKey = key_clone(settings->RdpServerRsaKey);

		if (!_settings->RdpServerRsaKey)
			goto out_fail;
	}

	_settings->ChannelCount = settings->ChannelCount;
	_settings->ChannelDefArraySize = settings->ChannelDefArraySize;

	if (_settings->ChannelDefArraySize > 0)
	{
		_settings->ChannelDefArray =
		    (CHANNEL_DEF*)calloc(settings->ChannelDefArraySize, sizeof(CHANNEL_DEF));

		if (!_settings->ChannelDefArray)
			goto out_fail;

		CopyMemory(_settings->ChannelDefArray, settings->ChannelDefArray,
		           sizeof(CHANNEL_DEF) * settings->ChannelDefArraySize);
	}
	else
		_settings->ChannelDefArray = NULL;

	_settings->MonitorCount = settings->MonitorCount;
	_settings->MonitorDefArraySize = settings->MonitorDefArraySize;

	if (_settings->MonitorDefArraySize > 0)
	{
		_settings->MonitorDefArray =
		    (rdpMonitor*)calloc(settings->MonitorDefArraySize, sizeof(rdpMonitor));

		if (!_settings->MonitorDefArray)
			goto out_fail;

		CopyMemory(_settings->MonitorDefArray, settings->MonitorDefArray,
		           sizeof(rdpMonitor) * settings->MonitorDefArraySize);
	}
	else
		_settings->MonitorDefArray = NULL;

	_settings->MonitorIds = (UINT32*)calloc(16, sizeof(UINT32));

	if (!_settings->MonitorIds)
		goto out_fail;

	CopyMemory(_settings->MonitorIds, settings->MonitorIds, 16 * sizeof(UINT32));
	_settings->ReceivedCapabilities = malloc(32);

	if (!_settings->ReceivedCapabilities)
		goto out_fail;

	_settings->OrderSupport = malloc(32);

	if (!_settings->OrderSupport)
		goto out_fail;

	if (!_settings->ReceivedCapabilities || !_settings->OrderSupport)
		goto out_fail;

	CopyMemory(_settings->ReceivedCapabilities, settings->ReceivedCapabilities, 32);
	CopyMemory(_settings->OrderSupport, settings->OrderSupport, 32);

	_settings->BitmapCacheV2CellInfo =
	    (BITMAP_CACHE_V2_CELL_INFO*)malloc(sizeof(BITMAP_CACHE_V2_CELL_INFO) * 6);

	if (!_settings->BitmapCacheV2CellInfo)
		goto out_fail;

	CopyMemory(_settings->BitmapCacheV2CellInfo, settings->BitmapCacheV2CellInfo,
	           sizeof(BITMAP_CACHE_V2_CELL_INFO) * 6);
	_settings->GlyphCache = malloc(sizeof(GLYPH_CACHE_DEFINITION) * 10);

	if (!_settings->GlyphCache)
		goto out_fail;

	_settings->FragCache = malloc(sizeof(GLYPH_CACHE_DEFINITION));

	if (!_settings->FragCache)
		goto out_fail;

	CopyMemory(_settings->GlyphCache, settings->GlyphCache, sizeof(GLYPH_CACHE_DEFINITION) * 10);
	CopyMemory(_settings->FragCache, settings->FragCache, sizeof(GLYPH_CACHE_DEFINITION));

	_settings->ClientAutoReconnectCookie =
	    (ARC_CS_PRIVATE_PACKET*)malloc(sizeof(ARC_CS_PRIVATE_PACKET));

	if (!_settings->ClientAutoReconnectCookie)
		goto out_fail;

	_settings->ServerAutoReconnectCookie =
	    (ARC_SC_PRIVATE_PACKET*)malloc(sizeof(ARC_SC_PRIVATE_PACKET));

	if (!_settings->ServerAutoReconnectCookie)
		goto out_fail;

	CopyMemory(_settings->ClientAutoReconnectCookie, settings->ClientAutoReconnectCookie,
	           sizeof(ARC_CS_PRIVATE_PACKET));
	CopyMemory(_settings->ServerAutoReconnectCookie, settings->ServerAutoReconnectCookie,
	           sizeof(ARC_SC_PRIVATE_PACKET));
	_settings->ClientTimeZone = (LPTIME_ZONE_INFORMATION)malloc(sizeof(TIME_ZONE_INFORMATION));

	if (!_settings->ClientTimeZone)
		goto out_fail;

	CopyMemory(_settings->ClientTimeZone, settings->ClientTimeZone, sizeof(TIME_ZONE_INFORMATION));

	_settings->RedirectionPasswordLength = settings->RedirectionPasswordLength;
	if (settings->RedirectionPasswordLength > 0)
	{
		_settings->RedirectionPassword = malloc(_settings->RedirectionPasswordLength);
		if (!_settings->RedirectionPassword)
		{
			_settings->RedirectionPasswordLength = 0;
			goto out_fail;
		}

		CopyMemory(_settings->RedirectionPassword, settings->RedirectionPassword,
		           _settings->RedirectionPasswordLength);
	}

	_settings->RedirectionTsvUrlLength = settings->RedirectionTsvUrlLength;
	if (settings->RedirectionTsvUrlLength > 0)
	{
		_settings->RedirectionTsvUrl = malloc(_settings->RedirectionTsvUrlLength);
		if (!_settings->RedirectionTsvUrl)
		{
			_settings->RedirectionTsvUrlLength = 0;
			goto out_fail;
		}

		CopyMemory(_settings->RedirectionTsvUrl, settings->RedirectionTsvUrl,
		           _settings->RedirectionTsvUrlLength);
	}

	_settings->TargetNetAddressCount = settings->TargetNetAddressCount;

	if (settings->TargetNetAddressCount > 0)
	{
		_settings->TargetNetAddresses =
		    (char**)calloc(settings->TargetNetAddressCount, sizeof(char*));

		if (!_settings->TargetNetAddresses)
		{
			_settings->TargetNetAddressCount = 0;
			goto out_fail;
		}

		for (index = 0; index < settings->TargetNetAddressCount; index++)
		{
			_settings->TargetNetAddresses[index] = _strdup(settings->TargetNetAddresses[index]);

			if (!_settings->TargetNetAddresses[index])
			{
				while (index)
					free(_settings->TargetNetAddresses[--index]);

				free(_settings->TargetNetAddresses);
				_settings->TargetNetAddresses = NULL;
				_settings->TargetNetAddressCount = 0;
				goto out_fail;
			}
		}

		if (settings->TargetNetPorts)
		{
			_settings->TargetNetPorts =
			    (UINT32*)calloc(settings->TargetNetAddressCount, sizeof(UINT32));

			if (!_settings->TargetNetPorts)
				goto out_fail;

			for (index = 0; index < settings->TargetNetAddressCount; index++)
				_settings->TargetNetPorts[index] = settings->TargetNetPorts[index];
		}
	}

	_settings->DeviceCount = settings->DeviceCount;
	_settings->DeviceArraySize = settings->DeviceArraySize;
	_settings->DeviceArray =
	    (RDPDR_DEVICE**)calloc(_settings->DeviceArraySize, sizeof(RDPDR_DEVICE*));

	if (!_settings->DeviceArray && _settings->DeviceArraySize)
	{
		_settings->DeviceCount = 0;
		_settings->DeviceArraySize = 0;
		goto out_fail;
	}

	if (_settings->DeviceArraySize < _settings->DeviceCount)
	{
		_settings->DeviceCount = 0;
		_settings->DeviceArraySize = 0;
		goto out_fail;
	}

	for (index = 0; index < _settings->DeviceCount; index++)
	{
		_settings->DeviceArray[index] = freerdp_device_clone(settings->DeviceArray[index]);

		if (!_settings->DeviceArray[index])
			goto out_fail;
	}

	_settings->StaticChannelCount = settings->StaticChannelCount;
	_settings->StaticChannelArraySize = settings->StaticChannelArraySize;
	_settings->StaticChannelArray =
	    (ADDIN_ARGV**)calloc(_settings->StaticChannelArraySize, sizeof(ADDIN_ARGV*));

	if (!_settings->StaticChannelArray && _settings->StaticChannelArraySize)
	{
		_settings->StaticChannelArraySize = 0;
		_settings->ChannelCount = 0;
		goto out_fail;
	}

	if (_settings->StaticChannelArraySize < _settings->StaticChannelCount)
	{
		_settings->StaticChannelArraySize = 0;
		_settings->ChannelCount = 0;
		goto out_fail;
	}

	for (index = 0; index < _settings->StaticChannelCount; index++)
	{
		_settings->StaticChannelArray[index] =
		    freerdp_static_channel_clone(settings->StaticChannelArray[index]);

		if (!_settings->StaticChannelArray[index])
			goto out_fail;
	}

	_settings->DynamicChannelCount = settings->DynamicChannelCount;
	_settings->DynamicChannelArraySize = settings->DynamicChannelArraySize;
	_settings->DynamicChannelArray =
	    (ADDIN_ARGV**)calloc(_settings->DynamicChannelArraySize, sizeof(ADDIN_ARGV*));

	if (!_settings->DynamicChannelArray && _settings->DynamicChannelArraySize)
	{
		_settings->DynamicChannelCount = 0;
		_settings->DynamicChannelArraySize = 0;
		goto out_fail;
	}

	if (_settings->DynamicChannelArraySize < _settings->DynamicChannelCount)
	{
		_settings->DynamicChannelCount = 0;
		_settings->DynamicChannelArraySize = 0;
		goto out_fail;
	}

	for (index = 0; index < _settings->DynamicChannelCount; index++)
	{
		_settings->DynamicChannelArray[index] =
		    freerdp_dynamic_channel_clone(settings->DynamicChannelArray[index]);

		if (!_settings->DynamicChannelArray[index])
			goto out_fail;
	}

	if (settings->ActionScript)
		_settings->ActionScript = _strdup(settings->ActionScript);
	if (settings->XSelectionAtom)
		_settings->XSelectionAtom = _strdup(settings->XSelectionAtom);
	rc = TRUE;
out_fail:
	return rc;
}

BOOL freerdp_settings_copy(rdpSettings* _settings, const rdpSettings* settings)
{
	BOOL rc;

	if (!settings || !_settings)
		return FALSE;

	/* This is required to free all non string buffers */
	freerdp_settings_free_internal(_settings);
	/* This copies everything except allocated non string buffers. reset all allocated buffers to
	 * NULL to fix issues during cleanup */
	rc = freerdp_settings_clone_keys(_settings, settings);

	_settings->LoadBalanceInfo = NULL;
	_settings->ServerRandom = NULL;
	_settings->ClientRandom = NULL;
	_settings->RdpServerCertificate = NULL;
	_settings->RdpServerRsaKey = NULL;
	_settings->ChannelDefArray = NULL;
	_settings->MonitorDefArray = NULL;
	_settings->MonitorIds = NULL;
	_settings->ReceivedCapabilities = NULL;
	_settings->OrderSupport = NULL;
	_settings->BitmapCacheV2CellInfo = NULL;
	_settings->GlyphCache = NULL;
	_settings->FragCache = NULL;
	_settings->ClientAutoReconnectCookie = NULL;
	_settings->ServerAutoReconnectCookie = NULL;
	_settings->ClientTimeZone = NULL;
	_settings->RedirectionPassword = NULL;
	_settings->RedirectionTsvUrl = NULL;
	_settings->TargetNetAddresses = NULL;
	_settings->DeviceArray = NULL;
	_settings->StaticChannelArray = NULL;
	_settings->DynamicChannelArray = NULL;
	_settings->ActionScript = NULL;
	_settings->XSelectionAtom = NULL;
	if (!rc)
		goto out_fail;

	/* Begin copying */
	if (!freerdp_settings_int_buffer_copy(_settings, settings))
		goto out_fail;
	return TRUE;
out_fail:
	freerdp_settings_free_internal(_settings);
	return FALSE;
}

rdpSettings* freerdp_settings_clone(const rdpSettings* settings)
{
	rdpSettings* _settings = (rdpSettings*)calloc(1, sizeof(rdpSettings));

	if (!freerdp_settings_copy(_settings, settings))
		goto out_fail;

	return _settings;
out_fail:
	freerdp_settings_free(_settings);
	return NULL;
}
#ifdef _WIN32
#pragma warning(pop)
#endif
