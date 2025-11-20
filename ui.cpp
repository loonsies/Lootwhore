#include "Lootwhore.h"
#include <filesystem>
#include <algorithm>
#include <string>
#include <cmath>

auto Lootwhore::Direct3DInitialize(IDirect3DDevice8* device) -> bool
{
    // Store the incoming parameters for later use..
    this->m_Device = device;

    return true;
}

auto Lootwhore::Direct3DBeginScene(bool is_rendering_backbuffer) -> void
{
    UNREFERENCED_PARAMETER(is_rendering_backbuffer);
}

auto Lootwhore::Direct3DEndScene(bool is_rendering_backbuffer) -> void
{
    UNREFERENCED_PARAMETER(is_rendering_backbuffer);
}

auto Lootwhore::Direct3DPresent(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) -> void
{
    UNREFERENCED_PARAMETER(pSourceRect);
    UNREFERENCED_PARAMETER(pDestRect);
    UNREFERENCED_PARAMETER(hDestWindowOverride);
    UNREFERENCED_PARAMETER(pDirtyRegion);

    if (m_ShowUI)
    {
        RenderUI();
    }
}

auto Lootwhore::Direct3DSetRenderState(D3DRENDERSTATETYPE State, DWORD* Value) -> bool
{
    UNREFERENCED_PARAMETER(State);
    UNREFERENCED_PARAMETER(Value);

    return false;
}

auto Lootwhore::Direct3DDrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) -> bool
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(StartVertex);
    UNREFERENCED_PARAMETER(PrimitiveCount);

    return false;
}

auto Lootwhore::Direct3DDrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) -> bool
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(minIndex);
    UNREFERENCED_PARAMETER(NumVertices);
    UNREFERENCED_PARAMETER(startIndex);
    UNREFERENCED_PARAMETER(primCount);

    return false;
}

auto Lootwhore::Direct3DDrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) -> bool
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(PrimitiveCount);
    UNREFERENCED_PARAMETER(pVertexStreamZeroData);
    UNREFERENCED_PARAMETER(VertexStreamZeroStride);

    return false;
}

auto Lootwhore::Direct3DDrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) -> bool
{
    UNREFERENCED_PARAMETER(PrimitiveType);
    UNREFERENCED_PARAMETER(MinVertexIndex);
    UNREFERENCED_PARAMETER(NumVertexIndices);
    UNREFERENCED_PARAMETER(PrimitiveCount);
    UNREFERENCED_PARAMETER(pIndexData);
    UNREFERENCED_PARAMETER(IndexDataFormat);
    UNREFERENCED_PARAMETER(pVertexStreamZeroData);
    UNREFERENCED_PARAMETER(VertexStreamZeroStride);

    return false;
}

void Lootwhore::RenderUI()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Check if there are any active items in treasure pool
    bool hasActiveItems = false;
    for (int i = 0; i < 10; i++)
    {
        Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);
        if (pTreasureItem && pTreasureItem->ItemId > 0)
        {
            hasActiveItems = true;
            break;
        }
    }

    // Auto-open UI when new items are detected
    if (mSettings.EnableAutoOpen && HasNewItemsInPool() && !m_ShowUI)
    {
        m_ShowUI               = true;
        m_WindowOpenedManually = false; // Mark as automatically opened
        UpdatePoolItemTracking();       // Update tracking only when auto-opening
    }

    // Auto-close logic: close if enabled, was opened automatically, and conditions are met
    if (m_EnableAutoClose && !m_WindowOpenedManually && m_ShowUI)
    {
        bool shouldClose = false;

        // Original auto-close: when no active items
        if (!hasActiveItems)
        {
            shouldClose = true;
        }

        // Auto-close: when all items are handled or automatic
        if (mSettings.AutoCloseWhenHandled && hasActiveItems && AllItemsHandledOrAutomatic())
        {
            shouldClose = true;
        }

        if (shouldClose)
        {
            m_ShowUI = false;
            UpdatePoolItemTracking(); // Update tracking when closing
            return;
        }
    }

    if (!m_ShowUI)
        return;

    // Create window title with unhandled item count
    int unhandledCount      = GetUnhandledItemCount();
    std::string windowTitle = "Lootwhore";
    if (unhandledCount > 0)
    {
        windowTitle += " (" + std::to_string(unhandledCount) + ")";
    }
    windowTitle += "###LootwhoreWindow";

    imgui->SetNextWindowSize(ImVec2(800 * mSettings.UIScale, 600 * mSettings.UIScale), ImGuiCond_FirstUseEver);
    if (imgui->Begin(windowTitle.c_str(), &m_ShowUI))
    {
        // Apply UI scaling to this window only
        imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * mSettings.UIScale);
        RenderProfileControls();

        imgui->Separator();

        // Main tab bar
        if (imgui->BeginTabBar("LootwhoreTabBar"))
        {
            if (imgui->BeginTabItem("LootPool"))
            {
                RenderLootPoolTab();
                imgui->EndTabItem();
            }

            if (imgui->BeginTabItem("AutoLot"))
            {
                RenderLotTab();
                imgui->EndTabItem();
            }

            if (imgui->BeginTabItem("AutoPass"))
            {
                RenderPassTab();
                imgui->EndTabItem();
            }

            if (imgui->BeginTabItem("AutoDrop"))
            {
                RenderAutoDropTab();
                imgui->EndTabItem();
            }

            if (imgui->BeginTabItem("AutoIgnore"))
            {
                RenderIgnoreTab();
                imgui->EndTabItem();
            }

            if (imgui->BeginTabItem("Settings"))
            {
                RenderSettingsTab();
                imgui->EndTabItem();
            }

            imgui->EndTabBar();
        }
        imgui->PopFont();
    }
    imgui->End();

    // If the window was just closed by the user (X button), reset the manual flag
    if (!m_ShowUI)
    {
        m_WindowOpenedManually = false;
        UpdatePoolItemTracking(); // Update tracking when manually closing
    }

    // Render modals
    RenderCreateProfileModal();
    RenderConfirmationModal();
}

