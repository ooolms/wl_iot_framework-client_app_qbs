import qbs


CppApplication
{
	Depends {name: "Qt"; submodules: ["core","network"]}
	cpp.includePaths:[
		"../wl_iot_framework/wliotproxy-src-base/libwliotproxy-base/include",
		"../wl_iot_framework/wliotproxy-src-base/libVDIL/include",
		"../wl_iot_framework/wliotproxy-src-client/libwliotproxy/include",
	]
	cpp.dynamicLibraries:["wliotproxy"]

	files:[
        "CmdArgParser.cpp",
        "CmdArgParser.h",
        "main.cpp",
    ]
}
