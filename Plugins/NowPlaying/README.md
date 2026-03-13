# NowPlaying Plugin for TrafficMonitor

This plugin displays currently playing media information from Windows using the Windows Media Control API.

## Features

- Displays the currently playing song/media title and artist
- Automatically detects media from any compatible Windows application (Spotify, Windows Media Player, browser, etc.)
- Shows tooltip with detailed information including album name
- Configurable display options

## Requirements

- Windows 10 version 1903 (build 18362) or later
- Visual Studio 2022 with:
  - C++ Desktop Development workload
  - Windows 10/11 SDK
  - C++/WinRT support
  - MFC support

## Building the Plugin

1. Open `TrafficMonitorPlugins.sln` in Visual Studio 2022
2. Select the NowPlaying project in Solution Explorer
3. Choose your target platform (Win32, x64, or ARM64EC)
4. Build the project (Build > Build NowPlaying)
5. The compiled DLL will be in `bin\[Platform]\[Configuration]\NowPlaying.dll`

## Installation

1. Copy the `NowPlaying.dll` file to the `plugins` folder in your TrafficMonitor installation directory
2. Restart TrafficMonitor
3. Go to Options > General Settings > Plugin Management to verify the plugin is loaded
4. Right-click on the taskbar widget and select "Display Settings"
5. Check "Now Playing" to display it in the taskbar

## Configuration

The plugin saves its configuration to `NowPlaying.ini` in the TrafficMonitor config directory.

Available settings:
- `show_artist`: Display artist name (default: true)
- `show_title`: Display song title (default: true)
- `max_length`: Maximum display length before truncation (default: 50)

## How It Works

The plugin uses the Windows Runtime (WinRT) GlobalSystemMediaTransportControls API to access the currently playing media information. This is the same API that Windows uses for the media controls in the notification center and on keyboards with media keys.

The plugin will display:
- Artist - Title (when both are available)
- "No media playing" when nothing is playing
- Truncated text with "..." when the combined text exceeds the max length

## Supported Media Players

Any application that integrates with Windows Media Controls will work, including:
- Spotify
- Windows Media Player
- VLC Player
- Chrome/Edge (for web-based media)
- Groove Music
- iTunes
- And many more

## Troubleshooting

If the plugin shows "No media playing" when media is actually playing:
1. Make sure you're running Windows 10 1903 or later
2. Ensure the media player supports Windows Media Controls
3. Try pausing and playing the media again
4. Check that the media player has permission to show in the media control overlay

## Technical Details

- Uses C++/WinRT for accessing Windows Runtime APIs
- Implements the ITMPlugin interface for TrafficMonitor integration
- Requires C++20 standard and /await:strict compiler option
- Links against windowsapp.lib for WinRT functionality

## License

Copyright (C) 2026

## Version

1.00