void Lootwhore::LoadProfileList()
{
    m_ProfileList.clear();

    std::string profilePath = m_AshitaCore->GetInstallPath();
    profilePath += "config\\lootwhore\\profiles\\";

    try
    {
        if (std::filesystem::exists(profilePath))
        {
            for (const auto& entry : std::filesystem::directory_iterator(profilePath))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".xml")
                {
                    std::string filename = entry.path().stem().string();
                    m_ProfileList.push_back(filename);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        pOutput->error_f("Error loading profile list: %s", e.what());
    }

    // Ensure we have at least a default option
    if (m_ProfileList.empty())
    {
        m_ProfileList.push_back("default");
    }

    // Auto-select and load last used profile if available
    if (!mState.CurrentProfile.empty())
    {
        auto it = std::find(m_ProfileList.begin(), m_ProfileList.end(), mState.CurrentProfile);
        if (it != m_ProfileList.end())
        {
            m_SelectedProfileIndex = static_cast<int>(std::distance(m_ProfileList.begin(), it));
            LoadProfile(mState.CurrentProfile.c_str());
        }
        else
        {
            m_SelectedProfileIndex = 0;
            mState.CurrentProfile  = m_ProfileList[0];
            LoadProfile(m_ProfileList[0].c_str());
        }
    }
    else
    {
        m_SelectedProfileIndex = 0;
        mState.CurrentProfile  = m_ProfileList[0];
        LoadProfile(m_ProfileList[0].c_str());
    }
    // Save the selected profile to settings
    std::string settingsFile = mState.MyName + ".xml";
    SaveSettings(settingsFile.c_str());
}

void Lootwhore::CreateNewProfile()
{
    if (strlen(m_NewProfileName) == 0)
    {
        pOutput->error("Profile name cannot be empty.");
        return;
    }

    std::string profileName(m_NewProfileName);

    // Check if profile already exists
    bool profileExists = false;
    for (const auto& existingProfile : m_ProfileList)
    {
        if (existingProfile == profileName)
        {
            profileExists = true;
            break;
        }
    }

    if (profileExists)
    {
        // Show confirmation modal for overwriting
        m_ShowConfirmationModal = true;
        m_ConfirmationMessage   = "A profile with this name already exists:\n\n" + profileName + "\n\nDo you want to overwrite it?";
        m_ConfirmationAction    = "overwrite";
        m_ConfirmationTarget    = profileName;
        m_ConfirmationStartTime = std::chrono::steady_clock::now();
    }
    else
    {
        // Create new profile directly
        SaveProfile(profileName.c_str(), true);
        LoadProfileList();

        // Find and select the new profile
        for (size_t i = 0; i < m_ProfileList.size(); i++)
        {
            if (m_ProfileList[i] == profileName)
            {
                m_SelectedProfileIndex = static_cast<int>(i);
                break;
            }
        }

        pOutput->message_f("Created new profile: %s", profileName.c_str());
        m_ShowCreateProfileModal   = false;
        m_ProfileNameAlreadyExists = false;
        memset(m_NewProfileName, 0, sizeof(m_NewProfileName));
    }
}

void Lootwhore::DeleteSelectedProfile()
{
    if (m_SelectedProfileIndex < 0 || m_SelectedProfileIndex >= static_cast<int>(m_ProfileList.size()))
        return;

    std::string profileName = m_ProfileList[m_SelectedProfileIndex];

    // Show confirmation modal instead of immediately deleting
    m_ShowConfirmationModal = true;
    m_ConfirmationMessage   = "Are you sure you want to delete the profile:\n\n" + profileName + "?";
    m_ConfirmationAction    = "delete";
    m_ConfirmationTarget    = profileName;
    m_ConfirmationStartTime = std::chrono::steady_clock::now();
}

void Lootwhore::SaveCurrentProfile()
{
    if (m_SelectedProfileIndex < 0 || m_SelectedProfileIndex >= static_cast<int>(m_ProfileList.size()))
        return;

    std::string profileName = m_ProfileList[m_SelectedProfileIndex];

    // Show confirmation modal before overwriting
    m_ShowConfirmationModal = true;
    m_ConfirmationMessage   = "Are you sure you want to save over the profile:\n\n" + profileName + "?";
    m_ConfirmationAction    = "save";
    m_ConfirmationTarget    = profileName;
    m_ConfirmationStartTime = std::chrono::steady_clock::now();
}

void Lootwhore::RenderProfileControls()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Profile dropdown
    imgui->SetNextItemWidth(200);

    const char* currentProfile = (m_SelectedProfileIndex >= 0 && m_SelectedProfileIndex < static_cast<int>(m_ProfileList.size()))
                                     ? m_ProfileList[m_SelectedProfileIndex].c_str()
                                     : "None";

    if (imgui->BeginCombo("##ProfileCombo", currentProfile))
    {
        for (int i = 0; i < static_cast<int>(m_ProfileList.size()); i++)
        {
            bool isSelected = (m_SelectedProfileIndex == i);
            if (imgui->Selectable(m_ProfileList[i].c_str(), isSelected))
            {
                m_SelectedProfileIndex = i;
                LoadProfile(m_ProfileList[i].c_str());
                mState.CurrentProfile = m_ProfileList[i];
                // Save the selected profile to settings
                std::string settingsFile = mState.MyName + ".xml";
                SaveSettings(settingsFile.c_str());
            }
            if (isSelected)
                imgui->SetItemDefaultFocus();
        }
        imgui->EndCombo();
    }

    imgui->SameLine();
    if (imgui->Button("Load"))
    {
        if (m_SelectedProfileIndex >= 0 && m_SelectedProfileIndex < static_cast<int>(m_ProfileList.size()))
        {
            LoadProfile(m_ProfileList[m_SelectedProfileIndex].c_str());
            mState.CurrentProfile = m_ProfileList[m_SelectedProfileIndex];
            // Save the selected profile to settings
            std::string settingsFile = mState.MyName + ".xml";
            SaveSettings(settingsFile.c_str());
            pOutput->message_f("Loaded profile: %s", m_ProfileList[m_SelectedProfileIndex].c_str());
        }
    }

    // Profile management buttons
    imgui->SameLine();
    if (imgui->Button("New"))
    {
        m_ShowCreateProfileModal = true;
        memset(m_NewProfileName, 0, sizeof(m_NewProfileName));
        m_ProfileNameAlreadyExists = false;
    }

    imgui->SameLine();
    if (imgui->Button("Delete"))
    {
        DeleteSelectedProfile();
    }

    imgui->SameLine();
    if (imgui->Button("Save"))
    {
        SaveCurrentProfile();
    }
}

