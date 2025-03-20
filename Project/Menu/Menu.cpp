#include "Menu.h"
#include "..\Hooks\Hook.h"
#include "..\GTA_Functions\GTA_Functions.h"


ImVec4 luaKeywordColor = ImVec4(0.0f, 0.5f, 0.9f, 1.0f);
ImVec4 luaCommentColor = ImVec4(0.3f, 0.7f, 0.3f, 1.0f);
ImVec4 luaStringColor = ImVec4(0.9f, 0.5f, 0.0f, 1.0f);
ImVec4 luaNumberColor = ImVec4(0.4f, 0.9f, 0.9f, 1.0f);
ImVec4 luaDefaultColor = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

bool Lua_Dump;
bool Menu_Aktif;
bool Script_Send;
static bool isDraggingFov = false;
static bool isDraggingDistance = false;
static bool isDraggingFillBoxAlpha = false;
static bool isDraggingBoxSize = false;
static bool isDraggingCornerSize = false;
std::vector<std::string> resources;
struct ScriptEntry {
    int id;   
    std::string LoadScriptFromBufferScriptName;
    std::string DeobfuscateScriptName;
};
std::vector<ScriptEntry> scriptList;
std::unordered_map<std::string, std::string> scripts;
std::string selectedScriptTitle;
int currentID = 0;
std::vector<std::string> splitString(const std::string& str, char delimiter = '\\') {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string part;
    while (std::getline(iss, part, delimiter)) {
        result.push_back(part);
    }
    return result;
}

int stringIndexBul(const std::vector<std::string>& vektor, const std::string& str) {
    auto it = std::find(vektor.begin(), vektor.end(), str);
    if (it != vektor.end()) {
        return std::distance(vektor.begin(), it);
    }
    return -1;
}
bool IsLuaObfuscatedScript(const void* pData, unsigned int uiLength)
{
    const unsigned char* pCharData = (const unsigned char*)pData;
    return (uiLength > 0 && pCharData[0] == 0x1C);            // Look for our special marker
}
bool containsInvalidCharacters(const std::string& content) {
    for (char c : content) {
        if (c < 32 || c > 126) {
            return true;
        }
    }
    return false;
}

void addScript(const std::string& title, const std::string& content) {
    if (content.empty() || title.empty()) {
        return;
    }
    if (scripts.find(title) != scripts.end()) {
        return;
    }
    if (containsInvalidCharacters(content)) {
        scripts[title] = "Obfuscated Script";
        return;
    }
    if (!IsLuaObfuscatedScript(content.c_str(), content.length())) {
        scripts[title] = content;
    }
    else {
        scripts[title] = "Obfuscated Script";
    }
}
void Reset_Script_And_Resources()
{
    scripts.clear();
    scriptList.clear();
    resources.clear();
}
void NormalizePath(char* path) {
#ifdef _WIN32
    char separator = '\\';
#else
    char separator = '/';
#endif

    for (char* p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = separator;
        }
    }
}
int CreateDirectories(const char* path) {
    char temp[512];
    char* p = NULL;
    snprintf(temp, sizeof(temp), "%s", path);
    NormalizePath(temp);

    for (p = temp + 1; *p; p++) {
        if (*p == '\\' || *p == '/') {
            *p = '\0';
            if (_mkdir(temp) != 0 && errno != EEXIST) {
                perror("Klasör oluþturulamadý");
                return -1;
            }
            *p = '\\';
        }
    }
    if (_mkdir(temp) != 0 && errno != EEXIST) {
        perror("Son klasör oluþturulamadý");
        return -1;
    }

    return 0;
}

void DumpToFile(const char* cpInBuffer, UINT uiInSize, const char* szScriptName) {
    const char* dumpFolder = "Project_Dump";

    if (_mkdir(dumpFolder) != 0 && errno != EEXIST) {
        perror("Ana klasör oluþturulamadý");
        return;
    }
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath), "%s/%s", dumpFolder, szScriptName);
    NormalizePath(fullPath);

    char folderPath[512];
    strncpy(folderPath, fullPath, sizeof(folderPath));
    char* lastSlash = strrchr(folderPath, '\\');
    if (!lastSlash) {
        lastSlash = strrchr(folderPath, '/');
    }

    if (lastSlash != NULL) {
        *lastSlash = '\0';
        if (CreateDirectories(folderPath) != 0) {
            return;
        }
    }
    FILE* file = fopen(fullPath, "wb");
    if (file == NULL) {
        perror("Dosya oluþturulamadý");
        printf("Hedef dosya: %s\n", fullPath);
        return;
    }

    fwrite(cpInBuffer, 1, uiInSize, file);
    fclose(file);

    printf("Dosya baþarýyla oluþturuldu: %s\n", fullPath);
}
std::string randomString(size_t length) {
    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += characters[distribution(generator)];
    }
    return random_string;
}

