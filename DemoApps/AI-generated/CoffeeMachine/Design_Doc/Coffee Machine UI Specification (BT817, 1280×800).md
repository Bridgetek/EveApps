# Coffee Machine UI Specification

Bridgetek BT817 (EVE4) · 1280×800 landscape LCD · English / 中文 · Version 1.0 (design draft)

This spec describes the screens in the canvas design: Home, Customize, Brewing and Language, each in English and Chinese. Values are in pixels unless noted.

## 1. Scope and principles

- The UI only requests actions and renders status. All brew control and safety logic stays on the separate brew controller.
- One language is shown at a time. The user picks it on the Language screen.
- Every touch target is at least 76 px high, with at least 24 px between targets.
- Layout is identical in both languages, so switching language changes only text.
- Dark theme with one amber accent, readable in bright kitchens and cafés.

## 2. Display and layout grid

| Item | Value |
| --- | --- |
| Resolution / orientation | 1280×800, landscape |
| Outer margin | 24 |
| Gap between tiles and panels | 24 |
| Status bar | 1280×72 (top) |
| Content area | 1280×608 (12 px vertical padding inside) |
| Action bar | 1280×120 (bottom) |

The three zones appear on every screen except Language, which has no status pills.

## 3. Visual style

### 3.1 Color tokens

| Token | Hex | Use |
| --- | --- | --- |
| bg | #17110D | Screen background |
| panel | #2A1F18 | Pills, panels, tile base |
| tile-glow | #4D3828 → #241912 | Radial gradient behind drink art |
| control | #3A2B21 | Buttons, unselected segments, slider track |
| text | #F3E6D3 | Primary text |
| text-muted | #B9A58F | Labels, secondary text |
| accent | #E0902F | Selected state, progress, Start button |
| on-accent | #1A0F08 | Text on accent and on Stop |
| stop | #E5675A | Stop button |

### 3.2 Typography

| Role | Size | Weight |
| --- | --- | --- |
| Brew time | 84 | 600 |
| Drink name (Customize, Brewing) | 44 | 600 |
| Start / Stop button label | 36 | 500 |
| Tile name, button label, status text | 30–32 | 500 |
| Section label, secondary line | 28 | 400 |
| Tile sub-line, pill text | 24 | 400 |

Fonts: Outfit for Latin, Noto Sans SC for Chinese. On the BT817 both are converted with the EVE Asset Builder into a few fixed sizes (24, 28, 32, 44, 84) stored in flash. Chinese uses a subset font containing only the characters in the string table.

### 3.3 Drink illustrations

Eight shaded illustrations (cup, glass, foam, steam, shadow). They are placeholders: replace them with studio photos (about 600×450, dark background) when available. Store them as ASTC bitmaps in external flash.

## 4. Components

| Component | Size | Notes |
| --- | --- | --- |
| Drink tile | 290×280, radius 28 | Art on top, name 32, sub-line 24 (volume). Pressed: shift 3 px and lighten. |
| Action button | height 88, radius 20, side padding 40 | Label 30. Primary variant: accent fill, on-accent text. |
| Status pill | height 48, radius 36, side padding 22 | Text 24, muted on panel. |
| Segmented selector | segment height 76, radius 16, gap 12 | Selected segment: accent fill, on-accent text, weight 600. |
| Slider | track 12 high, knob 48 diameter | Value label above, right aligned, 28 bold. Filled part in accent. |
| Progress ring | 340 diameter, radius 150, stroke 20 | Track control color, progress accent, round cap, starts at 12 o'clock. |
| Language card | 440×320, radius 28 | Selected: 4 px accent border and check badge (44) in the top right. |

States: normal, pressed, disabled (40% opacity, no touch tag). Press feedback appears while the touch tag is held and the action fires on release inside the same tag.

## 5. Screens

### 5.1 Home

- **Status bar:** time, machine state (Ready / Busy / Error), pills for boiler temperature, water %, beans % and grounds %, and the language pill (EN or 中文, accent filled).
- **Content:** 4×2 grid of drink tiles: Espresso 40 ml, Lungo 110 ml, Americano 180 ml, Cappuccino 180 ml, Latte 240 ml, Flat White 160 ml, Hot Water 200 ml, Milk Foam 120 ml.
- **Action bar:** Cleaning, Hot water, hint text centered ("Tap a drink to start"), Standby.
- **Actions:** tile → Customize with that drink. Language pill → Language. Cleaning → cleaning flow. Standby → dimmed standby screen.