void Lootwhore::RenderCreateProfileModal()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    if (!m_ShowCreateProfileModal)
        return;

    imgui->SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
    imgui->OpenPopup("Create New Profile");

    if (imgui->BeginPopupModal("Create New Profile", nullptr, ImGuiWindowFlags_NoResize))
    {
        // Apply UI scaling to this modal window only
        imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * mSettings.UIScale);

        imgui->Text("Enter a name for creating a new profile");
        imgui->Separator();

        if (m_ProfileNameAlreadyExists)
        {
            imgui->PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            imgui->Text("A profile with the entered name already exists");
            imgui->PopStyleColor();
        }

        imgui->SetNextItemWidth(-1);
        if (imgui->InputText("##ModalInput", m_NewProfileName, sizeof(m_NewProfileName)))
        {
            if (strlen(m_NewProfileName) == 0)
            {
                m_ProfileNameAlreadyExists = false;
            }
        }

        if (imgui->Button("OK", ImVec2(120, 0)))
        {
            if (strlen(m_NewProfileName) > 0)
            {
                CreateNewProfile();
                // If profile didn't exist, modals get closed automatically
                // If profile exists, CreateNewProfile will show confirmation modal
                if (!m_ShowConfirmationModal)
                {
                    m_ShowCreateProfileModal   = false;
                    m_ProfileNameAlreadyExists = false;
                    imgui->CloseCurrentPopup();
                }
            }
        }
        imgui->SameLine();
        if (imgui->Button("Cancel", ImVec2(120, 0)))
        {
            m_ShowCreateProfileModal   = false;
            m_ProfileNameAlreadyExists = false;
            memset(m_NewProfileName, 0, sizeof(m_NewProfileName));
            imgui->CloseCurrentPopup();
        }

        imgui->PopFont();
        imgui->EndPopup();
    }
}

void Lootwhore::RenderConfirmationModal()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    if (!m_ShowConfirmationModal)
        return;

    imgui->SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
    imgui->OpenPopup("Confirm Action");

    if (imgui->BeginPopupModal("Confirm Action", nullptr, ImGuiWindowFlags_NoResize))
    {
        // Apply UI scaling to this modal window only
        imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * mSettings.UIScale);

        // Display the confirmation message
        imgui->Text("%s", m_ConfirmationMessage.c_str());
        imgui->Separator();

        // Calculate remaining time
        auto now             = std::chrono::steady_clock::now();
        auto elapsed         = std::chrono::duration_cast<std::chrono::seconds>(now - m_ConfirmationStartTime).count();
        int remainingSeconds = CONFIRMATION_TIMER_SECONDS - static_cast<int>(elapsed);

        if (remainingSeconds < 0)
            remainingSeconds = 0;

        imgui->Spacing();

        // OK button - only clickable after timer expires
        bool canConfirm = (remainingSeconds <= 0);

        // Create button label with timer or "OK"
        char buttonLabel[32];
        if (canConfirm)
        {
            snprintf(buttonLabel, sizeof(buttonLabel), "OK");
        }
        else
        {
            snprintf(buttonLabel, sizeof(buttonLabel), "OK (%d)", remainingSeconds);
        }

        // Change button appearance based on whether timer has expired
        if (!canConfirm)
        {
            imgui->PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        }

        bool buttonPressed = imgui->Button(buttonLabel, ImVec2(120, 0));

        if (!canConfirm)
        {
            imgui->PopStyleVar();
            buttonPressed = false; // Prevent action even if clicked
        }

        if (buttonPressed && canConfirm)
        {
            // Perform the action
            if (m_ConfirmationAction == "delete")
            {
                std::string profilePath = m_AshitaCore->GetInstallPath();
                profilePath += "config\\lootwhore\\profiles\\" + m_ConfirmationTarget + ".xml";

                try
                {
                    if (std::filesystem::exists(profilePath))
                    {
                        std::filesystem::remove(profilePath);
                        pOutput->message_f("Deleted profile: %s", m_ConfirmationTarget.c_str());
                        LoadProfileList();
                        m_SelectedProfileIndex = 0;
                    }
                }
                catch (const std::exception& e)
                {
                    pOutput->error_f("Error deleting profile: %s", e.what());
                }
            }
            else if (m_ConfirmationAction == "overwrite")
            {
                // Perform the overwrite
                SaveProfile(m_ConfirmationTarget.c_str(), true);
                LoadProfileList();

                // Find and select the profile
                for (size_t i = 0; i < m_ProfileList.size(); i++)
                {
                    if (m_ProfileList[i] == m_ConfirmationTarget)
                    {
                        m_SelectedProfileIndex = static_cast<int>(i);
                        break;
                    }
                }

                pOutput->message_f("Overwritten profile: %s", m_ConfirmationTarget.c_str());

                // Close the create profile modal as well
                m_ShowCreateProfileModal   = false;
                m_ProfileNameAlreadyExists = false;
                memset(m_NewProfileName, 0, sizeof(m_NewProfileName));
            }
            else if (m_ConfirmationAction == "save")
            {
                // Save the current profile
                SaveProfile(m_ConfirmationTarget.c_str(), true);
                pOutput->message_f("Saved current settings to profile: %s", m_ConfirmationTarget.c_str());
            }

            // Close the confirmation modal
            m_ShowConfirmationModal = false;
            m_ConfirmationMessage   = "";
            m_ConfirmationAction    = "";
            m_ConfirmationTarget    = "";
            imgui->CloseCurrentPopup();
        }

        imgui->SameLine();
        if (imgui->Button("Cancel", ImVec2(120, 0)))
        {
            m_ShowConfirmationModal = false;
            m_ConfirmationMessage   = "";
            m_ConfirmationAction    = "";
            m_ConfirmationTarget    = "";
            imgui->CloseCurrentPopup();
        }

        imgui->PopFont();
        imgui->EndPopup();
    }
}

