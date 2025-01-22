# Piksy

> 🚧 **Work in Progress**<br>
> Piksy is an evolving C++ application for effortless **sprite sheet editing**, **frame extraction**, and **animation handling**. It's currently **replacing** [this project](https://github.com/adia-dev/spritesheet_editor) but remains under heavy development!

<p align="center">
  <video width="400" autoplay loop muted playsinline src="https://github.com/user-attachments/assets/86629652-91dd-44be-9b04-788dc5fe345e" />
</p>


## 🚀 Key Features

- **Sprite Sheet Organization**  
  Load and preview your sprite sheets, neatly organize files, and quickly navigate through folders.

- **Frame Extraction**  
  Automatically detect frames with computer vision (or create them manually for full control).

- **Animation Editing**  
  Combine frames into animations, reorder them on-the-fly, and preview motion instantly.

- **Color Swapping**  
  Tap into a handy color tool to replace any target hue with a new color, saving manual editing time.

- **Export & Save**  
  Save all animations as JSON and export textures as PNG. Perfect for engine-ready assets!

> **Tip**<br>
> Piksy is great for game devs, hobbyists, and artists needing a streamlined workflow to handle sprite sheets.

---

## 🎉 Screenshots & Videos (so far, subject to changes)

![image](https://github.com/user-attachments/assets/173d540e-8946-4ec3-8d6b-f81dce3f0954)

<img width="1443" alt="image" src="https://github.com/user-attachments/assets/4df922f3-36c4-4b1c-9741-29dd2df35931" />


---

## 🔧 Getting Started

1. **Clone**
   ```bash
   git clone https://github.com/adia-dev/piksy.git
   cd piksy
   ```
2. **Build** (example using CMake):
   ```bash
   make
   # or
   make -j4
   ```
3. **Run**

   ```bash
   make run
   # or
   build/Debug/bin/piksy

   ```

> **Info**<br>
> You’ll need to have SDL2, SDL2_image, SDL2_ttf, and OpenCV installed for a successful build (if building from source). Pre-built binaries may come later.

---

## 🔨 Usage Overview

1. **Load or Drop a Sprite Sheet**
   - Use the "File" menu or simply drag-and-drop your sprite sheet into Piksy.
2. **Extract Frames**
   - Choose the "Extract" tool, draw a selection, and Piksy detects frames in that region.
3. **Create & Preview Animations**
   - Group these frames into sequences. Adjust timing and naming on-the-fly.
4. **Export**
   - Save the entire texture or export your animation data to JSON with a quick menu option.

> **Warning**<br>
> Some tools and features are incomplete—please report any quirks or bugs you encounter.

---

## 📂 Project Structure (Brief)

```
piksy/
├─ include/
│   ├─ core/         # App setup & configuration
│   ├─ command/      # Actions (Export, Save, etc.)
│   ├─ components/   # UI panels (e.g., Console, Viewport)
│   ├─ rendering/    # Rendering logic (SDL, textures)
│   ...
├─ src/
│   ├─ core/
│   ├─ command/
│   ├─ components/
│   ├─ rendering/
│   ...
└─ CMakeLists.txt
```

---

## 🌱 Current Status & Roadmap

- Basic workflow (load → extract → animate → export) is functional.
- UI/UX is still evolving; certain settings may not persist.
- More polishing around frame detection and color swapping is planned.

---

## 🙌 Contributing

We welcome pull requests and issue reports! To contribute:

1. [Fork the repo](https://github.com/adia-dev/piksy/fork).
2. Create a branch for your feature/bug fix.
3. Submit a PR for review.

> **Info**<br>
> We value all feedback—whether it’s UI suggestions, performance tweaks, or big new features.

---

> **Thank you** for checking out Piksy! We hope to make your sprite editing journey easier and faster. Stay tuned for updates!
