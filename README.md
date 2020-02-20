
# Revolter-Firefox

Based on revolter-firefox (https://github.com/revolter-firefox/revolter-firefox) by Xmader (https://github.com/Xmader/)

A web browser with built-in censorship evasion, without the need for a VPN! (Alpha Release)
Works by using a combination of ommitting and spoofing SNI headers.
All Google services (Play, YT) require that Google is not blocked by your ISP.

Features over the original:
 - Bypasses HSTS warnings, allowing sites like YouTube to work.
 - Code updated to the latest ESR68 code.
 - Integrated bypass and compatibility rules for websites.
 - Windows and Linux builds, removing the need to download ESR68 and modify it.

The following websites have extra rules to fix compatibility and/or make them harder to block:
 - Blogspot
 - Google Play
 - Pinterest
 - Reddit
 - The Pirate Bay
 - Urban Dictionary
 - YouTube

The following websites work without any special bypass rules:
 - 4chan
 - Discord
 - Facebook
 - Facebook Messenger
 - Instagram
 - Kongregate
 - Ninja Kiwi
 - Pixiv
 - Steam Community

The following websites work but are not fully tested:
 - Hulu (Streaming not tested, may not work due to DRM)

The following websites are broken but can possibly be fixed:
 - Netflix
 - Tor Project (Certificate error, otherwise works fine)
 - Twitch (Currently chat only)

The following websites currently do not work and probably never will:
 - Twitter (Central servers are easy to block)

Things you can try that may make websites accessible are:
 1. Add https:// to the start of the URL (instead of http://)
 2. Turn on DNS over HTTPS (Settings > Network Settings).
 3. Turn on Encrypted SNI (Go to about:config, search for "network.security.esni.enabled" and change it to true).
 4. Install HTTPS Everywhere.
 5. Change your system DNS. You may need to use a DNS over HTTPS/TLS client.

If a website is broken or blocked, please either submit an issue or fill out this Google Form:
https://forms.gle/L6iSHYJN8d1WRZho6

TODO:
 - Android builds. These will likely not happen if the SSL library is linked to the NDK and not compiled but otherwise will. Needs further testing to confirm.
 - macOS builds. These will happen as soon as I can get Mac cross-compilation working.
 - Remove confirmation for HTTPS Everywhere installation

Coming Soon(TM):
 - iOS builds. If these happen, it will likely be in the form of an IPA for jailbroken devices as I am not willing to spend a ton of money and/or time for a Mac/Hackintosh and iOS Dev Cert.
 - Making some websites (Tor Project, Twitch, etc.) properly work. Removing HTTPS cert checks works but that's a bad idea for obvious reasons. Will likely have to rewrite many functions in the HTTPS code for this.
 - Fixing initial HTTPS error when connecting to a website for the very first time. Low priority as it only happens once.

Build Instructions:

Linux:
 - cp mozconfig-linux mozconfig
 - Then follow the Firefox build instructions.
 - ./mach package generates a tar.bz2 archive (located in [objdir]/dist/)

Windows:
 - Install Visual Studio 2017 and Rust v1.38.
 - Copy mozconfig-win to mozconfig
 - Then follow the Firefox build instructions.
 - ./mach package generates a zip file and exe installer.

Android (Experimental):
 - cp mozconfig-linux mozconfig
 - Uncomment all options, then uncomment only the target you're building for (eg. aarch64). For ARM no target is needed.
 - Follow build instructions for Android

Mac:
 - Mac builds are still being tested and may or may not work.

## License

MPL-2.0