void Lootwhore::RenderLootPoolTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    imgui->PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 0.0f));
    imgui->PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));
    imgui->PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, 2.0f));

    if (imgui->BeginTable("LootPoolTable", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit))
    {
        imgui->TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch, 200.0f);
        imgui->TableSetupColumn("Winner", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        imgui->TableSetupColumn("Timer", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        imgui->TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        imgui->TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        imgui->TableHeadersRow();

        float rowHeight = 20.0f * mSettings.UIScale;

        for (int i = 0; i < 10; i++)
        {
            imgui->TableNextRow(ImGuiTableRowFlags_None, rowHeight);

            Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);

            imgui->TableSetColumnIndex(0);
            if (pTreasureItem && pTreasureItem->ItemId > 0)
            {
                IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);
                if (pItem)
                {
                    std::string selectableId = std::string(pItem->Name[0]) + "##" + std::to_string(i);
                    if (imgui->Selectable(selectableId.c_str(), m_SelectedItemId == pTreasureItem->ItemId && m_SelectedItemSlot == i, ImGuiSelectableFlags_None, ImVec2(0, rowHeight)))
                    {
                        if (m_SelectedItemId == pTreasureItem->ItemId && m_SelectedItemSlot == i)
                        {
                            m_ShowItemPreview  = false;
                            m_SelectedItemId   = 0;
                            m_SelectedItemSlot = -1;
                        }
                        else
                        {
                            m_SelectedItemId   = pTreasureItem->ItemId;
                            m_SelectedItemSlot = i;
                            m_ShowItemPreview  = true;
                        }
                    }
                }
                else
                {
                    std::string idLabel = "Item ID: " + std::to_string(pTreasureItem->ItemId) + "##" + std::to_string(i);
                    if (imgui->Selectable(idLabel.c_str(), m_SelectedItemId == pTreasureItem->ItemId && m_SelectedItemSlot == i, ImGuiSelectableFlags_None, ImVec2(0, rowHeight)))
                    {
                        if (m_SelectedItemId == pTreasureItem->ItemId && m_SelectedItemSlot == i)
                        {
                            m_ShowItemPreview  = false;
                            m_SelectedItemId   = 0;
                            m_SelectedItemSlot = -1;
                        }
                        else
                        {
                            m_SelectedItemId   = pTreasureItem->ItemId;
                            m_SelectedItemSlot = i;
                            m_ShowItemPreview  = true;
                        }
                    }
                }
            }
            else
            {
                std::string emptyId = "Empty##" + std::to_string(i);
                imgui->Selectable(emptyId.c_str(), false, ImGuiSelectableFlags_Disabled, ImVec2(0, rowHeight));
            }

            imgui->TableSetColumnIndex(1);
            if (pTreasureItem && pTreasureItem->ItemId > 0)
            {
                if (pTreasureItem->WinningLot > 0 && strlen((const char*)pTreasureItem->WinningEntityName) > 0)
                {
                    const char* winnerName = (const char*)pTreasureItem->WinningEntityName;
                    bool isSelf            = false;
                    if (!mState.MyName.empty() && winnerName)
                    {
                        std::string winnerStr(winnerName);
                        std::string selfStr = mState.MyName;
                        std::transform(winnerStr.begin(), winnerStr.end(), winnerStr.begin(), ::tolower);
                        std::transform(selfStr.begin(), selfStr.end(), selfStr.begin(), ::tolower);
                        if (winnerStr == selfStr)
                            isSelf = true;
                    }
                    int lotNum = pTreasureItem->WinningLot;
                    char lotStr[8];
                    snprintf(lotStr, sizeof(lotStr), " (%03d)", lotNum);
                    constexpr int maxCellChars = 24;
                    int maxNameLen             = maxCellChars - (int)strlen(lotStr);
                    std::string winnerDisplay  = winnerName;
                    if ((int)winnerDisplay.length() > maxNameLen && maxNameLen > 3)
                    {
                        winnerDisplay = winnerDisplay.substr(0, maxNameLen - 3) + "...";
                    }
                    winnerDisplay += lotStr;
                    if (isSelf)
                    {
                        imgui->TextColored(ImVec4(0.7f, 0.5f, 1.0f, 1.0f), "%s", winnerDisplay.c_str());
                    }
                    else
                    {
                        imgui->Text("%s", winnerDisplay.c_str());
                    }
                }
                else
                {
                    imgui->Text("-");
                }
            }
            else
            {
                imgui->Text("-");
            }

            imgui->TableSetColumnIndex(2);
            if (pTreasureItem && pTreasureItem->ItemId > 0)
            {
                auto now      = std::chrono::steady_clock::now();
                auto elapsed  = std::chrono::duration_cast<std::chrono::seconds>(now - mState.PoolSlots[i].EntryTime).count();
                int remaining = 300 - (int)elapsed;

                if (remaining > 0)
                {
                    int minutes = remaining / 60;
                    int seconds = remaining % 60;
                    if (remaining <= 30)
                    {
                        imgui->TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%d:%02d", minutes, seconds);
                    }
                    else if (remaining <= 60)
                    {
                        imgui->TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%d:%02d", minutes, seconds);
                    }
                    else
                    {
                        imgui->TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d:%02d", minutes, seconds);
                    }
                }
                else
                {
                    imgui->TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Expired");
                }
            }
            else
            {
                imgui->Text("-");
            }

            imgui->TableSetColumnIndex(3);
            if (pTreasureItem && pTreasureItem->ItemId > 0)
            {
                if (pTreasureItem->Lot == 0)
                {
                    imgui->TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Pending");
                }
                else if (pTreasureItem->Lot >= 65535)
                {
                    imgui->TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Passed");
                }
                else
                {
                    if (pTreasureItem->WinningLot > 0 && pTreasureItem->Lot == pTreasureItem->WinningLot)
                    {
                        imgui->TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "%d", pTreasureItem->Lot);
                    }
                    else
                    {
                        imgui->TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d", pTreasureItem->Lot);
                    }
                }
            }
            else
            {
                imgui->Text("-");
            }

            imgui->TableSetColumnIndex(4);
            if (pTreasureItem && pTreasureItem->ItemId > 0)
            {
                // Lot button (show if allowed)
                bool canLot = false;
                if (mState.PoolSlots[i].Id != 0 &&
                    mState.PoolSlots[i].Status == LotState::Untouched &&
                    mState.PoolSlots[i].PacketAttempts < mSettings.MaxRetry &&
                    std::chrono::steady_clock::now() >= mState.PoolSlots[i].Lockout)
                {
                    canLot = true;
                }
                if (canLot)
                {
                    imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * 0.75f);
                    if (imgui->Button(("Lot##" + std::to_string(i)).c_str(), ImVec2(35.0f * mSettings.UIScale, rowHeight - 2.0f)))
                    {
                        LotItem(i);
                    }
                    imgui->PopFont();
                    imgui->SameLine();
                }

                // Pass button (show if allowed) - can pass even after lotting
                bool canPass = false;
                if (mState.PoolSlots[i].Id != 0 &&
                    (mState.PoolSlots[i].Status == LotState::Untouched || mState.PoolSlots[i].Status == LotState::Lotted) &&
                    mState.PoolSlots[i].PacketAttempts < mSettings.MaxRetry &&
                    std::chrono::steady_clock::now() >= mState.PoolSlots[i].Lockout)
                {
                    canPass = true;
                }
                if (canPass)
                {
                    imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * 0.75f);
                    if (imgui->Button(("Pass##" + std::to_string(i)).c_str(), ImVec2(40.0f * mSettings.UIScale, rowHeight - 2.0f)))
                    {
                        PassItem(i);
                    }
                    imgui->PopFont();
                    imgui->SameLine();
                }

                // Add to dropdown menu (left-click)
                imgui->PushFont(imgui->GetFont(), imgui->GetFontSize() * 0.75f);
                if (imgui->Button(("Auto##" + std::to_string(i)).c_str(), ImVec2(40.0f * mSettings.UIScale, rowHeight - 2.0f)))
                {
                    imgui->OpenPopup(("AddToPopup##" + std::to_string(i)).c_str());
                }
                imgui->PopFont();

                // Right-click context menu for quick add to pass/drop
                if (imgui->IsItemClicked(ImGuiMouseButton_Right))
                {
                    imgui->OpenPopup(("QuickAddPopup##" + std::to_string(i)).c_str());
                }

                if (imgui->BeginPopup(("AddToPopup##" + std::to_string(i)).c_str()))
                {
                    // Check current state of item in lists
                    bool isInLotList    = (mProfile.ItemMap.find(pTreasureItem->ItemId) != mProfile.ItemMap.end() &&
                                        mProfile.ItemMap[pTreasureItem->ItemId] == LotReaction::Lot);
                    bool isInPassList   = (mProfile.ItemMap.find(pTreasureItem->ItemId) != mProfile.ItemMap.end() &&
                                         mProfile.ItemMap[pTreasureItem->ItemId] == LotReaction::Pass);
                    bool isInIgnoreList = (mProfile.ItemMap.find(pTreasureItem->ItemId) != mProfile.ItemMap.end() &&
                                           mProfile.ItemMap[pTreasureItem->ItemId] == LotReaction::Ignore);
                    bool isInDropList   = (std::find(mProfile.AutoDrop.begin(), mProfile.AutoDrop.end(), pTreasureItem->ItemId) != mProfile.AutoDrop.end());

                    // Lot List menu item
                    if (isInLotList)
                        imgui->PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green for active

                    if (imgui->MenuItem(isInLotList ? "Remove from Lot List" : "Add to Lot List"))
                    {
                        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);
                        if (isInLotList)
                        {
                            mProfile.ItemMap.erase(pTreasureItem->ItemId);
                            if (pItem)
                                pOutput->message_f("Removed %s from lot list.", pItem->Name[0]);
                        }
                        else
                        {
                            mProfile.ItemMap[pTreasureItem->ItemId] = LotReaction::Lot;
                            if (pItem)
                                pOutput->message_f("Added %s to lot list.", pItem->Name[0]);
                        }
                    }

                    if (isInLotList)
                        imgui->PopStyleColor();

                    // Pass List menu item
                    if (isInPassList)
                        imgui->PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red for active

                    if (imgui->MenuItem(isInPassList ? "Remove from Pass List" : "Add to Pass List"))
                    {
                        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);
                        if (isInPassList)
                        {
                            mProfile.ItemMap.erase(pTreasureItem->ItemId);
                            if (pItem)
                                pOutput->message_f("Removed %s from pass list.", pItem->Name[0]);
                        }
                        else
                        {
                            mProfile.ItemMap[pTreasureItem->ItemId] = LotReaction::Pass;
                            if (pItem)
                                pOutput->message_f("Added %s to pass list.", pItem->Name[0]);
                        }
                    }

                    if (isInPassList)
                        imgui->PopStyleColor();

                    // Ignore List menu item
                    if (isInIgnoreList)
                        imgui->PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray for active

                    if (imgui->MenuItem(isInIgnoreList ? "Remove from Ignore List" : "Add to Ignore List"))
                    {
                        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);
                        if (isInIgnoreList)
                        {
                            mProfile.ItemMap.erase(pTreasureItem->ItemId);
                            if (pItem)
                                pOutput->message_f("Removed %s from ignore list.", pItem->Name[0]);
                        }
                        else
                        {
                            mProfile.ItemMap[pTreasureItem->ItemId] = LotReaction::Ignore;
                            if (pItem)
                                pOutput->message_f("Added %s to ignore list.", pItem->Name[0]);
                        }
                    }

                    if (isInIgnoreList)
                        imgui->PopStyleColor();

                    // Drop List menu item
                    if (isInDropList)
                        imgui->PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange for active

                    if (imgui->MenuItem(isInDropList ? "Remove from Drop List" : "Add to Drop List"))
                    {
                        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);
                        if (isInDropList)
                        {
                            mProfile.AutoDrop.remove(pTreasureItem->ItemId);
                            if (pItem)
                                pOutput->message_f("Removed %s from auto drop list.", pItem->Name[0]);
                        }
                        else
                        {
                            mProfile.AutoDrop.push_back(pTreasureItem->ItemId);
                            if (pItem)
                                pOutput->message_f("Added %s to auto drop list.", pItem->Name[0]);
                        }
                    }

                    if (isInDropList)
                        imgui->PopStyleColor();

                    imgui->EndPopup();
                }

                // Quick add popup (right-click menu)
                if (imgui->BeginPopup(("QuickAddPopup##" + std::to_string(i)).c_str()))
                {
                    IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pTreasureItem->ItemId);

                    if (imgui->MenuItem("Add to Pass List"))
                    {
                        mProfile.ItemMap[pTreasureItem->ItemId] = LotReaction::Pass;
                        if (pItem)
                            pOutput->message_f("Added %s to pass list.", pItem->Name[0]);
                    }

                    if (imgui->MenuItem("Add to Drop List"))
                    {
                        mProfile.AutoDrop.push_back(pTreasureItem->ItemId);
                        if (pItem)
                            pOutput->message_f("Added %s to auto drop list.", pItem->Name[0]);
                    }

                    imgui->EndPopup();
                }
            }
            else
            {
                // Empty row - no buttons needed since we have fixed row height
            }
        }

        imgui->EndTable();
    }

    imgui->PopStyleVar(3);

    // Lot All and Pass All buttons
    imgui->PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    if (imgui->Button("Lot All"))
    {
        int lotCount = 0;
        for (int i = 0; i < 10; i++)
        {
            // Use same logic as LotItem: check mState.PoolSlots for eligibility
            if (mState.PoolSlots[i].Id == 0)
                continue;
            if (mState.PoolSlots[i].Status != LotState::Untouched)
                continue;
            if (mState.PoolSlots[i].PacketAttempts >= mSettings.MaxRetry)
                continue;
            if (std::chrono::steady_clock::now() < mState.PoolSlots[i].Lockout)
                continue;
            LotItem(i);
            lotCount++;
        }
        if (lotCount == 0)
            pOutput->message("There were no valid items to lot.");
        else if (lotCount == 1)
            pOutput->message("Lotted 1 item.");
        else
            pOutput->message_f("Lotted %d items.", lotCount);
    }

    imgui->SameLine();
    if (imgui->Button("Pass All"))
    {
        int passCount = 0;
        for (int i = 0; i < 10; i++)
        {
            // Use same logic as PassItem: check mState.PoolSlots for eligibility
            if (mState.PoolSlots[i].Id == 0)
                continue;
            if (mState.PoolSlots[i].Status != LotState::Untouched)
                continue;
            if (mState.PoolSlots[i].PacketAttempts >= mSettings.MaxRetry)
                continue;
            if (std::chrono::steady_clock::now() < mState.PoolSlots[i].Lockout)
                continue;
            PassItem(i);
            passCount++;
        }
        if (passCount == 0)
            pOutput->message("There were no valid items to pass.");
        else if (passCount == 1)
            pOutput->message("Passed 1 item.");
        else
            pOutput->message_f("Passed %d items.", passCount);
    }
    imgui->PopStyleVar();

    // Item preview below the table
    RenderItemPreview();

    imgui->Separator();
}

