# Nasdara Morse Decoder

**Nasdara Morse Decoder** is a DayZ mod that intercepts in-game Morse code transmissions to obtain coordinates and access passwords for active bunkers on the **Nasdara** map. Designed to encourage dynamic PvP and exploration around bunker events, it supports both pure radio/notification gameplay and optional **DayZ Expansion Navigation** map markers.

**Developed by:** Whonixxx for Nasdara map fans  
🔗 **Official Steam Workshop:** (https://steamcommunity.com/sharedfiles/filedetails/?id=3844581645)

---

## ✨ Features & Supported Languages
- **Immersive Sequential Decoding:** Waits a few seconds after each bunker's Morse transmission finishes before displaying its coordinates and password.
- **Optional Map Integration:** Supports **DayZ Expansion Navigation** 2D/3D markers with customizable radius, color, and icons.
- **Automatic Client Localization:** Automatically translates notifications based on the player's game language (**English**, **Portuguese (Brazil)**, and **Spanish**).

---

## 🎮 How to Use
1. Equip a **Handheld Transceiver** (with a working battery) in your character's hands
2. Turn the radio **ON** and tune it to the **80 MHz** frequency
3. Keep the radio active in your hands to intercept and sequentially decode the active bunker coordinates and passwords after each transmission ends

---

## 🛠️ Server Installation
1. Subscribe and download the mod via **Steam Workshop**
2. Copy the `@NasdaraMorseDecoder` folder to your server's root directory
3. Copy the `whonixxx.bikey` file from the `keys` folder into your server's `keys` folder
4. Add `@NasdaraMorseDecoder` to your server launch parameters: `-mod=@NasdaraMorseDecoder`

---

## ⚙️ Configuration Options
The mod automatically generates a configuration file at:  
`profiles\NasdaraMorseDecoder\config.json`

| Option | Description | Default |
|--------|-------------|---------|
| `EnableExpansionMarkers` | `1` = Enables markers on DayZ Expansion Navigation (if installed), `0` = Disables markers (radio/notification only) | `1` |
| `ExpansionShow3D` | `1` = Shows 3D icon projected on screen, `0` = Shows only on 2D map and GPS (recommended for exploration) | `0` |
| `MarkerIcon` | Expansion Navigation marker icon (`"Exclamationmark"`, `"Questionmark"`, `"Compass"`, `"Skull"`) | `"Exclamationmark"` |
| `MarkerColorR` | RGB marker color on map - Red component (`0-255`) | `255` |
| `MarkerColorG` | RGB marker color on map - Green component (`0-255`) | `165` |
| `MarkerColorB` | RGB marker color on map - Blue component (`0-255`) | `0` |
| `ApproxRadiusMeters` | Imprecision radius in meters around the bunker (`0` = exact position) | `150.0` |

---

## 📻 Gameplay Mechanics
- Morse code transmissions are broadcast periodically by the server
- Uses **sequential broadcasting** with a delay after each bunker's signal finishes for greater immersion and PvP tension
- Coordinates are displayed in the format: `XXX/YYY/ZZZ / AAA/BBB/CCC`
- Each bunker has a unique lock code required for access
- Players must hold a working Handheld Transceiver in their hands to receive decoded messages

---

## 🤝 Contributing (Help Wanted)
Pull Requests are very welcome! This mod was created to keep DayZ utilities 100% free for the community. If you'd like to help improve it, here are some great areas to contribute:
- [ ] Add marker support for **LBmaster Advanced Groups**
- [ ] Add marker support for **Vanilla++ Map**
- [ ] Make bunker coordinates and frequencies configurable via JSON to support other maps (Namalsk, Deer Isle, etc.)
- [ ] Compatibility updates for future DayZ patches

**Contribution Terms:** By submitting a Pull Request to this repository, you agree that your contribution is voluntary and free of charge, granting the maintainer the right to include, modify, and distribute it within the official mod without financial compensation. All contributors will be credited here and on the Steam Workshop page!

---

## 📜 License & Permissions
**All Rights Reserved.** See the [LICENSE](LICENSE) file for full details.
- Forking on GitHub is permitted solely for personal study or submitting Pull Requests to this repository
- **NO REPACKS ALLOWED:** You are NOT allowed to repack, re-upload, or redistribute this mod (or modified versions) on the Steam Workshop or inside server packs
- Free to use on any DayZ server (including Bohemia-approved monetized servers) when loaded directly from the official Steam Workshop page

> **Note on Dependencies:** **DayZ Expansion Navigation** is **NOT** bundled or repacked inside this mod. Map markers are an optional integration — if you want to use them, you must subscribe to and load **DayZ Expansion Navigation** separately from the official Steam Workshop. Without it, the mod works standalone via radio and text notifications.
