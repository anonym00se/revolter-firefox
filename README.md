
# Revolter-Firefox

Based on revolter-firefox (https://github.com/revolter-firefox/revolter-firefox) by Xmader (https://github.com/Xmader/)

A web browser with built-in censorship evasion, without the need for a VPN! (Alpha)
Works by using a combination of ommitting and spoofing SNI headers.
All Google services (Play, YT) require that Google is not blocked by your internet.

Features over the original:
 - Bypasses HSTS warnings, allowing sites like YouTube to work.
 - Code updated to the latest ESR68 code.
 - Integrated bypass and compatibility rules for websites.

The following websites are supported and have extra bypass rules to make it harder to block them:
 - 4chan
 - Blogspot
 - Google Play
 - Pinterest
 - Reddit
 - The Pirate Bay
 - Urban Dictionary
 - YouTube

The following websites work without any special bypass rules:
 - Discord
 - Facebook
 - Hulu
 - Instagram
 - Messenger
 - Kongregate
 - Ninja Kiwi
 - Pixiv
 - Steam Community

The following websites are broken but can possibly be fixed:
 - Netflix
 - Tor Project (Certificate error, otherwise works fine)
 - Twitch (Currently chat only)

The following websites currently do not work and probably never will:
 - Twitter

Things you can try that may make more websites accessible are:
 1. Turn on DNS over HTTPS (Settings > Network Settings).
 2. Turn on Encrypted SNI. Go to about:config, search for "esni" and enable it.
 3. Install HTTPS Everywhere.

If a website is broken or blocked, please either submit an issue or fill out this Google Form:
https://forms.gle/L6iSHYJN8d1WRZho6

Upcoming Features:
 - Builds for Windows and Linux. These will happen when the browser is more stable.
 - Integration with HTTPS Everywhere or a similar addon

Coming Soon(TM):
 - Android builds. These may happen at some point.
 - MacOS Builds. These will not happen for the forseeable future.

Build Instructions:
 - For Linux builds simply follow the Firefox build instructions.
 - Windows builds are still being tested and may or may not work.
 - Android builds are still being tested and may or may not work.

## License

MPL-2.0