### 5.2 Customize

- **Left panel (420 wide):** large drink illustration, drink name 44, summary line (volume · strength · temperature).
- **Right panel:** Strength (1–5 segments, default 3), Size (slider), Milk ratio (slider, drinks with milk only), Temperature (Warm / Hot / Extra hot).
- **Action bar:** Back (left), Start (right, 420 wide, accent).
- **Rules:** controls that do not apply to a drink are hidden, not disabled. Ranges per drink come from the brew controller.

### 5.3 Brewing

- **Center:** progress ring, elapsed time 84 and total time below it, drink name, and step line: Grinding → Brewing → Milk. Completed steps get a check, the active step uses accent text.
- **Action bar:** Stop (420 wide, stop color), always visible and always enabled.
- **Finish:** at 100%, show a Done message for 3 s, then return to Home. Stop returns to Home after the controller confirms.

### 5.4 Language

- Two cards: English and 中文 (简体中文). Selecting one applies immediately, saves the choice and enables Back.
- Title shows "Language · 语言" and Back shows "Back · 返回", so the screen reads correctly in both languages.
- Entry point: language pill on any status bar. A Settings item can open the same screen later.

## 6. Navigation flow

| From | Event | To |
| --- | --- | --- |
| Standby | Touch | Home |
| Home | Tap drink tile | Customize |
| Home | Tap language pill | Language |
| Language | Select / Back | Previous screen |
| Customize | Back | Home |
| Customize | Start | Brewing |
| Brewing | Done / Stop | Home |
| Any | Fault from controller | Alert banner (see 8) |
| Home | 60 s idle | Standby |

## 7. Language and strings

- Two string tables with the same keys (strings\_en, strings\_zh). Never hard-code text in drawing code.
- Chinese drink names: 意式浓缩, 长萃咖啡, 美式咖啡, 卡布奇诺, 拿铁, 澳白, 热水, 奶泡. Chinese labels: 浓度, 容量, 奶量, 温度, 温, 热, 烫, 返回, 开始, 停止, 研磨, 萃取, 打奶, 清洁, 待机.
- Store the language code in non-volatile memory and load the Chinese font only when selected.
- Text must fit on one line with 16 px spare in both languages.

## 8. Alerts (to be designed)

Full-width banner at the top of the content area with one clear action.

| Condition | Message (EN) | Action |
| --- | --- | --- |
| Water tank low | Refill water tank | Dismiss when refilled |
| Grounds bin full | Empty grounds bin | Confirm emptied |
| Beans low | Refill beans | Dismiss |
| Descale due | Descaling required | Start descale / Later |
| Fault | Service needed (code) | Show code, block brewing |

## 9. BT817 implementation notes

- Panel: confirm RGB vs LVDS and set REG\_HSIZE/VSIZE and timing from the panel datasheet.
- Coordinates above 511 need VERTEX\_FORMAT or VERTEX\_TRANSLATE; use CMD\_SETBITMAP with flash addresses for assets.
- Keep each display list under 8 KB. Record static elements (backgrounds, headers) once with CMD\_NEWLIST / CMD\_ENDLIST and replay them with CMD\_CALLLIST.
- Use CMD\_GRADIENT or radial art bitmaps for tile backgrounds; CMD\_GAUGE or arcs for the ring; CMD\_SLIDER for sliders.
- Each tile, button and selector has a unique TAG. Read REG\_TOUCH\_TAG at 60 Hz and debounce 30 ms.
- Render at 30–60 fps only on Brewing; on other screens redraw on events.

## 10. UI ↔ brew controller interface

| Direction | Message | Fields |
| --- | --- | --- |
| UI → controller | BREW | drink id, volume, strength, temperature, milk ratio |
| UI → controller | STOP | none |
| UI → controller | CLEAN / STANDBY | none |
| Controller → UI | STATUS | state, step, progress %, boiler °C, water/beans/grounds % |
| Controller → UI | ALERT | code, severity |

Transport: UART or CAN with checksum and a 500 ms heartbeat. If the heartbeat is lost, the UI shows a fault banner and disables Start.

## 11. Acceptance checks

- All eight drinks start with one tap plus Start.
- Stop works within 200 ms of touch on every Brewing frame.
- Language switch applies without restart and survives power-off.
- Chinese and English text fit in every control.
- Touch works with gloves and wet hands (panel setting) and tiles show press feedback within one frame.