void Lootwhore::RenderAutoDropTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Search bar for adding items to auto drop list
    RenderSearchBar("Enter item name or ID to add to auto drop list", [this](const char* itemName) {
        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemByName(itemName, 0);
        if (pItem)
        {
            mProfile.AutoDrop.push_back(pItem->Id);
            pOutput->message_f("Added %s to auto drop list.", pItem->Name[0]);
        }
    });

    imgui->Separator();

    if (imgui->BeginTable("AutoDropTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        imgui->TableSetupColumn("Item");
        imgui->TableSetupColumn("Action");
        imgui->TableHeadersRow();

        std::vector<uint16_t> itemsToRemove;

        for (uint16_t itemId : mProfile.AutoDrop)
        {
            imgui->TableNextRow();

            // Item name
            imgui->TableSetColumnIndex(0);
            IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(itemId);
            if (pItem)
            {
                imgui->Text("%s", pItem->Name[0]);
            }
            else
            {
                imgui->Text("Item ID: %d", itemId);
            }

            // Remove button
            imgui->TableSetColumnIndex(1);
            if (imgui->Button(("Remove##" + std::to_string(itemId)).c_str()))
            {
                itemsToRemove.push_back(itemId);
            }
        }

        // Remove items that were marked for removal
        for (uint16_t itemId : itemsToRemove)
        {
            mProfile.AutoDrop.remove(itemId);
        }

        imgui->EndTable();
    }

    if (mProfile.AutoDrop.empty())
    {
        imgui->Text("No items in auto drop list");
    }
}

