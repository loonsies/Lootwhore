#ifndef __ASHITA_Lootwhore_H_INCLUDED__
#define __ASHITA_Lootwhore_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita\plugins\sdk\ashita.h"
#include "common\Utilities.h"
#include "common\Output.h"
#include "common\Settings.h"
#include "common\safePacketInjector.h"
#include "Structs.h"
#include <random>

class Lootwhore : IPlugin
{
private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
    uint32_t m_PluginId;
    IDirect3DDevice8* m_Device; // Pointer to the games Direct3D device object.
    OutputHelpers* pOutput;
    SettingsHelper* pSettings;
    safePacketInjector* pPacket;
    Profile_t mProfile;
    Settings_t mSettings;
    State_t mState;
    DWORD pWardrobe;
    DWORD pZoneFlags;
    DWORD pZoneOffset;
    typedef void (Lootwhore::*CommandHandler)(vector<string>, int, CommandHelp help);
    struct CommandInformation
    {
        CommandHandler handler;
        CommandHelp help;
    };
    std::map<string, CommandInformation, cistringcmp> mCommandMap;
    std::default_random_engine mRandomEngine;
    std::uniform_int_distribution<int32_t> mRandomDistribution;

    // UI related members
    bool m_ShowUI;
    int m_SelectedProfileIndex;
    char m_NewProfileName[256];
    int m_CurrentTab;
    std::vector<std::string> m_ProfileList;

    // Modal state for new profile creation
    bool m_ShowCreateProfileModal;
    bool m_ProfileNameAlreadyExists;

    // Item preview and search functionality
    uint16_t m_SelectedItemId;
    int m_SelectedItemSlot;
    char m_SearchBuffer[256];
    bool m_ShowItemPreview;

    // Window opening tracking
    bool m_WindowOpenedManually;
    bool m_EnableAutoClose;
    uint16_t m_PreviousPoolItems[10]; // Track previous pool state for detecting new items

public:
    const char* GetName(void) const override
    {
        return "Lootwhore";
    }
    const char* GetAuthor(void) const override
    {
        return "Thorny";
    }
    const char* GetDescription(void) const override
    {
        return "Insert description here.";
    }
    const char* GetLink(void) const override
    {
        return "Insert link here.";
    }
    double GetVersion(void) const override
    {
        return 1.20f;
    }
    int32_t GetPriority(void) const override
    {
        return 0;
    }
    uint32_t GetFlags(void) const override
    {
        return (uint32_t)(Ashita::PluginFlags::Legacy | Ashita::PluginFlags::UseDirect3D);
    }

    //main.cpp
    bool Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) override;
    void Release(void) override;
    void InitializeCommands();
    void InitializeState();

    //commands.cpp
    bool HandleCommand(int32_t mode, const char* command, bool injected) override;
    void HandleCommandImport(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandProfile(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandExport(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandReset(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandDefault(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandSmartPass(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandRarePass(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandAutoStack(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandZoneReset(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandAdd(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandRemove(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandAddDrop(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandRemoveDrop(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandAddStore(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandRemoveStore(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandList(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandLot(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandPass(std::vector<string> args, int argcount, CommandHelp help);
    void HandleCommandHelp(std::vector<string> args, int argcount, CommandHelp help);
    void PrintHelpText(CommandHelp help, bool description);

    //fileio.cpp
    void LoadDefaultSettings(bool forceReload);
    void LoadSettings(const char* Name);
    void SaveSettings(const char* Name);
    void LoadDefaultProfile(bool forceReload);
    void ImportProfile(const char* Profile);
    void LoadProfile(const char* Profile);
    void SaveProfile(const char* Profile, bool AppendPath);

    //packets.cpp
    bool HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;
    bool HandleOutgoingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;
    void HandleIncomingPacket0x0A(uint8_t* modified);
    void HandleIncomingPacket0xD2(uint8_t* modified);
    void HandleIncomingPacket0xD3(uint8_t* modified);
    void HandleOutgoingPacket0x15();

    //logic.cpp
    void HandleTreasureSlot(int Slot);
    void HandleInventory();

    //helpers.cpp
    bool CheckRarePass(uint16_t ItemId);
    bool IsPositiveInteger(std::string text);
    void LotItem(int Slot);
    void PassItem(int Slot);
    void DropItem(Ashita::FFXI::item_t* item);
    void MergeItems(Ashita::FFXI::item_t* source, Ashita::FFXI::item_t* destination);
    void StoreItem(int* FreeSpace, Ashita::FFXI::item_t* item);
    void checkBags();

    //ui.cpp
    void RenderUI();
    void LoadProfileList();
    void CreateNewProfile();
    void DeleteSelectedProfile();
    void SaveCurrentProfile();
    void RenderProfileControls();
    void RenderCreateProfileModal();
    void RenderLootPoolTab();
    void RenderAutoDropTab();
    void RenderIgnoreTab();
    void RenderLotTab();
    void RenderPassTab();
    void RenderSettingsTab();
    void RenderItemPreview();
    void RenderSearchBar(const char* hint, std::function<void(const char*)> onItemSelected);

    // UI Helpers
    void HelpMarker(const char* desc);
    bool HasNewItemsInPool();
    bool AllItemsHandledOrAutomatic();
    void UpdatePoolItemTracking();
    int GetUnhandledItemCount();

    auto Direct3DInitialize(IDirect3DDevice8* device) -> bool override;
    auto Direct3DBeginScene(bool isRenderingBackBuffer) -> void override;
    auto Direct3DEndScene(bool isRenderingBackBuffer) -> void override;
    auto Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) -> void override;
    auto Direct3DSetRenderState(D3DRENDERSTATETYPE State, DWORD* Value) -> bool override;
    auto Direct3DDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) -> bool override;
    auto Direct3DDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) -> bool override;
    auto Direct3DDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) -> bool override;
    auto Direct3DDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) -> bool override;
};
#endif