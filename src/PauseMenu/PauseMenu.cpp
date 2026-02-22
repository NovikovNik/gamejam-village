#include "PauseMenu.h"

#include <Renderer/Renderer.h>
#include <AudioSystem/AudioSystem.h>
#include <Game/Game.h>
#include <EventBus/EventBus.h>
#include <Events/GameShutdownEvent.h>
#include <ProgressSystem/AppDataSaveHelper.h>
#include <FileSystem/FileSystem.h>
#include <SDL3/SDL.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <filesystem>

// ---------------------------------------------------------------------------
// Layout  (logical pixels matching Game::windowWidth / windowHeight)
// ---------------------------------------------------------------------------
namespace {

constexpr float BOX_W       = 340.f;
constexpr float BOX_H       = 420.f;
constexpr float ROW_H       = 44.f;
constexpr float ROWS_TOP    = 56.f;   // y-offset of first row inside box
constexpr float SLIDER_W    = 160.f;
constexpr float SLIDER_H    = 10.f;
constexpr float LABEL_X     = 20.f;
constexpr float VALUE_X     = 145.f;  // slider / value start x (relative to box)
constexpr float VOLUME_STEP = 0.05f;  // 5 % per key press

constexpr float FONT_SIZE_TITLE = 22.f;
constexpr float FONT_SIZE_ITEM  = 16.f;

// Palette
constexpr SDL_Color COL_OVERLAY    {  0,  0,  0, 200 };
constexpr SDL_Color COL_BOX        { 18, 18, 26, 230 };
constexpr SDL_Color COL_BORDER     { 80, 80,120, 255 };
constexpr SDL_Color COL_SELECTED   { 55, 55,100, 220 };
constexpr SDL_Color COL_HOVERED    { 40, 40, 75, 160 };
constexpr SDL_Color COL_TRACK      { 50, 50, 60, 255 };
constexpr SDL_Color COL_FILL       { 90,190,120, 255 };
constexpr SDL_Color COL_FILL_MUTED { 70, 70, 70, 255 };
constexpr SDL_Color COL_DRAG       {130,220,150, 255 };  // brighter fill while dragging
constexpr SDL_Color COL_WHITE      {255,255,255, 255 };
constexpr SDL_Color COL_DIM        {170,170,170, 255 };
constexpr SDL_Color COL_TITLE      {220,220,255, 255 };
constexpr SDL_Color COL_MUTE_ON    {220, 80, 80, 255 };
constexpr SDL_Color COL_MUTE_OFF   { 90,200, 90, 255 };
constexpr SDL_Color COL_EXIT       {220, 80, 80, 255 };
constexpr SDL_Color COL_RESTART    {220,140, 40, 255 };

enum Row { ROW_MUTE = 0, ROW_MASTER, ROW_MUSIC, ROW_SFX, ROW_EXIT, ROW_RESTART, ROW_COUNT };

// ---- state ----------------------------------------------------------------
bool  g_isOpen    = false;
int   g_selection = -1;  // keyboard-selected row (-1 = mouse-only mode)
int   g_hoverRow  = -1;  // row under mouse cursor
int   g_dragRow   = -1;  // -1 = not dragging

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

inline float BoxX() { return (Game::windowWidth  - BOX_W) * 0.5f; }
inline float BoxY() { return (Game::windowHeight - BOX_H) * 0.5f; }

// Returns row index [0, ROW_COUNT) or -1 if outside any row.
int HitTestRow(float mx, float my) {
    const float bx = BoxX(), by = BoxY();
    if (mx < bx || mx > bx + BOX_W) return -1;

    // Volume rows
    const float volTop = by + ROWS_TOP;
    const float volBot = volTop + (ROW_SFX + 1) * ROW_H;
    if (my >= volTop && my < volBot)
        return static_cast<int>((my - volTop) / ROW_H);

    // Action button rows (Exit, then gap, then Restart)
    constexpr float RESTART_GAP = 48.f;
    const float btnTop  = volBot + 9.f;
    const float exitTop = btnTop;
    const float exitBot = exitTop + ROW_H;
    const float rstTop  = exitBot + RESTART_GAP;
    const float rstBot  = rstTop  + ROW_H;
    if (my >= exitTop && my < exitBot) return ROW_EXIT;
    if (my >= rstTop  && my < rstBot)  return ROW_RESTART;

    return -1;
}

// Whether (mx, my) falls within the slider track + generous vertical hit area.
bool HitTestSlider(float mx, float my, int row) {
    if (row <= ROW_MUTE || row >= ROW_COUNT) return false;
    const float bx  = BoxX(), by = BoxY();
    const float sx  = bx + VALUE_X;
    const float sy  = by + ROWS_TOP + row * ROW_H + ROW_H * 0.5f - SLIDER_H * 0.5f;
    return mx >= sx && mx <= sx + SLIDER_W &&
           my >= sy - 10.f && my <= sy + SLIDER_H + 10.f;
}

// Compute a [0,1] volume value from a raw mouse x position over the slider.
float SliderValueFromX(float mx) {
    const float sx = BoxX() + VALUE_X;
    return std::clamp((mx - sx) / SLIDER_W, 0.f, 1.f);
}

void ActivateRow(int row) {
    if (row == ROW_EXIT) {
        EventBus::instance().EmitEvent<GameShutdownEvent>();
    } else if (row == ROW_RESTART) {
        // Delete save file
        const auto savePath = AppDataSaveHelper::GetGameSavePath();
        std::error_code ec;
        std::filesystem::remove(savePath, ec);
        // Delete village folder next to the executable
        const auto villageDir = FileSystemManager::GetExecutableDir() / "village";
        std::filesystem::remove_all(villageDir, ec);
        EventBus::instance().EmitEvent<GameShutdownEvent>();
    }
}

void SetRowVolume(int row, float v) {
    switch (row) {
        case ROW_MASTER: AudioSystem::SetMasterVolume(v); break;
        case ROW_MUSIC:  AudioSystem::SetMusicVolume(v);  break;
        case ROW_SFX:    AudioSystem::SetSfxVolume(v);    break;
        default: break;
    }
}

float GetRowVolume(int row) {
    switch (row) {
        case ROW_MASTER: return AudioSystem::GetMasterVolume();
        case ROW_MUSIC:  return AudioSystem::GetMusicVolume();
        case ROW_SFX:    return AudioSystem::GetSfxVolume();
        default: return 0.f;
    }
}

// ---------------------------------------------------------------------------
// Rendering helpers
// ---------------------------------------------------------------------------

void DrawSlider(float x, float y, float volume, bool dragging) {
    Renderer::DrawFilledRectScreen(x, y, SLIDER_W, SLIDER_H, COL_TRACK);
    const float fillW = std::clamp(volume, 0.f, 1.f) * SLIDER_W;
    SDL_Color fill = dragging ? COL_DRAG : (AudioSystem::IsMuted() ? COL_FILL_MUTED : COL_FILL);
    if (fillW > 0.f) {
        Renderer::DrawFilledRectScreen(x, y, fillW, SLIDER_H, fill);
    }
    // Thumb knob
    const float knobX = x + fillW - 4.f;
    const float knobY = y - 3.f;
    Renderer::DrawFilledRectScreen(knobX, knobY, 8.f, SLIDER_H + 6.f,
                                   dragging ? COL_WHITE : fill);
}

std::string VolPct(float v) {
    return std::to_string(static_cast<int>(std::round(v * 100.f))) + "%";
}

void RenderRow(int row, float rowY) {
    const float bx       = BoxX();
    const bool  isKeysel = (g_selection == row);
    const bool  isHover  = (g_hoverRow == row);
    const bool  isDrag   = (g_dragRow  == row);

    // Background highlight
    if (isKeysel || isDrag) {
        Renderer::DrawFilledRectScreen(bx + 8.f, rowY + 2.f, BOX_W - 16.f, ROW_H - 4.f, COL_SELECTED);
    } else if (isHover) {
        Renderer::DrawFilledRectScreen(bx + 8.f, rowY + 2.f, BOX_W - 16.f, ROW_H - 4.f, COL_HOVERED);
    }

    const float lx      = bx + LABEL_X;
    const float ty      = rowY + 14.f;
    const int   fs      = static_cast<int>(FONT_SIZE_ITEM);
    const auto  fid     = Renderer::TextId(make_nnTex("charriot"));
    const bool  active  = isKeysel || isHover || isDrag;
    const SDL_Color& textCol = active ? COL_WHITE : COL_DIM;

    switch (row) {
        case ROW_MUTE: {
            const bool isMuted = AudioSystem::IsMuted();
            Renderer::DrawTextScreen(fid, "Mute", lx, ty, fs, &textCol);
            Renderer::DrawTextScreen(fid,
                                     isMuted ? "ON" : "OFF",
                                     bx + VALUE_X, ty, fs,
                                     isMuted ? &COL_MUTE_ON : &COL_MUTE_OFF);
            break;
        }
        case ROW_EXIT: {
            const SDL_Color& col = (isKeysel || isHover) ? COL_WHITE : COL_EXIT;
            Renderer::DrawTextScreen(fid, "Exit", lx, ty, fs, &col);
            break;
        }
        case ROW_RESTART: {
            const SDL_Color& col = (isKeysel || isHover) ? COL_WHITE : COL_RESTART;
            Renderer::DrawTextScreen(fid, "Restart  (clears save)", lx, ty, fs, &col);
            break;
        }
        default: {
            static const char* labels[] = { "", "Master", "Music", "SFX" };
            const float v    = GetRowVolume(row);
            const float slY  = rowY + ROW_H * 0.5f - SLIDER_H * 0.5f;
            Renderer::DrawTextScreen(fid, labels[row], lx, ty, fs, &textCol);
            DrawSlider(bx + VALUE_X, slY, v, isDrag);
            Renderer::DrawTextScreen(fid, VolPct(v),
                                     bx + VALUE_X + SLIDER_W + 10.f, ty, fs, &textCol);
            break;
        }
    }
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

namespace PauseMenu {

void Open()  { g_isOpen = true; g_selection = -1; g_hoverRow = -1; g_dragRow = -1; }
void Close() { g_isOpen = false; g_dragRow = -1; }
void Toggle(){ g_isOpen ? Close() : Open(); }
bool IsOpen(){ return g_isOpen; }

void UpdateAndRender() {
    if (!g_isOpen) return;

    const float sw = static_cast<float>(Game::windowWidth);
    const float sh = static_cast<float>(Game::windowHeight);
    const float bx = BoxX();
    const float by = BoxY();

    // Full-screen dim overlay
    Renderer::DrawFilledRectScreen(0.f, 0.f, sw, sh, COL_OVERLAY);

    // Box background
//    Renderer::DrawFilledRectScreen(bx, by, BOX_W, BOX_H, COL_BOX);
//
//    // Border (2 px edges)
//    Renderer::DrawFilledRectScreen(bx,               by,               BOX_W, 2.f,   COL_BORDER);
//    Renderer::DrawFilledRectScreen(bx,               by + BOX_H - 2.f, BOX_W, 2.f,   COL_BORDER);
//    Renderer::DrawFilledRectScreen(bx,               by,               2.f,   BOX_H, COL_BORDER);
//    Renderer::DrawFilledRectScreen(bx + BOX_W - 2.f, by,               2.f,   BOX_H, COL_BORDER);

    // Title
    const auto fid = Renderer::TextId(make_nnTex("charriot"));
    Renderer::DrawTextScreen(fid, "SETTINGS",
                             bx + 108.f, by + 14.f,
                             static_cast<int>(FONT_SIZE_TITLE), &COL_TITLE);

    // Divider
    Renderer::DrawFilledRectScreen(bx + 8.f, by + 52.f, BOX_W - 16.f, 1.f, COL_BORDER);

    // Volume rows
    for (int i = 0; i < ROW_SFX + 1; ++i) {
        RenderRow(i, by + ROWS_TOP + i * ROW_H);
    }

    // Divider before action buttons
    const float divY = by + ROWS_TOP + (ROW_SFX + 1) * ROW_H + 4.f;
    Renderer::DrawFilledRectScreen(bx + 8.f, divY, BOX_W - 16.f, 1.f, COL_BORDER);

    // Action button rows (Restart has extra gap to separate it visually)
    constexpr float RESTART_GAP = 48.f;
    RenderRow(ROW_EXIT,    divY + 4.f);
    RenderRow(ROW_RESTART, divY + 4.f + ROW_H + RESTART_GAP);
}

bool HandleKeyDown(SDL_Keycode key) {
    if (!g_isOpen) return false;

    // Any key press switches to keyboard navigation mode
    if (key != SDLK_ESCAPE) g_hoverRow = -1;

    // Give keyboard a selection to work with if none is active
    const int ksel = (g_selection < 0) ? 0 : g_selection;

    switch (key) {
        case SDLK_ESCAPE:
            Close();
            return true;

        case SDLK_UP:
            g_selection = (ksel - 1 + ROW_COUNT) % ROW_COUNT;
            return true;

        case SDLK_DOWN:
            g_selection = (ksel + 1) % ROW_COUNT;
            return true;

        case SDLK_RETURN:
        case SDLK_RETURN2:
        case SDLK_SPACE:
            if (ksel == ROW_MUTE) AudioSystem::SetMuted(!AudioSystem::IsMuted());
            else if (ksel == ROW_EXIT || ksel == ROW_RESTART) ActivateRow(ksel);
            g_selection = ksel;
            return true;

        case SDLK_LEFT:
            g_selection = ksel;
            switch (ksel) {
                case ROW_MUTE:   AudioSystem::SetMuted(true); break;
                default: SetRowVolume(ksel, GetRowVolume(ksel) - VOLUME_STEP); break;
            }
            return true;

        case SDLK_RIGHT:
            g_selection = ksel;
            switch (ksel) {
                case ROW_MUTE:   AudioSystem::SetMuted(false); break;
                default: SetRowVolume(ksel, GetRowVolume(ksel) + VOLUME_STEP); break;
            }
            return true;

        default:
            return true; // eat all keys while menu is open
    }
}

bool HandleEvent(SDL_Event* event) {
    if (!g_isOpen || !event) return false;

    switch (event->type) {

        case SDL_EVENT_MOUSE_MOTION: {
            const float mx = event->motion.x;
            const float my = event->motion.y;

            // Update hover highlight
            g_hoverRow  = HitTestRow(mx, my);
            g_selection = -1; // mouse takes over from keyboard selection

            // Drag in progress: update volume continuously
            if (g_dragRow > ROW_MUTE) {
                SetRowVolume(g_dragRow, SliderValueFromX(mx));
            }
            return true;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event->button.button != SDL_BUTTON_LEFT) return true;
            const float mx = event->button.x;
            const float my = event->button.y;

            // Click outside box: close menu
            const float bx = BoxX(), by = BoxY();
            if (mx < bx || mx > bx + BOX_W || my < by || my > by + BOX_H) {
                Close();
                return true;
            }

            const int row = HitTestRow(mx, my);
            if (row < 0) return true;

            if (row == ROW_MUTE) {
                AudioSystem::SetMuted(!AudioSystem::IsMuted());
            } else if (row == ROW_EXIT || row == ROW_RESTART) {
                ActivateRow(row);
            } else {
                // Click anywhere on a volume row snaps the slider + starts drag
                SetRowVolume(row, SliderValueFromX(mx));
                g_dragRow = row;
            }
            return true;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if (event->button.button != SDL_BUTTON_LEFT) return true;
            if (g_dragRow > ROW_MUTE) {
                // Final position on release
                SetRowVolume(g_dragRow, SliderValueFromX(event->button.x));
            }
            g_dragRow = -1;
            return true;
        }

        default:
            return false;
    }
}

} // namespace PauseMenu