void Lootwhore::RenderIgnoreTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Search bar for adding items to ignore list
    RenderSearchBar("Enter item name or ID to add to auto ignore list", [this](const char* itemName) {
        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemByName(itemName, 0);
        if (pItem)
        {
            mProfile.ItemMap[pItem->Id] = LotReaction::Ignore;
            pOutput->message_f("Added %s to ignore list.", pItem->Name[0]);
        }
    });

    imgui->Separator();

    if (imgui->BeginTable("IgnoreTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        imgui->TableSetupColumn("Item");
        imgui->TableSetupColumn("Action");
        imgui->TableHeadersRow();

        std::vector<uint16_t> itemsToRemove;

        for (auto& pair : mProfile.ItemMap)
        {
            if (pair.second == LotReaction::Ignore)
            {
                imgui->TableNextRow();

                // Item name
                imgui->TableSetColumnIndex(0);
                IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pair.first);
                if (pItem)
                {
                    imgui->Text("%s", pItem->Name[0]);
                }
                else
                {
                    imgui->Text("Item ID: %d", pair.first);
                }

                // Remove button
                imgui->TableSetColumnIndex(1);
                if (imgui->Button(("Remove##ignore" + std::to_string(pair.first)).c_str()))
                {
                    itemsToRemove.push_back(pair.first);
                }
            }
        }

        // Remove items that were marked for removal
        for (uint16_t itemId : itemsToRemove)
        {
            mProfile.ItemMap.erase(itemId);
        }

        imgui->EndTable();
    }

    // Check if there are any ignore items
    bool hasIgnoreItems = false;
    for (const auto& pair : mProfile.ItemMap)
    {
        if (pair.second == LotReaction::Ignore)
        {
            hasIgnoreItems = true;
            break;
        }
    }

    if (!hasIgnoreItems)
    {
        imgui->Text("No items set to ignore");
    }
}

void Lootwhore::RenderLotTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Search bar for adding items to lot list
    RenderSearchBar("Enter item name or ID to add to auto lot list", [this](const char* itemName) {
        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemByName(itemName, 0);
        if (pItem)
        {
            mProfile.ItemMap[pItem->Id] = LotReaction::Lot;
            pOutput->message_f("Added %s to lot list.", pItem->Name[0]);
        }
    });

    imgui->Separator();

    if (imgui->BeginTable("LotTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        imgui->TableSetupColumn("Item");
        imgui->TableSetupColumn("Action");
        imgui->TableHeadersRow();

        std::vector<uint16_t> itemsToRemove;

        for (auto& pair : mProfile.ItemMap)
        {
            if (pair.second == LotReaction::Lot)
            {
                imgui->TableNextRow();

                // Item name
                imgui->TableSetColumnIndex(0);
                IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pair.first);
                if (pItem)
                {
                    imgui->Text("%s", pItem->Name[0]);
                }
                else
                {
                    imgui->Text("Item ID: %d", pair.first);
                }

                // Remove button
                imgui->TableSetColumnIndex(1);
                if (imgui->Button(("Remove##lot" + std::to_string(pair.first)).c_str()))
                {
                    itemsToRemove.push_back(pair.first);
                }
            }
        }

        // Remove items that were marked for removal
        for (uint16_t itemId : itemsToRemove)
        {
            mProfile.ItemMap.erase(itemId);
        }

        imgui->EndTable();
    }

    // Check if there are any lot items
    bool hasLotItems = false;
    for (const auto& pair : mProfile.ItemMap)
    {
        if (pair.second == LotReaction::Lot)
        {
            hasLotItems = true;
            break;
        }
    }

    if (!hasLotItems)
    {
        imgui->Text("No items set to lot");
    }
}