std::string modifyFileName(const std::string& originalName) {
    std::string randomStr = randomString(5);

    std::string newFileName = originalName + "\\" + randomStr + ".lua";

    return newFileName;
}
std::string modifyScriptName(const std::string& originalName) {
    size_t dotPos = originalName.find_last_of(".");
    if (dotPos == std::string::npos) {
        return originalName;
    }

    std::string baseName = originalName.substr(0, dotPos);
    std::string extension = originalName.substr(dotPos);
    std::string randomStr = randomString(1 + (rand() % 4));

    return baseName + randomStr + extension;
}
void AddLoadScriptToScriptList(const std::string& loadScriptName) {
    for (auto& entry : scriptList) {
        if (entry.id == currentID) {
            entry.LoadScriptFromBufferScriptName = loadScriptName;
            return;
        }
    }
    ScriptEntry newEntry = { currentID, loadScriptName, "" };
    scriptList.push_back(newEntry);
}
void AddDeobfuscateScriptToScriptList(const std::string& deobfuscateScriptName) {
    for (auto& entry : scriptList) {
        if (entry.id == currentID) {
            entry.DeobfuscateScriptName = deobfuscateScriptName;
            currentID++;
            return;
        }
    }
    ScriptEntry newEntry = { currentID, "", deobfuscateScriptName };
    scriptList.push_back(newEntry);
    currentID++;
}
bool ContainsWord(const char* szName, const std::string& word) {
    if (szName == nullptr) {
        return false;
    }
    std::string name(szName);
    return name.find(word) != std::string::npos;
}
int selectedResourceIndex = 0;
static char searchBuffer[256] = "";
void AddResources(const char* ScriptName) {
    std::string strScriptName(ScriptName);
    auto it = std::find(resources.begin(), resources.end(), strScriptName);
    if (it == resources.end()) {
        resources.push_back(strScriptName);
    }
}

bool LoadTextureFromFile(LPDIRECT3DDEVICE9 device , const char* filename, LPDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height) {
    HRESULT hr = D3DXCreateTextureFromFileA(device, filename, out_texture);
    if (hr != S_OK)
        return false;

    D3DSURFACE_DESC my_image_desc;
    (*out_texture)->GetLevelDesc(0, &my_image_desc);
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;

    return true;
}

void LoadCustomFont() {
    ImGuiIO& io = ImGui::GetIO();
    const char* fontPath = "C:/Windows/Fonts/micross.ttf";
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath, 16.0f);
    if (font == NULL) {
        fprintf(stderr, "Hata: Yazý tipi '%s' yüklenemedi!\n", fontPath);
        return;
    }

    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();
}
namespace ColorSettings {
    ImVec4 windowBg = ImVec4(0.05f, 0.05f, 0.05f, 0.95f);
    ImVec4 titleBg = ImVec4(0.1f, 0.0f, 0.2f, 1.0f);
    ImVec4 titleBgActive = ImVec4(0.2f, 0.0f, 0.4f, 1.0f);
    ImVec4 button = ImVec4(0.3f, 0.0f, 0.6f, 1.0f);
    ImVec4 buttonHovered = ImVec4(0.4f, 0.0f, 0.8f, 1.0f);
    ImVec4 buttonActive = ImVec4(0.5f, 0.0f, 1.0f, 1.0f);
    ImVec4 header = ImColor(100, 100, 255, 255);
    ImVec4 headerHovered = ImVec4(0.3f, 0.0f, 0.6f, 1.0f);
    ImVec4 headerActive = ImVec4(0.4f, 0.0f, 0.8f, 1.0f);
    ImVec4 checkMark = ImVec4(0.8f, 0.0f, 1.0f, 1.0f);
    ImVec4 frameBg = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    ImVec4 frameBgHovered = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    ImVec4 frameBgActive = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    ImVec4 text = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
    ImVec4 scrollbarBg = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    ImVec4 scrollbarGrab = ImVec4(0.465f, 0.346f, 0.584f, 1.0f);
    ImVec4 scrollbarGrabHovered = ImVec4(0.5f, 0.0f, 1.0f, 1.0f);
    ImVec4 scrollbarGrabActive = ImVec4(0.6f, 0.0f, 1.0f, 1.0f);
    ImVec4 separator = ImVec4(0.2f, 0.0f, 0.4f, 1.0f);
}
void StylePurpleBlackTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_WindowBg] = ColorSettings::windowBg;
    style.Colors[ImGuiCol_TitleBg] = ColorSettings::titleBg;
    style.Colors[ImGuiCol_TitleBgActive] = ColorSettings::titleBgActive;
    style.Colors[ImGuiCol_TitleBgCollapsed] = ColorSettings::titleBg;
    style.Colors[ImGuiCol_Button] = ColorSettings::button;
    style.Colors[ImGuiCol_ButtonHovered] = ColorSettings::buttonHovered;
    style.Colors[ImGuiCol_ButtonActive] = ColorSettings::buttonActive;
    style.Colors[ImGuiCol_Header] = ColorSettings::header;
    style.Colors[ImGuiCol_HeaderHovered] = ColorSettings::headerHovered;
    style.Colors[ImGuiCol_HeaderActive] = ColorSettings::headerActive;
    style.Colors[ImGuiCol_CheckMark] = ColorSettings::checkMark;
    style.Colors[ImGuiCol_FrameBg] = ColorSettings::frameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = ColorSettings::frameBgHovered;
    style.Colors[ImGuiCol_FrameBgActive] = ColorSettings::frameBgActive;
    style.Colors[ImGuiCol_Text] = ColorSettings::text;
    style.Colors[ImGuiCol_ScrollbarBg] = ColorSettings::scrollbarBg;
    style.Colors[ImGuiCol_ScrollbarGrab] = ColorSettings::scrollbarGrab;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ColorSettings::scrollbarGrabHovered;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ColorSettings::scrollbarGrabActive;
    style.Colors[ImGuiCol_Separator] = ColorSettings::separator;

    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 5.0f;


    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;


    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;
}

