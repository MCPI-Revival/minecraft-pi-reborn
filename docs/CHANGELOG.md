# Changelog

**2.2.11**
* Add "Close Current Screen On Death" Feature Flag (Enabled By Default) To Prevent Bugs
* Fix More Furnace UI Bugs When Using "Disable 'gui_blocks' Atlas"

**2.2.10**
* Fix Bug With Picking Up Items In "Remove Creative Mode Restrictions" Mode

**2.2.9**
* Fix String Sanitization
* Store Files In `/usr/lib`

**2.2.8**
* Add "Hide Chat Messages" Optional Feature Flag
* Add "Remove Creative Mode Restrictions" Optional Feature Flag
* Improve GLFW->SDL Mouse Motion Event Conversion
* Performance Optimizations
* Make Majority Of Server-Specific Logging Code Also Apply To The Client
* Simple Benchmark Mode
* Fix Typo When Audio Source File Doesn't Exist
* Improve Build System

**2.2.7**
* Fix Crash When OpenAL Is Unavailable
* Fix Command Input In Server

**2.2.5**
* Fix Bug In Texture Scaling Code

**2.2.5**
* Scale Animated Textures
* Add More Blocks To Expanded Creative Inventory
* Reduce Unnecessary Logging
* Log IPs In Server Mode

**2.2.4**
* Instead Of Crashing, Disable Polling Block Hits In Survival Mode Using The API

**2.2.3**
* Fix Crash When Taking Odd-Sized Screenshots

**2.2.2**
* Add More Missing Sound Events
* Make Missing Sound Event Cause Warning Rather Than Crash

**2.2.1**
* Prevent ``random.burp`` Sound From Crashing Game
* Always Cleanup Media Layer, Even On Crash
* Resolve All Sounds On Startup

**2.2.0**
* Sound Support
* Split Off "Allow Joining Survival Servers" From Game-Mode Mod
* Separate Headless Code From Server Code
* Fix Bug Where ``RakNetInstance`` Starts Pinging Potential Servers Before The "Join Game" Screen Is Opened
* Clean-Up Code
* Remove Support For Debian Buster

**2.1.8**
* Fix Crash On ARM Systems

**2.1.7**
* Fix On 64-Bit ARM Systems

**2.1.6**
* Optimize Media Layer Proxy

**2.1.5**
* Print Error Message If RakNet Fails To Start

**2.1.4**
* Fix ``RakNet::RakString`` Security Bug

**2.1.3**
* Workaround Broken Library Search Path On Some ARM 32-Bit Systems

**2.1.2**
* Fix Library Loading On ARM 32-Bit Systems

**2.1.1**
* Fix Symlink Code

**2.1.0**
* Allow Binding ``Q`` Key To Item Dropping
* Expose More Feature Flags
* Replace ``Mob Spawning`` Feature Flag With ``Force Mob Spawning``
* Fix ``ESC`` Key In Options Menu When ``Miscellaneous Input Fixes`` Is Enabled

**2.0.9**
* Fix Translucent Preview Items In Furnace UI Being Fully Opaque When The ``gui_blocks`` Atlas Is Disabled

**2.0.8**
* Use Default Port In ``servers.txt`` If Not Specified

**2.0.7**
* Fix Sign Text Not Updating In Multiplayer When Exiting Editing UI Using Escape Button
* Fix Item Dropping Not Working

**2.0.6**
* Fix Toolbar Size In Normal GUI

**2.0.5**
* Show Reborn Version In Start Screen
* External Server Support

**2.0.4**
* Optimize Media Layer Proxy

**2.0.3**
* Make ``kill`` Admin Command Print Death Message

**2.0.2**
* Fix Mouse Cursor Bugs

**2.0.1**
* Fix Blank Screen On Twister OS

**2.0.0**
* Major Rewrite