void Lootwhore::RenderPassTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    // Search bar for adding items to pass list
    RenderSearchBar("Enter item name or ID to add to auto pass list", [this](const char* itemName) {
        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemByName(itemName, 0);
        if (pItem)
        {
            mProfile.ItemMap[pItem->Id] = LotReaction::Pass;
            pOutput->message_f("Added %s to pass list.", pItem->Name[0]);
        }
    });

    imgui->Separator();

    if (imgui->BeginTable("PassTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        imgui->TableSetupColumn("Item");
        imgui->TableSetupColumn("Action");
        imgui->TableHeadersRow();

        std::vector<uint16_t> itemsToRemove;

        for (auto& pair : mProfile.ItemMap)
        {
            if (pair.second == LotReaction::Pass)
            {
                imgui->TableNextRow();

                // Item name
                imgui->TableSetColumnIndex(0);
                IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(pair.first);
                if (pItem)
                {
                    imgui->Text("%s", pItem->Name[0]);
                }
                else
                {
                    imgui->Text("Item ID: %d", pair.first);
                }

                // Remove button
                imgui->TableSetColumnIndex(1);
                if (imgui->Button(("Remove##pass" + std::to_string(pair.first)).c_str()))
                {
                    itemsToRemove.push_back(pair.first);
                }
            }
        }

        // Remove items that were marked for removal
        for (uint16_t itemId : itemsToRemove)
        {
            mProfile.ItemMap.erase(itemId);
        }

        imgui->EndTable();
    }

    // Check if there are any pass items
    bool hasPassItems = false;
    for (const auto& pair : mProfile.ItemMap)
    {
        if (pair.second == LotReaction::Pass)
        {
            hasPassItems = true;
            break;
        }
    }

    if (!hasPassItems)
    {
        imgui->Text("No items set to pass");
    }
}

void Lootwhore::RenderItemPreview()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    if (!m_ShowItemPreview || m_SelectedItemId == 0)
        return;

    if (imgui->BeginChild("##ItemPreviewChild", ImVec2(0, 150), true, ImGuiWindowFlags_None))
    {
        IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(m_SelectedItemId);
        if (pItem)
        {
            imgui->BeginGroup();

            // Item name and ID with icon placeholder
            imgui->Text("[Icon] %s [%d]", pItem->Name[0], m_SelectedItemId);
            imgui->Separator();

            // Item description (first line)
            if (strlen(pItem->Description[0]) > 0)
            {
                imgui->TextWrapped("%s", pItem->Description[0]);
                imgui->Spacing();
            }

            // Item level and jobs
            if (pItem->Level > 0)
            {
                imgui->Text("Level: %d", pItem->Level);
            }

            // Job restrictions
            if (pItem->Jobs != 0)
            {
                std::string jobs;
                // FFXI uses 1-based job IDs, so bit 1 = WAR (job 1), bit 2 = MNK (job 2), etc.
                if (pItem->Jobs & 0x2)
                    jobs += "WAR ";
                if (pItem->Jobs & 0x4)
                    jobs += "MNK ";
                if (pItem->Jobs & 0x8)
                    jobs += "WHM ";
                if (pItem->Jobs & 0x10)
                    jobs += "BLM ";
                if (pItem->Jobs & 0x20)
                    jobs += "RDM ";
                if (pItem->Jobs & 0x40)
                    jobs += "THF ";
                if (pItem->Jobs & 0x80)
                    jobs += "PLD ";
                if (pItem->Jobs & 0x100)
                    jobs += "DRK ";
                if (pItem->Jobs & 0x200)
                    jobs += "BST ";
                if (pItem->Jobs & 0x400)
                    jobs += "BRD ";
                if (pItem->Jobs & 0x800)
                    jobs += "RNG ";
                if (pItem->Jobs & 0x1000)
                    jobs += "SAM ";
                if (pItem->Jobs & 0x2000)
                    jobs += "NIN ";
                if (pItem->Jobs & 0x4000)
                    jobs += "DRG ";
                if (pItem->Jobs & 0x8000)
                    jobs += "SMN ";
                if (pItem->Jobs & 0x10000)
                    jobs += "BLU ";
                if (pItem->Jobs & 0x20000)
                    jobs += "COR ";
                if (pItem->Jobs & 0x40000)
                    jobs += "PUP ";
                if (pItem->Jobs & 0x80000)
                    jobs += "DNC ";
                if (pItem->Jobs & 0x100000)
                    jobs += "SCH ";
                if (pItem->Jobs & 0x200000)
                    jobs += "GEO ";
                if (pItem->Jobs & 0x400000)
                    jobs += "RUN ";

                if (!jobs.empty())
                {
                    imgui->Text("Jobs: %s", jobs.c_str());
                }
            }

            // Item flags and type information
            if (pItem->Flags != 0)
            {
                std::string flags;
                if (pItem->Flags & 0x0800)
                    flags += "Rare ";
                if (pItem->Flags & 0x0400)
                    flags += "Ex ";
                if (pItem->Flags & 0x0040)
                    flags += "NoAH ";
                if (pItem->Flags & 0x0001)
                    flags += "WallHanging ";
                if (pItem->Flags & 0x0002)
                    flags += "Flag1 ";
                if (pItem->Flags & 0x0004)
                    flags += "Flag2 ";
                if (!flags.empty())
                {
                    imgui->Text("Flags: %s", flags.c_str());
                }
            }

            // Stack size
            if (pItem->StackSize > 1)
            {
                imgui->Text("Stack Size: %d", pItem->StackSize);
            }

            // Item type/category info
            imgui->Text("Type: %d, Category: %d", pItem->Type, pItem->ItemLevel);

            imgui->EndGroup();
        }
        imgui->EndChild();
    }
}

void Lootwhore::RenderSearchBar(const char* hint, std::function<void(const char*)> onItemSelected)
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    imgui->SetNextItemWidth(-100.0f);

    if (imgui->InputTextWithHint("##SearchInput", hint, m_SearchBuffer, sizeof(m_SearchBuffer)))
    {
        // Search is performed as user types
    }

    imgui->SameLine();
    if (imgui->Button("Add Item"))
    {
        if (strlen(m_SearchBuffer) > 0)
        {
            // Try to find item by name or ID
            IItem* pItem = nullptr;

            // Check if it's a number (item ID)
            if (IsPositiveInteger(m_SearchBuffer))
            {
                int itemId = atoi(m_SearchBuffer);
                if (itemId > 0 && itemId < 65535)
                {
                    pItem = m_AshitaCore->GetResourceManager()->GetItemById(itemId);
                }
            }
            else
            {
                // Search by name
                pItem = m_AshitaCore->GetResourceManager()->GetItemByName(m_SearchBuffer, 0);
            }

            if (pItem && strlen(pItem->Name[0]) > 0)
            {
                onItemSelected(pItem->Name[0]);
                memset(m_SearchBuffer, 0, sizeof(m_SearchBuffer)); // Clear search after adding
            }
            else
            {
                pOutput->error_f("Item not found: %s", m_SearchBuffer);
            }
        }
    }
}