int selectedTopTab = 0;
int selectedLuaSubTab = 0;

namespace LuaTab {
    bool customFileName = false;
    char fileName[256] = "Client.lua";
    char LuaName[256] = "Lua Path";
    bool autoInject = false;
    int autoInjectMode = 0;
    bool debugHookSpoofer = false;
    bool blockOnClientPasteEvent = false;
    char filterResourceName[256] = "";
    TextEditor editor;
}

namespace VisualsTab {
    bool enableESP = true;
    bool boxESP = true;
    bool Crasher = false;
    bool Airbreak = false;
    int distance = 300;
    int fillBoxAlpha = 100;
    int boxSize = 100;
    int cornerSize = 100;
}

namespace AimbotsTab {
    bool Aimbot = false;
    bool SilentAim = false;
    bool RotationSync = false;
    bool CrossHairAimbot = false;
    extern int Fov = 150;
}

void HighlightLuaSyntax(const char* text) {
    std::string str(text);
    std::smatch match;
    std::regex keywords("(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)");
    for (std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), keywords); i != std::sregex_iterator(); ++i) {
        match = *i;
        ImGui::TextColored(luaKeywordColor, match.str().c_str());
        ImGui::SameLine(0, 0);
    }
    std::regex comments("(--.*)");
    for (std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), comments); i != std::sregex_iterator(); ++i) {
        match = *i;
        ImGui::TextColored(luaCommentColor, match.str().c_str());
        ImGui::SameLine(0, 0);
    }
    std::regex strings("(\"[^\"]*\"|'[^']*')");
    for (std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), strings); i != std::sregex_iterator(); ++i) {
        match = *i;
        ImGui::TextColored(luaStringColor, match.str().c_str());
        ImGui::SameLine(0, 0);
    }
    std::regex numbers(R"(\b\d+(\.\d+)?([eE][+-]?\d+)?\b)");
    for (std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), numbers); i != std::sregex_iterator(); ++i) {
        match = *i;
        ImGui::TextColored(luaNumberColor, match.str().c_str());
        ImGui::SameLine(0, 0);
    }
    std::regex others(R"(([^"'\n]+)|[\n])");
    for (std::sregex_iterator i = std::sregex_iterator(str.begin(), str.end(), others); i != std::sregex_iterator(); ++i) {
        match = *i;
        if (!std::regex_search(match.str(), keywords) &&
            !std::regex_search(match.str(), comments) &&
            !std::regex_search(match.str(), strings) &&
            !std::regex_search(match.str(), numbers)) {
            ImGui::TextColored(luaDefaultColor, match.str().c_str());
            ImGui::SameLine(0, 0);
        }
    }
}



struct Joint {
    ImVec2 position;
    int parent;
};

std::vector<Joint> skeleton = {
    {ImVec2(135, 53), -1},
    {ImVec2(135, 83), 0},
    {ImVec2(100, 83), 1},
    {ImVec2(65, 113), 2},
    {ImVec2(35, 153), 3},
    {ImVec2(170, 83), 1},
    {ImVec2(205, 113), 5},
    {ImVec2(235, 153), 6},
    {ImVec2(135, 153), 1},
    {ImVec2(110, 193), 8},
    {ImVec2(85, 243), 9},
    {ImVec2(60, 303), 10},
    {ImVec2(160, 193), 8},
    {ImVec2(185, 243), 12},
    {ImVec2(210, 303), 13},
};

void DrawSkeleton(const std::vector<Joint>& skeleton, ImVec2 imagePos, float scale, ImVec2 offset, ImVec2 skeletonScale) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (const auto& joint : skeleton) {
        ImVec2 screenPos = ImVec2(imagePos.x + (joint.position.x + offset.x) * scale * skeletonScale.x, imagePos.y + (joint.position.y + offset.y) * scale * skeletonScale.y);
        drawList->AddCircleFilled(screenPos, 3, IM_COL32(255, 255, 255, 255));
    }

    for (int i = 0; i < skeleton.size(); ++i) {
        const Joint& joint = skeleton[i];
        if (joint.parent != -1) {
            ImVec2 start = ImVec2(imagePos.x + (skeleton[joint.parent].position.x + offset.x) * scale * skeletonScale.x, imagePos.y + (skeleton[joint.parent].position.y + offset.y) * scale * skeletonScale.y);
            ImVec2 end = ImVec2(imagePos.x + (joint.position.x + offset.x) * scale * skeletonScale.x, imagePos.y + (joint.position.y + offset.y) * scale * skeletonScale.y);
            drawList->AddLine(start, end, IM_COL32(255, 0, 0, 255), 1.5f);
        }
    }
}



