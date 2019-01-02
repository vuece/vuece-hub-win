# vuece-hub-win
A Windows app that hosts and streams your personal music from your Windows PC to your Vuece Android Music Player, in real-time. 

Vuece Hub is NOT limited to your local WIFI network, it streams music over Internet in real-time with no extra user account or membership registration required. You simply sign in with your Google Account

Before compiling the code, you need to generate "Vuece Libraries.props" from "Vuece Libraries.props.in"

and change the following values to your real path:

    <VueceExternals>c:\vuece-libjingle\externals\</VueceExternals>
    <VueceLibJingleLocation>c:\vuece-libjingle\libjingle\</VueceLibJingleLocation>
    <VueceClientCoreLocation>c:\vuece-libjingle\client-core</VueceClientCoreLocation>

Then run cmake on taglib

    extract tar ball

and run

    cmake -DENABLE_STATIC=ON -DENABLE_STATIC_RUNTIME=ON -DZLIB_INCLUDE_DIR="c:\vuece-libjingle\externals\zlib\zlib\include" -DZLIB_LIBRARY="c:\vuece-libjingle\externals\zlib\zlib" -G "Visual Studio 10"

Then open windows solution file

1. Open the property dialog of project 'tag'
2. Change the value of 'Output Directory' to '$(SolutionDir)$(Configuration)\'
3. Build tag project, then make sure the output library file is generated in to vuece-hub-win/Debug or Release folder
