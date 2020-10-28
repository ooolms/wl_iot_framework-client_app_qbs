import qbs


CppApplication
{
	Depends {name: "Qt"; submodules: ["core","network"]}
	cpp.includePaths:[
		"../wl_iot_framework/wliotproxy-src/libwliotproxy-base/include",
		"../wl_iot_framework/wliotproxy-src/libVDIL/include",
		"../wl_iot_framework/wliotproxy-src/libwliotproxy/include",
	]
	cpp.dynamicLibraries:["wliotproxy"]

	files:[
        "CmdArgParser.cpp",
        "CmdArgParser.h",
        "main.cpp",
    ]
}
