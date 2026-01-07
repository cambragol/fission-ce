<p align="center">
  <img src="files/fallout-fission-logo.jpg" alt="Fallout: F.I.S.S.I.O.N. Logo" width="1024">
</p>

# Fallout: F.I.S.S.I.O.N.
*Flexible Isometric Simulation System for Interactive Open‑world Nuclear‑roleplaying*

Fallout: F.I.S.S.I.O.N. is a next‑generation, cross‑platform reimplementation of Fallout 1 & 2 (Fallout2-CE) that preserves the original isometric, turn‑based gameplay while adding modding enhancements, widescreen support, and true community‑driven extensibility. Run it on Windows, Linux, macOS, Android, iOS—and even in browsers.

> ⚛️ **Powered by the F.I.S.S.I.O.N. Engine**
> *Flexible. Isometric. Simulation. System. Interactive. Open‑world. Nuclear‑roleplay.*
> **Split from the past, Powering the future**

---

## Key Features

- 🔲 **Authentic isometric, turn‑based experience** (SPECIAL, original Fallout gameplay)
- 💻 **True cross‑platform support**: Windows, macOS, Linux, iOS, Android, Web
- 🖥️ **Widescreen & high‑res scaling** with pixel‑perfect aspect preservation
- 🧩 **Modular, customizable systems**—community mods plug in seamlessly (working now)
- 📦 **100% compatible** with original Fallout 1 & 2 assets  (can't run Fallout 1 yet... one day)
- 🌎 **Future‑proof**: easily extended for new content and Fallout 2 integration (working now)

---

## 🔠 F.I.S.S.I.O.N. Breakdown

```
╔═══════════════════════╦════════════════════════════════════════════╗
║      ATTRIBUTE        ║               DESCRIPTION                  ║
╠═══════════════════════╬════════════════════════════════════════════╣
║ F – Flexible          ║ Adaptable, moddable, and future‑ready      ║
║ I – Isometric         ║ Faithful to classic 2D grid perspective    ║
║ S – Simulation        ║ Manages AI, world rules, stats, turn timing║
║ S – System            ║ Unified architecture for engine/runtime    ║
║ I – Interactive       ║ Dynamic player choices and feedback        ║
║ O – Open‑world        ║ Seamless large‑map exploration             ║
║ N – Nuclear‑roleplay  ║ Immersive post‑nuclear RPG experience      ║
╚═══════════════════════╩════════════════════════════════════════════╝
```

---

## ⚠️ Mod/Game Compatibility

**Fully supported**:
- Fallout 2
- Fallout: Nevada
- Fallout: Sonora

**Not supported yet, maybe never**:
- Fallout 1
- Fallout Nevada or Sonora 'repacks'
- Restoration Project
- Fallout: Et Tu
- Olympus 2207
- Resurrection, Yesterday (untested)

(For full Fallout 1 support see [Fallout1-CE](https://github.com/alexbatalov/fallout1-ce).)

---

## 💾 Installation

### Prerequisites
You must own **Fallout 2** (GOG, Steam, or Epic Games version) and have it fully installed. F.I.S.S.I.O.N. is a drop-in replacement for `Fallout2.exe` and requires the complete game data.

**Supported base installations:**
- **Vanilla Fallout 2** - The classic game
- **Fallout: Nevada** - Russian total conversion mod
- **Fallout: Sonora** - Russian total conversion mod

### Quick Installation
1. **Ensure you have a working vanilla Fallout 2 installation**
2. **Download** the latest [F.I.S.S.I.O.N. release](https://github.com/cambragol/fission-ce/releases)
3. **Extract** the F.I.S.S.I.O.N. files into your Fallout 2 folder
4. **Run** `fallout-fission.exe` (Windows) or `fallout-fission.app` (macOS) instead of the original executable

That's it! F.I.S.S.I.O.N. automatically loads all existing content and adds its enhanced modding capabilities.

### Platform-Specific Instructions

#### 🪟 Windows

```
# Example: Installing into a Steam Fallout 2 installation
# 1\. Navigate to your Fallout 2 folder (typically):
cd "C:\Program Files (x86)\Steam\steamapps\common\Fallout 2"
# 2\. Extract F.I.S.S.I.O.N. files here
# 3\. Run fallout-fission.exe
```

#### 🍎 macOS

```
# Example: Using a GOG Fallout 2 installation
# 1. Right-click Fallout2.app → "Show Package Contents"
# 2. Navigate to Contents/Resources/Data/
# 3. Extract F.I.S.S.I.O.N. files here
# 4. Run fallout-fission.app
```

#### 🐧 Linux

```
# Example: Using a Wine Fallout 2 installation
# 1. Navigate to your Fallout 2 Wine prefix
cd ~/.wine/drive_c/Program\ Files/Fallout\ 2/
# 2. Extract F.I.S.S.I.O.N. files
# 3. Run with: wine fallout-fission.exe
```

### 📁 Expected Folder Structure

After installation, your Fallout 2 folder should contain:

```
Fallout 2/
├── fallout-fission.exe     # F.I.S.S.I.O.N. executable (Windows)
├── fallout-fission.app     # F.I.S.S.I.O.N. application (macOS)
├── fission.dat            # Engine data file
├── master.dat            # Original game data
├── critter.dat           # Original game data
├── patch000.dat          # Patch data (if present)
├── data/                 # Game data folder
│   ├── proto/           # Proto files (vanilla + mods)
│   ├── text/            # Text files (vanilla + mods)
│   └── maps/            # Map files
└── data/lists/          # Auto-generated mod reports (created on first run)
```

### ⚠️ Important Notes

-   F.I.S.S.I.O.N. does NOT include game data - you must own Fallout 2

-   Your existing saves will work - F.I.S.S.I.O.N. maintains full compatibility

-   Mods stay where they are - F.I.S.S.I.O.N. reads from existing `data/` structure

-   No configuration needed for basic use - just replace the executable

---

## ⚙️ Configuration

Use the in-game 'preferences' screen for 'graphics' configuration.

Other configuration can be done fia the fallout2.cfg file.

For advanced tweaks, use the [sfall] sections in 'fallout2.cfg' (Sfall):

```
[sfall-misc]
WorldMapTravelMarkers=1
GaplessMusic=1
EnhancedBarter=1
```

---

## 🛠️ Contributing

Contributions are welcome! Please open issues or pull requests on GitHub.

---

## 📜 License

Released under the [Sustainable Use License](LICENSE.md).