void Lootwhore::RenderSettingsTab()
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    imgui->Text("UI Behavior Settings");
    imgui->Separator();

    // UI Scale input with 0.05 increments (50 to 200, representing 0.50 to 2.00)
    int scaleInt = (int)roundf(mSettings.UIScale * 100.0f);
    if (imgui->InputInt("UI Scale", &scaleInt, 5, 10))
    {
        // Clamp to valid range (50 to 200, representing 0.50 to 2.00)
        scaleInt = max(50, min(200, scaleInt));
        // Ensure it's a multiple of 5 (0.05 increments)
        scaleInt = (scaleInt / 5) * 5;

        mSettings.UIScale = scaleInt / 100.0f;

        std::string settingsFile = mState.MyName + ".xml";
        SaveSettings(settingsFile.c_str());
    }
    imgui->SameLine();
    imgui->Text("(%.2f)", mSettings.UIScale);
    imgui->SameLine();
    HelpMarker("Adjusts the scale of all UI elements in 0.05 increments. Values: 50-200 (0.50-2.00). 100 = normal size.");

    imgui->Spacing();

    // Auto-open window setting
    if (imgui->Checkbox("Auto-open window when new items are added to treasure pool", &mSettings.EnableAutoOpen))
    {
        std::string settingsFile = mState.MyName + ".xml";
        SaveSettings(settingsFile.c_str());
    }
    imgui->SameLine();
    HelpMarker("When enabled, the window will automatically open when new items are detected in the treasure pool.");

    imgui->Spacing();

    // Auto-close window setting
    if (imgui->Checkbox("Auto-close window when treasure pool is empty", &mSettings.EnableAutoClose))
    {
        // Update the runtime setting and save changes
        m_EnableAutoClose        = mSettings.EnableAutoClose;
        std::string settingsFile = mState.MyName + ".xml";
        SaveSettings(settingsFile.c_str());
    }
    imgui->SameLine();
    HelpMarker("When enabled, the window will automatically close if it was opened automatically due to new treasure pool items and all items have been cleared from the pool.");

    imgui->Spacing();

    // Auto-close when handled setting
    if (imgui->Checkbox("Auto-close window when all items are handled or automatic", &mSettings.AutoCloseWhenHandled))
    {
        std::string settingsFile = mState.MyName + ".xml";
        SaveSettings(settingsFile.c_str());
    }
    imgui->SameLine();
    HelpMarker("When enabled, the window will automatically close when all items have been lotted/passed by the player or are in auto-lot/pass/ignore/drop lists.");
}

bool Lootwhore::HasNewItemsInPool()
{
    bool hasNewItems = false;
    for (int i = 0; i < 10; i++)
    {
        Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);
        uint16_t currentItemId                      = (pTreasureItem && pTreasureItem->ItemId > 0) ? pTreasureItem->ItemId : 0;

        // Check if this is a new item (wasn't in the previous state)
        if (currentItemId > 0 && m_PreviousPoolItems[i] != currentItemId)
        {
            hasNewItems = true;
            break;
        }
    }
    return hasNewItems;
}

bool Lootwhore::AllItemsHandledOrAutomatic()
{
    for (int i = 0; i < 10; i++)
    {
        Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);
        if (!pTreasureItem || pTreasureItem->ItemId == 0)
            continue; // Empty slot, skip

        // Check if player has already lotted or passed
        if (pTreasureItem->Lot == 0) // Not yet handled by player
        {
            // Check if item is in one of the auto-lists
            bool isAutomatic = false;

            // Check if in auto-lot list
            auto lotItem = mProfile.ItemMap.find(pTreasureItem->ItemId);
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Lot)
            {
                isAutomatic = true;
            }

            // Check if in auto-pass list
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Pass)
            {
                isAutomatic = true;
            }

            // Check if in ignore list
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Ignore)
            {
                isAutomatic = true;
            }

            // Check if in auto-drop list
            auto dropItem = std::find(mProfile.AutoDrop.begin(), mProfile.AutoDrop.end(), pTreasureItem->ItemId);
            if (dropItem != mProfile.AutoDrop.end())
            {
                isAutomatic = true;
            }

            // If this item requires manual action and hasn't been handled, return false
            if (!isAutomatic)
            {
                return false;
            }
        }
    }
    return true; // All items are either handled or automatic
}

void Lootwhore::UpdatePoolItemTracking()
{
    for (int i = 0; i < 10; i++)
    {
        Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);
        m_PreviousPoolItems[i]                      = (pTreasureItem && pTreasureItem->ItemId > 0) ? pTreasureItem->ItemId : 0;
    }
}

int Lootwhore::GetUnhandledItemCount()
{
    int unhandledCount = 0;

    for (int i = 0; i < 10; i++)
    {
        Ashita::FFXI::treasureitem_t* pTreasureItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetTreasurePoolItem(i);
        if (!pTreasureItem || pTreasureItem->ItemId == 0)
            continue; // Empty slot, skip

        // Check if player has already lotted or passed
        if (pTreasureItem->Lot == 0) // Not yet handled by player
        {
            // Check if item is in one of the auto-lists
            bool isAutomatic = false;

            // Check if in auto-lot list
            auto lotItem = mProfile.ItemMap.find(pTreasureItem->ItemId);
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Lot)
            {
                isAutomatic = true;
            }

            // Check if in auto-pass list
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Pass)
            {
                isAutomatic = true;
            }

            // Check if in ignore list
            if (lotItem != mProfile.ItemMap.end() && lotItem->second == LotReaction::Ignore)
            {
                isAutomatic = true;
            }

            // Check if in auto-drop list
            auto dropItem = std::find(mProfile.AutoDrop.begin(), mProfile.AutoDrop.end(), pTreasureItem->ItemId);
            if (dropItem != mProfile.AutoDrop.end())
            {
                isAutomatic = true;
            }

            // If this item requires manual action and hasn't been handled, count it
            if (!isAutomatic)
            {
                unhandledCount++;
            }
        }
    }

    return unhandledCount;
}

void Lootwhore::HelpMarker(const char* desc)
{
    auto imgui = m_AshitaCore->GetGuiManager();
    if (!imgui)
        return;

    imgui->TextDisabled("(?)");
    if (imgui->IsItemHovered())
    {
        imgui->SetTooltip("%s", desc);
    }
}