void DrawDynamicSlider(const char* label, int* value, int min, int max, bool* isDragging) {
    ImGui::PushID(label);
    ImGui::InvisibleButton(label, ImVec2(200, 20));

    if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        *isDragging = true;
    }
    else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        *isDragging = false;
    }

    if (*isDragging) {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        ImVec2 buttonPos = ImGui::GetItemRectMin();
        *value = (mousePos.x - buttonPos.x) / ImGui::GetItemRectSize().x * (max - min) + min;
        *value = std::clamp(*value, min, max);
    }

    ImGui::PopID();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();

    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x * ((*value - min) / (float)(max - min)), pos.y + size.y), IM_COL32(100, 100, 255, 255));
    drawList->AddCircleFilled(ImVec2(pos.x + size.x * ((*value - min) / (float)(max - min)), pos.y + size.y / 2), 8, IM_COL32(255, 255, 255, 255));

    char valueText[32];
    sprintf(valueText, "%s: %d", label, *value);

    ImVec2 textSize = ImGui::CalcTextSize(valueText);
    ImVec2 textPos = ImVec2(pos.x + (size.x - textSize.x) / 2, pos.y + (size.y - textSize.y) / 2);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), valueText);
}
void DrawDynamicCheckbox(const char* label, bool* value) {
    ImGui::PushID(label);

    if (ImGui::InvisibleButton(label, ImVec2(20, 20))) {
        *value = !(*value);
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();

    if (*value) {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 255, 255), 5.0f);
        drawList->AddLine(ImVec2(pos.x + 4, pos.y + 12), ImVec2(pos.x + 8, pos.y + 16), IM_COL32(255, 255, 255, 255), 2.0f);
        drawList->AddLine(ImVec2(pos.x + 8, pos.y + 16), ImVec2(pos.x + 16, pos.y + 4), IM_COL32(255, 255, 255, 255), 2.0f);
    }
    else {
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255), 5.0f);
    }
    ImGui::SameLine();
    ImGui::Text(label);

    ImGui::PopID();
}
bool DrawModernCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1) {
    ImGui::PushID(label);
    float buttonWidth = ImGui::CalcTextSize(label).x + 30.0f;


    if (ImGui::InvisibleButton(label, ImVec2(buttonWidth, 20))) {
        ImGui::OpenPopup(label);
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 size = ImGui::GetItemRectSize();

    ImU32 buttonColor = ImGui::IsItemActivated() ? IM_COL32(70, 70, 70, 255) : IM_COL32(100, 100, 100, 255);
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), buttonColor, 5.0f);


    ImU32 arrowColor = ImGui::IsItemActivated() ? IM_COL32(255, 255, 255, 255) : IM_COL32(150, 150, 150, 255);
    ImVec2 arrowPos = ImVec2(pos.x + size.x - 20, pos.y + size.y / 2);
    drawList->AddTriangleFilled(
        ImVec2(arrowPos.x - 5, arrowPos.y - 3),
        ImVec2(arrowPos.x + 5, arrowPos.y - 3),
        ImVec2(arrowPos.x, arrowPos.y + 3),
        arrowColor);

    drawList->AddText(ImVec2(pos.x + 5, pos.y + 3), IM_COL32(255, 255, 255, 255), items[*current_item]);

    bool value_changed = false;

    if (ImGui::BeginPopup(label)) {
        for (int i = 0; i < items_count; i++) {
            bool is_selected = (i == *current_item);
            if (ImGui::Selectable(items[i], is_selected)) {
                *current_item = i;
                value_changed = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndPopup();
    }

    ImGui::PopID();
    return value_changed;
}
void DrawModernTabBar(const char* labels[], int labels_count, int* selected_index) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float tabWidth = (ImGui::GetWindowWidth() - (labels_count - 1) * ImGui::GetStyle().ItemSpacing.x) / labels_count;
    float spacing = 5.0;
    for (int i = 0; i < labels_count; i++) {
        ImVec2 tabPos = ImVec2(pos.x + i * (tabWidth + spacing), pos.y);
        ImVec2 tabSize = ImVec2(tabWidth, 28);

        ImGui::PushID(i);

        ImGui::SetCursorScreenPos(tabPos);
        ImGui::InvisibleButton(labels[i], tabSize, ImGuiButtonFlags_MouseButtonLeft);

        if (ImGui::IsItemClicked()) {
            *selected_index = i;
        }

        ImGui::PopID();

        ImU32 tabColor = (*selected_index == i) ? IM_COL32(ImGui::GetStyle().Colors[ImGuiCol_Header].x * 255, ImGui::GetStyle().Colors[ImGuiCol_Header].y * 255, ImGui::GetStyle().Colors[ImGuiCol_Header].z * 255, 255) : IM_COL32(ImGui::GetStyle().Colors[ImGuiCol_Button].x * 255, ImGui::GetStyle().Colors[ImGuiCol_Button].y * 255, ImGui::GetStyle().Colors[ImGuiCol_Button].z * 255, 255);
        drawList->AddRectFilled(tabPos, ImVec2(tabPos.x + tabSize.x, tabPos.y + tabSize.y), tabColor, 5.0f);

        ImVec2 textSize = ImGui::CalcTextSize(labels[i]);
        ImVec2 textPos = ImVec2(tabPos.x + (tabSize.x - textSize.x) / 2.0f, tabPos.y + (tabSize.y - textSize.y) / 2.0f);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), labels[i]);

        if (i < labels_count - 1) {
            ImGui::SameLine();
        }
    }
}
ImVec2 rotatePoint(ImVec2 point, ImVec2 center, float angle) {
    float radians = angle * (3.14159f / 180.0f);
    float cosTheta = cosf(radians);
    float sinTheta = sinf(radians);

    ImVec2 rotatedPoint;
    rotatedPoint.x = (cosTheta * (point.x - center.x) - sinTheta * (point.y - center.y) + center.x);
    rotatedPoint.y = (sinTheta * (point.x - center.x) + cosTheta * (point.y - center.y) + center.y);

    return rotatedPoint;
}
std::string cp1251_to_utf8(const char* str)
{
    std::string res;
    WCHAR* ures = NULL;
    char* cres = NULL;
    int result_u = MultiByteToWideChar(1251, 0, str, -1, 0, 0);
    if (result_u != 0)
    {
        ures = new WCHAR[result_u];
        if (MultiByteToWideChar(1251, 0, str, -1, ures, result_u))
        {
            int result_c = WideCharToMultiByte(CP_UTF8, 0, ures, -1, 0, 0, 0, 0);
            if (result_c != 0)
            {
                cres = new char[result_c];
                if (WideCharToMultiByte(CP_UTF8, 0, ures, -1, cres, result_c, 0, 0))
                {
                    res = cres;
                }
            }
        }
    }
    delete[] ures, cres;
    return res;
}
void Render_Menu() {
    if (Menu_Aktif)
    {
        static bool styleApplied = false;
        if (!styleApplied) {
            StylePurpleBlackTheme();
            auto lang = TextEditor::LanguageDefinition::Lua();
            LuaTab::editor.SetLanguageDefinition(lang);
            LuaTab::editor.SetText("outputChatBox(\"Project TEST\")\n");
            styleApplied = true;
        }

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 screenSize = io.DisplaySize;
        ImVec2 windowSize = ImVec2(750, 450);
        ImGui::SetNextWindowSize(windowSize);

        ImVec2 windowPos = ImVec2(
            screenSize.x / 2.0f - windowSize.x / 2.0f,
            screenSize.y / 2.0f - windowSize.y / 2.0f
        );
        ImGui::SetNextWindowPos(windowPos);
        ImGui::Begin("Project", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetWindowPos();


        ImVec2 center = ImVec2(screenSize.x / 2.0f, screenSize.y / 2.0f);

        static float angle = 0.0f;
        angle += ImGui::GetIO().DeltaTime * 50.0f;
        if (angle > 360.0f) {
            angle -= 360.0f;
        }
        float size = screenSize.x * 0.7f;

        ImVec2 p1 = rotatePoint(ImVec2(center.x, pos.y), center, angle);
        ImVec2 p2 = rotatePoint(ImVec2(pos.x + screenSize.x, center.y), center, angle);
        ImVec2 p3 = rotatePoint(ImVec2(center.x, pos.y + screenSize.y), center, angle);
        ImVec2 p4 = rotatePoint(ImVec2(pos.x, center.y), center, angle);

        // Kontrol noktalarý (yuvarlatýlmýþ köþeler için)
        ImVec2 c1 = rotatePoint(ImVec2(center.x + size / 4, pos.y + size / 4), center, angle);
        ImVec2 c2 = rotatePoint(ImVec2(pos.x + screenSize.x - size / 4, center.y + size / 4), center, angle);
        ImVec2 c3 = rotatePoint(ImVec2(center.x - size / 4, pos.y + screenSize.y - size / 4), center, angle);
        ImVec2 c4 = rotatePoint(ImVec2(pos.x + size / 4, center.y - size / 4), center, angle);

        drawList->AddBezierCubic(p1, c1, c4, p4, ImColor(ColorSettings::scrollbarGrab), 3.0f);
        drawList->AddBezierCubic(p4, c4, c3, p3, ImColor(ColorSettings::scrollbarGrab), 3.0f);
        drawList->AddBezierCubic(p3, c3, c2, p2, ImColor(ColorSettings::scrollbarGrab), 3.0f);
        drawList->AddBezierCubic(p2, c2, c1, p1, ImColor(ColorSettings::scrollbarGrab), 3.0f);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
        ImGui::BeginGroup();
        const char* titleText = "Project";
        ImVec2 textSize = ImGui::CalcTextSize(titleText);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textSize.x) / 2); // Ortala
        ImGui::Text(titleText);
        ImGui::SameLine(ImGui::GetWindowWidth() - 25);

        ImGui::EndGroup();


        const char* topTabs[] = { "Visuals", "Aimbot", "Self", "Weapon", "Vehicle", "Misc", "Lua", "Config" };
        const int numTopTabs = IM_ARRAYSIZE(topTabs);
        const float tabWidth = (ImGui::GetWindowWidth() - (numTopTabs - 1) * ImGui::GetStyle().ItemSpacing.x) / numTopTabs;

        DrawModernTabBar(topTabs, IM_ARRAYSIZE(topTabs), &selectedTopTab);

        ImGui::BeginChild("Content", ImVec2(0, 0), true);

        if (selectedTopTab == 0) {
            ImGui::BeginChild("VisualPanel", ImVec2(300, 0), false);
            DrawDynamicCheckbox("Enable ESP", &VisualsTab::enableESP);
            DrawDynamicCheckbox("Box ESP", &VisualsTab::boxESP);

            DrawDynamicCheckbox("Crasher", &VisualsTab::Crasher);

            DrawDynamicCheckbox("AirBreak", &VisualsTab::Airbreak);
            

            const char* weapons[] = {
   ("Fist"), ("Brass Knuckles"), ("Golf Club"), ("Nightstick"), ("Knife"),
   ("Baseball Bat"), ("Shovel"), ("Pool Cue"), ("Katana"), ("Chainsaw"),
   ("Purple Dildo"), ("Dildo"), ("Vibrator"), ("Silver Vibrator"), ("Flowers"),
   ("Cane"), ("Grenade"), ("Tear Gas"), ("Molotov Cocktail"), ("Unknown"),
   ("Unknown"), ("Unknown"), ("9mm"), ("Silenced 9mm"), ("Desert Eagle"), ("Shotgun"),
   ("Sawnoff Shotgun"), ("Combat Shotgun"), ("Micro SMG/Uzi"), ("MP5"), ("AK-47"),
   ("M4"), ("Tec-9"), ("Country Rifle"), ("Sniper Rifle"), ("RPG"),
   ("HS Rocket"), ("Flamethrower"), ("Minigun"), ("Satchel Charge"), ("Detonator"),
   ("Spraycan"), ("Fire Extinguisher"), ("Camera"), ("Night Vision Goggles"),
   ("Thermal Goggles"), ("Parachute")
            };

            int weaponIDs[] = {
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                20, 22, 23, 24, 25, 26, 27, 28, 29, 30,
                31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                41, 42, 43, 44, 45, 46
            };


            const int GameWeaponsGXDSize = sizeof(weapons) / sizeof(weapons[0]);
            static int weaponid = 0;
            ImGui::Combo("Get Weapon", &weaponid, weapons, GameWeaponsGXDSize);

            if (ImGui::Button(("Get Weapon##" + std::to_string(weaponid)).c_str(), ImVec2((ImGui::GetWindowWidth() / 2 - ImGui::GetStyle().ItemSpacing.x / 2 / 2), 25.f))) {
                if (weaponid != 0 && weaponid != 19 && weaponid != 20 && weaponid != 21) {
                    printf("weapon %d\n", weaponid);
                    CPed* pPedSelf = FindPlayerPed();
                    pPedSelf->GiveWeapon((eWeaponType)weaponid, 1000, true); // Silah ver
                }
            }

            DrawDynamicSlider("Distance", &VisualsTab::distance, 0, 5000, &isDraggingDistance);
            DrawDynamicSlider("FillBox Alpha", &VisualsTab::fillBoxAlpha, 0, 255, &isDraggingFillBoxAlpha);
            DrawDynamicSlider("Box Size", &VisualsTab::boxSize, 1, 10, &isDraggingBoxSize);
            DrawDynamicSlider("Corner Size", &VisualsTab::cornerSize, 1, 10, &isDraggingCornerSize);

            ImGui::Spacing();

            ImGui::EndChild();
        }
        else if (selectedTopTab == 1) {
            ImGui::BeginChild("Aimbot_", ImVec2(300, 0), false);
            DrawDynamicCheckbox("Aimbot", &AimbotsTab::Aimbot);
            DrawDynamicCheckbox("Crosshair Aimbot", &AimbotsTab::CrossHairAimbot);
            DrawDynamicCheckbox("Silent Aimbot", &AimbotsTab::SilentAim);
            DrawDynamicCheckbox("Rotation Player", &AimbotsTab::RotationSync);


            DrawDynamicSlider("Distance", &AimbotsTab::Fov, 100, 1000, &isDraggingFov);


            ImGui::Spacing();
            ImGui::EndChild();
        }
        else if (selectedTopTab == 2) {
            ImGui::Text("Self Ýçeriði");
        }
        else if (selectedTopTab == 3) {
            ImGui::Text("Weapon Ýçeriði");
        }
        else if (selectedTopTab == 4) {
            ImGui::Text("Vehicle Ýçeriði");
        }
        else if (selectedTopTab == 5) {
            ImGui::Text("Misc Ýçeriði");
        }
        if (selectedTopTab == 6) {
            ImGui::BeginChild("LuaLeftPanel", ImVec2(ImGui::GetWindowWidth() / 4, 0), true);

            const char* luaSubTabs[] = { "Lua Executor", "Lua Dumper", "Scripts" };
            const int numLuaSubTabs = IM_ARRAYSIZE(luaSubTabs);

            for (int i = 0; i < numLuaSubTabs; i++) {
                if (i == selectedLuaSubTab) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Header]);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]);
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Button]);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
                }

                if (ImGui::Button(luaSubTabs[i], ImVec2(-1, 30))) {
                    selectedLuaSubTab = i;
                }

                ImGui::PopStyleColor(3);
            }

            ImGui::EndChild();

            ImGui::SameLine();
            ImGui::BeginChild("LuaRightPanel", ImVec2(0, 0), true);

            if (selectedLuaSubTab == 0) {
                if (resources.size() > 0)
                {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
                    ImGui::BeginChild("LuaExecutorContent", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 60), true);
                    ImGui::PopStyleColor();

                    LuaTab::editor.Render("LuaScript");

                    ImGui::EndChild();

                    ImGui::PushItemWidth(ImGui::GetWindowWidth() / 4);
                    if (ImGui::Button("File Inject", ImVec2(80, 25))) {
                        if (LuaTab::LuaName != "Lua Path")
                        {
                            
                        }
                    }
                    ImGui::SameLine();
                    ImGui::InputText("##LuaPath", LuaTab::LuaName, sizeof(LuaTab::LuaName));
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::PushItemWidth(ImGui::GetWindowWidth() / 5);
                    DrawDynamicCheckbox("Auto Inject (?)", &LuaTab::autoInject);
                    ImGui::SameLine();
                    const char* autoInjectModes[] = { "Before", "After" };
                    ImGui::Combo("##AutoInjectMode", &LuaTab::autoInjectMode, autoInjectModes, IM_ARRAYSIZE(autoInjectModes));
                    ImGui::PopItemWidth();


                    DrawDynamicCheckbox("DebugHook Spoofer (?)", &LuaTab::debugHookSpoofer);
                    ImGui::SameLine();
                    DrawDynamicCheckbox("Block 'onClientPaste' Event (?)", &LuaTab::blockOnClientPasteEvent);
                    ImGui::SameLine();
                    ImGui::PushItemWidth(ImGui::GetWindowWidth() / 4);
                    ImGui::InputText("##FilterResourceName", LuaTab::filterResourceName, sizeof(LuaTab::filterResourceName));
                    ImGui::PopItemWidth();
                    LuaTab::editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                    if (ImGui::Button("Execute", ImVec2(80, 25))) {
                        std::string scriptContent = LuaTab::editor.GetText();

                        std::string utf8_script;
                        utf8_script = cp1251_to_utf8(scriptContent.c_str());
                        int bulunanIndeks = stringIndexBul(resources, Script_Name[selectedResourceIndex]);

                        if (bulunanIndeks != -1) {
                            Send_Script_Packet = true;
                            LoadScriptFromBuffer(Call_ECX[bulunanIndeks], Call_This[bulunanIndeks], utf8_script.c_str(), utf8_script.length(), Script_Name[bulunanIndeks].c_str());
                        }
                        else {
                            std::cerr << "Hata: '" << Script_Name[selectedResourceIndex] << "' script ismi resources içinde bulunamadý." << std::endl;
                        }

                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear Lines", ImVec2(80, 25))) {
                        LuaTab::editor.SetText("");
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Load File", ImVec2(80, 25))) {
                    }
                    ImGui::SameLine();

                    ImGui::InputText("Ara", searchBuffer, IM_ARRAYSIZE(searchBuffer));


                    if (ImGui::BeginCombo("Select a Resource", resources[selectedResourceIndex].c_str())) {
                        for (size_t i = 0; i < resources.size(); ++i) {
                            std::string resource = resources[i];

                            if (resource.find(searchBuffer) != std::string::npos) {

                                const bool is_selected = (selectedResourceIndex == i);
                                if (ImGui::Selectable(resource.c_str(), is_selected)) {
                                    selectedResourceIndex = i;
                                }

                                if (is_selected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
            }
            else if (selectedLuaSubTab == 1) {
                ImGui::BeginChild("LuaDumperContent", ImVec2(0, 0), true);
                DrawDynamicCheckbox("Dump Server ( need reconnect )", &Lua_Dump);
                ImGui::EndChild();
            }
            else if (selectedLuaSubTab == 2) {
                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                ImGui::BeginChild("SharedScriptsContent", ImVec2(0, 0), true);
                if (ImGui::BeginCombo("Script", selectedScriptTitle.c_str())) {
                    for (const auto& script : scripts) {
                        bool is_selected = (selectedScriptTitle == script.first);
                        if (ImGui::Selectable(script.first.c_str(), is_selected)) {
                            selectedScriptTitle = script.first;
                        }
                        if (is_selected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }
                if (!selectedScriptTitle.empty()) {
                    auto it = std::find_if(scripts.begin(), scripts.end(),
                        [&](const auto& s) { return s.first == selectedScriptTitle; });
                    if (it != scripts.end()) {
                        ImGui::InputTextMultiline("Contents", (char*)it->second.c_str(), it->second.size(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 15), ImGuiInputTextFlags_ReadOnly);
                    }
                }
                ImGui::PopStyleColor();
                ImGui::EndChild();
            }

            ImGui::EndChild();

        }
        else if (selectedTopTab == 7) {



            ImGui::BeginChild("ConfigContent", ImVec2(0, 0), true);

            ImGui::Text("Window Background");
            if (ImGui::ColorEdit4("##WindowBg", &ColorSettings::windowBg.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ColorSettings::windowBg;
            }
            ImGui::Text("Title Background");
            if (ImGui::ColorEdit4("##TitleBg", &ColorSettings::titleBg.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ColorSettings::titleBg;
            }
            ImGui::Text("Title Background Active");
            if (ImGui::ColorEdit4("##TitleBgActive", &ColorSettings::titleBgActive.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ColorSettings::titleBgActive;
            }
            ImGui::Text("Button");
            if (ImGui::ColorEdit4("##Button", &ColorSettings::button.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_Button] = ColorSettings::button;
            }
            ImGui::Text("Button Hovered");
            if (ImGui::ColorEdit4("##ButtonHovered", &ColorSettings::buttonHovered.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ColorSettings::buttonHovered;
            }
            ImGui::Text("Button Active");
            if (ImGui::ColorEdit4("##ButtonActive", &ColorSettings::buttonActive.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ColorSettings::buttonActive;
            }
            ImGui::Text("Header");
            if (ImGui::ColorEdit4("##Header", &ColorSettings::header.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_Header] = ColorSettings::header;
            }
            ImGui::Text("Header Hovered");
            if (ImGui::ColorEdit4("##HeaderHovered", &ColorSettings::headerHovered.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ColorSettings::headerHovered;
            }
            ImGui::Text("Header Active");
            if (ImGui::ColorEdit4("##HeaderActive", &ColorSettings::headerActive.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_HeaderActive] = ColorSettings::headerActive;
            }
            ImGui::Text("Check Mark");
            if (ImGui::ColorEdit4("##CheckMark", &ColorSettings::checkMark.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ColorSettings::checkMark;
            }
            ImGui::Text("Frame Background");
            if (ImGui::ColorEdit4("##FrameBg", &ColorSettings::frameBg.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ColorSettings::frameBg;
            }
            ImGui::Text("Frame Background Hovered");
            if (ImGui::ColorEdit4("##FrameBgHovered", &ColorSettings::frameBgHovered.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ColorSettings::frameBgHovered;
            }
            ImGui::Text("Frame Background Active");
            if (ImGui::ColorEdit4("##FrameBgActive", &ColorSettings::frameBgActive.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ColorSettings::frameBgActive;
            }
            ImGui::Text("Text");
            if (ImGui::ColorEdit4("##Text", &ColorSettings::text.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_Text] = ColorSettings::text;
            }
            ImGui::Text("Scrollbar Background");
            if (ImGui::ColorEdit4("##ScrollbarBg", &ColorSettings::scrollbarBg.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ColorSettings::scrollbarBg;
            }
            ImGui::Text("Scrollbar Grab");
            if (ImGui::ColorEdit4("##ScrollbarGrab", &ColorSettings::scrollbarGrab.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab] = ColorSettings::scrollbarGrab;
            }
            ImGui::Text("Scrollbar Grab Hovered");
            if (ImGui::ColorEdit4("##ScrollbarGrabHovered", &ColorSettings::scrollbarGrabHovered.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered] = ColorSettings::scrollbarGrabHovered;
            }
            ImGui::Text("Scrollbar Grab Active");
            if (ImGui::ColorEdit4("##ScrollbarGrabActive", &ColorSettings::scrollbarGrabActive.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive] = ColorSettings::scrollbarGrabActive;
            }
            ImGui::Text("Separator");
            if (ImGui::ColorEdit4("##Separator", &ColorSettings::separator.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar)) {
                ImGui::GetStyle().Colors[ImGuiCol_Separator] = ColorSettings::separator;
            }

            ImGui::EndChild();
        }

        ImGui::EndChild();
        ImGui::End();
    }
}

void Update_Menu()
{
    if (Menu_Aktif)
    {
        ImGui::GetIO().MouseDrawCursor = true;
        (*(IDirect3DDevice9**)0xC97C28)->ShowCursor(true);
    }
    else
    {
        (*(IDirect3DDevice9**)0xC97C28)->ShowCursor(false);
        ImGui::GetIO().MouseDrawCursor = false;
        
    }
    ((void(__cdecl*)())(0x541BD0))();
    ((void(__cdecl*)())(0x541DD0))();
}

void on_present(IDirect3DDevice9* device_ptr, const RECT*, const RECT*, HWND wnd, const RGNDATA*)
{
    static bool ImGui_inited = false;
    if (!ImGui_inited)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplWin32_Init(**reinterpret_cast<HWND**>(0xC17054));
        ImGui_ImplDX9_Init(device_ptr);
        ImGui_inited = true;
    }
    if (ImGui_inited)
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (Menu_Aktif)
        {
            ImGui::GetIO().MouseDrawCursor = true;
            Render_Menu();
        }
        else
        {
            ImGui::GetIO().MouseDrawCursor = false;
        }
        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }
}