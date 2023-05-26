#!/bin/sh

# sets VERSION to the value if RELEASE if there are any,
# otherwise it sets VERSION to revision number
if [ "$3" ]; then
VERSION="$3"
else
VERSION="$2"
fi

# Generates Info.plist while applying $VERSION

echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" 
\"http://www.apple.com/DTDs/Prop$
<plist version=\"1.0\">
<dict>
        <key>CFBundleDevelopmentRegion</key>
        <string>English</string>
        <key>CFBundleDisplayName</key>
        <string>Open Transport Tycoon</string>
        <key>CFBundleExecutable</key>
        <string>ttd</string>
        <key>CFBundleGetInfoString</key>
        <string>$VERSION, Copyright 2004 The Open Transport Tycoon team</string>
        <key>CFBundleIconFile</key>
        <string>ttd.icns</string>
        <key>CFBundleIdentifier</key>
        <string>org.ludde-ottd.ludde-ottd</string>
        <key>CFBundleInfoDictionaryVersion</key>
        <string>6.0</string>
        <key>CFBundleName</key>
        <string>ottd</string>
        <key>CFBundlePackageType</key>
        <string>APPL</string>
        <key>CFBundleShortVersionString</key>
        <string>$VERSION</string>
        <key>CFBundleVersion</key>
        <string>$VERSION</string>
        <key>NSHumanReadableCopyright</key>
        <string>Copyright 2004 The Open Transport Tycoon team</string>
        <key>NSPrincipalClass</key>
        <string>NSApplication</string>
</dict>
</plist>" > "$1"/contents/Info.plist
